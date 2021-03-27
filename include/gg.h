/**
 * Implementation of single commodity flow based formulation for subtour elimination 
 * constraints by Gavish and Graves.
 */

#ifndef GG_H
#define GG_H

#include <cplex.h>
#include "utility.h"


/**
 * Retrieving position for yij variables. Yij's are n^2.
 *
 * @param i The index of node i
 * @param j The index of node j
 * @param num_nodes The number of nodes in the graph
 * @returns the index mapped from (i,j)
 */
static int y_pos(int i, int j, int num_nodes);

/**
 * Adds the gg subtour elimination constraints in a flow based formulation.
 *
 * @param inst The instance pointer of the problem
 * @param env The cplex's environment
 * @param lp The cplex's problem object
 */ 
void add_gg_constraints(instance *inst, CPXENVptr env, CPXLPptr lp);


#endif