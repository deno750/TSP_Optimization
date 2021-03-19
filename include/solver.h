#ifndef SOLVER_H

#define SOLVER_H

#include <cplex.h>
#include "utility.h"
#include "plot.h"
#include "distutil.h"


#define EPS 1e-5

static void build_udir_model(instance *inst, CPXENVptr env, CPXLPptr lp);
static void build_dir_model(instance *inst, CPXENVptr env, CPXLPptr lp);
static void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
static int x_udir_pos(int i, int j, int num_nodes);
static int x_dir_pos(int i, int j, int num_nodes);
static double calc_dist(int i, int j, instance *inst);
static int plot_solution(instance *inst, double *xstar);

int TSP_opt(instance *inst) {
    int error;
    CPXENVptr env = CPXopenCPLEX(&error);       // generate new environment, in err will be saved errors
    CPXLPptr lp = CPXcreateprob(env, &error, inst->name);   // create new empty linear programming problem (no variables, no constraints ...)
    
    // Build the model (add variable and constrains to the empty one)
    build_model(inst, env, lp);

    // Setting the time limit to cplex
    if (inst->params.time_limit >= 0) {
        double time_limit = inst->params.time_limit;
        CPXsetdblparam(env, CPXPARAM_TimeLimit, time_limit);
    }

    //Optimize the model (the solution is stored inside the env variable)
    int status = CPXmipopt(env, lp);
    if (status) {
        if (inst->params.verbose >= 5) {
            printf("Cplex error code: %d\n", status);
        }
        print_error("Cplex solver encountered an error.");
    }

    // Use the solution
    int ncols = CPXgetnumcols(env, lp);
	double *xstar = (double *) calloc(ncols, sizeof(double));
	if ( CPXgetx(env, lp, xstar, 0, ncols-1) ) print_error("CPXgetx() error");	
    if (inst->params.verbose >= 1) {

        printf("Optimal solution found!\n");

        // Next level of verbosity
        if (inst->params.verbose >= 2) {
            printf("\nThe optimal edges are:\n\n");

            // TODO: Plot using a new data structure that contains edges in order to generalize the plot for
            // undirected and directeg edges
            for ( int i = 0; i < inst->num_nodes; i++ ){
                for ( int j = i+1; j < inst->num_nodes; j++ ){
                    // Zero is considered when the absolute value of number is <= EPS. 
                    // One is considered when the absolute value of number is > EPS
                    if ( fabs(xstar[x_udir_pos(i,j,inst->num_nodes)]) > EPS ) printf("x(%3d,%3d) = 1\n", i+1,j+1);
                }
            }

        }

        printf("\n");

    }
	

    plot_solution(inst, xstar);

    free(xstar);

    //Free the problem and close cplex environment
    CPXfreeprob(env, &lp);
    CPXcloseCPLEX(&env);
    return error;
}

/**
 * Builds the model for a symmetric graph
 */
static void build_udir_model(instance *inst, CPXENVptr env, CPXLPptr lp) {
    char xctype = 'B';  // B=binary variable
    char *names = (char *) calloc(100, sizeof(char));

    // We add one variable at time. We may also add them all in a single shot. i<j
    for (int i = 0; i < inst->num_nodes; i++) {

        for (int j = i+1; j < inst->num_nodes; j++) {
            
            sprintf(names, "x(%d,%d)", i+1, j+1);

            // Variables treated as single value arrays.
            double obj = calc_dist(i, j, inst); 
            double lb = 0.0;
            double ub = 1.0;

            int status = CPXnewcols(env, lp, 1, &obj, &lb, &ub, &xctype, &names);
            if (status) {
                print_error("An error occured inserting a new variable");
            }
            int numcols = CPXgetnumcols(env, lp);
            if (numcols - 1 != x_udir_pos(i, j, inst->num_nodes)) { // numcols -1 because we need the position index of the new variable
                print_error("Wrong position of variable");
            }
        }
    }

    // Adding the constraints
    for (int h = 0; h < inst->num_nodes; h++) {
        double rhs = 2.0;
        char sense = 'E';
        sprintf(names, "degree(%d)", h+1);
        int status = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, &names);
        if (status) {
            print_error("An error occured inserting a new constraint");
        }

        for (int i = 0; i < inst->num_nodes; i++) {
            if (i == h) continue;

            status = CPXchgcoef(env, lp, h, x_udir_pos(h, i, inst->num_nodes), 1.0);
            if (status) {
                print_error("An error occured in filling a constraint");
            }
        }
        
    }

    //free(names[0]);
    free(names);
}







