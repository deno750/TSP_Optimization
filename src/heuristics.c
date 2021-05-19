#include "heuristics.h"

#include "distutil.h"
#include "convexhull.h"

#include <float.h>
#include <sys/stat.h>
#include <unistd.h>
 
#define WRONG_STARTING_NODE 1
#define TIME_LIMIT_EXCEEDED 2

static int greedy(instance *inst, int starting_node) {
    if (starting_node >= inst->num_nodes) {
        return WRONG_STARTING_NODE;
    }

    struct timeval start, end;
    gettimeofday(&start, 0);
    int *visited = CALLOC(inst->num_nodes, int);
    double obj = 0;

    int curr = starting_node;
    visited[starting_node] = 1;
    int status = 0;
    while (1) {
        gettimeofday(&end, 0);
        double elapsed = get_elapsed_time(start, end);
        if (inst->params.time_limit > 0 && elapsed > inst->params.time_limit) {
            status = TIME_LIMIT_EXCEEDED;
            break;
        }
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
            // Closing the tsp cycle 
            inst->solution.edges[curr].i = curr;
            inst->solution.edges[curr].j = starting_node;
            break;
        }
        
        inst->solution.edges[curr].i = curr;
        inst->solution.edges[curr].j = minidx;
        visited[minidx] = 1;
        obj += mindist;
        curr = minidx;

        
    }
    inst->solution.obj_best = obj;
    FREE(visited);
    return status;
}



int HEU_greedy(instance *inst) {
    int status;
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
    double obj = 0;
    for (int i = 0; i < hsize - 1; i++) {
        edge e;
        e.i = hindex[i];
        e.j = hindex[i+1];
        inst->solution.edges[hindex[i]] = e;
        edges[i] = e;
        obj += calc_dist(e.i, e.j, inst);
        
        /*
        plot_solution(inst);
        sleep(1);*/
    }
    // Closing the hamyltonian cycle 
    edge last_edge;
    last_edge.i = hindex[hsize - 1];
    last_edge.j = hindex[0];
    inst->solution.edges[last_edge.i] = last_edge;
    obj += calc_dist(last_edge.i, last_edge.j, inst);
    
    
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
            inst->solution.edges[e1.i] = e1;
            inst->solution.edges[e2.i] = e2;
            edges[best_edge_idx] = e1;
            edges[num_visited++] = e2;
            nodes_visited[i] = 1;
            obj += min_mileage; 

            /*
            plot_solution(inst);
            sleep(1);*/
        }
    }
    inst->solution.obj_best = obj;
    FREE(hindex);
    FREE(nodes_visited);
    FREE(hull);
    FREE(edges);
    return 0;
}

void reverse(int start_node, int end_node, int *prev, edge *edges, int num_nodes) {
    int currnode = start_node;
        
    while (1) {
        int node = prev[currnode];
        edges[currnode].j = node;
        currnode = node;
        if (node == end_node) {
            break;
        }
    }

    //LOG_D("\n\n");
    for (int k = 0; k < num_nodes; k++) {
        prev[edges[k].j] = k;
        //LOG_D("%d -> %d",  edges[k].i,  edges[k].j);
    }
}

int HEU_2opt(instance *inst) {

    struct timeval start, end;
    gettimeofday(&start, 0);
    int status = HEU_greedy(inst);
    if (status == TIME_LIMIT_EXCEEDED) {
        LOG_I("Constructive heuristics time exceeded");
        return TIME_LIMIT_EXCEEDED;
    }
    double minchange;
    int *prev = MALLOC(inst->num_nodes, int);
    MEMSET(prev, -1, inst->num_nodes, int);
    for (int i = 0; i < inst->num_nodes; i++) {
        prev[inst->solution.edges[i].j] = i;
    }
    plot_solution(inst);
    int mina = 0;
    int minb = 0;
    while(1) {
        gettimeofday(&end, 0);
        double elapsed = get_elapsed_time(start, end);
        if (inst->params.time_limit > 0 && elapsed > inst->params.time_limit) {
            status = TIME_LIMIT_EXCEEDED;
            LOG_I("2-opt heuristics time exceeded");
            break;
        }
        minchange = 0;
        for (int i = 0; i < inst->num_nodes - 1; i++) {
            for (int j = i+1; j < inst->num_nodes; j++) {
                int a = i;
                int b = j;
                int a1 = inst->solution.edges[a].j;
                int b1 = inst->solution.edges[b].j;
                double change = calc_dist(a, b, inst) + calc_dist(a1, b1, inst) - calc_dist(a, a1, inst) - calc_dist(b, b1, inst);
                if (change < minchange) {
                    minchange = change;
                    mina = i;
                    minb = j;
                }
            }
        }
        if (minchange >= 0) {
            break;
        }
        /*for (int k = 0; k < inst->num_nodes; k++) {
            LOG_D("%d -> %d",  inst->solution.edges[k].i,  inst->solution.edges[k].j);
        }
        LOG_D("\n=======\n");*/
        int a1 = inst->solution.edges[mina].j;
        int b1 = inst->solution.edges[minb].j; 
        int a1_succ = inst->solution.edges[a1].j;
        int b1_succ = inst->solution.edges[b1].j;
        inst->solution.edges[mina].j = minb;
        //plot_solution(inst);
        //sleep(1);
        inst->solution.edges[a1].j = b1;
        //plot_solution(inst);
        //sleep(1);
        
        reverse(minb, a1, prev, inst->solution.edges, inst->num_nodes);
        
        //plot_solution(inst);
        //sleep(1);
        
    }
    //plot_solution(inst);
    inst->solution.obj_best = 0.0;
    for (int i = 0; i < inst->num_nodes; i++) {
        edge e = inst->solution.edges[i];
        inst->solution.obj_best += calc_dist(e.i, e.j, inst);
    }
    FREE(prev);
    return status;
}

