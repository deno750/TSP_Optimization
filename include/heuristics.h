#ifndef HEURISTICS_H
#define HEURISTICS_H

#include "utility.h"

#define WRONG_STARTING_NODE 1
#define TIME_LIMIT_EXCEEDED 2

/**
 * Applies a greedy algorithm to solve the instance
 * 
 * @param inst The instance pointer of the problem
 * @return The error code
 */
int HEU_greedy(instance *inst);

/**
 * Applies a greedy algorithm to solve the instance trying with all starting nodes possible
 * 
 * @param inst The instance pointer of the problem
 * @return The error code
 */
int HEU_Greedy_iter(instance *inst);

/**
 * Applies a the extra mileage algorithm to solve the instance
 * 
 * @param inst The instance pointer of the problem
 * @return The error code
 */
int HEU_extramileage(instance *inst);

/**
 * Applies the 2-opt algorithm to solve the instance. This algorithm MUST be executed after
 * an initialization algorithm. Use HEU_2opt to apply the 2-opt algoritm with an integrated initialization
 * 
 * @param inst The instance pointer of the problem
 * @param skip_node A list of nodes which the algorithm must not touch
 * @param stored_prev A list of previous nodes that 2-opt calculates
 * @return The error code
 */
int alg_2opt(instance *inst);

/**
 * Applies the 2-opt algorithm to solve the instance
 * 
 * @param inst The instance pointer of the problem
 * @return The error code
 */
int HEU_2opt(instance *inst);

/**
 * Applies a the 3-opt algorithm to solve the instance
 * 
 * @param inst The instance pointer of the problem
 * @return The error code
 */
int HEU_3opt(instance *inst);

/**
 * Applies the GRASP algorithm
 * 
 * @param inst The instance pointer of the problem
 * @return The error code
 */
int HEU_Grasp(instance *inst);

/**
 * Applies the iterated version of GRASP algorithm
 * 
 * @param inst The instance pointer of the problem
 * @param time_lime The time limit to let grasp solve the problem. It is useful when used in combination with another algorithm to avoid grasp take all the time limit available
 * @return The error code
 */
int HEU_Grasp_iter(instance *inst, int time_lim);

/**
 * Applies the grasp algorithm and 2-opt refinement
 * 
 * @param inst The instance pointer of the problem
 * @return The error code
 */
int HEU_Grasp2opt(instance *inst);

int HEU_VNS(instance *inst);

int HEU_Genetic(instance *inst);
#endif