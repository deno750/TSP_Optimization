#ifndef CALLBACK_H
#define CALLBACK_H

#include <cplex.h>

/**
 * The callback function used by cplex for the callback cuts.
 * 
 * @param context Cplex's context. 
 * @param contextId The id which should be CPX_CALLBACKCONTEXT_CANDIDATE for callback method
 * @param userhandle The data structure retrieved in the callback. In this case instance
 * @return 0 when no errors occur on adding the cut
 */
int CPXPUBLIC SEC_cuts_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle );

#endif