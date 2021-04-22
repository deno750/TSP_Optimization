#include "solver.h"

#include <sys/time.h>
#include <string.h>
#include <sys/stat.h>
#include "distutil.h"
#include "mtz.h"
#include "gg.h"
#include "benders.h"
#include "callback.h"

static void set_cplex_params(CPXENVptr env, instance_params params) {
    if (params.time_limit > 0) { // Time limits <= 0 not allowed
        double time_limit = params.time_limit;
        CPXsetdblparam(env, CPXPARAM_TimeLimit, time_limit);
    }
    if (params.seed >= 0) {
        CPXsetintparam(env, CPX_PARAM_RANDOMSEED, params.seed);
    }
    if (params.num_threads > 0) {
        CPXsetintparam(env, CPXPARAM_Threads, params.num_threads);
    }

    // Cplex precision
    CPXsetdblparam(env, CPX_PARAM_EPINT, 0.0);		
	CPXsetdblparam(env, CPX_PARAM_EPGAP, 1e-9);	 
	CPXsetdblparam(env, CPX_PARAM_EPRHS, 1e-9); 
}

static void print_solution(instance *inst) {
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
}

static int solve_problem(CPXENVptr env, CPXLPptr lp, instance *inst) {
    int status;
    if (inst->params.sol_type == SOLVE_LOOP) {
        // Solve using benders algorithm
        status = benders_loop(inst, env, lp);
    } else {

        if (inst->params.sol_type == SOLVE_CALLBACK || inst->params.sol_type == SOLVE_CALLBACK2 || inst->params.sol_type == SOLVE_CALLBACK3) {
            int ncols = CPXgetnumcols(env, lp);
            inst->num_columns = ncols; // The callbacks need the number of cols
            CPXLONG contextid;
            if (inst->params.sol_type == SOLVE_CALLBACK2) {
                contextid = CPX_CALLBACKCONTEXT_CANDIDATE | CPX_CALLBACKCONTEXT_RELAXATION;
            } else {
                contextid = CPX_CALLBACKCONTEXT_CANDIDATE;
            }
            status = CPXcallbacksetfunc(env, lp, contextid, SEC_cuts_callback, inst);
            if (status) print_error("CPXcallbacksetfunc() error");
        }

        status = CPXmipopt(env, lp);
    }
    return status;
}

int TSP_opt(instance *inst) {
    int error;
    CPXENVptr env = CPXopenCPLEX(&error);       // generate new environment, in err will be saved errors
    CPXLPptr lp = CPXcreateprob(env, &error, inst->name);   // create new empty linear programming problem (no variables, no constraints ...)
    
    // Build the model (add variable and constrains to the empty one)
    build_model(inst, env, lp);

    // Setting cplex's parameters
    set_cplex_params(env, inst->params);

    // Tells cplex to store the log files
    save_cplex_log(env, inst);

    //Optimize the model (the solution is stored inside the env variable)
    struct timeval start, end;
    gettimeofday(&start, 0);
    int status = solve_problem(env, lp, inst);
    gettimeofday(&end, 0);
    if (status) {
        if (inst->params.verbose >= 5) {
            printf("Cplex error code: %d\n", status);
        }
        print_error("Cplex solver encountered an error.");
    }
    double elapsed = get_elapsed_time(start, end);
    inst->solution.time_to_solve = elapsed;
    
    // Use the solution
    int ncols = CPXgetnumcols(env, lp);
	double *xstar = CALLOC(ncols, double);
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
    
    // Storing the solutions edges into an array
    save_solution_edges(inst, xstar);

    export_tour(inst);
    
    print_solution(inst);
	
    plot_solution(inst);

    if (inst->params.perf_prof) {
        printf("%0.6f", elapsed);
    } else {
        printf("\n\n\nTIME TO SOLVE %0.6fs\n\n\n", elapsed); // Time should be printed only when no errors occur
    }

    free(xstar);

    //Free the problem and close cplex environment
    CPXfreeprob(env, &lp);
    CPXcloseCPLEX(&env);
    return error;
}

static void build_udir_model(instance *inst, CPXENVptr env, CPXLPptr lp) {
    char xctype = 'B';  // B=binary variable
    char *names = CALLOC(100, char);

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
    char *names = CALLOC(100, char);
    
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
    if (sol_type == SOLVE_MTZ) {
        add_mtz_constraints(inst, env, lp, 0);
    } else if (sol_type == SOLVE_MTZL) {
        add_mtz_lazy_constraints(inst, env, lp, 0);
    } else if (sol_type == SOLVE_MTZI) {
        add_mtz_constraints(inst, env, lp, 1);
    } else if (sol_type == SOLVE_MTZLI) {
        add_mtz_lazy_constraints(inst, env, lp, 1);
    } else if (sol_type == SOLVE_MTZ_IND) {
        add_mtz_indicator_constraints(inst, env, lp);
    } else if (sol_type == SOLVE_GG) {
        add_gg_constraints(inst, env, lp);
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