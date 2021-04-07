/**
 *  Utility functions and data types 
 */
#ifndef UTILITY_H

#define UTILITY_H

#include <cplex.h>


#define VERSION "TSP 0.2.1"

// Constant that is useful for numerical errors
#define EPS 1e-5


// ================ Weight types =====================
#define EUC_2D 0 // weights are Euclidean distances in 2-D
#define MAX_2D 1 // weights are maximum distances in 2-D
#define MAN_2D 2 // weights are Manhattan distances in 2-D
#define CEIL_2D 3 // weights are Euclidean distances in 2-D rounded up
#define GEO 4 // weights are geographical distances
#define ATT 5 // special distance function for problems att48 and att532 (pseudo-Euclidean)


// =============== Solvers available ==================
#define SOLVE_MTZ       100
#define SOLVE_MTZL      101
#define SOLVE_MTZI      102
#define SOLVE_MTZLI     103
#define SOLVE_MTZ_IND   104
#define SOLVE_GG        105
#define SOLVE_LOOP      106



// ================ Edge types =======================
#define UDIR_EDGE 0
#define DIR_EDGE 1


// ==================== STRUCTS ==========================

// Struct which stores the parameters of the problem
typedef struct {
    int type;  // Describes if the graph is directed or undirected
    int num_threads; 
    int time_limit;
    int verbose; // Verbose level of debugging printing
    int integer_cost;
    int sol_type;
    int seed; // Seed for random generation
    char *file_path;
} instance_params;

// Definition of Point
typedef struct {
    double x;
    double y;
} point;

// Edge that connects node i and node j.
// Directed edge: i -> j
// Undirected edge: i - j
typedef struct {
    int i; // Index of node i
    int j; // Index of node j
} edge;

typedef struct {
    double obj_best; // Stores the best value of the objective function
    edge *edges; // List the solution's edges
    double time_to_solve; 
} solution;

// Instance data structure where all the information of the problem are stored
typedef struct {
    instance_params params;

    int num_nodes;
    int weight_type;
    char *name;
    char *comment;
    point *nodes;

    solution solution;
} instance;

// ===================== FUNCTIONS =============================

// Error print function
void print_error(const char *err); 

/**
 * Calculates the maximum between two doubles
 *
 * @param d1 The first double value
 * @param d2 The second double value
 * @returns the maximum value
 */
double dmax(double d1, double d2);

/**
 * Transforms the indexes (i, j) to a scalar index k for
 * undirecred graph;
 * (i, j) -> k where k is the position index in an array
 * of the edge that connects togheter node i and node j.
 *
 * @param i The index of node i
 * @param j The index of node j
 * @param num_nodes The number of nodes in the graph
 * @returns the index mapped from (i,j)
 * 
 */
int x_udir_pos(int i, int j, int num_nodes);


/**
 * Transforms the indexes (i, j) to a scalar index k for
 * directed graph;
 * (i, j) -> k where k is the position index in an array
 * of the edge that connects togheter node i and node j.
 *
 * @param i The index of node i
 * @param j The index of node j
 * @param num_nodes The number of nodes in the graph
 * @returns the index mapped from (i,j)
 * 
 */
int x_dir_pos(int i, int j, int num_nodes);

/**
 * Parses the input from the comand line
 *
 * @param argc The argc from the main function
 * @param argv The argv from the main function
 * @param inst The instance pointer of the problem
 */ 
void parse_comand_line(int argc, const char *argv[], instance *inst);

/**
 * Deallocates an instance from memory
 *
 * @param inst The instance pointer of the problem to deallocate
 */
void free_instance(instance *inst);

/**
 * Parses the problem data
 *
 * @param inst The instance pointer of the problem
 */ 
void parse_instance(instance *inst);

/**
 * Prints the parameters of the instance
 *
 * @param inst The instance of the problem
 */ 
void print_instance(instance inst);

/**
 * Exports the found tour in a .tour file
 *
 * @param inst The instance pointer of the problem
 */
void export_tour(instance *inst);

/**
 * Stores the solution given by xstar into a list of edges. 
 * With the list of edges is much more easier to retrieve the
 * solution informaiton and helps to generalize better between
 * the undirected and directed graph solutions. 
 *
 * @param inst The instance pointer of the problem
 * @param xstar The solution values pointer
 */
void save_solution_edges(instance *inst, double *xstar);

/**
 * Plots the solution.
 * 
 * @param inst The instance pointer of the problem
 * @returns 0 when no errors, 1 otherwise.
 */
int plot_solution(instance *inst);


/**
 * Saves the model in a .lp file
 * 
 * @param env The cplex's environment
 * @param lp The cplex's problem object
 * @param name The name file
 */
void save_lp(CPXENVptr env, CPXLPptr lp, char *name);

#endif