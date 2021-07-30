#include "tabusearch.h"

#include "heuristics.h"
#include "distutil.h"
#include <unistd.h>
#include <float.h>

//Struct used to keep track of the policy
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

////////////////////////////////////////////////////////
///////////////// POLICIES /////////////////////////////
////////////////////////////////////////////////////////

//Step policy
static void step_policy(tenure_policy *policy, int curr_iter) {
    if (curr_iter % 100 == 0) { // The number of steps is an hyper parameter
        policy->current_tenure = policy->current_tenure == policy->min_tenure ? policy->max_tenure : policy->min_tenure;
    }
}

//Linear policy
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

//Random Policy: tenure change randomly
static void random_policy(tenure_policy *policy, int curr_iter) {
    if (curr_iter == 1 || curr_iter % 100 == 0) { // The number of steps is an hyper parameter
        policy->current_tenure = policy->min_tenure + (int) (URAND() * (policy->max_tenure - policy->min_tenure));
    } 
}

// Check whether a node is currently in tabu list or not. If the node has expired the tenure time,
// the node is removed from tabu list
int check_tenure(int* node_val, int iter, int tenure) {
    if (iter < 0 || tenure < 0) { return 0; }
    if (*node_val == 0) return 0;
    if (iter - *node_val > tenure) {
        *node_val = 0;
        return 0;
    }
    
    return 1;
}

//Pick 2 edges that are not crossing and it cross them (the reverse of 2-opt)
int worsening_move(instance *inst, int *skip_node, int iter, int *tabu_size, int tenure) {
    //Start counting time from now
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
                check_tenure(&(skip_node[b1]), iter, tenure))) {
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


int alg_2optv2(instance *inst, int *skip_node, int *stored_prev, const int iter, const int tenure) {
    struct timeval start, end;
    gettimeofday(&start, 0);
    double minchange;
    int status = 0;
    int *prev = MALLOC(inst->num_nodes, int);
    MEMSET(prev, -1, inst->num_nodes, int);
    for (int i = 0; i < inst->num_nodes; i++) {
        prev[inst->solution.edges[i].j] = i;
    }
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
            if (skip_node && skip_node[i] && check_tenure(&(skip_node[i]), iter, tenure)) { continue; } // Checking here for i can remove an useless cycle
            for (int j = i+1; j < inst->num_nodes; j++) {
                int a = i;
                int b = j;
                int a1 = inst->solution.edges[a].j;
                int b1 = inst->solution.edges[b].j;
                if (skip_node && 
                    (check_tenure(&(skip_node[a]), iter, tenure)  || 
                    check_tenure(&(skip_node[b]), iter, tenure)   || 
                    check_tenure(&(skip_node[a1]), iter, tenure)  || 
                    check_tenure(&(skip_node[b1]), iter, tenure))) {
                        continue;
                    }
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
        int a1 = inst->solution.edges[mina].j;
        int b1 = inst->solution.edges[minb].j; 
        inst->solution.edges[mina].j = minb;
        inst->solution.edges[a1].j = b1;
        reverse_path(inst, minb, a1, prev);
        
    }
    inst->solution.obj_best = 0.0;
    for (int i = 0; i < inst->num_nodes; i++) {
        edge e = inst->solution.edges[i];
        inst->solution.obj_best += calc_dist(e.i, e.j, inst);
    }
    if(stored_prev) {
        memcpy(stored_prev, prev, sizeof(int) * inst->num_nodes);
    }
    FREE(prev);
    return status;
}

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
    tenure_policy.min_tenure = ceil(inst->num_nodes * 0.02);// Ceil in order to have 1 for small instances. //15; // Hyper parameter
    tenure_policy.max_tenure = round(inst->num_nodes * 0.1);//inst->num_nodes / 10; // Hyper parameter
    if (tenure_policy.min_tenure == tenure_policy.max_tenure) {
        tenure_policy.max_tenure += 2;
    } else if (tenure_policy.max_tenure < tenure_policy.min_tenure) {
        int tmp = tenure_policy.min_tenure;
        tenure_policy.min_tenure = tenure_policy.max_tenure;
        tenure_policy.max_tenure = tmp;
    }


    tenure_policy.current_tenure = tenure_policy.min_tenure;
    tenure_policy.incr_tenure = 0;

    int iter = 1;
    while (1) {
        gettimeofday(&end, 0);
        double elapsed = get_elapsed_time(start, end);
        if (inst->params.time_limit > 0 && elapsed > inst->params.time_limit) {
            status = TIME_LIMIT_EXCEEDED;
            LOG_I("Tabu Search time exceeded");
            break;
        }
        status = alg_2optv2(inst, tabu_node, prev, iter, tenure_policy.current_tenure);

        if (inst->solution.obj_best < best_obj) {
            best_obj = inst->solution.obj_best;
            memcpy(best_sol, inst->solution.edges, inst->num_nodes * sizeof(edge));
            if (inst->params.verbose >= 3) {
                LOG_I("Updated incumbent: %f", best_obj);
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
            a = rand_choice(0, inst->num_nodes);
            b = rand_choice(0, inst->num_nodes);
            a1 = inst->solution.edges[a].j;
            b1 = inst->solution.edges[b].j;
            if (tabu_node[a] || tabu_node[b] || tabu_node[a1] || tabu_node[b1]) {
                a = 0; b = 0; a1 = 0; b1 = 0;
            }
        }
        inst->solution.edges[a].j = b;
        inst->solution.edges[a1].j = b1;
        reverse_path(inst, b, a1, prev);

        (*policy_ptr)(&tenure_policy, iter);

        if (inst->params.verbose >= 5) {
            LOG_I("Current tenure %d", tenure_policy.current_tenure);
        }
        //printf("\n\n\n\n");
        //plot_solution(inst);
        //sleep(1);
        
        int node_fixed = random_choice(a, b, a1, b1);
        tabu_node[node_fixed] = iter;
        
        iter++;
    }

    inst->solution.obj_best = best_obj;
    memcpy(inst->solution.edges, best_sol, inst->num_nodes * sizeof(edge));
    status = alg_2optv2(inst, NULL, NULL, -1, -1); // A very fast 2-opt which removes the remaining crossing edges (generally few crossing edges)
    FREE(tabu_node);
    FREE(prev);
    return status;
}

//Wrapper for tabu step
int HEU_Tabu_step(instance *inst) {
    return tabu(inst, step_policy);
}

//Wrapper for tabu lin
int HEU_Tabu_lin(instance *inst) {
    return tabu(inst, linear_policy);
}

//Wrapper for tabu rand
int HEU_Tabu_rand(instance *inst) {
    return tabu(inst, random_policy);
}

