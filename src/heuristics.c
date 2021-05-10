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

    int *visited = CALLOC(inst->num_nodes, int);
    double obj = 0;

    int curr = starting_node;
    visited[starting_node] = 1;
    while (1) {
        int minidx = -1;
        double mindist = DBL_MAX;
        for (int i = 0; i < inst->num_nodes; i++) {
            if (curr == i || visited[i]) { continue; }
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
        visited[minidx] = 1;
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
    int *nodes_visited = CALLOC(inst->num_nodes, int); // Stores nodes visited in tour
    int num_visited = hsize;
    
    int k = 0;
    for (int i = 0; i < hsize; i++) {
        point p1 = hull[i];
        for (int j = 0; j < inst->num_nodes; j++) {
            point p2 = inst->nodes[j];
            if (p1.x == p2.x && p1.y == p2.y) {
                hindex[k] = j;
                nodes_visited[j] = 1;
                k++;
                break;
            }
        }
    }
    // initialized convex hull edges
    edge *edges = CALLOC(inst->num_nodes, edge);
    for (int i = 0; i < hsize - 1; i++) {
        inst->solution.xbest[x_udir_pos(hindex[i], hindex[i+1], inst->num_nodes)] = 1.0;
        edge e;
        e.i = hindex[i];
        e.j = hindex[i+1];
        edges[i] = e;
    }
    save_solution_edges(inst, inst->solution.xbest);
    plot_solution(inst);
    while (num_visited < inst->num_nodes) {
        for (int i = 0; i < inst->num_nodes; i++) {
            if (nodes_visited[i]) { continue; }
            
            double min_mileage = DBL_MAX;
            edge best_edge;
            int best_edge_idx = -1;
            for (int j = 0; j < num_visited; j++) {
                edge e = edges[j];
                int a = e.i;
                int b = e.j;
                int c = i;
                double cost1 = calc_dist(a, c, inst);
                double cost2 = calc_dist(c, b, inst);
                double cost3 = calc_dist(a, b, inst);
                double deltacost = cost1 + cost2 - cost3;
                if (deltacost < min_mileage) {
                    min_mileage = deltacost;
                    best_edge = e;
                    best_edge_idx = j;
                }
            }
            if (best_edge_idx == -1) {
                break;
            }
            edge e1;
            e1.i = best_edge.i;
            e1.j = i;
            edge e2;
            e2.i = i;
            e2.j = best_edge.j;
            inst->solution.xbest[x_udir_pos(best_edge.i, best_edge.j, inst->num_nodes)] = 0.0;
            inst->solution.xbest[x_udir_pos(e1.i, e1.j, inst->num_nodes)] = 1.0;
            inst->solution.xbest[x_udir_pos(e2.i, e2.j, inst->num_nodes)] = 1.0;
            edges[best_edge_idx] = e1;
            edges[num_visited++] = e2;
            nodes_visited[i] = 1;

            save_solution_edges(inst, inst->solution.xbest);
            plot_solution(inst);
        }
    }
    return 0;
}