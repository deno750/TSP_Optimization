#include "heuristics.h"

#include "distutil.h"
#include "float.h"

#include <sys/stat.h>
 
#define WRONG_STARTING_NODE 1

int is_connected(instance *inst, double* xstar) {

    int num_comp = 0;
    int* comp = MALLOC(inst->num_nodes, int);
    MEMSET(comp, -1.0, inst->num_nodes, int);
    for (int i = 0; i < inst->num_nodes; i++ ) {
		if ( comp[i] >= 0 ) continue; 

		// a new component is found
		num_comp++;
		int current_node = i;
		int visit_comp = 0; // 
        int comp_members = 1;
		while ( !visit_comp ) { // go and visit the current component
			comp[current_node] = num_comp;
			visit_comp = 1; // We set the flag visited to true until we find the successor
			for ( int j = 0; j < inst->num_nodes; j++ ) {
                if (current_node == j || comp[j] >= 0) continue;
				if (fabs(xstar[x_udir_pos(current_node,j,inst->num_nodes)]) >= EPS ) {
					current_node = j;
					visit_comp = 0;
                    comp_members++;
					break;
				}
			}
		}	
	}
    FREE(comp);
    return num_comp == 1;
}

void HEU_kruskal(instance *inst) {
    int num_edges = inst->num_nodes * (inst->num_nodes - 1) / 2;
    double *dists = MALLOC(num_edges, double);
    MEMSET(dists, -1, num_edges, double);
    int k = 0;
    for (int i = 0; i < inst->num_nodes; i++) {
        //point p1 = inst->nodes[i];
        for (int j = i + 1; j < inst->num_nodes; j++) {
            
            //point p2 = inst->nodes[j];
            double distance = calc_dist(i, j, inst);
            /*if (k == 0) {
                dists[k++] = distance;
                continue;
            }
            double temp = distance; 
            for (int h = 0; h < k; h++) {
                if (distance >= dists[h]) { continue; }
            }*/
            dists[k++];
        }
    }
    

    FREE(dists);
}

static int greedy(instance *inst, int starting_node) {
    if (starting_node >= inst->num_nodes) {
        return WRONG_STARTING_NODE;
    }

    int *visited = CALLOC(inst->num_columns, int);
    double obj = 0;

    int curr = starting_node;
    int called = 0;
    while (1) {
        int minidx = -1;
        double mindist = DBL_MAX;
        //LOG_D("CALLED %d", ++called);
        for (int i = 0; i < inst->num_nodes; i++) {
            if (curr == i || visited[x_udir_pos(curr, i, inst->num_nodes)]) { continue; }
            double currdist = calc_dist(curr, i, inst);
            if (currdist < mindist) {
                mindist = currdist;
                minidx = i;
            }
        }
        if (minidx == -1) {
            break;
        }
        
        
        inst->solution.xbest[x_udir_pos(curr, minidx, inst->num_nodes)] = 1.0;
        visited[x_udir_pos(curr, minidx, inst->num_nodes)] = 1;
        obj += mindist;
        curr = minidx;
    }
    inst->solution.obj_best = obj;
    FREE(visited);
}

int HEU_greedy(instance *inst) {
    int status;
    double *xbest = CALLOC(inst->num_columns, double);
    double objbest = DBL_MAX;
    /*for (int i = 0; i < inst->num_nodes; i++) {
        if (inst->params.verbose >= 4) {
            LOG_I("Starting node %d", i);
        }
        
        status = greedy(inst, i);
        if (status) {
            return status;
        }
        if (inst->solution.obj_best < objbest) {
            LOG_D("FOUND BEST OBJ");
            objbest = inst->solution.obj_best;
            memcpy(xbest, inst->solution.xbest, inst->num_columns);
        }
    }*/
    status = greedy(inst, 0);
    //inst->solution.obj_best = objbest;
    //memcpy(inst->solution.xbest, xbest, inst->num_columns);
    FREE(xbest);
    return 0;
}