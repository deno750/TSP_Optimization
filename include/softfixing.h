#ifndef SOFTFIXING_H
#define SOFTFIXING_H

#include <cplex.h>
#include "utility.h"

#define SOFT_FIX_MIN_IMPROVEMENT 0.015 // 1.5%
#define SOFT_FIX_MAX_LITTLE_IMPROVEMENTS 3

/**
 * Function which the soft fixing solving procedure occurs
 * 
 * @param inst The instance pointer of the problem
 * @param env The cplex's environment
 * @param lp The cplex's problem object
 **/
int soft_fixing_solver(instance *inst, CPXENVptr env, CPXLPptr lp);

#endif