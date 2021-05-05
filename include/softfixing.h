#ifndef SOFTFIXING_H
#define SOFTFIXING_H

#include <cplex.h>
#include "utility.h"

#define SOFT_FIX_TIME_LIM_DEFAULT 900

/**
 * Function which the soft fixing solving procedure occurs
 * 
 * @param inst The instance pointer of the problem
 * @param env The cplex's environment
 * @param lp The cplex's problem object
 **/
int soft_fixing_solver(instance *inst, CPXENVptr env, CPXLPptr lp);

#endif