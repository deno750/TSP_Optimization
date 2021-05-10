/**
 *  Utility functions and data types 
 */
#ifndef UTILITY_H

#define UTILITY_H

#include <cplex.h>
#include <sys/time.h>
#include <string.h>


#define VERSION "TSP 0.4"

#define MALLOC(nnum,type) ( (type *) malloc (nnum * sizeof(type)) )
#define CALLOC(nnum,type) ( (type *) calloc (nnum, sizeof(type)) )
#define MEMSET(ptr,defval,nnum,type) {                  \
    type *new_ptr = (type*) ptr;                        \
    type newval = (type) defval;                        \
    if (newval == ((type)0) || newval == ((type)-1)) {  \
        memset(new_ptr, newval, nnum * sizeof(type));   \
    } else {                                            \
        for (int i = 0; i < nnum; i++) {                \
            new_ptr[i] = newval;                        \
        }                                               \
    }                                                   \
}               

#define DEBUG // Comment when the debugging logs are not needed

#ifdef DEBUG
#define LOG_D(fmt, ...) {fprintf(stdout, "[DEBUG] ");fprintf(stdout, fmt, ## __VA_ARGS__);fprintf(stdout, "\n");}
#else
#define LOG_D(fmt, ...) // An empty macro
#endif
#define LOG_I(fmt, ...) {fprintf(stdout, "[INFO]  ");fprintf(stdout, fmt, ## __VA_ARGS__);fprintf(stdout, "\n");}
#define LOG_E(fmt, ...) {fprintf(stderr, "[ERROR] ");fprintf(stderr, fmt, ## __VA_ARGS__);fprintf(stdout, "\n");fflush(NULL);exit(1);}
#define FREE(ptr) free(ptr); ptr=NULL;
#define LEN(arr) (sizeof(arr) / sizeof(*arr))


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
#define SOLVE_MTZ               100
#define SOLVE_MTZL              101
#define SOLVE_MTZI              102
#define SOLVE_MTZLI             103
#define SOLVE_MTZ_IND           104
#define SOLVE_GG                105
#define SOLVE_LOOP              106
#define SOLVE_CALLBACK          107 // Uses the callback before the updating of the incubement
#define SOLVE_UCUT              108 // Uses user cuts callback
#define SOLVE_HARD_FIXING       109 // Uses the hard fixing heuristic with fixed probability
#define SOLVE_HARD_FIXING2      110 // Uses the hard fixing heuristic with decremental probability
#define SOLVE_SOFT_FIXING       111 // Uses the hard fixing heuristic
#define SOLVE_GREEDY            112 // Uses the greedy heuristic



// The default solver definition
#define SOLVE_DEFAULT SOLVE_CALLBACK
// The default edge definition
#define DEFAULT_EDGE UDIR_EDGE
// The default method name definition
#define SOLVER_DEFAULT_NAME "INCUBEMENT CALLBACK"



// ================ Edge types =======================
#define UDIR_EDGE 0
#define DIR_EDGE 1


// ==================== STRUCTS ==========================

typedef struct {
    int id;
    int edge_type; // Describes if the graph is directed or undirected
    char* name;
    int use_cplex;
} sol_method;

// Struct which stores the parameters of the problem
typedef struct {
    char* file_path;
    int num_threads; 
    int time_limit;
    sol_method method;
    int verbose; // Verbose level of debugging printing
    int integer_cost;
    int seed; // Seed for random generation
    int perf_prof; // Need to know wheter the computation is executed for performance profile
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
    double *xbest; // The best solution found in heuristics implementations
} solution;

// Instance data structure where all the information of the problem are stored
typedef struct {
    instance_params params;

    char *name;
    char *comment;
    point *nodes;
    int num_nodes;
    int weight_type;
    int num_columns; // The number of variables. It is used in callback method

    solution solution;
} instance;

// ===================== FUNCTIONS =============================

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
 * @param name The file name
 */
void save_lp(CPXENVptr env, CPXLPptr lp, char *name);

/**
 * Counts the number of subtours in the graph
 * 
 * @param inst The instance pointer of the problem
 * @param xtar The solution returned by cplex as form of array
 * @param successors An array whose the successor of each node is stored. 
 * The size of this array must be equal to the number of nodes in the problem (i.e. inst->num_nodes).
 * @param comp An array whose the subtour which belongs each node is stored.
 * The size of this array must be equal to the number of nodes in the problem (i.e. inst->num_nodes).
 * @returns The number of subtorus. The returned value must be >= 1. If not, an error occured
 */
int count_components(instance *inst, double* xstar, int* successors, int* comp);

int count_components_adv(instance *inst, double* xstar, int* successors, int* comp, edge* close_cycle_edges, int* num_closed_cycles);

/**
 * Prepares the passed parameters to be comfortable with SEC constraints for functions CPXaddnewrows and CPXcallbackrejectcandidate
 * 
 * @param inst The instance pointer of the problem
 * @param tour The subtour considered for adding the SEC constraint
 * @param comp An array whose the subtour which belongs each node is stored. This array can be obtained from count_components function
 * @param sense The cplex parameter "sense". In this case this function will set sense to "L"
 * @param indexes The cplex's indexes of the variables which are considered in the selected subtour are returned from this function
 * @param values The value of the variables in the SEC constraint are returned from this function
 * @param rhs The righten side |S| - 1 of SEC constraints
 * @returns The number of variable to add in the constraint.
 */
int prepare_SEC(instance *inst, int tour, int *comp, char *sense, int *indexes, double *values, double *rhs);

/**
 * Tells cplex to store the logs file during the execution of the problem. The logs are named accordignly with the instance name and stored in logs folder
 * 
 * @param env The cplex's environment
 * @param inst The instance pointer of the problem
 */
void save_cplex_log(CPXENVptr env, instance *inst);

/**
 * Helpe function which calculates the elapsed time between passed times
 * 
 * @param start The time val struct of the starting time of the measurement
 * @param start The time val struct of the ending time of the measurement
 */
double get_elapsed_time(struct timeval start, struct timeval end);

#endif