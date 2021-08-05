#ifndef VNS_H
#define VNS_H

#include "utility.h"

/**
 * Uses the VNS metaheuristic algorithm to solve the instance problem
 * 
 * @param inst The instance pointer of the problem
 * 
 * @returns The status code 0 when no errors occur
 * 
 */
int HEU_VNS(instance *inst);

#endif