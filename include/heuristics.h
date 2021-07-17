#ifndef HEURISTICS_H
#define HEURISTICS_H

#include "utility.h"

#define WRONG_STARTING_NODE 1
#define TIME_LIMIT_EXCEEDED 2

/**
 * Applies a greedy algorithm to solve the instance starting from a specified starting node
 * 
 * @param inst The instance pointer of the problem
 * @param starting_node The node from where the greedy search starts
 * @return The error code
 */
int greedy(instance *inst, int starting_node);

/**
 * Applies a greedy algorithm to solve the instance which starts from starting node 0
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
 * Applies the 2-opt algorithm to solve the instance. This algorithm MUST be executed after
 * an initialization algorithm. Use HEU_2opt to apply the 2-opt algoritm with an integrated initialization
 * 
 * @param inst The instance pointer of the problem
 * @return The error code
 */
int alg_2opt(instance *inst);

/**
 * Applies the 2-opt algorithm using grasp initialization
 * 
 * @param inst The instance pointer of the problem
 * @return The error code
 */
int HEU_2opt_grasp(instance *inst);

/**
 * Applies the 2-opt algorithm using iterative grasp initialization
 * 
 * @param inst The instance pointer of the problem
 * @return The error code
 */
int HEU_2opt_grasp_iter(instance *inst);

/**
 * Applies the 2-opt algorithm using greedy initialization
 * 
 * @param inst The instance pointer of the problem
 * @return The error code
 */
int HEU_2opt_greedy(instance *inst);

/**
 * Applies the 2-opt algorithm using iterative greedy initialization
 * 
 * @param inst The instance pointer of the problem
 * @return The error code
 */
int HEU_2opt_greedy_iter(instance *inst);

/**
 * Applies the 2-opt algorithm using extra mileage initialization
 * 
 * @param inst The instance pointer of the problem
 * @return The error code
 */
int HEU_2opt_extramileage(instance *inst);

int HEU_VNS(instance *inst);
#endif