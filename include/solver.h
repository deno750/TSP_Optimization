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
    CPXENVptr env = CPXopenCPLEX(&error);
    CPXLPptr lp = CPXcreateprob(env, &error, inst->name);
    
    build_model(inst, env, lp);

    int status = CPXmipopt(env, lp);
    if (status) {
        if (inst->params.verbose >= 5) {
            printf("Cplex error code: %d\n", status);
        }
        print_error("Cplex solver encountered an error.");
    }

    CPXfreeprob(env, &lp);
    CPXcloseCPLEX(&env);
    return error;
}

static void build_model(instance *inst, CPXENVptr env, CPXLPptr lp) {
    
    char xctype = 'B';
    char **colname = (char **) calloc(1 , sizeof(char*)); // Cplex wants an array of variable names (i.e. char array of array)
    colname[0] = (char *) calloc(100, sizeof(char));


    // We add one variable at time. We may also add them all in a single shot.
    for (int i = 0; i < inst->num_nodes; i++) {

        for (int j = i+1; j < inst->num_nodes; j++) {
            
            sprintf(colname[0], "x(%d,%d)", i+1, j+1);

            // Variables treated as single value arrays.
            double obj = calc_dist(i, j, inst); 
            double lb = 0.0;
            double ub = 1.0;

            int status = CPXnewcols(env, lp, 1, &obj, &lb, &ub, &xctype, colname);
            int numcols = CPXgetnumcols(env, lp);
            if (numcols - 1 != x_pos(i, j, inst->num_nodes)) { // numcols -1 because we need the position index of the new variable
                print_error("Wrong position of variable");
            }
        }
    }

    // Adding the constraints

    // Saving the model in Lp file
    char modelPath[1024];
    sprintf(modelPath, "../model/%s.lp", inst->name);
    
    CPXwriteprob(env, lp, modelPath, NULL);
    
    free(colname[0]);
    free(colname);

}

static int x_pos(int i, int j, int num_nodes) {

   return i * num_nodes + j - ((i + 1) * (i + 2)) / 2;
}

static double calc_dist(int i, int j, instance *inst) {
    point node1 = inst->nodes[i];
    point node2 = inst->nodes[j];
    double dx = node1.x - node2.x;
    double dy = node2.y - node2.y;
    double dist = sqrt(dx*dx + dy*dy);
    int integer = 0; // We should know wheter the distance should be integer or not. New parameter in instance? 
    return integer ? round(dist) : dist;
}

#endif