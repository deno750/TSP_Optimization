#include "softfixing.h"
#include "solver.h"

int soft_fixing_solver(instance *inst, CPXENVptr env, CPXLPptr lp) {

    // For other emphasis params check there: https://www.ibm.com/docs/en/icos/20.1.0?topic=parameters-mip-emphasis-switch
    CPXsetintparam(env, CPXPARAM_Emphasis_MIP, CPX_MIPEMPHASIS_HEURISTIC); // We want that cplex finds an high quality solution earlier
    CPXsetintparam(env, CPX_PARAM_NODELIM, 0); // We limit the first solution space to the root node
    double time_limit = inst->params.time_limit > 0 ? inst->params.time_limit : DEFAULT_TIME_LIM;
    CPXsetdblparam(env, CPXPARAM_TimeLimit, time_limit);
    inst->solution.xbest = CALLOC(inst->num_columns, double); // The best solution found till now
    int cols_tot = CPXgetnumcols(env, lp);
    int *indexes = CALLOC(cols_tot, int);
    double *values = CALLOC(cols_tot, double);
    double *xh = CALLOC(cols_tot, double); // The current solution found

    struct timeval start, end; 
    gettimeofday(&start, 0);    // start counting elapsed time from now

    // First iteration: seek the first feasible solution
    int status = opt_best_solver(env, lp, inst);
    if (status) {LOG_E("CPXmipopt in hard fixing error code %d", status);}
    status = CPXgetx(env, lp, xh, 0, cols_tot - 1); // save the first solution found
    CPXsetintparam(env, CPX_PARAM_NODELIM, 100); 

    CPXsetintparam(env, CPXPARAM_Emphasis_MIP, CPX_MIPEMPHASIS_OPTIMALITY);

    double radius[] = {3, 5, 7, 9}; //Array of radious
    int rad_index = 0;
    double objval;
    double objbest = CPX_INFBOUND;
    CPXgetobjval(env, lp, &objbest);    //assign to best solution the initial computed by CPLEX
    int number_small_improvements = 0;
    char sense = 'G';   // >=
    int matbeg = 0;
    int num_iter = 0;   // keeps track of the number of iterations
    char *names = CALLOC(100, char);
    if (inst->params.verbose >= 3) {
        LOG_I("Updated incubement: %0.2f", objbest);    // print first solution
    }
    

    //While we are within the time limit and the radius array size
    while (1) {
        //if there are no more radious to use 
        if (rad_index >= LEN(radius)) {break;}  //stop

        //Check if the time_limit is reached
        gettimeofday(&end, 0);
        double elapsed = get_elapsed_time(start, end);
        if (elapsed >= time_limit) {
            break;
        }

        //Set remaining time
        double time_remain = time_limit - elapsed; // this is the time remained 
        CPXsetdblparam(env, CPXPARAM_TimeLimit, time_remain);
        if (inst->params.verbose >= 4) {
            LOG_I("Time remaining: %0.1f seconds",time_remain);
        }
       

        // Add new constraints according to the radius: SUM_{x_e=1}{x_e}>=n-radius
        int k = 0;
        for (int i = 0; i < cols_tot; i++) {
            if (xh[i] > 0.5) {
                indexes[k] = i;
                values[k] = 1.0;
                k++;
            }
        }
        double rhs = inst->num_nodes - radius[rad_index];   //n-radius
        sprintf(names, "new_constraint(%d)", num_iter);
        status = CPXaddrows(env, lp, 0, 1, k, &rhs, &sense, &matbeg, indexes, values, NULL, &names); 
        if (status) {
            LOG_E("CPXaddrows in softfixing error code %d", status);
        }

        // Solve the model
        status = CPXmipopt(env, lp);
        save_lp(env, lp, "AfterFixing");
        if (status) {
            LOG_E("CPXmipopt error code %d", status);
        }

        //Retreive the solution
        status = CPXgetx(env, lp, xh, 0, cols_tot - 1);
        CPXgetobjval(env, lp, &objval);
        if (status) { LOG_E("CPXgetx error code %d", status); }

        // Calculate how much the new solution is better then the previous
        double obj_improv = 1 - objval / objbest;
        LOG_D("Improvement %0.4f", obj_improv);

        if (objval < objbest) {
            //IF not improved much
            if (obj_improv < SOFT_FIX_MIN_IMPROVEMENT) {
                //LOG_D("NOT IMPROVED TOO MUCH");
                number_small_improvements++;
                //LOG_D("rad_index: %d Len Rad: %lu Num Small improv: %d", rad_index, LEN(radius), number_small_improvements);

                //After a certain amount fo small improvements, go use the next radious.
                if (number_small_improvements % SOFT_FIX_MAX_LITTLE_IMPROVEMENTS == 0 && rad_index < LEN(radius) - 1) {
                    rad_index++;    //use next radious
                    //LOG_D("CONSECUTIVE SMALL IMPROVMENETS. UPDATING THE PROB INDEX");
                }
            }else {    // If new solution is quite better than the previous
                number_small_improvements = 0;
            }

            //Update solution
            if (inst->params.verbose >= 3) {
                LOG_I("Updated incubement: %0.2f", objval);
            }
            objbest = objval;
            inst->solution.obj_best = objval;
            memcpy(inst->solution.xbest, xh, cols_tot * sizeof(double));
            if (!(inst->params.perf_prof)) {
                save_solution_edges(inst, xh);
                plot_solution(inst);
            }
        }
 
        // Remove the added soft-fixing constraints
        int numrows = CPXgetnumrows(env, lp);
        status = CPXdelrows(env, lp, numrows - 1, numrows - 1);
        if (status) {
            LOG_E("CPXdelrows error code %d", status);
        }
        //save_lp(env, lp, "AferRestoring");

        num_iter++;
    }
    
    
    FREE(indexes);
    FREE(values);
    FREE(xh);
    FREE(names);
    return 0;
}