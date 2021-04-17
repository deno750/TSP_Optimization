#include "callback.h"

#include <string.h>
#include <concorde.h>
#include <cut.h>
#include "utility.h"

static int add_SEC_cuts(instance *inst, CPXCALLBACKCONTEXTptr context, int current_tour, int *comp, int *indexes, double *values) {
    double rhs; 
    char sense;
    int matbeg = 0; // Contains the index of the beginning column. In this case we add 1 row at a time so no need for an array
    int nnz = prepare_SEC(inst, current_tour, comp, &sense, indexes, values, &rhs);
    return CPXcallbackrejectcandidate(context, 1, nnz, &rhs, &sense, &matbeg, indexes, values);   
}

static int add_SEC_cuts_fractional(instance *inst, CPXCALLBACKCONTEXTptr context, int current_tour, int *comp, int *indexes, double *values) {
    double rhs; 
    char sense;
    int matbeg = 0; // Contains the index of the beginning column. In this case we add 1 row at a time so no need for an array
    int purgeable = CPX_USECUT_FILTER;
	int local = 0;
    int nnz = prepare_SEC(inst, current_tour, comp, &sense, indexes, values, &rhs); 
    return CPXcallbackaddusercuts(context, 1, nnz, &rhs, &sense, &matbeg, indexes, values, &purgeable, &local); // add one violated usercut    
}

static int CPXPUBLIC SEC_cuts_callback_candidate(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, instance *inst ) {
    int ncols = inst->num_columns;
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

typedef struct {
    CPXCALLBACKCONTEXTptr context;
    instance *inst;

} relaxation_params;

static int doit_fn(double cutval, int num_nodes, int* members, void* param) {
    relaxation_params *params = (relaxation_params*) param;
    instance *inst = params->inst;
    CPXCALLBACKCONTEXTptr context = params->context;
    //printf("Cut Val: %f\n", cutval);
    //printf("num nodes %d\n", num_nodes);
    //printf("Doit function\n");

    double rhs = num_nodes - 1;
    char sense = 'L';
    int matbeg = 0;
    double *values = (double*) malloc(num_nodes * sizeof(double));
    memset(values, 1.0, num_nodes * sizeof(double));
    int purgeable = CPX_USECUT_FILTER;
	int local = 0;
    int status = CPXcallbackaddusercuts(context, 1, num_nodes, &rhs, &sense, &matbeg, members, values, &purgeable, &local);
    if (status) print_error("Error in CPXcallbackaddusercuts when conn comps = 1");
    return 0;
}

static int CPXPUBLIC SEC_cuts_callback_relaxation(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, instance *inst) {
    int depth = 0;
    int node = -1;
    int threadid = -1; 
    double objval = CPX_INFBOUND;
    CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODEDEPTH, &depth);
    CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODECOUNT, &node);
    CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID, &threadid); 
    if (node % 7 != 0) return 0; // hyperparameter tuning
    //if (depth > 10) return 0; // Hyperparameter tuning
    //printf("Relaxationnnn\n");
    int ncols = inst->num_columns;
    double *xstar = (double*) malloc(ncols * sizeof(double));
    int status = CPXcallbackgetrelaxationpoint(context, xstar, 0, ncols - 1 , &objval);
    if (status) {
        printf("Status: %d\n", status);
        print_error("CPXcallbackgetrelaxationpoint error");
    }
    int numcomps = 0;
    int *elist = (int*) malloc(2*ncols * sizeof(int)); // elist contains each pair of vertex such as (1,2), (1,3), (1,4), (2, 3), (2,4), (3,4) so in list becomes: 1,2,1,3,1,4,2,3,2,4,3,4
    int *compscount = NULL; 
    int *comps = NULL;
    int k = 0;
    for (int i = 0; i < inst->num_nodes; i++) {
        for (int j = i+1; j < inst->num_nodes; j++) {
            elist[k++] = i;
            elist[k++] = j;
        }
    }
    status = CCcut_connect_components(inst->num_nodes, ncols, elist, xstar, &numcomps, &compscount, &comps);
    if (status) {
        print_error("Error in CCcut_connect_components");
    }

    //printf("Num comps: %d\n", numcomps);

    if (numcomps == 1) {
        relaxation_params params = {.context = context, .inst = inst};
        status = CCcut_violated_cuts(inst->num_nodes, ncols, elist, xstar, 2.0 - EPS, doit_fn, &params);
        if (status) {
            print_error("Error in CCcut_connect_components");
        }
    }
    if (numcomps > 1) {
        int startindex = 0;

        int *components = (int*) malloc(inst->num_nodes * sizeof(int));

        for (int subtour = 0; subtour < numcomps; subtour++) {

            for (int i = startindex; i < startindex + compscount[subtour]; i++) {
                int index = comps[i];
                components[index] = subtour + 1;
            }

            startindex += compscount[subtour];
            
        }

        int *indexes = (int*) malloc(ncols * sizeof(int));
        double *values = (double*) malloc(ncols * sizeof(double));
        for (int subtour = 1; subtour <= numcomps; subtour++) {
            // For each subtour we add the constraints in one shot
            status = add_SEC_cuts_fractional(inst, context, subtour, components, indexes, values);
            if (status) {
                printf("Status: %d\n", status);
                print_error("Error with add_SEC_cuts");
            }
        }
        free(indexes);
        free(values);

        free(components);

    }

    
    free(xstar);
    free(elist);
    free(compscount);
    free(comps);
    return 0;
}

int CPXPUBLIC SEC_cuts_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle ) {
    instance *inst = (instance*) userhandle;
    if (contextid == CPX_CALLBACKCONTEXT_CANDIDATE) {
        return SEC_cuts_callback_candidate(context, contextid, inst);
    }
    if (contextid == CPX_CALLBACKCONTEXT_RELAXATION) {
        return SEC_cuts_callback_relaxation(context, contextid, inst);
    }
    return 1;
}