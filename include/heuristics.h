#ifndef HEURISTICS_H
#define HEURISTICS_H

#include "utility.h"

/**
 * Applies a greedy algorithm to solve the instance
 * 
 * @param inst The instance pointer of the problem
 * @return The error code
 */
int HEU_greedy(instance *inst);

/**
 * Applies a the extra mileage algorithm to solve the instance
 * 
 * @param inst The instance pointer of the problem
 * @return The error code
 */
int HEU_extramileage(instance *inst);

/**
 * Applies a the 2-opt algorithm to solve the instance
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

int HEU_Greedy_iter(instance *inst);

int HEU_Grasp(instance *inst);

int HEU_Grasp_iter(instance *inst);

int HEU_Grasp2opt(instance *inst);

int HEU_VNS(instance *inst);

int HEU_Tabu(instance *inst);

int HEU_Genetic(instance *inst);
#endif