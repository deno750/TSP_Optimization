/**
 * Implementation of sequential formulation for subtour eliminaiton constraints by
 * Miller, Tucker and Zemlin.
 */

#ifndef MTZ_H
#define MTZ_H

#include <cplex.h>
#include "utility.h"

/**
 * In asymmetric graphs, we have n^2 variables for xi,j. So the first n^2 positions are occupied.
 *
 * @param i The index of node i
 * @param num_nodes The number of nodes in the graph
 * @returns The index of the variables ui
 */
static int u_pos(int i, int num_nodes);

/**
 * Adds the u variables in the model
 * @param inst The instance pointer of the problem
 * @param env The cplex's environment
 * @param lp The cplex's problem object
 * @param names The array of variables' names
 */
static void add_u_variables(instance *inst, CPXENVptr env, CPXLPptr lp, char **names);

/**
 * Adds the mtz subtour elimination constraints using the big M trick
 * uj >= ui + 1 - M(1 - xij).
 * 
 * Param secd2 used when subtour elimination constraints of degree 2 should be added
 *
 * @param inst The instance pointer of the problem
 * @param env The cplex's environment
 * @param lp The cplex's problem object
 * @param sec2d 1 to add subtour elimination constraints of degree 2, 0 otherwise
 */ 
void add_mtz_constraints(instance *inst, CPXENVptr env, CPXLPptr lp, int secd2);

/**
 * Adds the mtz subtour elimination lazy constraints using the big M trick 
 * uj >= ui + 1 - M(1 - xij).
 * 
 * Param secd2 used when subtour elimination constraints of degree 2 should be added
 *
 * @param inst The instance pointer of the problem
 * @param env The cplex's environment
 * @param lp The cplex's problem object
 * @param sec2d 1 to add subtour elimination constraints of degree 2, 0 otherwise
 */ 
void add_mtz_lazy_constraints(instance *inst, CPXENVptr env, CPXLPptr lp, int secd2);

/**
 * Uses the cplex's indicator constraint for adding the mtz. 
 * Indicator constraints avoids the using of the big M trick
 * e.g. xij -> uj >= ui + 1
 *
 * @param inst The instance pointer of the problem
 * @param env The cplex's environment
 * @param lp The cplex's problem object
 */
void add_mtz_indicator_constraints(instance *inst, CPXENVptr env, CPXLPptr lp);

#endif