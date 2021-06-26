#include "genetic.h"

#include "heuristics.h"
#include "distutil.h"

#include <float.h>

typedef struct individual{
    int* cromosome; // List of visiting nodes. i.e. the order of nodes which are visited in the tsp
    double fitness;
} individual;


int rand_choice(int from, int to) {
    return from + ((int) (URAND() * (to - from)));
}

void fitness(instance* inst, individual* individual) {
    int prev_node = individual->cromosome[0];
    individual->fitness = 0;
    for (int i = 1; i < inst->num_nodes; i++) {
        int node = individual->cromosome[i];
        individual->fitness += calc_dist(prev_node, node, inst);
        prev_node = node;
    }
    individual->fitness += calc_dist(prev_node, individual->cromosome[0], inst);
}

void select_parents(int* parents, int parent_size, int pop_size) {
    MEMSET(parents, -1, parent_size, int);
    int count = 0;
    // Use a better policy to choose the parents. Only random is not good
    while (count < parent_size) {
        int rand_sel = rand_choice(0, pop_size - 1);
        int is_added = 0;
        for (int i = 0; i < count; i++) {
            if (rand_sel == parents[i]) {
                is_added = 1;
                break;
            }
        }
        if (is_added) { continue; }
        parents[count++] = rand_sel;
    }
}

void crossover(instance* inst, individual *population, int parent1, int parent2, int* cromosome) {
    individual p1 = population[parent1];
    individual p2 = population[parent2];
    int rand_index = rand_choice(1, inst->num_nodes - 2);
    int *visited = CALLOC(inst->num_nodes, int);
    int idx = 0;
    for (int i = 0; i < inst->num_nodes; i++) {
        
        if (i <= rand_index) {
            int node = p1.cromosome[i];
            visited[node] = 1;
            cromosome[idx] = node;
        } else {
            int node = p2.cromosome[i];
            if (visited[node]) { continue; }
            cromosome[idx] = node;
        }
        idx++;
    }

    if (idx < inst->num_nodes) {
        for (int i = 0; i <= rand_index; i++) {
            int node = p2.cromosome[i];
            if (visited[node]) { continue; }
            cromosome[idx++] = node;
        }
    }
    //LOG_D("OFFSPRING: %0.0f", offspring.fitness);
    
    FREE(visited);
}

void procreate(instance* inst, individual *population, int* parents, int parent_size, individual* offsprings) {
    
    int* cromosome = CALLOC(inst->num_nodes, int);
    int counter = 0;
    for (int i = 0; i < parent_size - 1; i++) {
        for (int j = i + 1; j < parent_size; j++) {
            int parent1 = parents[i];
            int parent2 = parents[j];
            
            crossover(inst, population, parent1, parent2, cromosome);
            memcpy(offsprings[counter].cromosome, cromosome, sizeof(int) * inst->num_nodes);
            fitness(inst, &(offsprings[counter]));
            
            //#ifdef DEBUG
            //printf("OFFSPRING: ");
            //for (int i = 0; i < inst->num_nodes; i++) {
            //    printf("%d ", offspring.cromosome[i]);
            //}
            //printf("      %0.0f", offspring.fitness);
            //printf("\n");
            //#endif
            counter++;
        }
        
    }
    FREE(cromosome);
}

void selection(instance* inst, individual* population, int pop_size, individual* offsprings, int off_size) {
    
    int N = pop_size + off_size;
    individual* total = CALLOC(N, individual);
    int *visited = CALLOC(N, int);
    int count = 0;
    for (int i = 0; i < pop_size; i++) {
        total[count++] = population[i];
    }
    for (int i = 0; i < off_size; i++) {
        total[count++] = offsprings[i];
    }
    
    count = 0;
    while (count < pop_size) {
        
        double min_fitness = DBL_MAX;
        int min_idx = -1;

        for (int i = 0; i < N; i++) {
            if (!visited[i] && total[i].fitness < min_fitness) {
                min_fitness = total[i].fitness;
                min_idx = i;
            }
        }
        // Diversification phase
        double rand_diversification = URAND();
        if (rand_diversification < 0.1) {
            int rand_index = rand_choice(0, N - 1);
            while (visited[rand_index] || rand_index == min_idx) {
                rand_index = rand_choice(0, N - 1);
            }
            min_idx = rand_index;
        }
        visited[min_idx] = 1;
        memcpy(population[count].cromosome, total[min_idx].cromosome, sizeof(int) * inst->num_nodes);
        population[count].fitness = total[min_idx].fitness;
        double rand_mut = URAND();

        // Mutation phase
        if (rand_mut < 0.05) {
            int rand_index1 = rand_choice(0, inst->num_nodes - 1);
            int rand_index2 = rand_choice(0, inst->num_nodes - 1);
            if (rand_index1 == rand_index2) {
                // Dont' assign manually. Just for test
                if (rand_index1 < inst->num_nodes - 1) {
                    rand_index2 = rand_index1 + 1;
                } else {
                    rand_index2 = rand_index1 - 1;
                }
                
            }
            //printf("\nBEFORE\n");
            //for (int i = 0; i < inst->num_nodes; i++) {
            //    printf("%d ", population[count].cromosome[i]);
            //}
            //printf("     %0.0f", population[count].fitness);
            int temp = population[count].cromosome[rand_index1];
            population[count].cromosome[rand_index1] = population[count].cromosome[rand_index2];
            population[count].cromosome[rand_index2] = temp;
            fitness(inst, &(population[count]));
            //printf("\nAFTER\n");
            //for (int i = 0; i < inst->num_nodes; i++) {
            //    printf("%d ", population[count].cromosome[i]);
            //}
            //printf("     %0.0f", population[count].fitness);
            //printf("\n\n\n");
            
        }
        count++;
    }
    FREE(visited);
    FREE(total);
}

