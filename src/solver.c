#include "solver.h"

#include <sys/time.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include "distutil.h"
#include "mtz.h"
#include "gg.h"
#include "benders.h"
#include "callback.h"
#include "hardfixing.h"
#include "softfixing.h"
#include "heuristics.h"
#include "tabusearch.h"

// USER CUT SOLVER
int opt_best_solver(CPXENVptr env, CPXLPptr lp, instance *inst) {
    int ncols = CPXgetnumcols(env, lp);
    inst->num_columns = ncols; // The callbacks need the number of cols
    CPXLONG contextid = CPX_CALLBACKCONTEXT_CANDIDATE | CPX_CALLBACKCONTEXT_RELAXATION;
    int status = CPXcallbacksetfunc(env, lp, contextid, SEC_cuts_callback, inst);
    if (status) LOG_E("CPXcallbacksetfunc() error returned status %d", status);
    status = CPXmipopt(env, lp);
    return status;
}

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

        LOG_I("Solution found!");

        if (inst->params.verbose >= 2) {
            LOG_I("The best bjective value is %f", inst->solution.obj_best);
        }
        

        // Next level of verbosity
        if (inst->params.verbose >= 5) {
            LOG_I("The optimal edges are:\n");

            for ( int i = 0; i < inst->num_nodes; i++ ){
                edge e = inst->solution.edges[i];
                LOG_I("x(%3d,%3d) = 1", e.i+1,e.j+1);
            }
        }

        printf("\n");

    }
}

static int solve_problem(CPXENVptr env, CPXLPptr lp, instance *inst) {
    int status;
    if (inst->params.method.id == SOLVE_LOOP) {
        // Solve using benders algorithm
        status = benders_loop(inst, env, lp);
    } else if (inst->params.method.id == SOLVE_HARD_FIXING) {
        status = hard_fixing_solver(inst, env, lp);
    } else if (inst->params.method.id == SOLVE_HARD_FIXING2) {
        status = hard_fixing_solver2(inst, env, lp);
    } else if (inst->params.method.id == SOLVE_SOFT_FIXING) {
        status = soft_fixing_solver(inst, env, lp);
    } else {
        if (inst->params.method.id == SOLVE_CALLBACK || inst->params.method.id == SOLVE_UCUT) {
            
            inst->ind = MALLOC(inst->num_columns, int);
            int k = 0;
            for (int i = 0; i < inst->num_nodes; i++) {
                for (int j = i + 1; j < inst->num_nodes; j++) {
                    inst->ind[k++] = x_udir_pos(i, j, inst->num_nodes);
                }
            }

            // As cplex's documentations says, the maximal number of threads used by cplex is 32 if not specified a higher number
            // Check it here: https://www.ibm.com/docs/en/icos/12.8.0.0?topic=parameters-global-thread-count
            int max_threads = inst->params.num_threads > 32 ? inst->params.num_threads : 32;
            inst->thread_seeds = CALLOC(max_threads, unsigned int);
            for (int i = 0; i < max_threads; i++) {
                unsigned int seed = (unsigned int) time(NULL);
                inst->thread_seeds[i] = (seed & 0xFFFFFFF0) | (i + 1);
            }

            CPXLONG contextid;
            if (inst->params.method.id == SOLVE_UCUT) {
                contextid = CPX_CALLBACKCONTEXT_CANDIDATE | CPX_CALLBACKCONTEXT_RELAXATION;
            } else {
                contextid = CPX_CALLBACKCONTEXT_CANDIDATE;
            }
            status = CPXcallbacksetfunc(env, lp, contextid, SEC_cuts_callback, inst);
            if (status) LOG_E("CPXcallbacksetfunc() error returned status %d", status);
        }

        status = CPXmipopt(env, lp);
    }
    return status;
}

