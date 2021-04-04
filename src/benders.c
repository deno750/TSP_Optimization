#include "benders.h"

#include <stdlib.h>

static int count_subtours(instance *inst) {

    
    return 0;
}

int *mycomp;
int *comp;
double *best_sol;

int kruskal_sst(CPXENVptr env, CPXLPptr lp, instance *inst) {
    int c1, c2 = 0;
    int n_connected_comp = 0;
    int max = -1;
    mycomp = (int*)calloc(inst->num_nodes, sizeof(int));
    comp = (int *) calloc(inst->num_nodes, sizeof(int));

    //INITIALIZATION
    for (int i = 0; i < inst->num_nodes; i++) {
        comp[i] = i;
    }
    //CONNECTED COMPONENT'S UNION
    for (int i = 0; i < inst->num_nodes; i++) {
        for (int j = i + 1; j < inst->num_nodes; j++) {
            if (best_sol[x_udir_pos(i, j, inst->num_nodes)] > 0.5) {
                if (comp[i] != comp[j]) {
                    c1 = comp[i];
                    c2 = comp[j];
                }
                for (int v = 0; v < inst->num_nodes; v++) {
                    if (comp[v] == c2)
                        comp[v] = c1;
                }
                
            }

        }
    }

    for (int i = 0; i < inst->num_nodes; i++) {
        
        mycomp[comp[i]] = 1;

    }

    int n = 0;
    for (int i = 0; i < inst->num_nodes; i++) {
        if (mycomp[i]!=0) {
            n++;
        }

    }
    
    
    return n;
}



void add_SEC(CPXENVptr env, CPXLPptr lp, instance *inst) {
	int nnz = 0;
	double rhs = -1.0;
	char sense = 'L';
	int ncols = CPXgetnumcols(env, lp);
	int *index = (int *)malloc( ncols* sizeof(int));
	double *value = (double *)malloc(ncols * sizeof(double));
	int matbeg = 0;
	char **cname = (char **)calloc(1, sizeof(char *));							// (char **) required by cplex...
	cname[0] = (char *)calloc(100, sizeof(char));

	for(int h=0; h < inst->num_nodes; h++){
		if(mycomp[h]!=0){
			for (int i = 0; i < inst->num_nodes; i++) {
				if (comp[i] != h) continue;
				rhs++;
				sprintf(cname[0], "SEC(%d)", i);

				for (int j = i + 1; j < inst->num_nodes; j++) {
					if (comp[j] == h) {
						index[nnz] = x_udir_pos(i, j, inst->num_nodes);
						value[nnz] = 1;
						nnz++;
					}
				}
			}
			if (CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &matbeg, index, value, NULL, cname)) print_error("wrong CPXaddrow");
			
		}

	}
}

int benders_loop(instance *inst, CPXENVptr env, CPXLPptr lp) {
    
    //METODO LOOP
	int done = 0;
	while (!done) {
		if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");		//CPXmipopt to solve the model
		int ncols = CPXgetnumcols(env, lp);
		best_sol = (double *)calloc(ncols, sizeof(double));				//best objective solution
		if (CPXgetx(env, lp, best_sol, 0, ncols - 1)) print_error("no solution avaialable");
		if (kruskal_sst(env, lp, inst) == 1) {
			done = 1;
		}
		
		else {
			add_SEC(env,lp,inst);
			
		}
		
	}

	int ncols = CPXgetnumcols(env, lp);
	
    return 0;
}

/*int benders_loop(instance *inst, CPXENVptr env, CPXLPptr lp) {
    int subtours = count_subtours(inst);

    while (subtours) {

        for (int i = 0; i < subtours; i++) {

        }


        CPXmipopt(env, lp);
        subtours = count_subtours(inst);
    }
    return 0;
}*/