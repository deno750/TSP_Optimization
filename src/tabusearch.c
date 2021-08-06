#include "tabusearch.h"

#include "heuristics.h"
#include "distutil.h"
#include <unistd.h>
#include <float.h>

////////////////////////////////////////////////////////
///////////////// HYPERPARAMETERS //////////////////////
////////////////////////////////////////////////////////
#define NUM_ITER 100 // It' the number of iterations where the tenure changes (step and rand policy)
#define MIN_TENURE_RATE 0.02 // The size of min tenure in percentage with the number of nodes. If the problem has 100 nodes and the rate is 0.02, min tenure will have a value od 2
#define MAX_TENURE_RATE 0.1 // The size of max tenure in percentage with the number of nodes. If the problem has 100 nodes and the rate is 0.1, max tenure will have a value od 10

// Struct used to keep track of the policy
typedef struct {
    int min_tenure;
    int max_tenure;
    int current_tenure;
    int incr_tenure; // Variable for checking whether the tenure should increase or decrease in linear policy
} tenure_policy;

////////////////////////////////////////////////////////
///////////////// POLICIES /////////////////////////////
////////////////////////////////////////////////////////

/**
 * Step policy callback: Tenure changes every 100 iterations
 * 
 * @param policy A pointer of tenure_policy
 * @param curr_iter The current algorithm's iteration
 */
static void step_policy(tenure_policy *policy, int curr_iter) {
    if (curr_iter % NUM_ITER == 0) {
        policy->current_tenure = policy->current_tenure == policy->min_tenure ? policy->max_tenure : policy->min_tenure;
    }
}

/**
 * Linear policy callback: Tenure changes in every iteration from min_tenure to max_tenure and viceversa 
 * in a zig zag way.
 * 
 * @param policy A pointer of tenure_policy
 * @param curr_iter The current algorithm's iteration
 * 
 */
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

/**
 * Random Policy callback: Tenure changes randomly every 100 iterations
 * 
 * @param policy A pointer of tenure_policy
 * @param curr_iter The current algorithm's iteration
 * 
 */
static void random_policy(tenure_policy *policy, int curr_iter) {
    if (curr_iter == 1 || curr_iter % NUM_ITER == 0) {
        policy->current_tenure = rand_choice(policy->min_tenure, policy->max_tenure + 1); // + 1 because rand_choice doesn't include the right bound
    } 
}

/** Check whether an edge is currently in tabu list or not. If the node has expired the tenure time,
 *  the node is removed from tabu list.
 * 
 * @param edge_val A pointer of an edge's tabu value
 * @param iter The algorithm's current iteration
 * @param tenure The current tenure 
 * 
 * @returns true if the current edge is inside tabu list and should be skipped, 0 otherwise
 */
int check_tenure(int* edge_val, const int iter, const int tenure) {
    if (iter < 0 || tenure < 0) { return 0; }
    if (*edge_val == 0) return 0;
    if (iter - *edge_val > tenure) {
        *edge_val = 0;
        return 0;
    }
    
    return 1;
}

/**
 * Applies 2-opt specifically designed for tabu search. It implements a skip procedure when it encounters
 * an edge in the tabu list.
 * 
 * @param inst The instance pointer of the problem
 * @param skip_edge The tabu list
 * @param stored_prev An array of the previous node for each node. It is useful for tabu when needs to reverse the path
 * of two swapped nodes outside of this function. This function indeed fullfils the stored_prev array
 * @param iter The algorithm's current iteration
 * @param tenure The current tenure
 * 
 * @returns The status code 0 when no errors occur
 */ 
