#ifndef SOLVER_H

#define SOLVER_H

#include <cplex.h>
#include "utility.h"

/**
 * Builds the model for a symmetric graph
 *
 * @param inst The instance pointer of the problem
 * @param env The cplex's environment
 * @param lp The cplex's problem object
 */
static void build_udir_model(instance *inst, CPXENVptr env, CPXLPptr lp);

/**
 * Builds the model for an asymmetric graph
 *
 * @param inst The instance pointer of the problem
 * @param env The cplex's environment
 * @param lp The cplex's problem object
 */
static void build_dir_model(instance *inst, CPXENVptr env, CPXLPptr lp);

/**
 * Builds the model 
 *
 * @param inst The instance pointer of the problem
 * @param env The cplex's environment
 * @param lp The cplex's problem object
 */
static void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);

/**
 * Solves the problem
 *
 * @param inst The instance pointer of the problem
 * @returns An error code when occurs. 0 when no errors occur
 */ 
int TSP_opt(instance *inst);

#endif