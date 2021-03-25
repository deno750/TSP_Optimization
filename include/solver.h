#ifndef SOLVER_H

#define SOLVER_H

#include <cplex.h>
#include "utility.h"

// Constant that is useful for numerical errors
#define EPS 1e-5

/**
 * Builds the model for a symmetric graph
 */
static void build_udir_model(instance *inst, CPXENVptr env, CPXLPptr lp);

/**
 * Builds the model for an asymmetric graph
 */
static void build_dir_model(instance *inst, CPXENVptr env, CPXLPptr lp);

/**
 * Builds the model 
 */
static void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);

/**
 * Calculating the distance based on the instance's weight_type
 */ 
static double calc_dist(int i, int j, instance *inst);

/**
 * Plots the solution.
 * 
 * Returns 0 when no errors, 1 otherwise.
 */
static int plot_solution(instance *inst);

/**
 * Solves the problem
 */ 
int TSP_opt(instance *inst);

#endif