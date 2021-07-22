#include "callback.h"

#include <concorde.h>
#include <cut.h>
#include "heuristics.h"


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
    double *xstar = MALLOC(ncols, double);
    double objval = CPX_INFBOUND;
    int currentnode = -1; CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODECOUNT, &currentnode);

    int status = CPXcallbackgetcandidatepoint(context, xstar, 0, ncols - 1 , &objval);
    if (status) LOG_E("CPXcallbackgetcandidatepoint() error code %d", status);

    int *succ = MALLOC(inst->num_nodes, int);
    MEMSET(succ, -1, inst->num_nodes, int);
    int *comp = MALLOC(inst->num_nodes, int);
    MEMSET(comp, -1, inst->num_nodes, int);
    int num_comp = count_components(inst, xstar, succ, comp);

    if (inst->params.verbose >= 5) {
        //LOG_I("Candidate callback");
        LOG_I("Num components candidate: %d", num_comp);
    }

    if (num_comp > 1) { // More than one tours found. Violated so add the cuts
        if (inst->params.verbose >= 5) {
            LOG_I("Added SEC cut in node %d", currentnode);
        }
        int *indexes = MALLOC(ncols, int);
        double *values = MALLOC(ncols, double);
        for (int subtour = 1; subtour <= num_comp; subtour++) {
            // For each subtour we add the constraints in one shot
            status = add_SEC_cuts(inst, context, subtour, comp, indexes, values);
            if (status) LOG_E("Error with add_SEC_cuts. Error code %d", status);
        }
        FREE(indexes);
        FREE(values);
        
    } else if (num_comp == 1 && inst->params.callback_2opt) {
        // Here we have a candidate TSP solution. It is not optimal and it may have crossing edges. We want to help cplex on finding a better
        // solution applying the 2-opt algorithm on the solution just found. 

        // To avoid race condition between threads, a new instance is created locally in each thread to compute 2opt refinement
        instance tempinst;
        copy_instance(&tempinst, inst);
        save_solution_edges(&tempinst, xstar);
        tempinst.solution.obj_best = objval; // 2opt needs the current objective value
        alg_2opt(&tempinst);
        if (inst->params.verbose >= 5) {
            LOG_I("Applied 2-opt refinement");
        }
        LOG_D("Incubement: %0.0f", tempinst.solution.obj_best);
        // Reinit xstar to 0. We want to reuse it in order to avoid another memory allocation
        MEMSET(xstar, 0.0, inst->num_columns, double);
       
        // Filling xstar with the solution found by 2-opt
        for (int i = 0; i < tempinst.num_nodes; i++) {
            edge e = tempinst.solution.edges[i];
            xstar[x_udir_pos(e.i, e.j, tempinst.num_nodes)] = 1.0;
        }

        // Saving the heuristic solution found with 2opt to cplex in order to improve the convergence speed. The 2-opt solution found is complete so no need to check it
        // Check here for other strategies: https://www.ibm.com/docs/en/icos/20.1.0?topic=manual-cpxcallbacksolutionstrategy
        int status = CPXcallbackpostheursoln(context, ncols, inst->ind, xstar, tempinst.solution.obj_best, CPXCALLBACKSOLUTION_NOCHECK);
        if (status) {
            LOG_I("An error occured on CPXcallbackpostheursoln");
        }
        free_instance(&tempinst);

    }

    FREE(xstar);
    FREE(succ);
    FREE(comp);
    return 0;
}

