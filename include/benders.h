/**
 * Implementation of Benders algorithm
 */

#ifndef BENDERS_H
#define BENDERS_H

#include <cplex.h>
#include "utility.h"

/**
 * Implements the benders algorithm to solve the problem. It uses CPXmipopt multiple times.
 * 
 * @param inst The instance pointer of the problem
 * @param env The cplex's environment
 * @param lp The cplex's problem object
 */
int benders_loop(instance *inst, CPXENVptr env, CPXLPptr lp);

#endif