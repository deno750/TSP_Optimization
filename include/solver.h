#ifndef SOLVER_H

#define SOLVER_H

#include <cplex.h>
#include "utility.h"

static void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);

int TSP_opt(instance *inst) {
    int error;
    CPXENVptr env = CPXopenCPLEX(&error);
    CPXLPptr lp = CPXcreateprob(env, &error, "TSP");
    
    build_model(inst, env, lp);

    CPXfreeprob(env, &lp);
    CPXcloseCPLEX(&env);
    return error;
}

static void build_model(instance *inst, CPXENVptr env, CPXLPptr lp) {

}

#endif