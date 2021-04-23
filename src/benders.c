#include "benders.h"

#include <sys/time.h>
#include <stdlib.h>
#include <string.h>

static int add_SEC(instance *inst, CPXENVptr env, CPXLPptr lp, int current_tour, int *comp, int *indexes, double *values, char *names) {
    double rhs;
    char sense;
    int matbeg = 0; // Contains the index of the beginning column. In this case we add 1 row at a time so no need for an array
    int nnz = prepare_SEC(inst, current_tour, comp, &sense, indexes, values, &rhs);
    LOG_D("Adding SEC cut");
    if (inst->params.verbose >= 5) {
        LOG_I("Tour %d has %d nodes", current_tour, (int) (rhs + 1)); // Since rhs is |S| - 1 we add +1 to get |S|.
    }
    return CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &matbeg, indexes, values, NULL, &names);    
}

int benders_loop(instance *inst, CPXENVptr env, CPXLPptr lp) {

    int* successors = MALLOC(inst->num_nodes, int); //malloc since those arrays will be initialized to -1
    int* comp = MALLOC(inst->num_nodes, int);
    
    char names[100];
    int numcomp = 1;
    int rowscount = 0;
    struct timeval start, end;
    gettimeofday(&start, 0);
    
    do {
        gettimeofday(&end, 0);
        double elapsed = get_elapsed_time(start, end);
        double timelimit = (double) inst->params.time_limit;
        if (timelimit > 0 && elapsed > timelimit) {
            free(successors);
            free(comp);
            return CPX_STAT_ABORT_TIME_LIM;
        }

        // We apply to cplex the residual time left to solve the problem
        if (timelimit > 0 && elapsed < timelimit) {
            double new_timelim = timelimit - elapsed; // The residual time limit that is left
            CPXsetdblparam(env, CPXPARAM_TimeLimit, new_timelim);
            double here = 0;
            CPXgetdblparam(env, CPXPARAM_TimeLimit, &here);
        }

        int status = CPXmipopt(env, lp);
        if (status) { 
            free(successors);
            free(comp);
            return status;
        }

        //Initialization of successors and comp arrays with -1
        MEMSET(successors, -1, inst->num_nodes, int);
        MEMSET(comp, -1, inst->num_nodes, int);
        /*for (int i = 0; i < inst->num_nodes; i++) {
            LOG_I("Comp %d", comp[i]);
        }*/
        
        int ncols = CPXgetnumcols(env, lp);

        // initialization of the following vectors is not useful here, so we use malloc because is faster than calloc
        int *indexes = MALLOC(ncols, int);
        double *values = MALLOC(ncols, double);
        double *xstar = MALLOC(ncols, double);

        status = CPXgetx(env, lp, xstar, 0, ncols-1);
        if (status) { LOG_E("Benders CPXgetx() error code %d", status); }
        numcomp = count_components(inst, xstar, successors, comp);
        if (inst->params.verbose >= 4) {
            LOG_I("NUM COMPONENTS: %d", numcomp);
        }
        
        // Condition numComp > 1 is needed in order to avoid to add the SEC constraints when the TSP's hamiltonian cycle is found
        for (int subtour = 1; subtour <= numcomp && numcomp > 1; subtour++) { // Connected components are numerated from 1
            sprintf(names, "SEC(%d)", ++rowscount);
            // For each subtour we add the constraints in one shot
            status = add_SEC(inst, env, lp, subtour, comp, indexes, values, names);
            if (status) { LOG_E("An error occurred adding SEC. Error code %d", status); }
            save_lp(env, lp, inst->name);
        }
        if (inst->params.verbose >= 5) {
            printf("\n");
        }

        free(indexes);
        free(values);
        free(xstar);
            
    } while (numcomp > 1);

    free(successors);
    free(comp);

    return 0;
}