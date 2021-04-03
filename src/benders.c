#include "benders.h"

#include <stdlib.h>

static int count_subtours(instance *inst) {

    
    return 0;
}

int benders_loop(instance *inst, CPXENVptr env, CPXLPptr lp) {
    int subtours = count_subtours(inst);

    while (subtours) {

        for (int i = 0; i < subtours; i++) {

        }


        CPXmipopt(env, lp);
        subtours = count_subtours(inst);
    }
    return 0;
}