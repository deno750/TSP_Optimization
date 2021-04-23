#include "mtz.h"

static int u_pos(int i, int num_nodes) {
    int n = num_nodes;
    return n * n + i;
}

static void add_u_variables(instance *inst, CPXENVptr env, CPXLPptr lp, char **names) {
    char xctype = 'I';
    for (int i = 0; i < inst->num_nodes; i++) {
        sprintf(*names, "u(%d)", i+1);

        // Variables treated as single value arrays.
        double obj = 0.0; 
        double lb = 0.0; 
        double ub = i == 0 ? 0.0 : inst->num_nodes - 2;

        int status = CPXnewcols(env, lp, 1, &obj, &lb, &ub, &xctype, names); 
        if (status) {
            LOG_E("CPXnewcols() error code %d", status);
        }
        int numcols = CPXgetnumcols(env, lp);
        if (numcols - 1 != u_pos(i, inst->num_nodes)) { // numcols -1 because we need the position index of the new variable
            LOG_E("Wrong position of variable in add_u_variable");
        }
    }
}

void add_mtz_constraints(instance *inst, CPXENVptr env, CPXLPptr lp, int secd2) {

    char* names = CALLOC(100, char);

    add_u_variables(inst, env, lp, &names);

    int BIG_M = inst->num_nodes - 1;
    char sense = 'L';
    

    int k = 1;
    
    // Mxij + ui - uj <= M - 1
    double rhs = BIG_M - 1.0;
    for (int i = 1; i < inst->num_nodes; i++) { // Excluding node 0
        for (int j = 1; j < inst->num_nodes; j++) { // Excluding node 0
            if (i == j) continue;
            sprintf(names, "constraint(%d)", k++);
            int num_rows = CPXgetnumrows(env, lp);

            int status = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, &names);
            if (status) {
                LOG_E("CPXnewrows() error code %d", status);
            }

            // Adding M*xij
            status = CPXchgcoef(env, lp, num_rows, x_dir_pos(i, j, inst->num_nodes), BIG_M);
            if (status)  LOG_E("An error occured in filling constraint x(i, j) big M. Error code %d", status);

            // Adding ui
            status = CPXchgcoef(env, lp, num_rows, u_pos(i, inst->num_nodes), 1);
            if (status)  LOG_E("An error occured in filling constraint u(i). Error code %d", status);
            
            // Adding -uj
            status = CPXchgcoef(env, lp, num_rows, u_pos(j, inst->num_nodes), -1);
            if (status)  LOG_E("An error occured in filling constraint u(j). Error code %d", status);
        }
    }

    //Adding: 1.0 * x_ij + 1.0 * x_ji <= 1
    if (secd2) {
        k = 1;
        rhs = 1.0;
        for (int i = 1; i < inst->num_nodes; i++) { // Excluding node 0
            for (int j = 1; j < inst->num_nodes; j++) { // Excluding node 0
                if (i == j) continue;
                sprintf(names, "ben(%d)", k++);
                int num_rows = CPXgetnumrows(env, lp);

                int status = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, &names);
                if (status) {
                    LOG_E("CPXnewrows() error code %d", status);
                }

                status = CPXchgcoef(env, lp, num_rows, x_dir_pos(i, j, inst->num_nodes), 1.0);
                if (status)  LOG_E("An error occured in filling constraint x(i, j). Error code %d", status);

                status = CPXchgcoef(env, lp, num_rows, x_dir_pos(j, i, inst->num_nodes), 1.0);
                if (status)  LOG_E("An error occured in filling constraint u(i). Error code %d", status);
            }
        }
    }
    

    free(names);
}

void add_mtz_lazy_constraints(instance *inst, CPXENVptr env, CPXLPptr lp, int secd2) {
    char* names = CALLOC(100, char);

    add_u_variables(inst, env, lp, &names);

	int izero = 0;
	int index[3]; 
	double value[3];

	// Add lazy constraints  1.0 * u_i - 1.0 * u_j + M * x_ij <= M - 1, for each arc (i,j) not touching node 0	
	double BIG_M = inst->num_nodes - 1.0;
	double rhs = BIG_M -1.0;
	char sense = 'L';
	int nnz = 3;
	for (int i = 1; i < inst->num_nodes; i++) { // excluding node 0
		for (int j = 1; j < inst->num_nodes; j++ ) { // excluding node 0 
			if ( i == j ) continue;
			sprintf(names, "uconsistency(%d,%d)", i+1, j+1);
			index[0] = u_pos(i,inst->num_nodes);	
			value[0] = 1.0;	
			index[1] = u_pos(j,inst->num_nodes);
			value[1] = -1.0;
			index[2] = x_dir_pos(i, j, inst->num_nodes);
			value[2] = BIG_M;
            int status = CPXaddlazyconstraints(env, lp, 1, nnz, &rhs, &sense, &izero, index, value, &names);
			if (status) LOG_E("wrong CPXlazyconstraints() for u-consistency. Error code %d", status);
		}
	}

    if (secd2) {
        //Adding: 1.0 * x_ij + 1.0 * x_ji <= 1
        int k = 1;
        rhs = 1.0;
        nnz = 2;
        for (int i = 1; i < inst->num_nodes; i++) { 
            for (int j = 1; j < inst->num_nodes; j++) {
                if (i == j) continue;
                sprintf(names, "ben(%d)", k++);
                index[0] = x_dir_pos(i, j, inst->num_nodes);	
                value[0] = 1.0;	
                index[1] = x_dir_pos(j, i, inst->num_nodes);
                value[1] = 1.0;
                int status = CPXaddlazyconstraints(env, lp, 1, nnz, &rhs, &sense, &izero, index, value, &names);
                if (status) LOG_E("wrong CPXlazyconstraints() for sec 2 degree. Error code %d", status);
            }
        }
    }
    
}

void add_mtz_indicator_constraints(instance *inst, CPXENVptr env, CPXLPptr lp) {
    char* names = CALLOC(100, char);

    add_u_variables(inst, env, lp, &names);
    double rhs = 1.0;
    char sense = 'G';
    int indexes[2];
    double vals[2] = {-1.0, 1.0}; // -ui + uj >= 1
    int k = 1;
    for (int i = 1; i < inst->num_nodes; i++) {
        for (int j = 1; j < inst->num_nodes; j++) {
            if (i == j) continue;
            sprintf(names, "indconst(%d)", k++);
            indexes[0] = u_pos(i, inst->num_nodes);
            indexes[1] = u_pos(j, inst->num_nodes);
            CPXaddindconstr(env, lp, x_dir_pos(i, j, inst->num_nodes), 0, 2, rhs, sense, indexes, vals, names);
        }
    }
}