int xpos_mtz(int i, int j, instance *inst)
{
	if((i >= inst->num_nodes) || (j >= inst->num_nodes) || (i<0) || (j<0))
	{
		print_error("Domain contraint not respected");
	}
	return (i*inst->num_nodes + j);
}
/**
 * Builds the model for an asymmetric graph
 */
static void build_dir_model(instance *inst, CPXENVptr env, CPXLPptr lp) {
    char xctype = 'B';  // B=binary variable
    char *names = (char *) calloc(100, sizeof(char));
    /*
    // We add one variable at time. We may also add them all in a single shot. i<j
    for (int i = 0; i < inst->num_nodes; i++) {

        for (int j = 0; j < inst->num_nodes; j++) {
            sprintf(names, "x(%d,%d)", i+1, j+1);

            // Variables treated as single value arrays.
            double obj = calc_dist(i, j, inst); 
            double lb = 0.0; 
            double ub = i != j ? 1.0 : 0.0; // if i==j: ub=0 else ub=1

            int status = CPXnewcols(env, lp, 1, &obj, &lb, &ub, &xctype, &names); 
            if (status) {
                print_error("An error occured inserting a new variable");
            }
            int numcols = CPXgetnumcols(env, lp);
            if (numcols - 1 != x_dir_pos(i, j, inst->num_nodes)) { // numcols -1 because we need the position index of the new variable
                print_error("Wrong position of variable");
            }
        }
    }

    // Adding the x(i,j) constraints
    int deg = 0;
    double rhs = 1.0; // Asymmetric graph
    char sense = 'E';   
    // Adding constraint x12 + x13 + ... + xij + ... = 1 For each i
    for (int i = 0; i < inst->num_nodes; i++) {
        sprintf(names, "degree(%d)", deg + 1);
        int status = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, &names);
        if (status) {
            print_error("An error occured inserting a new constraint");
        }
        for (int j = 0; j < inst->num_nodes; j++) {
            if (i == j) continue;
            status = CPXchgcoef(env, lp, deg, x_dir_pos(i, j, inst->num_nodes), 1.0);
            if (status) {
                print_error("An error occured in filling a constraint");
            }
        }
        deg++;
    }

    // Adding constraint x21 + x31 + ... + xij + ... = 1 For each j
    for (int j = 0; j < inst->num_nodes; j++) {
        sprintf(names, "degree(%d)", deg + 1);
        int status = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, &names);
        if (status) {
            print_error("An error occured inserting a new constraint");
        }
        for (int i = 0; i < inst->num_nodes; i++) {
            if (i == j) continue;
            status = CPXchgcoef(env, lp, deg, x_dir_pos(i, j, inst->num_nodes), 1.0);
            if (status) {
                print_error("An error occured in filling a constraint");
            }
        }
        deg++;
    }*/


    printf("solving with MTZ\n");


    // MTZ

    int M = inst->num_nodes-1;
	double obj;
	double lb, ub;				// lower bound and upper bound
	char binary = 'B';
	char general = 'I';

    // Adding x_i_j variables
	for(int i=0; i<inst->num_nodes; i++)
	{
		for(int j=0; j<inst->num_nodes; j++)
		{
			obj = (i==j)? 0.0 : calc_dist(i,j,inst);
			lb = 0.0;
			ub = (i==j)? 0.0 : 1.0;
			
			if(CPXnewcols(env, lp, 1, &obj, &lb, &ub, &binary, &names)) 
			{
				print_error(" wrong CPXnewcols on x var.s");
			}
			if(CPXgetnumcols(env,lp)-1 != xpos_mtz(i,j, inst))
			{
				print_error(" wrong position for x var.s");
			}
		}
	}

    // Adding u_i variables
	for(int i=0; i<inst->num_nodes; i++)
	{
		obj = 0.0; 		// since the u_i variables don't have to appear in the objective function
		lb = (i==0)? 1.0 : 2.0;  // as from the article MTZ
		ub = (i==0)? 1.0 : inst->num_nodes;
		if(CPXnewcols(env, lp, 1, &obj, &lb, &ub, &general, &names)) 
		{
			print_error(" wrong CPXnewcols on u var.s");
		}
	}

    // Adding in_degree constraints (summation over i of x_i_h = 1)
	for(int h=0; h<inst->num_nodes; h++)
	{
		int lastrow = CPXgetnumrows(env, lp);
		double rhs = 1.0;
		char sense = 'E';
		if(CPXnewrows(env, lp, 1, &rhs, &sense, NULL, &names)) 
		{
			print_error(" wrong CPXnewrows [x1]");
		}
		for(int i=0; i < inst->num_nodes; i++)
		{
			if(i == h) { continue; }
			else
			{
				if(CPXchgcoef(env, lp, lastrow, xpos_mtz(i, h, inst), 1.0)) 
				{
					print_error(" wrong CPXchgcoef [x1]");
				}
			}
		}
	}

    // Adding out degree constraints (summation over j of x_h_j = 1)
	for(int h=0; h<inst->num_nodes; h++)
	{
		int lastrow = CPXgetnumrows(env, lp);
		double rhs = 1.0;
		char sense = 'E';
		if(CPXnewrows(env, lp, 1, &rhs, &sense, NULL, &names)) 
		{
			print_error(" wrong CPXnewrows [x1]");
		}
		for(int j=0; j < inst->num_nodes; j++)
		{
			if(j == h) { continue; }
			else
			{
				if(CPXchgcoef(env, lp, lastrow, xpos_mtz(h, j, inst), 1.0)) 
				{
					print_error(" wrong CPXchgcoef [x1]");
				}
			}
		}
	}

    // Adding lazy constraints ( x_i_j + x_j_i <= 1 )
	for(int i=0; i<inst->num_nodes; i++)
	{
		for(int j=i+1; j<inst->num_nodes; j++)
		{
			double rhs = 1.0;			// right hand side
			char sense = 'L';
			int rcnt = 1;				// number of lazy constraint to add
			int nzcnt = 2;				// number of non-zero variables in the constraint
			double rmatval[] = {1.0, 1.0};		// coefficient of the non-zero variables
			// position of the variables to set (in terms of columns)
			int rmatind[] = { xpos_mtz(i,j,inst), xpos_mtz(j,i,inst) };
			//int rmatbeg[] = { 0, 2 };
			int rmatbeg = 0;			// start positions of the constraint
			
			if(CPXaddlazyconstraints(env, lp, rcnt, nzcnt, &rhs, &sense, &rmatbeg, rmatind, rmatval, &names)) 
			{
				print_error(" wrong lazy constraint x_i_j + x_j_i <= 1");
			}
		}
	}

    // Adding big-M lazy constraints ( M*x_i_j + u_i - u_j <= M-1 )
    for (int i = 0; i < inst->num_nodes; i++) { 
        if(i==0)
		{
			double rhs = 1.0;
			char sense = 'E';
			int rcnt = 1;
			int nzcnt = 1;
			double rmatval = 1.0;
			int rmatind = xpos_mtz(inst->num_nodes-1, inst->num_nodes-1, inst)+1;
			int rmatbeg = 0;
			//sprintf(names[0], "lazy_cost(u_1)");
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
				int rmatind[] = {num_x_var+i, num_x_var+j, xpos_mtz(i,j,inst)};
				int rmatbeg = 0;								// start positions of the constraint
				//sprintf(names[0], "lazy_const_u(%d,%d)", i+1, j+1);
				if(CPXaddlazyconstraints(env, lp, rcnt, nzcnt, &rhs, &sense, &rmatbeg, rmatind, rmatval, &names)) 
				{
					print_error(" wrong lazy M*x_i_j + u_i - u_j <= M-1");
				}
			}
		}

    }


    // Constraints: u1=1
    //...


    //free(names[0]);
    free(names);
}


