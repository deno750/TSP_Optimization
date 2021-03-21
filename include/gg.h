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
 */
static int y_pos(int i, int j, int num_nodes);

/**
 * Adds the gg subtour elimination constraints in a flow based formulation.
 */ 
void add_gg_constraints(instance *inst, CPXENVptr env, CPXLPptr lp);


#endif