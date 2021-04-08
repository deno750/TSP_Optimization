#include "benders.h"

#include <stdlib.h>
#include <string.h>

#include "utility.h"

static int add_SEC(instance *inst, CPXENVptr env, CPXLPptr lp, int current_tour, int *comp, char *sense, int *matbeg, int *indexes, double *values, char *names) {
    int nnz = 0; // Number of variables to add in the constraint
    int num_nodes = 0; // We need to know the number of nodes due the vincle |S| - 1
    for (int i = 0; i < inst->num_nodes; i++) {
        if (comp[i] != current_tour) continue;
            num_nodes++;

        for (int j = i+1; j < inst->num_nodes; j++) {
            if (comp[j] != current_tour) continue;
            indexes[nnz] = x_udir_pos(i, j, inst->num_nodes);
            values[nnz] = 1.0;
            nnz++;
        }
    }

    if (inst->params.verbose >= 5) {
        printf("Tour %d has %d nodes\n", current_tour, num_nodes);
    }
    
    double rhs =  num_nodes - 1; // |S| - 1

    // For each subtour we add the constraints in one shot
    return CPXaddrows(env, lp, 0, 1, nnz, &rhs, sense, matbeg, indexes, values, NULL, &names);    
}

int benders_loop(instance *inst, CPXENVptr env, CPXLPptr lp) {

    int* successors = (int*) malloc(inst->num_nodes * sizeof(int)); //malloc since those arrays will be initialized to -1
    int* comp = (int*) malloc(inst->num_nodes * sizeof(int));
    
    char names[100];
    char sense = 'L';
    int matbeg = 0; // Contains the index of the beginning column
    int numcomp = 1;
    int rowscount = 0;
    do {
        //Initialization of successors and comp arrays with -1
        memset(successors, -1, inst->num_nodes * sizeof(int)); 
        memset(comp, -1, inst->num_nodes * sizeof(int));

        int status = CPXmipopt(env, lp);
        if (status) { print_error("Benders CPXmipopt error"); }

        
        int ncols = CPXgetnumcols(env, lp);

        // initialization of the following vectors is not useful here, so we use malloc because is faster than calloc
        int *indexes = (int*) malloc(ncols * sizeof(int));
        double *values = (double*) malloc(ncols * sizeof(double));
        double *xstar = (double*) malloc(ncols * sizeof(double));

        status = CPXgetx(env, lp, xstar, 0, ncols-1);
        if (status) { print_error("Benders CPXgetx error"); }
        numcomp = count_components(inst, xstar, successors, comp);
        if (inst->params.verbose >= 4) {
            printf("NUM COMPONENTS: %d\n", numcomp);
        }
        
        // Condition numComp > 1 is needed in order to avoid to add the SEC constraints when the TSP's hamiltonian cycle is found
        for (int subtour = 1; subtour <= numcomp && numcomp > 1; subtour++) { // Connected components are numerated from 1
            sprintf(names, "SEC(%d)", ++rowscount);
            status = add_SEC(inst, env, lp, subtour, comp, &sense, &matbeg, indexes, values, names);
            if (status) { print_error("An error occurred adding SEC"); }
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