/**
 * Builds the model 
 */
static void build_model(instance *inst, CPXENVptr env, CPXLPptr lp) {

    // Checks the type of the edge in order to
    // build the correct model
    if (inst->params.type == UDIR_EDGE) {
        build_udir_model(inst, env, lp);
    } else {
        build_dir_model(inst, env, lp);
    }
    

    // Saving the model in .lp file
    char modelPath[1024];
    sprintf(modelPath, "../model/%s.lp", inst->name);
    
    CPXwriteprob(env, lp, modelPath, NULL);

}

/**
 * Transforms the indexes (i, j) to a scalar index k for
 * undirecred graph;
 * (i, j) -> k where k is the position index in an array
 * of the edge that connects togheter node i and node j.
 * 
 */
static int x_udir_pos(int i, int j, int num_nodes) {
    if (i == j) print_error("Indexes passed are equal!");
    if (i > num_nodes - 1 || j > num_nodes -1 ) {
        print_error("Indexes passed greater than the number of nodes");
    }
    // Since the problem has undirected edges that connects two nodes,
    // if we have i > j means that we have already the edge that connects j
    // to i. (i.e. we have already edge (j, i) so we switch index j with index i
    // to obtain that edge)
    if (i > j) return x_udir_pos(j, i, num_nodes);  
    return i * num_nodes + j - ((i + 1) * (i + 2)) / 2;
}




