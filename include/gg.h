#ifndef GG_H
#define GG_H

#include <cplex.h>
#include "utility.h"


int xpos_flow1(int i, int j, instance *inst) {
    return inst->num_nodes * i + j;
}

int ypos_flow1(int i, int j, instance *inst) {
    return (xpos_flow1(inst->num_nodes - 1, inst->num_nodes - 1, inst)) + 1 + inst->num_nodes * i + j;
}

void add_gg_constraints(instance *inst, CPXENVptr env, CPXLPptr lp, int (*x_pos_ptr)(int, int, int)) {
    if (x_pos_ptr == NULL) print_error("You cannot pass a NULL function pointer");

    char* names = (char *) calloc(100, sizeof(char));

    //inFlow-outFlow=1  for each h in V\{1}
    //...


    //0 <= yij <= n-2 for each i,j in V\{1}
    //...

    //yi1 = 0 for each i in V\{1}
    //...

    //y1j = (n-1)*x1j 
    //...



    //yij - 



    free(names);
}


#endif