static int violated_cuts_callback(double cutval, int num_nodes, int* members, void* param) {
    //LOG_D("Violated cuts callback");
    relaxation_callback_params *params = (relaxation_callback_params*) param;
    instance *inst = params->inst;
    CPXCALLBACKCONTEXTptr context = params->context;
    if (inst->params.verbose >= 5) {
        //LOG_I("Single component violated cuts");
        LOG_I("A cut with %d nodes has cut value of %f", num_nodes, cutval);
    }


    double rhs = num_nodes - 1;
    char sense = 'L';
    int matbeg = 0;
    int num_edges = num_nodes * (num_nodes - 1) / 2;
    //LOG_D("Num edges: %d\n", num_edges);
    double *values = MALLOC(num_edges, double);
    MEMSET(values, 1.0, num_edges, double);
    int *edges = MALLOC(num_edges, int);
    int k = 0;
    for (int i = 0; i < num_nodes; i++) {
        for (int j = 0; j < num_nodes; j++) {
            if (members[i] >= members[j]) { continue; } // undirected graph. If the node in index i is greated than the node in index j, we skip since (i,j) = (j,i)
            edges[k++] = x_udir_pos(members[i], members[j], inst->num_nodes);
            //LOG_D("X(%d,%d)", members[i], members[j]);
        }
    }
    int purgeable = CPX_USECUT_FILTER;
	int local = 0;
    int status = CPXcallbackaddusercuts(context, 1, num_edges, &rhs, &sense, &matbeg, edges, values, &purgeable, &local);
    FREE(values);
    if (status) LOG_E("CPXcallbackaddusercuts() when conn comps = 1. Error code %d", status);
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
    
    //LOG_D("Depth is %d", depth);
    //LOG_D("Current node %d", node);
    //LOG_D("Thread id: %d\n", threadid);
    
    // Uses nano time as seed parameter for randomness
    //struct timeval time_now;
    //gettimeofday(&time_now, 0);
    //unsigned int seed =  ((unsigned int) time_now.tv_usec) ^ threadid;
 
    // Uses the thread_seeds array in the instance to obtain a seed for randomness
    unsigned int seed = inst->thread_seeds[threadid];
    // Modifying an array on the fly in a threaded code is not a great idea. 
    // However, each thread accesses only in it's specific index on the array. 
    // So it cannot occur that two different threads access at the same location
    // in the array causing race condition
    inst->thread_seeds[threadid] += 1; 
    
    double rand_num = ((double) (rand_r(&seed))) / RAND_MAX;

    if (rand_num > 0.1) return 0; // Hyperparameter tuning
    //if (depth > 5) return 0; // Hyperparameter tuning
    if (inst->params.verbose >= 5) {
        LOG_I("Relaxation cut");
    }
    long ncols = inst->num_columns;
    double *xstar = MALLOC(ncols, double);
    int status = CPXcallbackgetrelaxationpoint(context, xstar, 0, ncols - 1 , &objval);
    if (status) {
        LOG_E("CPXcallbackgetrelaxationpoint() error code %d", status);
    }
    int numcomps = 0;
    int *elist = MALLOC(2*ncols, int); // elist contains each pair of vertex such as (1,2), (1,3), (1,4), (2, 3), (2,4), (3,4) so in list becomes: 1,2,1,3,1,4,2,3,2,4,3,4
    int *compscount = NULL; 
    int *comps = NULL;
    int k = 0;

    int num_edges = 0;
    for (int i = 0; i < inst->num_nodes; i++) {
        for (int j = i+1; j < inst->num_nodes; j++) {
            //if (fabs(xstar[x_udir_pos(i, j, inst->num_nodes)]) <= EPS) continue;
            elist[k++] = i;
            elist[k++] = j;
            num_edges++;
        }
    }
    // Checking whether or not the graph is connected with the fractional solution.
    status = CCcut_connect_components(inst->num_nodes, num_edges, elist, xstar, &numcomps, &compscount, &comps);
    if (status) {
        LOG_E("CCcut_connect_components() error code %d", status);
    }

    if (numcomps == 1) { 
        if (inst->params.verbose >= 5) {
            LOG_I("Single component");
        }
        relaxation_callback_params params = {.context = context, .inst = inst};
        // At this point we have a connected graph. This graph could not be a "tsp". We interpret the fractional
        // solution as capacity of a cut. A cut of a graph G is composed by S and T = V - S where V is the nodes set.
        // The capacity of the cut is the sum of all ingoing and outgoing edges of the cut. Since we have a TSP,
        // we want that for each cut, we have a capacity of 2 (i.e. one ingoing edge and one outgoing edge).
        // So we want to seek the cuts which don't have a capacity of 2. The cuts with capacity < 2 violates the 
        // constraints and we are going to add SEC to them.
        // NB: We use cutoff as 2.0 - EPS for numerical stability due the fractional values we obtain in the solution. 
        status = CCcut_violated_cuts(inst->num_nodes, ncols, elist, xstar, 2.0 - EPS, violated_cuts_callback, &params);
        if (status) {
            LOG_E("CCcut_violated_cuts() error code %d", status);
        }
    } else if (numcomps > 1) {
        if (inst->params.verbose >= 5) {
            LOG_I("Num components fractional: %d", numcomps);
        }
        int startindex = 0;

        int *components = MALLOC(inst->num_nodes, int);

        // Transforming the concorde's component format into our component format in order to use our addSEC function
        for (int subtour = 0; subtour < numcomps; subtour++) {

            for (int i = startindex; i < startindex + compscount[subtour]; i++) {
                int index = comps[i];
                components[index] = subtour + 1;
            }

            startindex += compscount[subtour];
            
        }

        int *indexes = MALLOC(ncols, int);
        double *values = MALLOC(ncols, double);
        for (int subtour = 1; subtour <= numcomps; subtour++) {
            // For each subtour we add the constraints in one shot
            status = add_SEC_cuts_fractional(inst, context, subtour, components, indexes, values);
            if (status) {
                LOG_E("Error with add_SEC_cuts. Error code %d", status);
            }
            if (inst->params.verbose >= 5) {
                LOG_I("Added SEC cuts to tour %d", subtour);
            }
        }
        FREE(indexes);
        FREE(values);

        FREE(components);

    }

    
    FREE(xstar);
    FREE(elist);
    FREE(compscount);
    FREE(comps);
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