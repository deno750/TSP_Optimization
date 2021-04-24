#include "hardfixing.h"

#include "solver.h"
#include "utility.h"

#define HARD_FIX_TIME_LIM_DEFAULT 20



//Function that UNfix the edges
void set_default_lb(CPXENVptr env, CPXLPptr lp, int ncols, int *indexes){
	double *zeros = MALLOC(ncols, double);
    MEMSET(zeros, 0.0, ncols, double);
	char *lbs = MALLOC(ncols, char); // Lower bound
    MEMSET(lbs, 'L', ncols, char);
    int status = CPXchgbds(env, lp, ncols, indexes, lbs, zeros); // this function changes the lower and/or upper bound
    if (status) {LOG_E("CPXchgbds() error code %d", status);}
    free(zeros);
    free(lbs);
}

//Function that fix the edges randomly
void random_fix(CPXENVptr env, CPXLPptr lp, double prob, int *ncols, int *indexes, double *xh){
    double rand_num;
	double one = 1.0;
	char lb = 'L'; // Lower Bound
    //*ncols = 0;
    if(prob < 0 || prob > 1) {LOG_E("probability must be in [0,1]");}
    int num_cols = CPXgetnumcols(env, lp);


    
	struct timeval start;
    for(int i = 0; i < num_cols; i++){
        gettimeofday(&start, NULL);
		srand(start.tv_usec);//Get a different seed at every iteration
		rand_num = (double) random() / RAND_MAX;
		
		if(xh[i] > 0.5 && rand_num < prob)
		{
			CPXchgbds(env, lp, 1, &i, &lb, &one);
		}
    }

    /*for(int i = 0; i < num_cols; i++){
		rand_num = (double) rand() / RAND_MAX;
		if(xh[i] > 0.5 && rand_num < prob){
            indexes[(*ncols)++] = i;
		}
	}
    double *ones = MALLOC(*ncols, double);
    MEMSET(ones, 1.0, *ncols, double);
    char *lbs = MALLOC(*ncols, char);
    MEMSET(lbs, 'L', *ncols, char);
    int status = CPXchgbds(env, lp, *ncols, indexes, lbs, ones); // this function changes the lower and/or upper bound
    if (status) {LOG_E("CPXchgbds() error code %d", status);}
    free(ones);
    free(lbs);*/
}

/*
int hard_fixing(instance *inst, CPXENVptr env, CPXLPptr lp, int total_timelimit) {

    double prob_rate[] = {0.9, 0.8, 0.5, 0.2};
    int current_prob_index=0;
    int not_improved=0;
    int done=0;
    double best_cost=10^9;  //initial best cost set to "infinity"

    struct timeval start, end;

    //Start counting time from now
    gettimeofday(&start, 0);

    // Initialize model without SEC
    //TODO ....

    //Calculate initial solution with small time limit or 0 node limit.
    //TODO....

    //Iterate untill time elapses <Time limit
    while(!done){
        gettimeofday(&end, 0);
        double elapsed = get_elapsed_time(start, end);

        if (elapsed > total_timelimit) {
            //STOP

        }


        //x_H best solution till now
        //....

        //Fix some edges. (change their lower bound to 1)
        //...

        //Call solver with small time limit.
        //.....

        

        //Get new solution cost
        double new_cost;
        //...

        // If solution cost is better than the seen so far
        if (new_cost>best_cost){
            // update best solution
            //....
        }
        else{
            //else: not improved++
            not_improved++;
        }
        
        //if not improved>=10: use next probability
        if(not_improved>=10):{ current_prob_index++;}

        // Save (timestamp,solution) to file .txt
        //....

        //Unfix all the variables.
        //....

        

    }

    


    



    return 0;
}*/

int hard_fixing_solver(instance *inst, CPXENVptr env, CPXLPptr lp) {
    // For other emphasis params check there: https://www.ibm.com/docs/en/icos/20.1.0?topic=parameters-mip-emphasis-switch
    CPXsetintparam(env, CPXPARAM_Emphasis_MIP, CPX_MIPEMPHASIS_HEURISTIC); // We want that cplex finds an high quality solution earlier
    CPXsetintparam(env, CPX_PARAM_NODELIM, 0);
    double time_limit = inst->params.time_limit > 0 ? inst->params.time_limit : HARD_FIX_TIME_LIM_DEFAULT;
    CPXsetdblparam(env, CPXPARAM_TimeLimit, time_limit);

    int status;

    int cols_tot = CPXgetnumcols(env, lp);
    int *indexes = MALLOC(cols_tot, int);
    double *xh = CALLOC(cols_tot, double);

    struct timeval start, end; 
    gettimeofday(&start, 0);

    status = opt_best_solver(env, lp, inst);
    if (status) {
        LOG_E("Best opt error code %d", status);
    }
    gettimeofday(&end, 0);

    double elapsed = get_elapsed_time(start, end);
    double time_remain = time_limit - elapsed; // this is the time remained that is going to be splitted with every mipopt execution

    status = CPXgetx(env, lp, xh, 0, cols_tot - 1);
    if (status) {LOG_E("CPXgetx error code %d", status);}

    int ncols_fixed;
    double prob = 0.7;
    unsigned int seed = inst->params.seed >= 0 ? inst->params.seed : 0;
    srand(seed); // This should go on the beginning of the program
    int num = 0;
    do {
        random_fix(env, lp, prob, &ncols_fixed, indexes, xh);
        LOG_D("COLS %d", ncols_fixed);
        save_lp(env, lp, "YEEEEE");

        status = CPXmipopt(env, lp);
        if (status) {LOG_E("CPXmipopt error code %d", status);}

        status = CPXgetx(env, lp, xh, 0, cols_tot - 1);
        if (status) {LOG_E("CPXgetx error code %d", status);}

        
        set_default_lb(env, lp, ncols_fixed, indexes);
        save_lp(env, lp, "YEEEEE2");
    } while(num++ < 3);
    
    

    free(indexes);
    free(xh);
    return 0;
}