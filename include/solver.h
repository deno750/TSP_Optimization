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
 * Solves the problem utilizing method using cplex
 *
 * @param inst The instance pointer of the problem
 * @returns An error code when occurs. 0 when no errors occur
 */ 
int TSP_opt(instance *inst);

/**
 * Solves the problem without the help of cplex
 *
 * @param inst The instance pointer of the problem
 * @returns An error code when occurs. 0 when no errors occur
 */ 
int TSP_heuc(instance *inst);

/**
 * Solves the problem with the best optimal solver found so far.
 * It is used on heuristics solvers which needs the fastest optimal solver.
 * 
 * The implemented solver is USER_CUT. 
 * It may be possible that in the future the implemented solver can be different.
 * 
 * 
 * @param inst The instance pointer of the problem
 * @param env The cplex's environment
 * @param lp The cplex's problem object
 */
int opt_best_solver(CPXENVptr env, CPXLPptr lp, instance *inst);

#endif