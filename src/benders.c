#include "benders.h"

#include <stdlib.h>
#include <string.h>

#include "utility.h"

/*
*ncomp = 0;
	for ( int i = 0; i < inst->nnodes; i++ )
	{
		succ[i] = -1;
		comp[i] = -1;
	}
	
	for ( int start = 0; start < inst->nnodes; start++ )
	{
		if ( comp[start] >= 0 ) continue;  // node "start" was already visited, just skip it

		// a new component is found
		(*ncomp)++;
		int i = start;
		int done = 0;
		while ( !done )  // go and visit the current component
		{
			comp[i] = *ncomp;
			done = 1;
			for ( int j = 0; j < inst->nnodes; j++ )
			{
				if ( i != j && xstar[xpos(i,j,inst)] > 0.5 && comp[j] == -1 ) // the edge [i,j] is selected in xstar and j was not visited before 
				{
					succ[i] = j;
					i = j;
					done = 0;
					break;
				}
			}
		}	
		succ[i] = start;  // last arc to close the cycle
		
		// go to the next component...
	}

*/

static int count_subtours(instance *inst, double* xstar) {
    int* successors = (int*) malloc(inst->num_nodes * sizeof(int)); //malloc faster than calloc
    memset(successors, -1, inst->num_nodes * sizeof(int)); // filling with -1
    int* comp = (int*) malloc(inst->num_nodes * sizeof(int));
    memset(comp, -1, inst->num_nodes * sizeof(int));


    for (int i = 0; i < inst->num_nodes; i++) {
        for (int j = 0; j < inst->num_nodes; j++) {

        }
    }




    free(successors);
    free(comp);
    
    return 0;
}

int benders_loop(instance *inst, CPXENVptr env, CPXLPptr lp) {
    /*int subtours = count_subtours(inst);

    while (subtours) {

        for (int i = 0; i < subtours; i++) {

        }


        CPXmipopt(env, lp);
        subtours = count_subtours(inst);
    }*/

    //count_subtours(inst);



    return 0;
}