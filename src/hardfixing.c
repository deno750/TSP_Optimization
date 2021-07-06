#include "hardfixing.h"

#include "solver.h"
#include "utility.h"


//Function that UNfix the edges
void set_default_lb(CPXENVptr env, CPXLPptr lp, int ncols, int *indexes){
	double *zeros = CALLOC(ncols, double); // Calloc returns all 0.0 bytes
	char *lbs = MALLOC(ncols, char); // Lower bound
    MEMSET(lbs, 'L', ncols, char);
    int status = CPXchgbds(env, lp, ncols, indexes, lbs, zeros); // this function changes the lower and/or upper bound
    if (status) {LOG_E("CPXchgbds() error code %d", status);}
    FREE(zeros);
    FREE(lbs);
}

void set_default_lb2(CPXENVptr env, CPXLPptr lp, int ncols, int *indexes){
    double zero = 0.0;
    char lb = 'L';
    int status = 0;
    for (int i = 0; i < ncols; i++) {
        status = CPXchgbds(env, lp, 1, &(indexes[i]), &lb, &zero);
        if (status) {LOG_E("CPXchgbds() error code %d", status);}
    }
}

void set_default_bounds(CPXENVptr env, CPXLPptr lp, int ncols, int *indexes, char *bounds){
    double value;
    char bound;
    int status = 0;
    for (int i = 0; i < ncols; i++) {
        bound = bounds[i];
        if (bound == 'L') {
            value = 0.0;
        } else if (bound == 'U') {
            value = 1.0;
        } else {
            LOG_E("An unknown bound is set: %c", bound);
        }
        status = CPXchgbds(env, lp, 1, &(indexes[i]), &bound, &value);
        if (status) {LOG_E("CPXchgbds() error code %d", status);}
    }
}

//Function that fix the edges randomly
void random_fix2(CPXENVptr env, CPXLPptr lp, double prob, int *ncols, int *indexes, double *xh){
    double rand_num;
	double one = 1.0;
	char lb = 'L'; // Lower Bound
    *ncols = 0;
    if(prob < 0 || prob > 1) {LOG_E("probability must be in [0,1]");}
    int num_cols = CPXgetnumcols(env, lp);

    for(int i = 0; i < num_cols; i++){
		rand_num = (double) rand() / RAND_MAX;
		
		if(xh[i] > 0.5 && rand_num < prob) {
			CPXchgbds(env, lp, 1, &i, &lb, &one);
            indexes[(*ncols)++] = i;
		}
    }
}

void random_fix(CPXENVptr env, CPXLPptr lp, double prob, int *ncols, int *indexes, double *xh){
    double rand_num;
    *ncols = 0;
    if(prob < 0 || prob > 1) { LOG_E("probability must be in [0,1]"); }
    int num_cols = CPXgetnumcols(env, lp);
    for(int i = 0; i < num_cols; i++) {
		rand_num = (double) rand() / RAND_MAX;
		if(xh[i] > 0.5 && rand_num < prob) {
            indexes[(*ncols)++] = i;
		}
	}
    double *ones = MALLOC(*ncols, double);
    MEMSET(ones, 1.0, *ncols, double);
    char *lbs = MALLOC(*ncols, char);
    MEMSET(lbs, 'L', *ncols, char);
    int status = CPXchgbds(env, lp, *ncols, indexes, lbs, ones); // this function changes the lower and/or upper bound
    if (status) { LOG_E("CPXchgbds() error code %d", status); }
    FREE(ones);
    FREE(lbs);
}


void advanced_fix(CPXENVptr env, CPXLPptr lp, instance *inst, double prob, int *ncols, int *indexes, char *bounds, double *xh, edge *close_cycle_edges) {
    double rand_num;
	double one = 1.0;
	char lb = 'L'; // Lower Bound
    *ncols = 0;
    
    if(prob < 0 || prob > 1) {LOG_E("probability must be in [0,1]");}
    int num_cols = CPXgetnumcols(env, lp);

    
    double *xfake = CALLOC(num_cols, double); // We create a fake solution where the the nodes selected have the value of 1. This is threated such as a solution with subtours.
    for(int i = 0; i < num_cols; i++){
		rand_num = URAND();
		
		if(xh[i] > 0.5 && rand_num < prob) {
			CPXchgbds(env, lp, 1, &i, &lb, &one);
            indexes[*ncols] = i;
            bounds[*ncols] = lb;
            (*ncols)++;
            xfake[i] = 1.0;
		}
    }
 
    int *succ = MALLOC(inst->num_nodes, int);
    MEMSET(succ, -1, inst->num_nodes, int);
    int *comp = MALLOC(inst->num_nodes, int);
    MEMSET(comp, -1, inst->num_nodes, int);

    // Adding the constraints where we set to zero the edges that could create closed loops which we don't want. Those constraints are not very necessary since
    // cplex is going to exclude them in advanced SECs, but those constraints can be very helpful because we are sure that the combination of the fixed variables
    // cannot create subtours.
    int num_closed_cycles = 0; // The number of edges that potentially can create loops
    // This function checks and returns the edges that can create loops when activated. 
    int numcomp = count_components_adv(inst, xfake, succ, comp, close_cycle_edges, &num_closed_cycles);
    char ub = 'U';
    double zero = 0.0;
    for (int i = 0; i < num_closed_cycles; i++) {
        edge e = close_cycle_edges[i];
        int index = x_udir_pos(e.i, e.j, inst->num_nodes);
        CPXchgbds(env, lp, 1, &index, &ub, &zero);
        indexes[*ncols] = index;
        bounds[*ncols] = ub;
        (*ncols)++;
    }

    FREE(xfake);
    FREE(succ);
    FREE(comp);    
}