double reverse_3opt(int i, int j, int k, instance *inst) {
    int a = i;
    int b = j;
    int c = k;
    int a1 = inst->solution.edges[a].j;
    //if (a1 == b || a1 == c) continue;
    int b1 = inst->solution.edges[b].j;
    //if (b1 == a || b1 == c) continue;
    int c1 = inst->solution.edges[c].j;
    //if (c1 == a || c1 == b) continue;
    // file:///C:/Users/denis/Downloads/jcssp.2012.846.8521.pdf fig 2
    /*double d0 = calc_dist(a, a1, inst) + calc_dist(b, b1, inst) + calc_dist(c, c1, inst);
    double d1 = calc_dist(a, b1, inst) + calc_dist(a1, c1, inst) + calc_dist(b, c, inst);
    double d2 = calc_dist(a, c, inst) + calc_dist(a1, b1, inst) + calc_dist(b, c1, inst);
    double d3 = calc_dist(a, b, inst) + calc_dist(a1, c, inst) + calc_dist(b1, c1, inst);
    double d4 = calc_dist(a, b1, inst) + calc_dist(a1, c, inst) + calc_dist(b, c1, inst);*/

    double d0 = calc_dist(a, a1, inst) + calc_dist(b, b1, inst) + calc_dist(c, c1, inst);
    double d1 = calc_dist(a, b1, inst) + calc_dist(a1, c1, inst) + calc_dist(b, c, inst);
    double d2 = calc_dist(a, c, inst) + calc_dist(a1, b1, inst) + calc_dist(b, c1, inst);
    double d3 = calc_dist(a, b, inst) + calc_dist(a1, c, inst) + calc_dist(b1, c1, inst);
    double d4 = calc_dist(a, b1, inst) + calc_dist(a1, c, inst) + calc_dist(b, c1, inst);
    if (d0 > d1) {

        return d1 - d0;
    } else if (d0 > d2) {

        return d2 - d0;
    } else if (d0 > d3) {

        return d3 - d0;
    } else if (d0 > d4) {

        return d4 - d0;
    }
    return 0.0;
}

int HEU_3opt(instance *inst) {
    struct timeval start, end;
    gettimeofday(&start, 0);
    int status = HEU_greedy(inst);
    if (status == TIME_LIMIT_EXCEEDED) {
        LOG_I("Constructive heuristics time exceeded");
        return TIME_LIMIT_EXCEEDED;
    }
    double minchange;
    int *prev = MALLOC(inst->num_nodes, int);
    MEMSET(prev, -1, inst->num_nodes, int);
    for (int i = 0; i < inst->num_nodes; i++) {
        prev[inst->solution.edges[i].j] = i;
    }
    plot_solution(inst);
    while (1) {
        double delta = 0.0;
        for (int i = 0; i < inst->num_nodes - 2; i++) {
            for (int j = i+1; j < inst->num_nodes - 1; j++) {
                for (int k = j+1; k < inst->num_nodes; k++) {
                    
                }
            }
        }
        if (delta >= 0.0) {
            break;
        }
    }
    //plot_solution(inst);
    inst->solution.obj_best = 0.0;
    for (int i = 0; i < inst->num_nodes; i++) {
        edge e = inst->solution.edges[i];
        inst->solution.obj_best += calc_dist(e.i, e.j, inst);
    }
    FREE(prev);
    return status;
    return 0;
}

int shake(edge *edges, int k, int num_nodes) {

    for (int i = 0; i < k; i++) {
        double random = rand() / RAND_MAX;
        int node = (int) random * num_nodes;
    }
    

}

int HEU_Grasp(instance *inst) {
    int maxiter = inst->num_nodes * 0.5;
    int status = 0;
    double bestobj = DBL_MAX;
    edge *bestedges = CALLOC(inst->num_nodes, edge);
    struct timeval start, end;
    gettimeofday(&start, 0);
    for (int i = 0; i < maxiter; i++) {
        gettimeofday(&end, 0);
        double elapsed = get_elapsed_time(start, end);
        if (inst->params.time_limit >= 0 && elapsed >= inst->params.time_limit) {
            status = TIME_LIMIT_EXCEEDED;
            break;
        }
        double rand_num = ((double) rand() / (double) RAND_MAX);
        int node = (int) (rand_num * (inst->num_nodes - 1)); // The starting node taken randomly
        if (inst->params.verbose >= 5) {
            LOG_I("GRASP starting node: %d", node);
        }
        status = greedy(inst, node);
        if (status) { break; }
        if (inst->solution.obj_best < bestobj) {
            if(inst->params.verbose >= 4) {
                LOG_I("New Best: %f", inst->solution.obj_best);
            }
            bestobj = inst->solution.obj_best;
            memcpy(bestedges, inst->solution.edges, inst->num_nodes * sizeof(edge));
        }
    }
    inst->solution.obj_best = bestobj;
    memcpy(inst->solution.edges, bestedges, inst->num_nodes * sizeof(edge));
    FREE(bestedges);
    return status;
}

int HEU_Grasp2opt(instance *inst) {
    int status = HEU_Grasp(inst);
    if(inst->params.verbose >= 5) {
        LOG_I("COMPLETED GRASP");
        LOG_I("STARTED 2-OPT RREFINEMENT");
    }
    plot_solution(inst);
    status = HEU_2opt(inst);
    return status;
}

int HEU_VNS(instance *inst) {



    return 0;
}