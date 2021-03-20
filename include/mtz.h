#ifndef MTZ_H
#define MTZ_H

#include <cplex.h>
#include "utility.h"

/**
 * In asymmetric graphs, we have n^2 constraint
 */
static int u_pos(int i, int num_nodes) {
    int n = num_nodes;
    return n * n + i;
}

void add_mtz_constraints(instance *inst, CPXENVptr env, CPXLPptr lp, int (*x_pos_ptr)(int, int, int)) {
    if (x_pos_ptr == NULL) {
        print_error("You cannot pass a NULL function pointer");
    }
    int BIG_M = inst->num_nodes - 1;
    char sense = 'L';
    char xctype = 'I';
    char* names = (char *) calloc(100, sizeof(char));
    
    for (int i = 0; i < inst->num_nodes; i++) {
        sprintf(names, "u(%d)", i+1);

        // Variables treated as single value arrays.
        double obj = 0.0; 
        double lb = 0.0; 
        double ub = i == 0 ? 0.0 : inst->num_nodes - 2;

        int status = CPXnewcols(env, lp, 1, &obj, &lb, &ub, &xctype, &names); 
        if (status) {
            print_error("An error occured inserting a new variable");
        }
        int numcols = CPXgetnumcols(env, lp);
        if (numcols - 1 != u_pos(i, inst->num_nodes)) { // numcols -1 because we need the position index of the new variable
            print_error("Wrong position of variable");
        }
    }

    int k = 1;
    
    // Mxij + ui - uj <= M - 1
    double rhs = BIG_M - 1.0;
    for (int i = 1; i < inst->num_nodes; i++) { // Excluding node 1
        for (int j = 1; j < inst->num_nodes; j++) { // Excluding node 1
            if (i == j) continue;
            sprintf(names, "inc(%d)", k++);
            int num_rows = CPXgetnumrows(env, lp);

            int status = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, &names);
            if (status) {
                print_error("Error vincle"); //TODO: Change error message
            }

            // Adding M*xij
            status = CPXchgcoef(env, lp, num_rows, (*x_pos_ptr)(i, j, inst->num_nodes), BIG_M);
            if (status)  print_error("An error occured in filling constraint x(i, j)");

            // Adding ui
            status = CPXchgcoef(env, lp, num_rows, u_pos(i, inst->num_nodes), 1);
            if (status)  print_error("An error occured in filling constraint u(i)");
            
            // Adding -uj
            status = CPXchgcoef(env, lp, num_rows, u_pos(j, inst->num_nodes), -1);
            if (status)  print_error("An error occured in filling constraint u(j)");
        }
    }


    free(names);
}



void add_mtz_lazy_constraints(instance *inst, CPXENVptr env, CPXLPptr lp, int (*x_pos_ptr)(int, int, int)) {

    int M = inst->num_nodes-1;
    char* names = (char *) calloc(100, sizeof(char));

    // Adding big-M lazy constraints ( M*x_i_j + u_i - u_j <= M-1 ) 
	for(int i=0; i<inst->num_nodes; i++)
	{	
		if(i==0)
		{
			double rhs = 1.0;
			char sense = 'E';
			int rcnt = 1;
			int nzcnt = 1;
			double rmatval = 1.0;
			int rmatind = (*x_pos_ptr)(inst->num_nodes-1, inst ->num_nodes-1, inst->num_nodes)+1;
			int rmatbeg = 0;
			
			if(CPXaddlazyconstraints(env, lp, rcnt, nzcnt, &rhs, &sense, &rmatbeg, &rmatind, &rmatval, &names)) 
			{
				print_error(" wrong lazy [u1]");
			}

		} 
		else
		{ 
			for(int j=1; j<inst->num_nodes; j++)
			{
				if(i==j) { continue; }
				int num_x_var = inst->num_nodes * inst->num_nodes; 	// == xpos_mtz(inst->nnodes-1, inst->nnodes-1, inst) + 1
				double rhs = (double) M - 1.0;					// right hand side
				char sense = 'L';
				int rcnt = 1;									// number of lazy constraint to add
				int nzcnt = 3;									// number of non-zero variables in the constraint
				double rmatval[] = {1.0, -1.0, (double) M};		// coefficient of the non-zero variables
				int rmatind[] = {num_x_var+i, num_x_var+j, (*x_pos_ptr)(i,j,inst->num_nodes)};
				int rmatbeg = 0;								// start positions of the constraint
				
				if(CPXaddlazyconstraints(env, lp, rcnt, nzcnt, &rhs, &sense, &rmatbeg, rmatind, rmatval, &names)) 
				{
					print_error(" wrong lazy M*x_i_j + u_i - u_j <= M-1");
				}
			}
		}
	}
}

#endif