/**
 * Transforms the indexes (i, j) to a scalar index k for
 * directed graph;
 * (i, j) -> k where k is the position index in an array
 * of the edge that connects togheter node i and node j.
 * 
 */
static int x_dir_pos(int i, int j, int num_nodes) {
    //if (i == j) { print_error("Indexes passed are equal!"); }
    if (i > num_nodes - 1 || j > num_nodes -1 ) {
        print_error("Indexes passed greater than the number of nodes");
    }
    return i * num_nodes + j;
}

/**
 * Calculating the distance based on the instance's weight_type
 */ 
static double calc_dist(int i, int j, instance *inst) {
    point node1 = inst->nodes[i];
    point node2 = inst->nodes[j];
    int integer = inst->params.integer_cost;
    if (inst->weight_type == EUC_2D) {
        return calc_euc2d(node1, node2, integer);
    } else if (inst->weight_type == ATT) {
        return calc_pseudo_euc(node1, node2, integer);
    } else if (inst->weight_type == MAN_2D) {
        return calc_man2d(node1, node2, integer);
    } else if (inst->weight_type == MAX_2D) {
        return calc_max2d(node1, node2, integer);
    } else if (inst->weight_type == CEIL_2D) {
        return calc_ceil2d(node1, node2);
    } else if (inst->weight_type == GEO) {
        return calc_geo(node1, node2, integer);
    }
    // Default: euclidian distance. Should be ok for most problems
    return calc_euc2d(node1, node2, integer);
}

/**
 * Plots the solution.
 * 
 * Returns 0 when no errors, 1 otherwise.
 */
static int plot_solution(instance *inst, double *xstar) {
    PLOT gnuplotPipe = plot_open();
    if (gnuplotPipe == NULL) {
        printf("GnuPlot is not installed. Make sure that you have installed GnuPlot in your system and it's added in your PATH");
        return 1;
    }
    plot_in_file(gnuplotPipe, inst->name);
    add_plot_param(gnuplotPipe, "plot '-' using 1:2 w linespoints pt 7");

    for (int i = 0; i < inst->num_nodes; i++) {
        for (int j = i+1; j < inst->num_nodes; j++) {
            if (fabs(xstar[x_udir_pos(i,j,inst->num_nodes)]) > EPS) {
                plot_edge(gnuplotPipe, inst->nodes[i], inst->nodes[j]);
            }
        }
    }
    
    plot_end_input(gnuplotPipe);

    plot_free(gnuplotPipe);

    return 0;
}

#endif