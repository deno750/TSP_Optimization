#ifndef SOLVER_H

#define SOLVER_H

#include <cplex.h>
#include "utility.h"


#define EPS 1e-5

static void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
static int x_pos(int i, int j, int num_nodes);
static double calc_dist(int i, int j, instance *inst);

int TSP_opt(instance *inst) {
    int error;
    CPXENVptr env = CPXopenCPLEX(&error);       // generate new environment, in err will be saved errors
    CPXLPptr lp = CPXcreateprob(env, &error, inst->name);   // create new empty linear programming problem (no variables, no constraints ...)
    
    // Build the model (add variable and constrains to the empty one)
    build_model(inst, env, lp);

    //Optimize the model (the solution is stored inside the env variable)
    int status = CPXmipopt(env, lp);
    if (status) {
        if (inst->params.verbose >= 5) {
            printf("Cplex error code: %d\n", status);
        }
        print_error("Cplex solver encountered an error.");
    }

    // Use the solution
    int ncols = CPXgetnumcols(env, lp);
	double *xstar = (double *) calloc(ncols, sizeof(double));
	if ( CPXgetx(env, lp, xstar, 0, ncols-1) ) print_error("CPXgetx() error");	
	for ( int i = 0; i < inst->num_nodes; i++ ){
		for ( int j = i+1; j < inst->num_nodes; j++ ){
			if ( xstar[x_pos(i,j,inst->num_nodes)] > 0.5 ) printf("  ... x(%3d,%3d) = 1\n", i+1,j+1);
		}
	}
	free(xstar);


    //Free the problem and close cplex environment
    CPXfreeprob(env, &lp);
    CPXcloseCPLEX(&env);
    return error;
}

static void build_model(instance *inst, CPXENVptr env, CPXLPptr lp) {
    
    char xctype = 'B';  // B=bynary variable
    char **names = (char **) calloc(1 , sizeof(char*)); // Cplex wants an array of variable names (i.e. char array of array)
    names[0] = (char *) calloc(100, sizeof(char));


    // We add one variable at time. We may also add them all in a single shot. i<j
    for (int i = 0; i < inst->num_nodes; i++) {

        for (int j = i+1; j < inst->num_nodes; j++) {
            
            sprintf(names[0], "x(%d,%d)", i+1, j+1);

            // Variables treated as single value arrays.
            double obj = calc_dist(i, j, inst); 
            double lb = 0.0;
            double ub = 1.0;

            int status = CPXnewcols(env, lp, 1, &obj, &lb, &ub, &xctype, names);
            if (status) {
                print_error("An error occured inserting a new variable");
            }
            int numcols = CPXgetnumcols(env, lp);
            if (numcols - 1 != x_pos(i, j, inst->num_nodes)) { // numcols -1 because we need the position index of the new variable
                print_error("Wrong position of variable");
            }
        }
    }

    // Adding the constraints
    for (int h = 0; h < inst->num_nodes; h++) {

        int lastRow = CPXgetnumrows(env, lp);
        double rhs = 2.0;
        char sense = 'E';
        sprintf(names[0], "constraint(%d)", h+1);
        int status = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, names);
        if (status) {
            print_error("An error occured inserting a new constraint");
        }

        for (int i = 0; i < inst->num_nodes; i++) {
            if (i == h) continue;

            status = CPXchgcoef(env, lp, lastRow, x_pos(h, i, inst->num_nodes), 1.0);
            if (status) {
                print_error("An error occured in filling a constraint");
            }
        }
        
    }

    // Saving the model in .lp file
    char modelPath[1024];
    sprintf(modelPath, "../model/%s.lp", inst->name);
    
    CPXwriteprob(env, lp, modelPath, NULL);
    
    free(names[0]);
    free(names);

}

// Get cplex internal reppresentation of nodes
static int x_pos(int i, int j, int num_nodes) {
    if (i == j) print_error("Indexes passed are equal!");
    if (i > j) return x_pos(j, i, num_nodes);
    return i * num_nodes + j - ((i + 1) * (i + 2)) / 2;
}

// Euclidean distance
static double calc_dist(int i, int j, instance *inst) {
    point node1 = inst->nodes[i];
    point node2 = inst->nodes[j];
    double dx = node1.x - node2.x;
    double dy = node1.y - node2.y;
    double dist = sqrt(dx*dx + dy*dy);
    int integer = 1; // We should know wheter the distance should be integer or not. New parameter in instance? 
    return integer ? round(dist) : dist;
}

#endif