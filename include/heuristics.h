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
#endif