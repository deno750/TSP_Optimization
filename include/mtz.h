#ifndef MTZ_H
#define MTZ_H

#include <cplex.h>
#include "utility.h"
#include "solver.h"

static int u_pos(int i, int num_nodes) {
    int n = num_nodes;
    return n * n + i;
}



// Mxij + ui - uj <= M - 1
void add_mtz_constraints(instance *inst, CPXENVptr env, CPXLPptr lp) {
    int BIG_M = inst->num_nodes - 1;
    double rhs = BIG_M - 1.0;
    char sense = 'L';
    char xctype = 'B';
    char* names = (char *) calloc(100, sizeof(char));
    
    for (int i = 0; i < inst->num_nodes; i++) {
        sprintf(names, "u(%d)", i+1);

        // Variables treated as single value arrays.
        double obj = 0.0; 
        double lb = 0.0; 
        double ub = i == 0 ? 0.0 : inst->num_nodes - 2; // if i==j: ub=0 else ub=1

        int status = CPXnewcols(env, lp, 1, &obj, &lb, &ub, &xctype, &names); 
        if (status) {
            print_error("An error occured inserting a new variable");
        }
        int numcols = CPXgetnumcols(env, lp);
        if (numcols - 1 != u_pos(i, inst->num_nodes)) { // numcols -1 because we need the position index of the new variable
            print_error("Wrong position of variable");
        }
    }

    int k = 1;
    //int num_rows = CPXgetnumrows(env, lp);
    //printf("Num rows: %d\n", num_rows);
    for (int i = 1; i < inst->num_nodes; i++) { // Excluding node 1
        for (int j = 1; j < inst->num_nodes; j++) { // Excluding node 1
            if (i == j) continue;
            sprintf(names, "inc(%d)", k++);
            int status = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, &names);
            if (status) {
                print_error(("Error vincle")); //TODO: Change error message
            }
            int num_rows = CPXgetnumrows(env, lp);
            
            status = CPXchgcoef(env, lp, num_rows - 1, x_dir_pos(i, j, inst->num_nodes), BIG_M);
            if (status)  print_error("An error occured in filling a constraint1");

            num_rows = CPXgetnumrows(env, lp);
            status = CPXchgcoef(env, lp, num_rows - 1, u_pos(i, inst->num_nodes), 1);
            if (status)  print_error("An error occured in filling a constraint2");
            
            num_rows = CPXgetnumrows(env, lp);
            status = CPXchgcoef(env, lp, num_rows - 1, u_pos(j, inst->num_nodes), -1);
            if (status)  print_error("An error occured in filling a constraint3");
        }
    }


    free(names);
}

#endif