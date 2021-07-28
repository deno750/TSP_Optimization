#include "genetic.h"

#include "heuristics.h"
#include "distutil.h"

#include <float.h>

// This struct represents an individual in the population. 
// Stores the cromosome and the fitness value. 
typedef struct individual{
    int* cromosome; // List of nodes in the order of which are visited in the tsp
    double fitness;
} individual;

/**
 * Transforms the cromosome representation to edge representation
 * 
 * @param inst The problem instance
 * @param individual The individual which cromosome is stored to edge representation
 */
void from_cromosome_to_edges(instance* inst, individual individual) {
    for (int i = 0; i < inst->num_nodes - 1; i++) {
        int index = individual.cromosome[i];
        inst->solution.edges[index].i = individual.cromosome[i];
        inst->solution.edges[index].j = individual.cromosome[i + 1];
    }
    int index = individual.cromosome[inst->num_nodes - 1];
    inst->solution.edges[index].i = individual.cromosome[inst->num_nodes - 1];
    inst->solution.edges[index].j = individual.cromosome[0];
}

/**
 * Calculates the fitness value of an individual. This value is stored in the 
 * fitness field of the individual struct
 * 
 * @param inst The problem instance
 * @param individual A reference of the individual which the fitness will be calculated
 */
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

static int compare_individuals(const void *lhs, const void *rhs) {
    const individual* lp = lhs;
    const individual* rp = rhs;

    return rp->fitness - lp->fitness;
    
}


/**
 * Picks from the population a random number of individuals which are going to be the parents to produce offsprings
 * 
 * @param parents A reference of the parents array
 * @param parent_size The number of parents (i.e. capacity of parents array)
 * @param pop_size The current number of individuals in the population
 */
void select_parents(individual* population, int* parents, int parent_size, int pop_size) {
    MEMSET(parents, -1, parent_size, int);
    int count = 0;

    int* visited = CALLOC(pop_size, int);

    // Rank based roulette wheel selection. Check this paper here: http://www.iaeng.org/publication/WCE2011/WCE2011_pp1134-1139.pdf

    // Ranking the fitnessess using qsort. Best fitness will be displaced at the end so it will have the highest rank
    qsort(population, pop_size, sizeof(individual), compare_individuals);

    // Calculating the cumulative sum. This is part of wheel selection.   
    double rank_sum = pop_size * (pop_size + 1) / 2;
    double* cum_sum = CALLOC(pop_size, double);

    cum_sum[0] = 1;
    for (int i = 1; i < pop_size; i++) {
        double sum = cum_sum[i - 1] + i + 1;
        cum_sum[i] = sum;
    }


    while (count < parent_size) {

        // Choosing the individual based on it's cumulative sum (i.e. such as probability). Part of wheel selection
        double random_num = rand_choice(1, rank_sum);

        // New way
        // This formula is Gauss' sum reversed formula n*(n+1) / 2 = m. When we set m as the random_num,
        // we can obtain n using this equation n^2 + n - 2*m = 0. The result of the equation returns the
        // index where it is contained the nearest greatest number near the rand_num.

        // Example: Let be [1,3,6,10,15,21,28,36,45] an array of cumulative sums. Let's take a random number x
        // between [1, 45]. Let x be 18. The index of 18 would be (-1 + sqrt(1 + 8*18)) / 2 = 5.52.
        // This is the index where the number 18 would be displaced in the array considering indexes starting
        // from 1. If we get the floor we obtain index = 5. Now in our example array which uses indexes that
        // starts from 0, in the position 5 we have the number 21 which is the nearest greater number from 18. 
        int index = (-1 + sqrt(1 + 8*random_num)) / 2.0; // Do not use round. Floor operation works better.
        double val = cum_sum[index];
        // We are certain that random_num < val
        if (random_num > val) {
            LOG_E("Wrong assumption");
        }
        if (!visited[index]) {
            parents[count++] = index;
            visited[index] = 1;
        }

        //Old way
        /*for (int i = 0; i < pop_size; i++) {
            double cumulative_sum = cum_sum[i];
            if (random_num < cum_sum[i] && !visited[i]) {
                parents[count++] = i;
                visited[i] = 1;
                break;
            }
        }*/
    }
    FREE(cum_sum);
    FREE(visited);
    
}

/**
 * Applies the crossover phase of the genetic algorithm. From two parents an offspring is generated.
 * 
 * @param inst The problem instance
 * @param population The list of individuals which composes the population
 * @param parent1 The index of the first parent in the population
 * @param parent2 The index of the second parent in the population
 * @param cromosome The offspring's cromosome that is generated from the two parents.
 */
