/**
 *  Utility functions and data types 
 */
#ifndef UTILITY_H

#define UTILITY_H


#define VERSION "TSP 0.2.1"


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

double dmax(double d1, double d2);

/**
 * Transforms the indexes (i, j) to a scalar index k for
 * undirecred graph;
 * (i, j) -> k where k is the position index in an array
 * of the edge that connects togheter node i and node j.
 * 
 */
int x_udir_pos(int i, int j, int num_nodes);


/**
 * Transforms the indexes (i, j) to a scalar index k for
 * directed graph;
 * (i, j) -> k where k is the position index in an array
 * of the edge that connects togheter node i and node j.
 * 
 */
int x_dir_pos(int i, int j, int num_nodes);

// Parses the input from the comand line
void parse_comand_line(int argc, const char *argv[], instance *inst);

void free_instance(instance *inst);

// Parses the problem data
void parse_instance(instance *inst);

void print_instance(instance inst);

#endif