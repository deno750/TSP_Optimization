#include "tabusearch.h"

#include "heuristics.h"
#include <unistd.h>
#include <float.h>

void reverse2(int start_node, int end_node, int *prev, edge *edges, int num_nodes) {
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

int step_tenure_policy(int min_tenure, int max_tenure, int curr_tenure, int current_iter) {
    if (current_iter % 1000 == 0) {
        if (curr_tenure == min_tenure) return max_tenure;
        if (curr_tenure == max_tenure) return min_tenure;
    }
    return curr_tenure;
}

int HEU_Tabu(instance *inst) {
    int status = 0;
    struct timeval start, end;
    gettimeofday(&start, 0);

    int *tabu_node = CALLOC(inst->num_nodes, int);
    int *prev = CALLOC(inst->num_nodes, int);
    //MEMSET(tabu_node, -1, inst->num_nodes, int);

    int grasp_time_lim = inst->params.time_limit / 5;
    HEU_Greedy_iter(inst); // Use grasp when completed

    plot_solution(inst);

    double best_obj = DBL_MAX;
    edge *best_sol = CALLOC(inst->num_nodes, edge);
    int min_tenure = 2;
    int max_tenure = inst->num_nodes / 10;
    int tenure = min_tenure;

    int incr_tenure = 0;

    int iter = 1; // Starting from 1 in order to have 0 on nodes which are not in tabu list
    while (1) {
        gettimeofday(&end, 0);
        double elapsed = get_elapsed_time(start, end);
        if (inst->params.time_limit > 0 && elapsed > inst->params.time_limit) {
            status = TIME_LIMIT_EXCEEDED;
            LOG_I("Tabu Search time exceeded");
            break;
        }
        status = alg_2opt(inst, tabu_node, prev);
        if (inst->solution.obj_best < best_obj) {
            best_obj = inst->solution.obj_best;
            memcpy(best_sol, inst->solution.edges, inst->num_nodes * sizeof(edge));
            LOG_I("Updated incubement: %f", best_obj);
            plot_solution(inst);
        }
        //plot_solution(inst);
        if (status) {
            LOG_I("2-opt move returned status %d", status);
            break;
        }
        // We're in local minimum

        int a = 0, b = 0, a1 = 0, b1 = 0; 
        // Seeking the pair edges to change
        while (a == b || a1 == b1 || a == a1 || a == b1 || b == a1 || b == b1) {
            a = (int) (URAND() * (inst->num_nodes - 1));
            b = (int) (URAND() * (inst->num_nodes - 1));
            a1 = inst->solution.edges[a].j;
            b1 = inst->solution.edges[b].j;
        }
        inst->solution.edges[a].j = b;
        inst->solution.edges[a1].j = b1;
        reverse2(b, a1, prev, inst->solution.edges, inst->num_nodes);
        //plot_solution(inst);
        /*for (int i = 0; i < inst->num_nodes; i++) {
            LOG_D("%d --- %d", inst->solution.edges[i].i, inst->solution.edges[i].j);
        }
        LOG_D("\n\n\n");*/

        // ============== STEP Policy ===================
        /*if (iter % 1000 == 0) {
            tenure = tenure == min_tenure ? max_tenure : min_tenure;
        } */
        if (tenure == max_tenure || tenure == min_tenure) {
            incr_tenure = !incr_tenure;
        }

        if (incr_tenure) {
            tenure++;
        } else {
            tenure--;
        }

        if (inst->params.verbose >= 4) {
            LOG_I("Current tenure %d", tenure);
        }
        for (int i = 0; i < inst->num_nodes; i++) {
            if (iter - tabu_node[i] > tenure) {
                tabu_node[i] = 0;
            }
        }
        double random = URAND();
        int node_fixed;
        if (random < 0.25) {
            node_fixed = a;
        } else if (random >= 0.25 && random < 0.5) {
            node_fixed = b;
        } else if (random >= 0.5 && random < 0.75) {
            node_fixed = a1;
        } else {
            node_fixed = b1;
        }
        tabu_node[node_fixed] = iter;
        
        //LOG_D("ITER %d", iter);
        iter++;
        //sleep(1);
    }

    FREE(tabu_node);
    return status;
}