int hard_fixing_solver(instance *inst, CPXENVptr env, CPXLPptr lp) {
    // For other emphasis params check there: https://www.ibm.com/docs/en/icos/20.1.0?topic=parameters-mip-emphasis-switch
    CPXsetintparam(env, CPXPARAM_Emphasis_MIP, CPX_MIPEMPHASIS_HEURISTIC); // We want that cplex finds an high quality solution earlier
    CPXsetintparam(env, CPX_PARAM_NODELIM, 0); // We limit the first solution space to the root node
    double time_limit = inst->params.time_limit > 0 ? inst->params.time_limit : HARD_FIX_TIME_LIM_DEFAULT;
    CPXsetdblparam(env, CPXPARAM_TimeLimit, time_limit);
    inst->solution.xbest = CALLOC(inst->num_columns, double); // The best solution found till now
    int cols_tot = CPXgetnumcols(env, lp);
    int *indexes = CALLOC(cols_tot, int);
    char *bounds = CALLOC(cols_tot, char);
    double *xh = CALLOC(cols_tot, double); // The current solution found
    edge *close_cycle_edges = CALLOC(inst->num_nodes, edge); // inst->num_nodes since we want to store the edges which closes the loops in the fixed edges and the number edges in tsp are at most the number of nodes. The fixed edges can be considered as subtours of tsp

    struct timeval start, end; 
    gettimeofday(&start, 0);

    // First iteration: seeking the first feasible solution
    int status = opt_best_solver(env, lp, inst);
    if (status) {LOG_E("CPXmipopt in hard fixing error code %d", status);}
    status = CPXgetx(env, lp, xh, 0, cols_tot - 1); // save the first solution found
    CPXsetintparam(env, CPX_PARAM_NODELIM, INT_MAX); // Resetting the node limit to infinite

    CPXsetintparam(env, CPXPARAM_Emphasis_MIP, CPX_MIPEMPHASIS_OPTIMALITY);
    //CPXsetintparam(env, CPXPARAM_Emphasis_MIP, CPX_MIPEMPHASIS_BALANCED);

    int ncols_fixed;
    double prob = 0.9;
    int prob_index = 0;
    double objval;
    double objbest = CPX_INFBOUND;
    int done = 0;
    while (!done) {
        done = 1;
        gettimeofday(&end, 0);
        double elapsed = get_elapsed_time(start, end);
        if (elapsed >= time_limit) {
            break;
        }
        double time_remain = time_limit - elapsed; // this is the time remained 
        CPXsetdblparam(env, CPXPARAM_TimeLimit, time_remain);
        //random_fix2(env, lp, prob, &ncols_fixed, indexes, xh);
        advanced_fix(env, lp, inst, prob, &ncols_fixed, indexes, bounds, xh, close_cycle_edges);
        status = CPXmipopt(env, lp);
        if (status) {LOG_I("CPXmipopt() error");}
        if (inst->params.verbose >= 5) {
            LOG_I("COLS %d", ncols_fixed);
        }
        save_lp(env, lp, "AfterFixing");
        if (status) {
            LOG_E("CPXmipopt error code %d", status);
        }

        status = CPXgetx(env, lp, xh, 0, cols_tot - 1);
        CPXgetobjval(env, lp, &objval);
        if (status) { LOG_D("CPXgetx error code %d", status); }
        double obj_improv = 1 - objval / objbest;
        LOG_D("Improvement %0.4f", obj_improv);
        if (objval < objbest && !status) {
            done = 0;
            if (inst->params.verbose >= 3) {
                LOG_I("Updated incubement: %f", objval);
            }
            objbest = objval;
            inst->solution.obj_best = objval;
            memcpy(inst->solution.xbest, xh, cols_tot * sizeof(double));
            save_solution_edges(inst, xh);
            plot_solution(inst);
        }

        
        // Unfix the variables
        //set_default_lb2(env, lp, ncols_fixed, indexes);
        set_default_bounds(env, lp, ncols_fixed, indexes, bounds);
        save_lp(env, lp, "AferRestoring");
    }
    
    
    FREE(indexes);
    FREE(bounds);
    FREE(xh);
    FREE(close_cycle_edges);
    return 0;
}