static int solve_problem_HEUC(instance *inst) {
    int status;
    if (inst->params.method.id == SOLVE_GREEDY) {
        status = HEU_greedy(inst);
    } else if (inst->params.method.id == SOLVE_GREEDY_ITER) {
        status = HEU_Greedy_iter(inst);
    } else if (inst->params.method.id == SOLVE_EXTR_MIL) {
        status = HEU_extramileage(inst);
    } else if (inst->params.method.id == SOLVE_2OPT) {
        status = HEU_2opt(inst);
    } else if (inst->params.method.id == SOLVE_3OPT) {
        status = HEU_3opt(inst);
    } else if (inst->params.method.id == SOLVE_GRASP) {
        status = HEU_Grasp(inst);
    } else if (inst->params.method.id == SOLVE_GRASP_ITER) {
        status = HEU_Grasp_iter(inst, inst->params.time_limit);
    } else if (inst->params.method.id == SOLVE_GRASP_REF) {
        status = HEU_Grasp2opt(inst);
    } else if (inst->params.method.id == SOLVE_VNS) {
        status = HEU_VNS(inst);
    } else if (inst->params.method.id == SOLVE_TABU_STEP) {
        status = HEU_Tabu_step(inst);
    } else if (inst->params.method.id == SOLVE_TABU_LIN) {
        status = HEU_Tabu_lin(inst);
    } else if (inst->params.method.id == SOLVE_TABU_RAND) {
        status = HEU_Tabu_rand(inst);
    } else if (inst->params.method.id == SOLVE_GENETIC) {
        status = HEU_Genetic(inst);
    }
    else {
        LOG_E("No Heuristic method specified!");
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

    
    if (inst->params.seed >= 0) {
        srand(inst->params.seed); // Setting the random seed for rand()
    }
    long ncols = CPXgetnumcols(env, lp);
    inst->num_columns = ncols; // The callbacks need the number of cols

    //Optimize the model (the solution is stored inside the env variable)
    struct timeval start, end;
    gettimeofday(&start, 0);
    int status = solve_problem(env, lp, inst);
    gettimeofday(&end, 0);
    if (status) {
        /*if (inst->params.verbose >= 5) {
            LOG_I("Cplex error code: %d", status);
        }*/
        if (status = CPX_STAT_ABORT_TIME_LIM) {
            LOG_I("Time limit exceeded");
        } else {
            LOG_E("Cplex solver encountered an error with error code: %d", status);
        }
    }
    double elapsed = get_elapsed_time(start, end);
    inst->solution.time_to_solve = elapsed;
    
    // Use the solution
    //int ncols = CPXgetnumcols(env, lp);
    if (inst->solution.xbest == NULL) {
        double *xstar = CALLOC(ncols, double);
        status = CPXgetx(env, lp, xstar, 0, ncols-1);
        if ( status ) {
            //Stats here: https://www.tu-chemnitz.de/mathematik/discrete/manuals/cplex/doc/refman/html/appendixB.html
            //int stat = CPXgetstat(env, lp);
            //LOG_I("Status: %d", stat);
            // Cplex error codes: https://www.tu-chemnitz.de/mathematik/discrete/manuals/cplex/doc/refman/html/appendixC2.html
            if (status == CPXERR_NO_SOLN) {
                LOG_E("No Solution exists");	
            }
            LOG_E("CPXgetx() error code %d", status);	
        }
        CPXgetobjval(env, lp, &(inst->solution.obj_best));

        // Storing the solutions edges into an array
        save_solution_edges(inst, xstar);
        FREE(xstar);
    } else {
        save_solution_edges(inst, inst->solution.xbest);
    }
	
    export_tour(inst);
    
    print_solution(inst);
	
    plot_solution(inst);

    if (inst->params.perf_prof) {
        printf("%0.6f", elapsed);
    } else {
        printf("\n\n\nTIME TO SOLVE %0.6fs\n\n\n", elapsed); // Time should be printed only when no errors occur
    }

    //Free the problem and close cplex environment
    CPXfreeprob(env, &lp);
    CPXcloseCPLEX(&env);
    return error;
}

int TSP_heuc(instance *inst) {

    if (inst->params.seed >= 0) {
        srand(inst->params.seed); // Setting the random seed for rand()
    }
    // In heuristic xbest is not used since it's a quadratic data structure. Since heuristics solves very large problems, the amount of memory required by xbest is very huge
    inst->num_columns = (long) inst->num_nodes * (inst->num_nodes - 1) / 2; 
    inst->solution.edges = CALLOC(inst->num_nodes, edge);
    //Optimize the model (the solution is stored inside the env variable)
    struct timeval start, end;
    gettimeofday(&start, 0);
    int status = solve_problem_HEUC(inst);
    gettimeofday(&end, 0);
    double elapsed = get_elapsed_time(start, end);
    inst->solution.time_to_solve = elapsed;
    
	
    export_tour(inst);
    
    print_solution(inst);
	
    plot_solution(inst);

    if (inst->params.perf_prof) {
        printf("%0.6f", elapsed);
    } else {
        printf("\n\n\nTIME TO SOLVE %0.6fs\n\n\n", elapsed); // Time should be printed only when no errors occur
    }

    return 0;
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
                LOG_E("An error occured inserting a new variable");
            }
            int numcols = CPXgetnumcols(env, lp);
            if (numcols - 1 != x_udir_pos(i, j, inst->num_nodes)) { // numcols -1 because we need the position index of the new variable
                LOG_E("Wrong position of variable in build_udri_model");
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
            LOG_E("CPXnewrows() error code %d", status);
        }

        for (int i = 0; i < inst->num_nodes; i++) {
            if (i == h) continue;

            status = CPXchgcoef(env, lp, h, x_udir_pos(h, i, inst->num_nodes), 1.0);
            if (status) {
                LOG_E("CPXchgcoef() error code %d", status);
            }
        }
        
    }

    FREE(names);
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
                LOG_E("CPXnewcols() error code %d", status);
            }
            int numcols = CPXgetnumcols(env, lp);
            if (numcols - 1 != x_dir_pos(i, j, inst->num_nodes)) { // numcols -1 because we need the position index of the new variable
                LOG_E("Wrong position of variable in build_dir_model");
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
            LOG_E("CPXnewrows() error code %d", status);
        }
        for (int j = 0; j < inst->num_nodes; j++) {
            if (i == j) continue;
            status = CPXchgcoef(env, lp, deg, x_dir_pos(i, j, inst->num_nodes), 1.0);
            if (status) {
                LOG_E("CPXchgcoef() error code %d", status);
            }
        }
        deg++;
    }

    // Adding constraint x21 + x31 + ... + xij + ... = 1 For each j
    for (int j = 0; j < inst->num_nodes; j++) {
        sprintf(names, "degree(%d)", deg + 1);
        int status = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, &names);
        if (status) {
            LOG_E("CPXnewrows() error code %d", status);
        }
        for (int i = 0; i < inst->num_nodes; i++) {
            if (i == j) continue;
            status = CPXchgcoef(env, lp, deg, x_dir_pos(i, j, inst->num_nodes), 1.0);
            if (status) {
                LOG_E("CPXchgcoef() error code %d", status);
            }
        }
        deg++;
    }

    int method = inst->params.method.id;
    if (method == SOLVE_MTZ) {
        add_mtz_constraints(inst, env, lp, 0);
    } else if (method == SOLVE_MTZL) {
        add_mtz_lazy_constraints(inst, env, lp, 0);
    } else if (method == SOLVE_MTZI) {
        add_mtz_constraints(inst, env, lp, 1);
    } else if (method == SOLVE_MTZLI) {
        add_mtz_lazy_constraints(inst, env, lp, 1);
    } else if (method == SOLVE_MTZ_IND) {
        add_mtz_indicator_constraints(inst, env, lp);
    } else if (method == SOLVE_GG) {
        add_gg_constraints(inst, env, lp);
    }

    FREE(names);
}

static void build_model(instance *inst, CPXENVptr env, CPXLPptr lp) {

    // Checks the type of the edge in order to
    // build the correct model
    if (inst->params.method.edge_type == UDIR_EDGE) {
        build_udir_model(inst, env, lp);
    } else {
        build_dir_model(inst, env, lp);
    }
    

    // Saving the model in .lp file

    save_lp(env, lp, inst->name);

}