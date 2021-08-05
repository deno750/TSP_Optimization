#ifndef TABU_SEARCH_H
#define TABU_SEARCH_H

#include "utility.h"

/**
 * Uses the tabu search metaheuristic using a step tenure policy
 * 
 * @param inst The instance of the problem
 */
int HEU_Tabu_step(instance *inst);

/**
 * Uses the tabu search metaheuristic using a linear change tenure policy from min_tenure to max_tenure and viceversa
 * 
 * @param inst The instance of the problem
 */
int HEU_Tabu_lin(instance *inst);

/**
 * Uses the tabu search metaheuristic using a random tenure policy
 * 
 * @param inst The instance of the problem
 */
int HEU_Tabu_rand(instance *inst);

#endif