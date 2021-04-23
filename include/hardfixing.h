/**
 * Implementation of hard fixing heuristic
 */
#ifndef HARDFIXING_H
#define HARDFIXING_H

#include <cplex.h>
#include "utility.h"

/**
 * Function which the hard fixing solving procedure occurs
 * 
 * @param inst
 * @param env
 * @param lp
 **/
int hard_fixing_solver(instance *inst, CPXENVptr env, CPXLPptr lp);

#endif