void fitness_metrics(individual* population, int pop_size, double* best, double* mean, int *best_idx) {
    *best = DBL_MAX;
    *mean = 0;
    for (int i = 0; i < pop_size; i++) {
        double fitness = population[i].fitness;
        *mean += fitness;
        if (fitness < *best) {
            *best = fitness;
            *best_idx = i;
        }
    }

    *mean /= pop_size;
}

void random_generation(int* cromosome, int num_nodes) {
    for (int i = 0; i < num_nodes; i++) {  
        cromosome[i] = i;
    }

    for (int i = 0; i < num_nodes; i++) {  
        int idx1 = rand_choice(0, num_nodes);
        int idx2 = rand_choice(0, num_nodes);

        int tmp = cromosome[idx1];
        cromosome[idx1] = cromosome[idx2];
        cromosome[idx2] = tmp;
    }
}

int HEU_Genetic(instance *inst) {
    int status = 0;
    struct timeval start, end;
    gettimeofday(&start, 0);
    int pop_size = 1000; // Population size

    individual *population = CALLOC(pop_size, individual);

    // Starting population initialization
    for (int i = 0; i < pop_size; i++) {
        population[i].cromosome = CALLOC(inst->num_nodes, int);

        // Using greedy for initialization. Might be a better way
        /*int start_node = rand_choice(0, inst->num_nodes - 1);
        greedy(inst, start_node);
        
        int node_idx = start_node;
        int node_iter = 0;
        while (node_iter < inst->num_nodes) {
            population[i].cromosome[node_iter++] = inst->solution.edges[node_idx].i;
            node_idx = inst->solution.edges[node_idx].j;
        }*/
        random_generation(population[i].cromosome, inst->num_nodes);
        fitness(inst, &(population[i]));

    }

    // End of population initialization

    // A debug print to be sure that the initialization was ok
    //#ifdef DEBUG
    //for (int i = 0; i < pop_size; i++) {
    //    for (int j = 0; j < inst->num_nodes; j++) {
    //        printf("%d ", population[i].cromosome[j]);
    //    }
    //    printf("      %0.0f", population[i].fitness);
    //    printf("\n");
    //}
    //#endif

    int generation = 1;
    int parent_size = 400;
    int* parents = CALLOC(parent_size, int); // Parents indexes
    int offspring_size = (parent_size - 1) * parent_size / 2;
    individual* offsprings = CALLOC(offspring_size, individual);
    for (int i = 0; i < offspring_size; i++) {
        offsprings[i].cromosome = CALLOC(inst->num_nodes, int);
    }
    double best_fitness = DBL_MAX;
    double mean_fitness = 0;
    int best_idx = 0;
    double incubement = best_fitness;
    while (1) {
        gettimeofday(&end, 0);
        double elapsed = get_elapsed_time(start, end);
        if (inst->params.time_limit > 0 && elapsed > inst->params.time_limit) {
            status = TIME_LIMIT_EXCEEDED;
            break;
        }

        fitness_metrics(population, pop_size, &best_fitness, &mean_fitness, &best_idx);
        if (best_fitness < incubement) {
            
            incubement = best_fitness;
            individual best_individual = population[best_idx];
            for (int i = 0; i < inst->num_nodes - 1; i++) {
                int index = best_individual.cromosome[i];
                inst->solution.edges[index].i = best_individual.cromosome[i];
                inst->solution.edges[index].j = best_individual.cromosome[i + 1];
            }
            int index = best_individual.cromosome[inst->num_nodes - 1];
            inst->solution.edges[index].i = best_individual.cromosome[inst->num_nodes - 1];
            inst->solution.edges[index].j = best_individual.cromosome[0];
            inst->solution.obj_best = best_fitness;
            //alg_2opt(inst);
            plot_solution(inst);
            LOG_I("UPDATED INCUBEMENT");
        }

        LOG_D("Generation %d -> Mean: %0.2f      Best: %0.0f", generation, mean_fitness, best_fitness);

        // Selection phase. i.e. selecting individuals which can go to the next generation
        select_parents(parents, parent_size, pop_size);
        
        //#ifdef DEBUG
        //for (int i = 0; i < parent_size; i++) {
        //    printf("%d ", parents[i]);
        //}
        //printf("\n");
        //#endif

        // A debug print to be sure that the initialization was ok
        


        // Generate new individuals by combining two parents
        procreate(inst, population, parents, parent_size, offsprings);
        // Select the most effective individuals which have the best fitness
        
        selection(inst, population, pop_size, offsprings, offspring_size);

        //#ifdef DEBUG
        //for (int i = 0; i < pop_size; i++) {
        //    for (int j = 0; j < inst->num_nodes; j++) {
        //        printf("%d ", population[i].cromosome[j]);
        //    }
        //    printf("      %0.0f", population[i].fitness);
        //    printf("\n");
        //}
        //printf("\n\n\n");
        //#endif

        generation++;
    }
    


    // Free allocations
    for (int i = 0; i < pop_size; i++) {
        FREE(population[i].cromosome);
    }
    FREE(population);
    FREE(parents);
    for (int i = 0; i < offspring_size; i++) {
        FREE(offsprings[i].cromosome);
    }
    FREE(offsprings);

    alg_2opt(inst);
    return status; 
}