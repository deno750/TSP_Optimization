#ifndef SOLVER_H

#define SOLVER_H

#include <cplex.h>
#include "utility.h"


#define EPS 1e-5

static void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);

int TSP_opt(instance *inst) {
    int error;
    CPXENVptr env = CPXopenCPLEX(&error);
    CPXLPptr lp = CPXcreateprob(env, &error, "TSP");
    
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

}

#endif