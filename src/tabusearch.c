#include "tabusearch.h"

#include "heuristics.h"
#include <unistd.h>
#include <float.h>

typedef struct tenure_policy {
    int min_tenure;
    int max_tenure;
    int current_tenure;
    int incr_tenure; // Variable for checking whether the tenure should increase or decrease in linear policy
} tenure_policy;

// Chooses between one of the four paramethers randomly
static int random_choice(int a, int b, int a1, int b1) {
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
    return node_fixed;
}

static void step_policy(tenure_policy *policy, int curr_iter) {
    if (curr_iter % 100 == 0) { // The number of steps is an hyper parameter
        policy->current_tenure = policy->current_tenure == policy->min_tenure ? policy->max_tenure : policy->min_tenure;
    }
}

static void linear_policy(tenure_policy *policy, int curr_iter) {
    if (policy->current_tenure > policy->max_tenure) {
        policy->current_tenure = policy->max_tenure;
    }
    if (policy->current_tenure < policy->min_tenure) {
        policy->current_tenure = policy->min_tenure;
    }
    if (policy->current_tenure == policy->max_tenure || policy->current_tenure == policy->min_tenure) {
        policy->incr_tenure = !policy->incr_tenure;
    }

    if (policy->incr_tenure) { policy->current_tenure++; } else { policy->current_tenure--; }
}

static void random_policy(tenure_policy *policy, int curr_iter) {
    if (curr_iter == 1 || curr_iter % 100 == 0) { // The number of steps is an hyper parameter
        policy->current_tenure = policy->min_tenure + (int) (URAND() * (policy->max_tenure - policy->min_tenure));
    } 
}




// The policy parameter 
static int tabu(instance *inst, void (*policy_ptr)(tenure_policy*, int)) {
    int status = 0;
    struct timeval start, end;
    gettimeofday(&start, 0);

    int *tabu_node = CALLOC(inst->num_nodes, int);
    int *prev = CALLOC(inst->num_nodes, int);

    int grasp_time_lim = inst->params.time_limit / 5;
    HEU_Greedy_iter(inst);//HEU_Grasp_iter(inst, grasp_time_lim);
    if (inst->params.verbose >= 5) {
        LOG_I("Completed initialization");
    }

    plot_solution(inst);

    double best_obj = DBL_MAX;
    edge *best_sol = CALLOC(inst->num_nodes, edge);
    tenure_policy tenure_policy;
    tenure_policy.min_tenure = 15; // Hyper parameter
    tenure_policy.max_tenure = inst->num_nodes / 10; // Hyper parameter

    // The following code is useful only with small instances (i.e. less than 200 nodes). Implemented just to avoid infinite loops for those instances or any other error
    /*if (min_tenure > max_tenure) {
        min_tenure = max_tenure / 2;
    }
    if (min_tenure <= 1) {
        min_tenure = 2;
    }*/


    tenure_policy.current_tenure = tenure_policy.min_tenure;
    tenure_policy.incr_tenure = 0;

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
        //LOG_D("Objective: %0.0f Best: %0.0f", inst->solution.obj_best, best_obj);
        if (inst->solution.obj_best < best_obj) {
            best_obj = inst->solution.obj_best;
            memcpy(best_sol, inst->solution.edges, inst->num_nodes * sizeof(edge));
            if (inst->params.verbose >= 3) {
                LOG_I("Updated incubement: %f", best_obj);
            }
            
            plot_solution(inst);
        }
        
        if (status) {
            LOG_I("2-opt move returned status %d", status);
            break;
        }
        // We're in local minimum now. We have to swap two edges and add a node in the tabu list

        int a = 0, b = 0, a1 = 0, b1 = 0; 
        // Seeking the pair edges to change. We don't want to choose two adiacent edges to swap
        while (a == b || a1 == b1 || a == a1 || a == b1 || b == a1 || b == b1) {
            a = (int) (URAND() * (inst->num_nodes - 1));
            b = (int) (URAND() * (inst->num_nodes - 1));
            a1 = inst->solution.edges[a].j;
            b1 = inst->solution.edges[b].j;
            if (tabu_node[a] || tabu_node[b] || tabu_node[a1] || tabu_node[b1]) {
                a = 0; b = 0; a1 = 0; b1 = 0;
                //LOG_D("FOUND TABU");
            }
        }
        if (tabu_node[a] || tabu_node[b] || tabu_node[a1] || tabu_node[b1]) {
            
            LOG_D("FOUND TABU");
        }
        inst->solution.edges[a].j = b;
        inst->solution.edges[a1].j = b1;
        reverse_path(inst, b, a1, prev);

        (*policy_ptr)(&tenure_policy, iter);

        if (inst->params.verbose >= 5) {
            LOG_I("Current tenure %d", tenure_policy.current_tenure);
        }
        for (int i = 0; i < inst->num_nodes; i++) {
            /*if (tabu_node[i]) {
                printf("%d: %d, ", i, iter - tabu_node[i]);
            }*/
            if (iter - tabu_node[i] > tenure_policy.current_tenure) {
                tabu_node[i] = 0;
            }
        }
        //printf("\n\n\n\n");
        //plot_solution(inst);
        //sleep(1);
        
        int node_fixed = random_choice(a, b, a1, b1);
        tabu_node[node_fixed] = iter;
        
        iter++;

        if (iter % 10 == 0) {
            //alg_2opt(inst, NULL, NULL);
        }
    }

    inst->solution.obj_best = best_obj;
    memcpy(inst->solution.edges, best_sol, inst->num_nodes * sizeof(edge));
    status = alg_2opt(inst, NULL, NULL); // A very fast 2-opt which removes the remaining crossing edges (generally few crossing edges)
    FREE(tabu_node);
    FREE(prev);
    return status;
}

int HEU_Tabu_step(instance *inst) {
    return tabu(inst, step_policy);
}

int HEU_Tabu_lin(instance *inst) {
    return tabu(inst, linear_policy);
}

int HEU_Tabu_rand(instance *inst) {
    return tabu(inst, random_policy);
}

