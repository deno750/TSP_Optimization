#include "hardfixing.h"

#include "solver.h"
#include "utility.h"

#define HARD_FIX_TIME_LIM_DEFAULT 1800



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

typedef struct {
    CPXENVptr env;
    CPXLPptr lp;
    instance *inst;
    int *ncols;
    int *indexes;
    char *bounds;
} callback_params;

void close_cycle_callback(int i, int j, void* data) {
    callback_params *param = (callback_params*) data;
    instance *inst = param->inst;
    CPXENVptr env = param->env;
    CPXLPptr lp = param->lp;
    int *ncols = param->ncols;
    int *indexes = param->indexes;
    char *bounds = param->bounds;
    int index = x_udir_pos(i, j, inst->num_nodes);
    char ub = 'U';
    double zero = 0.0;
    indexes[*ncols] = index;
    bounds[*ncols] = ub;
    (*ncols)++;
    CPXchgbds(env, lp, 1, &index, &ub, &zero);
}


void advanced_fix(CPXENVptr env, CPXLPptr lp, instance *inst, double prob, int *ncols, int *indexes, char *bounds, double *xh) {
    double rand_num;
	double one = 1.0;
	char lb = 'L'; // Lower Bound
    *ncols = 0;
    
    if(prob < 0 || prob > 1) {LOG_E("probability must be in [0,1]");}
    int num_cols = CPXgetnumcols(env, lp);

    
    double *xfake = CALLOC(num_cols, double);
    for(int i = 0; i < num_cols; i++){
		rand_num = (double) rand() / RAND_MAX;
		
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
    callback_params param = {.env = env, .lp = lp, .inst = inst, .indexes = indexes, .bounds = bounds, .ncols = ncols};
    int numcomp = count_components_adv(inst, xfake, succ, comp, close_cycle_callback, &param);

    FREE(xfake);
    FREE(succ);
    FREE(comp);    
}

int hard_fixing_solver(instance *inst, CPXENVptr env, CPXLPptr lp) {
    // For other emphasis params check there: https://www.ibm.com/docs/en/icos/20.1.0?topic=parameters-mip-emphasis-switch
    CPXsetintparam(env, CPXPARAM_Emphasis_MIP, CPX_MIPEMPHASIS_HEURISTIC); // We want that cplex finds an high quality solution earlier
    CPXsetintparam(env, CPX_PARAM_NODELIM, 0);
    double time_limit = inst->params.time_limit > 0 ? inst->params.time_limit : HARD_FIX_TIME_LIM_DEFAULT;
    //CPXsetdblparam(env, CPXPARAM_TimeLimit, time_limit);

    int status = opt_best_solver(env, lp, inst);
    int cols_tot = CPXgetnumcols(env, lp);
    int *indexes = MALLOC(cols_tot, int);
    char *bounds = MALLOC(cols_tot, char);
    double *xh = CALLOC(cols_tot, double); // The current solution found
    inst->solution.xbest = CALLOC(cols_tot, double); // The best solution found till now

    int ncols_fixed;
    double prob = 0.7;
    unsigned int seed = inst->params.seed >= 0 ? inst->params.seed : 0;
    srand(seed); // This should go on the beginning of the program
    int num = 0;
    double objval;
    double objbest = CPX_INFBOUND;
    struct timeval start, end; 
    int num_iter = 10;
    double time_lim_frac = time_limit / num_iter;
    gettimeofday(&start, 0);
    do {
        gettimeofday(&end, 0);
        double elapsed = get_elapsed_time(start, end);
        if (elapsed > time_limit) {
            break;
        }
        double time_remain = time_limit - elapsed; // this is the time remained 
        CPXsetdblparam(env, CPXPARAM_TimeLimit, time_lim_frac);
        //random_fix2(env, lp, prob, &ncols_fixed, indexes, xh);
        advanced_fix(env, lp, inst, prob, &ncols_fixed, indexes, bounds, xh);
        status = CPXmipopt(env, lp);
        LOG_I("COLS %d", ncols_fixed);
        save_lp(env, lp, "YEEEEE");
        if (status) {
            LOG_E("CPXmipopt error code %d", status);
        }
//0x00000001034cc000 "LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU\x82J\x01"
        status = CPXgetx(env, lp, xh, 0, cols_tot - 1);
        CPXgetobjval(env, lp, &objval);
        if (status) { LOG_D("CPXgetx error code %d", status); }
        if (objval < objbest && !status) {
            LOG_I("Updated incubement: %f", objval);
            objbest = objval;
            inst->solution.obj_best = objval;
            memcpy(inst->solution.xbest, xh, cols_tot * sizeof(double));
        }

        
        // Unfix the variables
        //set_default_lb2(env, lp, ncols_fixed, indexes);
        set_default_bounds(env, lp, ncols_fixed, indexes, bounds);
        save_lp(env, lp, "YEEEEE2");
    } while(num++ < num_iter);
    FREE(indexes);
    FREE(xh);
    return 0;
}