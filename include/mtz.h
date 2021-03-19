#ifndef MTZ_H
#define MTZ_H

#include <cplex.h>
#include "utility.h"

/**
 * In asymmetric graphs, we have n^2 constraint
 */
static int u_pos(int i, int num_nodes) {
    int n = num_nodes;
    return n * n + i;
}

void add_mtz_constraints(instance *inst, CPXENVptr env, CPXLPptr lp, int (*x_pos_ptr)(int, int, int)) {
    if (x_pos_ptr == NULL) {
        print_error("You cannot pass a NULL function pointer");
    }
    int BIG_M = inst->num_nodes - 1;
    char sense = 'L';
    char xctype = 'I';
    char* names = (char *) calloc(100, sizeof(char));
    
    for (int i = 0; i < inst->num_nodes; i++) {
        sprintf(names, "u(%d)", i+1);

        // Variables treated as single value arrays.
        double obj = 0.0; 
        double lb = 0.0; 
        double ub = i == 0 ? 0.0 : inst->num_nodes - 2;

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
    
    // Mxij + ui - uj <= M - 1
    double rhs = BIG_M - 1.0;
    for (int i = 1; i < inst->num_nodes; i++) { // Excluding node 1
        for (int j = 1; j < inst->num_nodes; j++) { // Excluding node 1
            if (i == j) continue;
            sprintf(names, "inc(%d)", k++);
            int num_rows = CPXgetnumrows(env, lp);

            int status = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, &names);
            if (status) {
                print_error("Error vincle"); //TODO: Change error message
            }

            // Adding M*xij
            status = CPXchgcoef(env, lp, num_rows, (*x_pos_ptr)(i, j, inst->num_nodes), BIG_M);
            if (status)  print_error("An error occured in filling constraint x(i, j)");

            // Adding ui
            status = CPXchgcoef(env, lp, num_rows, u_pos(i, inst->num_nodes), 1);
            if (status)  print_error("An error occured in filling constraint u(i)");
            
            // Adding -uj
            status = CPXchgcoef(env, lp, num_rows, u_pos(j, inst->num_nodes), -1);
            if (status)  print_error("An error occured in filling constraint u(j)");
        }
    }


    free(names);
}

#endif