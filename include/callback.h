#ifndef CALLBACK_H
#define CALLBACK_H

#include <cplex.h>
#include "utility.h"

// A data structure that is useful for passing the context and instance in a concorde's callback function
typedef struct {
    CPXCALLBACKCONTEXTptr context;
    instance *inst;
} relaxation_callback_params;

/**
 * The callback function used by cplex for the callback cuts.
 * 
 * @param context Cplex's context. 
 * @param contextId The id which should be CPX_CALLBACKCONTEXT_CANDIDATE or CPX_CALLBACKCONTEXT_RELAXATION for callback method
 * @param userhandle The data structure retrieved in the callback. In this case instance
 * @return 0 when no errors occur on adding the cut, 1 otherwise
 */
int CPXPUBLIC SEC_cuts_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle );

#endif