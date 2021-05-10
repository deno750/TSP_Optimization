/**
 * Implementation of hard fixing heuristic
 */
#ifndef HARDFIXING_H
#define HARDFIXING_H

#include <cplex.h>
#include "utility.h"

#define HARD_FIX_TIME_LIM_DEFAULT 60 * 15 // 15 min
#define HARD_FIX_MIN_IMPROVEMENT 0.015
#define HARD_FIX_MAX_LITTLE_IMPROVEMENTS 3

/**
 * Function which uses the hard fixing solver with fixed probability
 * 
 * @param inst The instance pointer of the problem
 * @param env The cplex's environment
 * @param lp The cplex's problem object
 * @return The error code
 **/
int hard_fixing_solver(instance *inst, CPXENVptr env, CPXLPptr lp);

/**
 * Function which uses the hard fixing solver with variable probabilities
 * 
 * @param inst The instance pointer of the problem
 * @param env The cplex's environment
 * @param lp The cplex's problem object
 * @return The error code
 **/
int hard_fixing_solver2(instance *inst, CPXENVptr env, CPXLPptr lp);

#endif