int hard_fixing_solver2(instance *inst, CPXENVptr env, CPXLPptr lp) {
    // For other emphasis params check there: https://www.ibm.com/docs/en/icos/20.1.0?topic=parameters-mip-emphasis-switch
    CPXsetintparam(env, CPXPARAM_Emphasis_MIP, CPX_MIPEMPHASIS_HEURISTIC); // We want that cplex finds an high quality solution earlier
    CPXsetintparam(env, CPX_PARAM_NODELIM, 0); // We limit the first solution space to the root node
    double time_limit = inst->params.time_limit > 0 ? inst->params.time_limit : HARD_FIX_TIME_LIM_DEFAULT;
    CPXsetdblparam(env, CPXPARAM_TimeLimit, time_limit);
    inst->solution.xbest = CALLOC(inst->num_columns, double); // The best solution found till now
    int cols_tot = CPXgetnumcols(env, lp);
    int *indexes = CALLOC(cols_tot, int);
    char *bounds = CALLOC(cols_tot, char);
    double *xh = CALLOC(cols_tot, double); // The current solution found
    edge *close_cycle_edges = CALLOC(inst->num_nodes, edge); // inst->num_nodes since we want to store the edges which closes the loops in the fixed edges and the number edges in tsp are at most the number of nodes. The fixed edges can be considered as subtours of tsp

    struct timeval start, end; 
    gettimeofday(&start, 0);

    // First iteration: seeking the first feasible solution
    int status = opt_best_solver(env, lp, inst);
    if (status) {LOG_E("CPXmipopt in hard fixing error code %d", status);}
    status = CPXgetx(env, lp, xh, 0, cols_tot - 1); // save the first solution found
    CPXsetintparam(env, CPX_PARAM_NODELIM, 100); 

    CPXsetintparam(env, CPXPARAM_Emphasis_MIP, CPX_MIPEMPHASIS_OPTIMALITY);

    int ncols_fixed;
    double prob[] = {0.9, 0.8, 0.7};
    int prob_index = 0;
    double objval;
    double objbest = CPX_INFBOUND;
    int done = 0;
    int number_little_improvements = 0;
    int number_no_improvements = 0;
    while (!done) {
        done = 1;
        gettimeofday(&end, 0);
        double elapsed = get_elapsed_time(start, end);
        if (elapsed >= time_limit) {
            break;
        }
        double time_remain = time_limit - elapsed; // this is the time remained 
        CPXsetdblparam(env, CPXPARAM_TimeLimit, time_remain);
        //random_fix2(env, lp, prob, &ncols_fixed, indexes, xh);
        advanced_fix(env, lp, inst, prob[prob_index], &ncols_fixed, indexes, bounds, xh, close_cycle_edges);
        status = CPXmipopt(env, lp);
        LOG_I("COLS %d", ncols_fixed);
        save_lp(env, lp, "AfterFixing");
        if (status) {
            LOG_E("CPXmipopt error code %d", status);
        }

        status = CPXgetx(env, lp, xh, 0, cols_tot - 1);
        CPXgetobjval(env, lp, &objval);
        if (status) { LOG_D("CPXgetx error code %d", status); }
        double obj_improv = 1 - objval / objbest;
        LOG_D("Improvement %0.4f", obj_improv);
        if (objval < objbest && !status) {
            done = 0;
            number_no_improvements = 0;
            if (obj_improv < HARD_FIX_MIN_IMPROVEMENT) {
                LOG_D("NOT IMPROVED TOO MUCH");
                number_little_improvements++;
                LOG_D("Prob_index: %d Len Prob: %lu", prob_index, LEN(prob));
                if (number_little_improvements % HARD_FIX_MAX_LITTLE_IMPROVEMENTS == 0 && prob_index < LEN(prob) - 1) {
                    prob_index++;
                    LOG_D("CONSECUTIVE LITTLE IMPROVEMENETS. UPDATING THE PROB INDEX");
                }
            } else {
                number_little_improvements = 0;
            }
            LOG_I("Updated incubement: %f", objval);
            objbest = objval;
            inst->solution.obj_best = objval;
            memcpy(inst->solution.xbest, xh, cols_tot * sizeof(double));
            save_solution_edges(inst, xh);
            plot_solution(inst);
        } else {
            number_no_improvements++;
            LOG_D("NOT IMPROVED AT ALL");
            done = 0;
            if (prob_index < LEN(prob) - 1) {
                LOG_D("NO IMPROVEMENTS. UPDATING THE PROB INDEX");
                prob_index++;
                number_little_improvements = 0;
            } else if (number_no_improvements % 3 == 0) { // Adding a tollerance of 3 no improvements to terminate the algorithm
                LOG_D("CONSECUTIVE NO IMPROVMENETS. TERMINATING THE ALGORITHM");
                done = 1;
            }
            
        }

        
        // Unfix the variables
        //set_default_lb2(env, lp, ncols_fixed, indexes);
        set_default_bounds(env, lp, ncols_fixed, indexes, bounds);
        save_lp(env, lp, "AferRestoring");
    }
    
    
    FREE(indexes);
    FREE(bounds);
    FREE(xh);
    FREE(close_cycle_edges);
    return 0;
}