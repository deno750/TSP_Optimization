/**
 *  Utility functions and data types 
 */
#ifndef UTILITY_H

#define UTILITY_H

#include <cplex.h>
#include <sys/time.h>
#include <string.h>


#define VERSION "TSP Optimization - 0.8"

#define MALLOC(nnum,type) ( (type *) malloc (nnum * sizeof(type)) )
#define CALLOC(nnum,type) ( (type *) calloc (nnum, sizeof(type)) )
#define REALLOC(ptr, nnum, type) ( realloc(ptr, nnum * sizeof(type)) )
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
#define URAND() ( ((double) rand()) / RAND_MAX )


// Constant that is useful for numerical errors
#define EPS 1e-5
#define DEFAULT_TIME_LIM 900


// ================ Weight types =====================
typedef enum {
    EUC_2D,     // weights are Euclidean distances in 2-D
    MAX_2D,     // weights are maximum distances in 2-D
    MAN_2D,     // weights are Manhattan distances in 2-D
    CEIL_2D,    // weights are Euclidean distances in 2-D rounded up
    GEO,        // weights are geographical distances
    ATT         // special distance function for problems att48 and att532 (pseudo-Euclidean)
} weight_type;

// =============== Solvers available ==================

typedef enum {
    SOLVE_MTZ,
    SOLVE_MTZL,
    SOLVE_MTZI,
    SOLVE_MTZLI,
    SOLVE_MTZ_IND,
    SOLVE_GG,
    SOLVE_LOOP,
    SOLVE_CALLBACK,     // Uses the callback before the updating of the incubement
    SOLVE_UCUT,         // Uses user cuts callback
    SOLVE_HARD_FIXING,  // Uses the hard fixing heuristic with fixed probability
    SOLVE_HARD_FIXING2, // Uses the hard fixing heuristic with decremental probability
    SOLVE_SOFT_FIXING,  // Uses the soft fixing heuristic
    SOLVE_GREEDY,       // Uses the greedy heuristic
    SOLVE_GREEDY_ITER,  // Uses the greedy heuristic with iterated starting node
    SOLVE_EXTR_MIL,     // Uses the extra mileage heuristic
    SOLVE_2OPT,         // Uses the 2 opt algorithm
    SOLVE_3OPT,         // Uses the 3 opt algorithm
    SOLVE_GRASP,        // Uses the GRASP algorithm
    SOLVE_GRASP_ITER,   // Uses the iterative GRASP algorithm
    SOLVE_GRASP_REF,    // Uses the GRASP and 2opt algorithm
    SOLVE_VNS,          // Uses the VNS local search algorithm
    SOLVE_TABU_STEP,    // Uses the Tabu search algorithm with step policy
    SOLVE_TABU_LIN,     // Uses the Tabu search algorithm with linear policy
    SOLVE_TABU_RAND,    // Uses the Tabu search algorithm with random policy
    SOLVE_GENETIC       // Uses the Genetic algorithm
} solver_type;


// The default solver definition
#define SOLVE_DEFAULT SOLVE_CALLBACK
// The default edge definition
#define DEFAULT_EDGE UDIR_EDGE
// The default method name definition
#define SOLVER_DEFAULT_NAME "INCUBEMENT CALLBACK"



// ================ Edge types =======================
typedef enum {
    UDIR_EDGE, // Undirected edge type
    DIR_EDGE // Directed edge type
} edge_type;


// ==================== STRUCTS ==========================

typedef struct {
    solver_type id;
    edge_type edge_type; // Describes if the graph is directed or undirected
    char* name;
    int use_cplex;
} sol_method;

// Struct which stores the parameters of the problem
typedef struct {
    char* file_path;
    int num_threads; 
    int time_limit;
    sol_method method;
    int verbose;        // Verbose level of debugging printing
    int integer_cost;
    int seed;           // Seed for random generation
    int perf_prof;      // Need to know wheter the computation is executed for performance profile
    int callback_2opt;  // Used in incubement callbacks for 2opt refinement
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
double obj_best;            // Stores the best value of the objective function
    edge *edges;            // List the solution's edges: list of pairs (i,j)
    double time_to_solve;   // Time used to solve the istance
    double *xbest;          // The best solution found in heuristics implementations
} solution;

// Instance data structure where all the information of the problem are stored
typedef struct {
    instance_params params;

    char *name;
    char *comment;
    point *nodes;
    int num_nodes;
    weight_type weight_type;
    long num_columns;           // The number of variables. It is used in callback method
    int* ind;                   // List of the indices of solution values in cplex. Needed for updating manually the incubement in cplex. Used in callbacks
    unsigned int* thread_seeds; // An array which contains the seed for each thread. Used in relaxation callback to create a randomness

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

/**
 * Counts the number of subtours in the graph and returns the edges which close the cycles for each subtour
 * 
 * @param inst The instance pointer of the problem
 * @param xtar The solution returned by cplex as form of array
 * @param successors An array whose the successor of each node is stored. 
 * The size of this array must be equal to the number of nodes in the problem (i.e. inst->num_nodes).
 * @param comp An array whose the subtour which belongs each node is stored.
 * The size of this array must be equal to the number of nodes in the problem (i.e. inst->num_nodes).
 * @param close_cycle_edges An array which stores the edges which close the cycles for each subtour.
 * @param num_closed_cycles The number closing cycle edges.
 * @returns The number of subtorus. The returned value must be >= 1. If not, an error occured
 */
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

/**
 * Reverses the path from a chosen starting node and end node. Useful for 2-opt
 * 
 * @param inst The instance pointer of the problem
 * @param start_node The starting node of the reversed path
 * @param end_node The end node of the reversed path 
 * @param prev An array which stores the previous node of each node (i.e. like a successors array)
 */
void reverse_path(instance *inst, int start_node, int end_node, int *prev);

/**
 * Copies the src instance to dst instance. Parameter like name, comment etc which are not useful
 * for the problem solution are setted to NULL. The copied instance is used to make calculations in multi-threaded
 * method such as callbacks etc and where a calculation for a solution in a specific method should not touch any field
 * of the original instance such as the current objective value and solution
 * 
 * @param dst The destination instance of the problem
 * @param src The source instance of the problem
 */
void copy_instance(instance *dst, instance *src);

#endif