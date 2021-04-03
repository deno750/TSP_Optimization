#ifndef BENDERS_H
#define BENDERS_H

#include <cplex.h>
#include "utility.h"

int benders_loop(instance *inst, CPXENVptr env, CPXLPptr lp);

#endif