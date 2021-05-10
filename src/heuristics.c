#include "heuristics.h"

#include "distutil.h"
#include "convexhull.h"

#include <float.h>
#include <sys/stat.h>
 
#define WRONG_STARTING_NODE 1

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

int HEU_extramileage(instance *inst) {

    int hsize;
    point *hull = convexHull(inst->nodes, inst->num_nodes, &hsize);
    int *hindex = CALLOC(hsize, int);
    LOG_D("Convex hull");
    int k = 0;
    for (int i = 0; i < hsize; i++) {
        point p1 = hull[i];
        for (int j = 0; j < inst->num_nodes; j++) {
            point p2 = inst->nodes[j];
            if (p1.x == p2.x && p1.y == p2.y) {
                hindex[k++] = j;
                break;
            }
        }
    }
    // initialized convex hull edges
    for (int i = 0; i < hsize - 1; i++) {
        inst->solution.xbest[x_udir_pos(hindex[i], hindex[i+1], inst->num_nodes)] = 1.0;
    }
    return 0;
}