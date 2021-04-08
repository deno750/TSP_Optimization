#include "callback.h"

#include "utility.h"

#include "string.h"

static int add_SEC_cuts(instance *inst, CPXCALLBACKCONTEXTptr context, int current_tour, int *comp, int *indexes, double *values) {
    double rhs; 
    char sense;
    int matbeg = 0; // Contains the index of the beginning column. In this case we add 1 row at a time so no need for an array
    int nnz = prepare_SEC(inst, current_tour, comp, &sense, indexes, values, &rhs);
    return CPXcallbackrejectcandidate(context, 1, nnz, &rhs, &sense, &matbeg, indexes, values);   
}

int CPXPUBLIC SEC_cuts_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle ) {
    instance *inst = (instance*) userhandle;

    int ncols = inst->num_nodes * (inst->num_nodes - 1) / 2; // Better to get the number of cols directly from cplex. What's the function to do that?
    double *xstar = (double*) malloc(ncols * sizeof(double));
    double objval = CPX_INFBOUND;
    int currentnode = -1; CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODECOUNT, &currentnode);

    int status = CPXcallbackgetcandidatepoint(context, xstar, 0, ncols - 1 , &objval);
    if (status) print_error("Error with CPXcallbackgetcandidatepoint");

    int *succ = (int*) malloc(inst->num_nodes * sizeof(int));
    memset(succ, -1, inst->num_nodes * sizeof(int));
    int *comp = (int*) malloc(inst->num_nodes * sizeof(int));
    memset(comp, -1, inst->num_nodes * sizeof(int));
    int num_comp = count_components(inst, xstar, succ, comp);

    if (num_comp > 1) { // More than one tours found. Violated so add the cuts
        if (inst->params.verbose >= 5) {
            printf("Added SEC cut in node %d\n", currentnode);
        }
        int *indexes = (int*) malloc(ncols * sizeof(int));
        double *values = (double*) malloc(ncols * sizeof(double));
        for (int subtour = 1; subtour <= num_comp; subtour++) {
            // For each subtour we add the constraints in one shot
            status = add_SEC_cuts(inst, context, subtour, comp, indexes, values);
            if (status) print_error("Error with add_SEC_cuts");
        }
        free(indexes);
        free(values);
        
    }

    free(xstar);
    free(succ);
    free(comp);
    return 0;
}