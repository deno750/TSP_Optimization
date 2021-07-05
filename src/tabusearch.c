#include "tabusearch.h"

#include "heuristics.h"
#include "distutil.h"
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

int check_tenure(int* node_val, int iter, int tenure) {
    if (*node_val == 0) return 0;
    if (iter - *node_val > tenure) {
        *node_val = 0;
        return 0;
    }
    
    return 1;
}

int worsening_move(instance *inst, int *skip_node, int iter, int *tabu_size, int tenure) {
    struct timeval start, end;
    gettimeofday(&start, 0);
    
    int status = 0;
    int *prev = MALLOC(inst->num_nodes, int);
    MEMSET(prev, -1, inst->num_nodes, int);
    for (int i = 0; i < inst->num_nodes; i++) {
        prev[inst->solution.edges[i].j] = i;
    }
    int mina = 0;
    int minb = 0;
    double maxDelta = 0;
    for (int i = 0; i < inst->num_nodes - 1; i++) {
        if (skip_node && skip_node[i]) { 
            if (check_tenure(&(skip_node[i]), iter, tenure)) {
                continue;
            }
        } // Checking here for i can remove an useless cycle
        for (int j = i+1; j < inst->num_nodes; j++) {
            int a = i;
            int b = j;
            int a1 = inst->solution.edges[a].j;
            int b1 = inst->solution.edges[b].j;
            // a1 == b1 never occurs because the edges are repsresented as directed. a->a1 then a1->b so it cannot be a->a1 b->a1
            if (a1 == b1 || a == b1 || b == a1) {continue;}
            
            if (skip_node && 
                (check_tenure(&(skip_node[a]), iter, tenure)  || 
                check_tenure(&(skip_node[b]), iter, tenure)   || 
                check_tenure(&(skip_node[a1]), iter, tenure)  || 
                check_tenure(&(skip_node[b1]), iter, tenure))
                ) {
                continue;
            }
            double delta = calc_dist(a, b, inst) + calc_dist(a1, b1, inst) - calc_dist(a, a1, inst) - calc_dist(b, b1, inst);
            if (delta > maxDelta) {
                maxDelta = delta;
                mina = i;
                minb = j;
            }
        }
    }
    //inst->solution.obj_best += maxDelta;
    int a1 = inst->solution.edges[mina].j;
    int b1 = inst->solution.edges[minb].j; 
    inst->solution.edges[mina].j = minb;
    inst->solution.edges[a1].j = b1;
    
        
    reverse_path(inst, minb, a1, prev);
    int node_fixed = random_choice(mina, minb, a1, b1);
    skip_node[node_fixed] = iter;
    (*tabu_size)++;
    
    
    inst->solution.obj_best = 0.0;
    for (int i = 0; i < inst->num_nodes; i++) {
        edge e = inst->solution.edges[i];
        inst->solution.obj_best += calc_dist(e.i, e.j, inst);
    }
    FREE(prev);
    return status;
}




// The policy parameter 
static int tabu(instance *inst, void (*policy_ptr)(tenure_policy*, int)) {
    int status = 0;
    struct timeval start, end;
    gettimeofday(&start, 0);

    int *tabu_node = CALLOC(inst->num_nodes, int);
    int *prev = CALLOC(inst->num_nodes, int);
    int tabu_size = 0;

    int grasp_time_lim = inst->params.time_limit / 5;
    HEU_Greedy_iter(inst);//HEU_Grasp_iter(inst, grasp_time_lim);
    alg_2opt(inst);
    if (inst->params.verbose >= 5) {
        LOG_I("Completed initialization");
    }

    plot_solution(inst);

    //double best_obj = DBL_MAX;
    if (inst->params.time_limit <= 0 && inst->params.verbose >= 3) {
        LOG_I("Default time lim %d setted.", DEFAULT_TIME_LIM);
    }
    int time_limit = inst->params.time_limit > 0 ? inst->params.time_limit : DEFAULT_TIME_LIM;
    double best_obj = inst->solution.obj_best;
    edge *best_sol = CALLOC(inst->num_nodes, edge);
    memcpy(best_sol, inst->solution.edges, inst->num_nodes * sizeof(edge));
    tenure_policy tenure_policy;
    tenure_policy.min_tenure = inst->num_nodes / 50; // Hyper parameter
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
        if (elapsed > time_limit) {
            status = TIME_LIMIT_EXCEEDED;
            LOG_I("Tabu Search time exceeded");
            break;
        }
        // This cycle can be avoided. In the 2opt move when is checking if the node is in tabu list can check whether the iteration is greater than current tenure and update it
        status = worsening_move(inst, tabu_node, iter, &tabu_size, tenure_policy.current_tenure);
        
        if (status) {
            LOG_I("2-opt move returned status %d", status);
            break;
        }
        if (tabu_size % tenure_policy.current_tenure == 0) {
            tabu_size = 0;
            MEMSET(tabu_node, 0, inst->num_nodes, int);
            status = alg_2opt(inst);
        }
        //LOG_D("Objective: %0.0f Best: %0.0f", inst->solution.obj_best, best_obj);
        if (inst->solution.obj_best < best_obj) {
            best_obj = inst->solution.obj_best;
            memcpy(best_sol, inst->solution.edges, inst->num_nodes * sizeof(edge));
            if (inst->params.verbose >= 3) {
                LOG_I("Updated incubement: %0.0f", best_obj);
                LOG_D("Current tenure %d", tenure_policy.current_tenure);
            }
            
            plot_solution(inst);
        }

        (*policy_ptr)(&tenure_policy, iter);

        if (inst->params.verbose >= 5) {
            LOG_I("Current tenure %d", tenure_policy.current_tenure);
        }
        
        iter++;
    }

    inst->solution.obj_best = best_obj;
    memcpy(inst->solution.edges, best_sol, inst->num_nodes * sizeof(edge));
    status = alg_2opt(inst); // A very fast 2-opt which removes the remaining crossing edges (generally few crossing edges)
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

