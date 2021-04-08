#ifndef CALLBACK_H
#define CALLBACK_H

#include <cplex.h>

int SEC_cuts_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle );

#endif