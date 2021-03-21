#include "gg.h"

static int y_pos(int i, int j, int num_nodes) {
   int n = num_nodes;
   return (n * n) + (i * n + j);
}

void add_gg_constraints(instance *inst, CPXENVptr env, CPXLPptr lp) {

    char* names = (char *) calloc(100, sizeof(char));
    char xctype = 'I';
    
    for (int i = 0; i < inst->num_nodes; i++) {
        for (int j = 0; j < inst->num_nodes; j++) {
            sprintf(names, "y(%d,%d)", i+1, j+1);
            double obj = 0.0; 
            double lb = 0.0; 
            double ub = j == 0 || i == j ? 0.0 : inst->num_nodes - 1; // The model with y <= N - 2 is wrong. With N - 1 the model is correct

            int status = CPXnewcols(env, lp, 1, &obj, &lb, &ub, &xctype, &names); 
            if (status) {
                print_error("An error occured inserting a new variable");
            }
            int numcols = CPXgetnumcols(env, lp);
            if (numcols - 1 != y_pos(i, j, inst->num_nodes)) { // numcols -1 because we need the position index of the new variable
                print_error("Wrong position of variable y");
            }
        }
    }

    //inFlow-outFlow=1  for each h in V\{1}
    char sense = 'E';
    double rhs = 1.0;
    int k = 1;
    for (int h = 1; h < inst->num_nodes; h++) {
        sprintf(names, "inoutflow(%d)", k++);
        int num_rows = CPXgetnumrows(env, lp);

        int status = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, &names);
        if (status) {
            print_error("Error adding new row"); 
        }
        for (int i = 0; i < inst->num_nodes; i++) {
            if (h == i) continue;
            status = CPXchgcoef(env, lp, num_rows, y_pos(i, h, inst->num_nodes), 1.0);
            if (status)  print_error("An error occured in filling constraint y(i, h)");
        }
        for (int j = 0; j < inst->num_nodes; j++) {
            if (h == j) continue;
            status = CPXchgcoef(env, lp, num_rows, y_pos(h, j, inst->num_nodes), -1.0);
            if (status)  print_error("An error occured in filling constraint y(h, j)");
        }
    }

    //y1j = (n-1)*x1j 
    rhs = 0;
    sense = 'E';
    for (int j = 1; j < inst->num_nodes; j++) {
        sprintf(names, "node1constraint(%d)", j);
        int num_rows = CPXgetnumrows(env, lp);
        int status = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, &names);
        status = CPXchgcoef(env, lp, num_rows, y_pos(0, j, inst->num_nodes), 1.0);
        if (status)  print_error("An error occured in filling constraint y(1, j)");
        status = CPXchgcoef(env, lp, num_rows, x_dir_pos(0, j, inst->num_nodes), - (inst->num_nodes - 1));
        if (status)  print_error("An error occured in filling constraint x(1, j)");
    }

    k = 1;
    sense = 'L';
    // yij - (N - 2) * xij <= 0
    rhs = 0;
    for (int i = 1; i < inst->num_nodes; i++) {
        for (int j = 1; j < inst->num_nodes; j++) {
            if (i == j) continue;
            sprintf(names, "linkingconstraint(%d)", k++);

            int num_rows = CPXgetnumrows(env, lp);
            int status = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, &names);

            status = CPXchgcoef(env, lp, num_rows, y_pos(i, j, inst->num_nodes), 1.0);
            if (status)  print_error("An error occured in filling constraint y(i, j)");
            status = CPXchgcoef(env, lp, num_rows, x_dir_pos(i, j, inst->num_nodes), - (inst->num_nodes - 2));
            if (status)  print_error("An error occured in filling constraint x(i, j)");
        }
    }
    free(names);
}