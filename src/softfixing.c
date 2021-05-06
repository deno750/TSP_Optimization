#include "softfixing.h"

#include "solver.h"

int soft_fixing_solver(instance *inst, CPXENVptr env, CPXLPptr lp) {

    // For other emphasis params check there: https://www.ibm.com/docs/en/icos/20.1.0?topic=parameters-mip-emphasis-switch
    CPXsetintparam(env, CPXPARAM_Emphasis_MIP, CPX_MIPEMPHASIS_HEURISTIC); // We want that cplex finds an high quality solution earlier
    CPXsetintparam(env, CPX_PARAM_NODELIM, 0); // We limit the first solution space to the root node
    double time_limit = inst->params.time_limit > 0 ? inst->params.time_limit : SOFT_FIX_TIME_LIM_DEFAULT;
    CPXsetdblparam(env, CPXPARAM_TimeLimit, time_limit);

    int cols_tot = CPXgetnumcols(env, lp);
    int *indexes = CALLOC(cols_tot, int);
    double *values = CALLOC(cols_tot, double);
    double *xh = CALLOC(cols_tot, double); // The current solution found
    struct timeval start, end; 
    gettimeofday(&start, 0);

    // First iteration: seeking the first feasible solution
    int status = opt_best_solver(env, lp, inst);
    if (status) {LOG_E("CPXmipopt in hard fixing error code %d", status);}
    status = CPXgetx(env, lp, xh, 0, cols_tot - 1); // save the first solution found
    CPXsetintparam(env, CPX_PARAM_NODELIM, 100); 

    CPXsetintparam(env, CPXPARAM_Emphasis_MIP, CPX_MIPEMPHASIS_OPTIMALITY);
    double radius[] = {3, 5, 7, 9};
    int rad_index = 0;
    double objval;
    double objbest = CPX_INFBOUND;
    int done = 0;
    int number_little_improvements = 0;
    char sense = 'G';
    int matbeg = 0;
    int num_iter = 0;
    char *names = CALLOC(100, char);
    while (!done) {
        done = 1;
        num_iter++;
        gettimeofday(&end, 0);
        double elapsed = get_elapsed_time(start, end);
        if (elapsed >= time_limit) {
            break;
        }
        double time_remain = time_limit - elapsed; // this is the time remained 
        CPXsetdblparam(env, CPXPARAM_TimeLimit, time_remain);
        int k = 0;
        for (int i = 0; i < cols_tot; i++) {
            if (xh[i] > 0.5) {
                indexes[k] = i;
                values[k] = 1.0;
                k++;
            }
        }
        double rhs = inst->num_nodes - radius[rad_index];
        sprintf(names, "new_constraint(%d)", num_iter);
        status = CPXaddrows(env, lp, 0, 1, k, &rhs, &sense, &matbeg, indexes, values, NULL, &names); 
        if (status) {
            LOG_E("CPXaddrows in softfixing error code %d", status);
        }  
        status = CPXmipopt(env, lp);
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
            if (obj_improv < SOFT_FIX_MIN_IMPROVEMENT) {
                LOG_D("NOT IMPROVED TOO MUCH");
                number_little_improvements++;
                LOG_D("Prob_index: %d Len Prob: %lu Num Little improv: %d", rad_index, LEN(radius), number_little_improvements);
                if (number_little_improvements % SOFT_FIX_MAX_LITTLE_IMPROVEMENTS == 0 && rad_index < LEN(radius) - 1) {
                    rad_index++;
                    LOG_D("CONSECUTIVE LITTLE IMPROVMENETS. UPDATING THE PROB INDEX");
                }
            }
            LOG_I("Updated incubement: %f", objval);
            objbest = objval;
            inst->solution.obj_best = objval;
            memcpy(inst->solution.xbest, xh, cols_tot * sizeof(double));
            save_solution_edges(inst, xh);
            plot_solution(inst);
        } else {
            if (rad_index < LEN(radius) - 1) {
                rad_index++;
                done = 0;
            }
        }

        
        // Delete the added constraint
        int numrows = CPXgetnumrows(env, lp);
        status = CPXdelrows(env, lp, numrows - 1, numrows - 1);
        if (status) {
            LOG_E("CPXdelrows error code %d", status);
        }
        save_lp(env, lp, "AferRestoring");
    }
    
    
    FREE(indexes);
    FREE(values);
    FREE(xh);
    FREE(names);
    return 0;
}