void crossover(instance* inst, individual *population, int parent1, int parent2, int* cromosome) {
    individual p1 = population[parent1];
    individual p2 = population[parent2];

    int *visited = CALLOC(inst->num_nodes, int);
    double rand_num = URAND();
    if (rand_num < 0.5) {
        // Crossover method 1
        // This method takes a random index which splits the cromosome. 
        int rand_index = rand_choice(0, inst->num_nodes);
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
    } else {
        // Crossover method 2
        // Using this method instead to split the cromosome's array in 2 parts, we want to obtain a random substring of the parent1's cromosome. 
        // This substring is going to be added in the same position of offspring's cromosome. The remaining offspring's cromosome positions are
        // going to be filled by the remaining nodes in parent's 2 cromosome in order of appearing from rand_index2. 
        // Here's an image which describes this procedure: https://miro.medium.com/max/1458/1*YhmzBBCyAG3rtEBbI0gz4w.jpeg
        int rand_index1 = rand_choice(0, inst->num_nodes);
        int rand_index2 = rand_choice(0, inst->num_nodes);
        if (rand_index1 > rand_index2) {
            int tmp = rand_index1;
            rand_index1 = rand_index2;
            rand_index2 = tmp;
        }
        if (rand_index1 == rand_index2) {
            if (rand_index1 > 0) {
                rand_index1 -= 1;
            } else {
                rand_index2 += 1;
            }
        }

        int nodes_added = 0;
        for (int i = rand_index1; i <= rand_index2; i++) {
            int node = p1.cromosome[i];
            visited[node] = 1;
            cromosome[i] = node;
            nodes_added++;
        }

        // A little bit confusing code. 
        // parent2_counter starts from rand_index2 + 1 because we want to start from the following node in parent2 array. It is an increasing number for every iteration
        // the while loop below does.
        // offspring_crom_counter increases only when a new entry is added to the offspring's cromosome. 
        // Both of these counters overflows the cromosome's size. So a modulo operation with number of nodes should be done in order to retrieve the correct index
        // of the partent 2 index and offspring's index
        int parent2_counter = rand_index2 + 1; 
        int offspring_crom_counter = parent2_counter;
        while (nodes_added < inst->num_nodes) {
            int p2_index = parent2_counter % inst->num_nodes; // Getting the current looking gene on parent2's cromosome.
            int node = p2.cromosome[p2_index];
            if (!visited[node]) {
                int offspring_index = offspring_crom_counter % inst->num_nodes; // Gettings the current index of offspring's cromosome where the new entry will be added
                cromosome[offspring_index] = node;
                nodes_added++;
                offspring_crom_counter++;
            }

            parent2_counter++;
        }
    }

    FREE(visited);
}

/**
 * Does the procreation phase where the parents generate new offsprings.
 * 
 * @param inst The problem instance
 * @param population The list of individuals which composes the population
 * @param parents The list of indexes of the parents in the population
 * @param parent_size The size of the parents array
 * @param offsprings The list of offsprings generated
 */
void procreate(instance* inst, individual *population, int* parents, int parent_size, individual* offsprings) {
    
    int* cromosome = CALLOC(inst->num_nodes, int);
    int counter = 0;

    for (int i = 0; i < parent_size; i++) {
        int j = (i + 1) % parent_size;
        int parent1 = parents[i];
        int parent2 = parents[j];
        crossover(inst, population, parent1, parent2, cromosome);
        memcpy(offsprings[counter].cromosome, cromosome, sizeof(int) * inst->num_nodes);
        fitness(inst, &(offsprings[counter]));

        counter++;
    }
    FREE(cromosome);
}

/**
 * Choses the best performing individuals. Sometimes to mantain diversification,
 * a random individual is also selected reghardless its fitness function with a probability of 10%. 
 * 
 * @param inst The problem instance
 * @param population The list of individuals which composes the population
 * @param pop_size The current number of individuals in the population
 * @param offsrpings The offsprings list
 * @param off_size The capacity of offsprings
 */
