#include "solver.h"

#include <sys/time.h>
#include <string.h>
#include <sys/stat.h>
#include "distutil.h"
#include "mtz.h"
#include "gg.h"
#include "benders.h"

int TSP_opt(instance *inst) {
    int error;
    CPXENVptr env = CPXopenCPLEX(&error);       // generate new environment, in err will be saved errors
    CPXLPptr lp = CPXcreateprob(env, &error, inst->name);   // create new empty linear programming problem (no variables, no constraints ...)
    
    // Build the model (add variable and constrains to the empty one)
    build_model(inst, env, lp);

    // Setting the time limit to cplex
    if (inst->params.time_limit > 0) { // Time limits <= 0 not allowed
        double time_limit = inst->params.time_limit;
        CPXsetdblparam(env, CPXPARAM_TimeLimit, time_limit);
    }
    if (inst->params.seed >= 0) {
        CPXsetintparam(env, CPX_PARAM_RANDOMSEED, inst->params.seed);
    }

    // Tells cplex to store the log files
    mkdir("../logs", 0777);
    char log_path[1024];
    sprintf(log_path, "../logs/%s.log", inst->name);
    CPXsetlogfilename(env, log_path, "w");

    //Optimize the model (the solution is stored inside the env variable)
    struct timeval start, end;
    gettimeofday(&start, 0);
    int status;
    
    if (inst->params.sol_type == SOLVE_LOOP) {
        // Continue to solve using benders algorithm
        status = benders_loop(inst, env, lp);
    } else {
        status = CPXmipopt(env, lp);
    }
    gettimeofday(&end, 0);
    if (status) {
        if (inst->params.verbose >= 5) {
            printf("Cplex error code: %d\n", status);
        }
        print_error("Cplex solver encountered an error.");
    }
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    double elapsed = seconds + microseconds * 1e-6;
    
    
    // Use the solution
    int ncols = CPXgetnumcols(env, lp);
	double *xstar = (double *) calloc(ncols, sizeof(double));
    status = CPXgetx(env, lp, xstar, 0, ncols-1);
	if ( status ) {
        //Stats here: https://www.tu-chemnitz.de/mathematik/discrete/manuals/cplex/doc/refman/html/appendixB.html
        int stat = CPXgetstat(env, lp);
        printf("Status: %d\n", stat);
        // Cplex error codes: https://www.tu-chemnitz.de/mathematik/discrete/manuals/cplex/doc/refman/html/appendixC2.html
        if (status == CPXERR_NO_SOLN) {
            print_error("No Solution exists");	
        }
        printf("Error Code: %d\n", status);
        print_error("CPXgetx() error");	
    }
    CPXgetobjval(env, lp, &(inst->solution.obj_best));

    if (inst->params.verbose >= 3) {
        
    }
    printf("\n\n\nTIME TO SOLVE %0.6fs\n\n\n", elapsed); // Time should be printed only when no errors occur
    
    // Storing the solutions edges into an array
    save_solution_edges(inst, xstar);

    export_tour(inst);
    
    
    if (inst->params.verbose >= 1) {

        printf("Optimal solution found!\n");

        if (inst->params.verbose >= 2) {
            printf("The best bjective value is %f\n", inst->solution.obj_best);
        }
        

        // Next level of verbosity
        if (inst->params.verbose >= 3) {
            printf("\nThe optimal edges are:\n\n");

            for ( int i = 0; i < inst->num_nodes; i++ ){
                edge e = inst->solution.edges[i];
                printf("x(%3d,%3d) = 1\n", e.i+1,e.j+1);
            }
        }

        printf("\n");

    }
	

    plot_solution(inst);

    free(xstar);

    //Free the problem and close cplex environment
    CPXfreeprob(env, &lp);
    CPXcloseCPLEX(&env);
    return error;
}

static void build_udir_model(instance *inst, CPXENVptr env, CPXLPptr lp) {
    char xctype = 'B';  // B=binary variable
    char *names = (char *) calloc(100, sizeof(char));

    // We add one variable at time. We may also add them all in a single shot. i<j
    for (int i = 0; i < inst->num_nodes; i++) {

        for (int j = i+1; j < inst->num_nodes; j++) {
            
            sprintf(names, "x(%d,%d)", i+1, j+1);

            // Variables treated as single value arrays.
            double obj = calc_dist(i, j, inst); 
            double lb = 0.0;
            double ub = 1.0;

            int status = CPXnewcols(env, lp, 1, &obj, &lb, &ub, &xctype, &names);
            if (status) {
                print_error("An error occured inserting a new variable");
            }
            int numcols = CPXgetnumcols(env, lp);
            if (numcols - 1 != x_udir_pos(i, j, inst->num_nodes)) { // numcols -1 because we need the position index of the new variable
                print_error("Wrong position of variable");
            }
        }
    }

    // Adding the constraints
    for (int h = 0; h < inst->num_nodes; h++) {
        double rhs = 2.0;
        char sense = 'E';
        sprintf(names, "degree(%d)", h+1);
        int status = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, &names);
        if (status) {
            print_error("An error occured inserting a new constraint");
        }

        for (int i = 0; i < inst->num_nodes; i++) {
            if (i == h) continue;

            status = CPXchgcoef(env, lp, h, x_udir_pos(h, i, inst->num_nodes), 1.0);
            if (status) {
                print_error("An error occured in filling a constraint");
            }
        }
        
    }

    free(names);
}

