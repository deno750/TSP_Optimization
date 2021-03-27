#ifndef SOLVER_H

#define SOLVER_H

#include <cplex.h>
#include "utility.h"

// Constant that is useful for numerical errors
#define EPS 1e-5

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
 * Calculating the distance based on the instance's weight_type
 *
 * @param i The node i index
 * @param j The node j index
 * @param inst The instance pointer of the problem
 * @returns the distance between node i and node j accordingly with the instance
 */ 
static double calc_dist(int i, int j, instance *inst);

/**
 * Plots the solution.
 * 
 * @param inst The instance pointer of the problem
 * @returns 0 when no errors, 1 otherwise.
 */
static int plot_solution(instance *inst);

/**
 * Stores the solution given by xstar into a list of edges. 
 * With the list of edges is much more easier to retrieve the
 * solution informaiton and helps to generalize better between
 * the undirected and directed graph solutions. 
 *
 * @param inst The instance pointer of the problem
 * @param xstar The solution values pointer
 */
static void save_solution_edges(instance *inst, double *xstar);

/**
 * Solves the problem
 *
 * @param inst The instance pointer of the problem
 * @returns An error code when occurs. 0 when no errors occur
 */ 
int TSP_opt(instance *inst);

#endif