void choose_survivors(instance* inst, individual* population, int pop_size, individual* offsprings, int off_size) {
    
    int N = pop_size + off_size;
    individual* total = CALLOC(N, individual);
    int *visited = CALLOC(N, int);
    int count = 0;
    
    for (int i = 0; i < off_size; i++) {
        total[count++] = offsprings[i];
    }
    for (int i = 0; i < pop_size; i++) {
        total[count++] = population[i];
    }

    count = 0;

    // Rank based roulette wheel selection
    qsort(total, N, sizeof(individual), compare_individuals);

    double rank_sum = N * (N + 1) / 2;
    double* cum_sum = CALLOC(N, double);
    cum_sum[0] = 1;
    for (int i = 1; i < N; i++) {
        double sum = cum_sum[i - 1] + i + 1;
        cum_sum[i] = sum;
    }

    while (count < pop_size) {
        double random_num = rand_choice(1, rank_sum);

        // New way

        // To understand this, go read the same piece of code in select_parents function
        int index = (-1 + sqrt(1 + 8*random_num)) / 2.0;
        double val = cum_sum[index];
        if (random_num < val && !visited[index]) {
            memcpy(population[count].cromosome, total[index].cromosome, sizeof(int) * inst->num_nodes);
            population[count].fitness = total[index].fitness;
            count++;
            visited[index] = 1;
        }

        // Old way
        /*for (int i = 0; i < pop_size; i++) {
            double cumulative_sum = cum_sum[i];
            if (random_num < cum_sum[i] && !visited[i]) {
                //memcpy(population[count].cromosome, total[i].cromosome, sizeof(int) * inst->num_nodes);
                population[count].fitness = total[i].fitness;
                count++;
                visited[i] = 1;
                break;
            }
        }*/
    }
    FREE(total);
    FREE(visited);
    FREE(cum_sum);
}

/**
 * Calculates the mean fitness, the best fitness value and the best individual's index in the population
 * 
 * @param population The list of individuals which composes the population
 * @param pop_size The current number of individuals in the population
 * @param best A reference where the best fitness value in the population will be stored
 * @param mean A reference where the mean fitness of the population will be stored
 * @param best_idx A reference where the index of the best individual will be stored 
 */
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

/**
 * Generates a random tour. This function is used to initialize randomly the initial population
 * 
 * @param cromosome Where the random tour will be stored. It must have the size equal to num_nodes
 * @param num_nodes The number of nodes in the instance
 */
void random_generation(int* cromosome, int num_nodes) {
    //Initialize the list of nodes with the numbers 1 to N
    for (int i = 0; i < num_nodes; i++) {  
        cromosome[i] = i;
    }

    //Choose randomly 2 nodes in the tour and swap them
    for (int i = 0; i < num_nodes; i++) {  
        int idx1 = rand_choice(0, num_nodes);
        int idx2 = rand_choice(0, num_nodes);

        int tmp = cromosome[idx1];
        cromosome[idx1] = cromosome[idx2];
        cromosome[idx2] = tmp;
    }
}

/**
 * The mutation phase is applied 
 * with a probability of 5%. With probability of 98% during the mutation phase, is applied a mutation where a random subtour
 * is chosen and reversed; in the remaining 2% is applied a 2-opt refinement. This 2-opt refinement works for only
 * 5 seconds. The completion of the 2-opt in this phase is not necessary. The 2-opt ideally is going to work better as the 
 * algorithm goes forward. In a very later generation, there would be low crossing edges so the 2-opt algorithm can 
 * complete under 5 seconds and return a better individual. In any case, even when the 2-opt does not complete, a better
 * individual is found because some crossing edges are removed.
 */
void mutation(instance* inst, individual* offsprings, int off_size) {
    for (int off = 0; off < off_size; off++) {

        double rand_mut = URAND() ;
        // Mutation phase
        if (rand_mut < 0.1) {
            // Mutation method 1
            // It takes two nodes and swaps them
            /*int rand_index1 = rand_choice(0, inst->num_nodes - 1);
            int rand_index2 = rand_choice(0, inst->num_nodes - 1);
            if (rand_index1 == rand_index2) {
                // Dont' assign manually. Just for test
                if (rand_index1 < inst->num_nodes - 1) {
                    rand_index2 = rand_index1 + 1;
                } else {
                    rand_index2 = rand_index1 - 1;
                }
                
            }
            int temp = population[count].cromosome[rand_index1];
            population[count].cromosome[rand_index1] = population[count].cromosome[rand_index2];
            population[count].cromosome[rand_index2] = temp;
            fitness(inst, &(population[count]));*/

            // Mutation method 2
            // It takes a subtour and reverses it. e.g. 1-4-3-7-9 becomes 9-7-3-4-1
            int rand_index1 = rand_choice(0, inst->num_nodes - 1);
            int rand_index2 = rand_choice(0, inst->num_nodes - 1);
            if (rand_index1 > rand_index2) {
                int tmp = rand_index1;
                rand_index1 = rand_index2;
                rand_index2 = tmp;
            }
            if (rand_index1 == rand_index2) {
                if (rand_index1 > 0) {
                    rand_index1 -= 1;
                } else {
                    rand_index2 += 1;
                }
            }
            int tot_iter = rand_index2 - rand_index1;
            int incr_idx = rand_index1;
            int decr_idx = rand_index2;
            for (int i = 0; i < tot_iter / 2; i++) {
                int tmp = offsprings[off].cromosome[incr_idx];
                offsprings[off].cromosome[incr_idx] = offsprings[off].cromosome[decr_idx];
                offsprings[off].cromosome[decr_idx] = tmp;
                incr_idx++;
                decr_idx--;
            }
        }
    }
}