int alg_2opt_tabu(instance *inst, int *skip_edge, int *stored_prev, const int iter, const int tenure) {
    struct timeval start, end;
    gettimeofday(&start, 0);
    double mindelta;
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
        mindelta = 0;
        for (int i = 0; i < inst->num_nodes - 1; i++) {
            for (int j = i+1; j < inst->num_nodes; j++) {
                int a = i;
                int b = j;
                int a1 = inst->solution.edges[a].j;
                int b1 = inst->solution.edges[b].j;
                if (b == a1 || b1 == a) {
                    continue;
                }
                int edge_idx1 = x_udir_pos(a, b, inst->num_nodes);
                int edge_idx2 = x_udir_pos(a, a1, inst->num_nodes);
                int edge_idx3 = x_udir_pos(b, b1, inst->num_nodes);
                int edge_idx4 = x_udir_pos(a, b1, inst->num_nodes);

                if (skip_edge && 
                    (check_tenure(&(skip_edge[edge_idx1]), iter, tenure)  || 
                    check_tenure(&(skip_edge[edge_idx2]), iter, tenure)   || 
                    check_tenure(&(skip_edge[edge_idx3]), iter, tenure)   || 
                    check_tenure(&(skip_edge[edge_idx4]), iter, tenure)   
                    )) {
                        continue;
                    }
                double delta = calc_dist(a, b, inst) + calc_dist(a1, b1, inst) - calc_dist(a, a1, inst) - calc_dist(b, b1, inst);
                if (delta < mindelta) {
                    mindelta = delta;
                    mina = i;
                    minb = j;
                }
            }
        }
        if (mindelta >= 0) {
            break;
        }
        int mina1 = inst->solution.edges[mina].j;
        int minb1 = inst->solution.edges[minb].j; 
        inst->solution.edges[mina].j = minb;
        inst->solution.edges[mina1].j = minb1;
        reverse_path(inst, minb, mina1, prev);
        
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

/**
 * The effective implementation of tabu search. It implements a tabu list for edges rather than nodes.
 * 
 * @param inst The instance pointer of the problem
 * @param policy_ptr The callback function pointer used for changing the algorithm's current tenure.
 * 
 * @returns The status code 0 when no errors occur.
 */
static int tabu(instance *inst, void (*policy_ptr)(tenure_policy*, int)) {
    int status = 0;

    //Start counting time from now
    struct timeval start, end;
    gettimeofday(&start, 0);

    int *tabu_edge = CALLOC(inst->num_columns, int);
    int *prev = CALLOC(inst->num_nodes, int);

    //Compute initial solution
    //int grasp_time_lim = inst->params.time_limit / 5;
    HEU_Greedy_iter(inst);
    if (inst->params.verbose >= 5) {
        LOG_I("Completed initialization");
    }

    plot_solution(inst);

    double best_obj = DBL_MAX;
    edge *best_sol = CALLOC(inst->num_nodes, edge);
    tenure_policy tenure_policy;
    tenure_policy.min_tenure = ceil(inst->num_nodes * MIN_TENURE_RATE);// Ceil in order to have 1 for small instances. // Hyper parameter
    tenure_policy.max_tenure = round(inst->num_nodes * MAX_TENURE_RATE); // Hyper parameter
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
        //Check elapsed time
        gettimeofday(&end, 0);
        double elapsed = get_elapsed_time(start, end);
        if (inst->params.time_limit > 0 && elapsed > inst->params.time_limit) {
            status = TIME_LIMIT_EXCEEDED;
            if (inst->params.verbose >= 3) {LOG_I("Tabu Search time exceeded");}
            break;
        }

        //Optimize
        status = alg_2opt_tabu(inst, tabu_edge, prev, iter, tenure_policy.current_tenure);

        //Update the best solution
        if (inst->solution.obj_best < best_obj) {
            best_obj = inst->solution.obj_best;
            memcpy(best_sol, inst->solution.edges, inst->num_nodes * sizeof(edge));
            if (inst->params.verbose >= 3) {
                LOG_I("Updated incumbent: %f", best_obj);
            }
            if (!inst->params.perf_prof) { plot_solution(inst); }
            
        }
        if (inst->params.verbose >= 4) {
            LOG_I("Current sol: %0.0f     Incumbent: %0.0f", inst->solution.obj_best, best_obj);
        }   
        
        if (status) {
            LOG_I("2-opt move returned status %d", status);
            break;
        }
        // We're in local minimum now. We have to swap two edges and add these two in the tabu list

        // Seeking the pair edges to change. We don't want to choose two adiacent edges to swap
        int a, b, a1, b1;
        while (1) {
            a = rand_choice(0, inst->num_nodes);
            b = rand_choice(0, inst->num_nodes);

            a1 = inst->solution.edges[a].j;
            b1 = inst->solution.edges[b].j;

            // Don't want the same node for a and b and don't want contiguous edges
            if (a == b || a1 == b || b1 == a) {
                continue;
            }

            // Checking whether the edges are in the tabu list. If they're in the tabu list, those edges should not be touched
            int edge_idx1 = x_udir_pos(a, a1, inst->num_nodes);
            int edge_idx2 = x_udir_pos(b, b1, inst->num_nodes);
            int edge_idx3 = x_udir_pos(a, b, inst->num_nodes);
            int edge_idx4 = x_udir_pos(a1, b1, inst->num_nodes);
            /*if (tabu_edge[edge_idx3] || tabu_edge[edge_idx4]) {
                printf("IN TABU a-b or a1-b1\n");
            }
            if (tabu_edge[edge_idx1] || tabu_edge[edge_idx2]) {
                printf("IN TABU a-a1 or b-b1\n");
            }*/
            // If the edges are not in tabu list, we can exit from this loop and swap these edges
            if (!check_tenure(&(tabu_edge[edge_idx1]), iter, tenure_policy.current_tenure) && 
                !check_tenure(&(tabu_edge[edge_idx2]), iter, tenure_policy.current_tenure) &&
                !check_tenure(&(tabu_edge[edge_idx3]), iter, tenure_policy.current_tenure) && 
                !check_tenure(&(tabu_edge[edge_idx4]), iter, tenure_policy.current_tenure)) {
                break;
            }
        }
        inst->solution.edges[a].j = b;
        inst->solution.edges[a1].j = b1;
        reverse_path(inst, b, a1, prev);

        (*policy_ptr)(&tenure_policy, iter);

        if (inst->params.verbose >= 5) {
            LOG_I("Current tenure %d", tenure_policy.current_tenure);
        }
        //plot_solution(inst);
        //sleep(1);
        
        //Put the 2 edges in the tabu list
        int edge_idx1 = x_udir_pos(a, a1, inst->num_nodes);
        int edge_idx2 = x_udir_pos(b, b1, inst->num_nodes);
        tabu_edge[edge_idx1] = iter;
        tabu_edge[edge_idx2] = iter;
        
        iter++;
    }

    inst->solution.obj_best = best_obj;
    memcpy(inst->solution.edges, best_sol, inst->num_nodes * sizeof(edge));
    FREE(tabu_edge);
    FREE(prev);
    FREE(best_sol);
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