static void build_dir_model(instance *inst, CPXENVptr env, CPXLPptr lp) {
    char xctype = 'B';  // B=binary variable
    char *names = (char *) calloc(100, sizeof(char));
    
    // We add one variable at time. We may also add them all in a single shot. i<j
    for (int i = 0; i < inst->num_nodes; i++) {

        for (int j = 0; j < inst->num_nodes; j++) {
            sprintf(names, "x(%d,%d)", i+1, j+1);
            // Variables treated as single value arrays.
            double obj = i != j ? calc_dist(i, j, inst) : 0.0; 
            double lb = 0.0; 
            double ub = i != j ? 1.0 : 0.0; // if i==j: ub=0 else ub=1

            int status = CPXnewcols(env, lp, 1, &obj, &lb, &ub, &xctype, &names); 
            if (status) {
                print_error("An error occured inserting a new variable");
            }
            int numcols = CPXgetnumcols(env, lp);
            if (numcols - 1 != x_dir_pos(i, j, inst->num_nodes)) { // numcols -1 because we need the position index of the new variable
                print_error("Wrong position of variable");
            }
        }
    }

    int deg = 0;
    double rhs = 1.0; // Asymmetric graph
    char sense = 'E';   
    // Adding constraint x12 + x13 + ... + xij + ... = 1 For each i
    for (int i = 0; i < inst->num_nodes; i++) {
        sprintf(names, "degree(%d)", deg + 1);
        int status = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, &names);
        if (status) {
            print_error("An error occured inserting a new constraint");
        }
        for (int j = 0; j < inst->num_nodes; j++) {
            if (i == j) continue;
            status = CPXchgcoef(env, lp, deg, x_dir_pos(i, j, inst->num_nodes), 1.0);
            if (status) {
                print_error("An error occured in filling a constraint");
            }
        }
        deg++;
    }

    // Adding constraint x21 + x31 + ... + xij + ... = 1 For each j
    for (int j = 0; j < inst->num_nodes; j++) {
        sprintf(names, "degree(%d)", deg + 1);
        int status = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, &names);
        if (status) {
            print_error("An error occured inserting a new constraint");
        }
        for (int i = 0; i < inst->num_nodes; i++) {
            if (i == j) continue;
            status = CPXchgcoef(env, lp, deg, x_dir_pos(i, j, inst->num_nodes), 1.0);
            if (status) {
                print_error("An error occured in filling a constraint");
            }
        }
        deg++;
    }

    int sol_type = inst->params.sol_type;
    const char* method_name = NULL;
    if (sol_type == SOLVE_MTZ) {
        method_name = "MTZ";
        add_mtz_constraints(inst, env, lp, 0);
    } else if (sol_type == SOLVE_MTZL) {
        method_name = "MTZ Lazy";
        add_mtz_lazy_constraints(inst, env, lp, 0);
    } else if (sol_type == SOLVE_MTZI) {
        method_name = "MTZ with subtour elimination constraints of degree 2";
        add_mtz_constraints(inst, env, lp, 1);
    } else if (sol_type == SOLVE_MTZLI) {
        method_name = "MTZ lazy with subtour elimination constraints of degree 2";
        add_mtz_lazy_constraints(inst, env, lp, 1);
    } else if (sol_type == SOLVE_MTZ_IND) {
        method_name = "MTZ with indicator constraints";
        add_mtz_indicator_constraints(inst, env, lp);
    } else if (sol_type == SOLVE_GG) {
        method_name = "GG";
        add_gg_constraints(inst, env, lp);
    }
    if (inst->params.verbose >= 4) {
        if (method_name != NULL) 
            printf("Solving with %s\n\n", method_name);
    }

    free(names);
}

static void build_model(instance *inst, CPXENVptr env, CPXLPptr lp) {

    // Checks the type of the edge in order to
    // build the correct model
    if (inst->params.type == UDIR_EDGE) {
        build_udir_model(inst, env, lp);
    } else {
        build_dir_model(inst, env, lp);
    }
    

    // Saving the model in .lp file

    save_lp(env, lp, inst->name);

}