int HEU_Genetic(instance *inst) {
    int status = 0;

    //Start counting time from now
    struct timeval start, end;
    gettimeofday(&start, 0);

    int pop_size = 1000; // Population size
    individual *population = CALLOC(pop_size, individual);//Allocate pupulation

    // Generate Initial population
    for (int i = 0; i < pop_size; i++) {
        population[i].cromosome = CALLOC(inst->num_nodes, int);

        double rand_num = URAND();
        if (rand_num < 0.0) {
            int start_node = rand_choice(0, inst->num_nodes);
            grasp(inst, start_node);
                
            int node_idx = start_node;
            int node_iter = 0;
            while (node_iter < inst->num_nodes) {
                population[i].cromosome[node_iter++] = inst->solution.edges[node_idx].i;
                node_idx = inst->solution.edges[node_idx].j;
            }
        } else {
            //generate a single individual
            random_generation(population[i].cromosome, inst->num_nodes);
        }
        
        

        //Evaluate the fitness of this individual
        fitness(inst, &(population[i]));

    }

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

    //Set time limit
    if (inst->params.time_limit <= 0 && inst->params.verbose >= 3) {
        LOG_I("Default time lim %d setted.", DEFAULT_TIME_LIM);
    }
    int time_limit = inst->params.time_limit > 0 ? inst->params.time_limit : DEFAULT_TIME_LIM;
    
    //Allocate memory for parents and offspring
    int generation = 1;
    int parent_size = (int) (pop_size * 0.6);
    int* parents = CALLOC(parent_size, int); // Parents indexes
    int offspring_size = parent_size;//(parent_size - 1) * parent_size / 2;
    individual* offsprings = CALLOC(offspring_size, individual);
    for (int i = 0; i < offspring_size; i++) {
        offsprings[i].cromosome = CALLOC(inst->num_nodes, int);
    }
    double best_fitness = DBL_MAX;
    double mean_fitness = 0;
    int best_idx = 0;
    double incubement = best_fitness;
    


    //Repeat until time limit is reached
    while (1) {
        //Check elapsed time
        gettimeofday(&end, 0);
        double elapsed = get_elapsed_time(start, end);
        if (elapsed > time_limit) {
            status = TIME_LIMIT_EXCEEDED;
            break;
        }

        //Compute the mean fitness, the best fitness value and the best individual's index in the population
        fitness_metrics(population, pop_size, &best_fitness, &mean_fitness, &best_idx);
        //If there is a tour in the current population that is better than the one seen so far, save it
        if (best_fitness < incubement) {
            incubement = best_fitness;
            individual best_individual = population[best_idx];
            inst->solution.obj_best = best_fitness;
            from_cromosome_to_edges(inst, best_individual); //Update best solution
            
            //plot_solution(inst);
            //if (inst->params.verbose >= 3) {LOG_I("UPDATED INCUBEMENT: %0.2f", best_fitness);}

        }
        if (generation % 100 == 0) {
            plot_solution(inst);
        }
        
        if (inst->params.verbose >= 4) {
            LOG_I("Generation %d -> Mean: %0.2f      Best: %0.0f     Incumbent: %0.0f", generation, mean_fitness, best_fitness, incubement);
        }

        //SELECTION: select individuals which can go to the next generation
        select_parents(population, parents, parent_size, pop_size);

        //CROSSOVER: Generate new individuals by combining two parents
        procreate(inst, population, parents, parent_size, offsprings);

        // Mutation phase
        mutation(inst, offsprings, offspring_size);
        
        //Replace the individuals of the current populations with the children that has better fitness
        choose_survivors(inst, population, pop_size, offsprings, offspring_size);

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

    if (inst->params.verbose >= 4) {
        LOG_I("Applying final 2opt refinement");
    }
    alg_2opt(inst);
    return status; 
}