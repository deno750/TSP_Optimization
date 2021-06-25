#include "genetic.h"

#include "heuristics.h"
#include "distutil.h"

#include <float.h>

typedef struct individual{
    int* cromosome; // List of visitin nodes. i.e. the order of nodes which are visited in the tsp
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
            cromosome[idx++] = node;
        } else {
            int node = p2.cromosome[i];
            if (visited[node]) { continue; }
            cromosome[idx++] = node;
        }
    }

    if (idx < inst->num_nodes) {
        for (int i = 0; i < rand_index; i++) {
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
        visited[min_idx] = 1;
        memcpy(population[count].cromosome, total[min_idx].cromosome, sizeof(int) * inst->num_nodes);
        population[count].fitness = total[min_idx].fitness;
        count++;
    }
    FREE(visited);
    FREE(total);
}

void fitness_metrics(individual* population, int pop_size, double* best, double* mean) {
    *best = DBL_MAX;
    *mean = 0;
    for (int i = 0; i < pop_size; i++) {
        double fitness = population[i].fitness;
        *mean += fitness;
        if (fitness < *best) {
            *best = fitness;
        }
    }

    *mean /= pop_size;
}

int HEU_Genetic(instance *inst) {
    int pop_size = 100; // Population size

    individual *population = CALLOC(pop_size, individual);

    // Starting population initialization
    for (int i = 0; i < pop_size; i++) {
        population[i].cromosome = CALLOC(inst->num_nodes, int);

        // Using greedy for initialization. Might be a better way
        int start_node = rand_choice(0, inst->num_nodes - 1);
        greedy(inst, start_node);
        
        int node_idx = start_node;
        int node_iter = 0;
        while (node_iter < inst->num_nodes) {
            population[i].cromosome[node_iter++] = inst->solution.edges[node_idx].i;
            node_idx = inst->solution.edges[node_idx].j;
        }
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
    int parent_size = 20;
    int* parents = CALLOC(parent_size, int); // Parents indexes
    int offspring_size = (parent_size - 1) * parent_size / 2;
    individual* offsprings = CALLOC(offspring_size, individual);
    for (int i = 0; i < offspring_size; i++) {
        offsprings[i].cromosome = CALLOC(inst->num_nodes, int);
    }
    double best_fitness = 0;
    double mean_fitness = 0;
    while (generation < 1000) {

        fitness_metrics(population, pop_size, &best_fitness, &mean_fitness);

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
    return 0; 
}