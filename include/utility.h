/**
 *  Utility functions and data types 
 */
#ifndef UTILITY_H

#define UTILITY_H

#include <string.h>
#include <stdlib.h>
#include <errno.h>


#define VERSION "TSP 0.1"


// ================ Weight types =====================
#define EUC_2D 0 // weights are Euclidean distances in 2-D
#define MAX_2D 1 // weights are maximum distances in 2-D
#define MAN_2D 2 // weights are Manhattan distances in 2-D
#define CEIL_2D 3 // weights are Euclidean distances in 2-D rounded up
#define GEO 4 // weights are geographical distances
#define ATT 5 // special distance function for problems att48 and att532 (pseudo-Euclidean)


// ==================== STRUCTS ==========================

// Struct which stores the parameters of the problem
typedef struct {
    int type; 
    int num_threads; 
    int time_limit;
    int verbose; // Verbose level of debugging printing 
    char *file_path;
} instance_params;

// Definition of Point
typedef struct {
    double x;
    double y;
} point;

// Instance data structure where all the information of the problem are stored
typedef struct {
    instance_params params;

    int num_nodes;
    int weight_type;
    char *name;
    char *comment;
    point *nodes;
} instance;

// ===================== FUNCTIONS =============================

// Error print function
void print_error(const char *err) { printf("\n\n ERROR: %s \n\n", err); fflush(NULL); exit(1); } 

// Parses the input from the comand line
void parse_comand_line(int argc, const char *argv[], instance *inst) {

    if (argc <= 1) {
        printf("Type \"%s --help\" to see available comands\n", argv[0]);
        exit(1);
    }

    inst->params.type = 0;
    inst->params.time_limit = -1; //Default time limit value. -1 means no constraints in time limit 
    inst->params.num_threads = 1; //Default value is one thread
    inst->params.file_path = NULL;
    inst->params.verbose = 1; //Default verbose level of 1
    int need_help = 0;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp("-f", argv[i]) == 0) { 
            const char* path = argv[++i];
            inst->params.file_path = (char *) calloc(strlen(path), sizeof(char));
            strcpy(inst->params.file_path, path); 
            continue; 
        } // Input file
        if (strcmp("-t", argv[i]) == 0) { inst->params.time_limit = atoi(argv[++i]); continue; }
        if (strcmp("-threads", argv[i]) == 0) { inst->params.num_threads = atoi(argv[++i]); continue; }
        if (strcmp("-verbose", argv[i]) == 0) { inst->params.verbose = atoi(argv[++i]); continue; }
        if (strcmp("--v", argv[i]) == 0 || strcmp("--version", argv[i]) == 0) { printf("Version %s\n", VERSION); exit(0);} //Version of the software
        if (strcmp("--help", argv[i]) == 0) { need_help = 1; continue; } // For comands documentation
        need_help = 1;
    }

    // Print the functions available
    if (need_help) {
        printf("-f <file's path>          To pass the problem's path\n");
        printf("-t <time>                 The time limit\n");
        printf("-threads <num threads>    The number of threads to use\n");
        printf("-verbose <level>          The verbosity level of the printing\n");
        printf("--v, --version            Software's current version\n");
        exit(0);
    }
}

void free_instance(instance *inst) {
    free(inst->params.file_path);
    free(inst->name);
    free(inst->comment);
    free(inst->nodes);
}

// Parses the problem data
void parse_instance(instance *inst) {
    if (inst->params.file_path == NULL) { print_error("You didn't pass any file!"); }

    //Default values
    inst->num_nodes = -1;
    inst->weight_type = -1;

    // Open file
    FILE *fp = fopen(inst->params.file_path, "r");
    if(fp == NULL) { print_error("Unable to open file!"); }
   
    char line[128];          // 1 line of the file
    char *par_name;          // name of the parameter in the readed line
    char *token1;            // value of the parameter in the readed line
    char *token2;            // second value of the parameter in the readed line (used for reading coordinates)
    int active_section = 0;  // 0=reading parameters, 1=NODE_COORD_SECTION, 2=EDGE_WEIGHT_SECTION
    char sep[] = " :\n\t\r"; // separator for parsing

    // Read the file line by line
    while(fgets(line, sizeof(line), fp) != NULL) {
        if (strlen(line) <= 1 ) continue; // skip empty lines
        par_name = strtok(line, sep);

        if(strncmp(par_name, "NAME", 4) == 0){
			active_section = 0;
            token1 = strtok(NULL, sep);
            inst->name = (char *) calloc(strlen(token1), sizeof(char));         
            strncpy(inst->name,token1, strlen(token1));
			continue;
		}

		if(strncmp(par_name, "COMMENT", 7) == 0){
			active_section = 0;   
            //We don't do nothing with this parameter because we don't care about the comment  
			continue;
		}   

        if(strncmp(par_name, "TYPE", 4) == 0){
            token1 = strtok(NULL, sep);  
            if(strncmp(token1, "TSP", 3) != 0) print_error(" format error:  only TYPE == TSP implemented so far!!!!!!");
            active_section = 0;
            continue;
		}

        if(strncmp(par_name, "DIMENSION", 9) == 0 ){
            token1 = strtok(NULL, sep);
            inst->num_nodes = atoi(token1);
            inst->nodes = (point *) calloc(inst->num_nodes, sizeof(point));
            active_section = 0;  
            continue;
		}

        if(strncmp(par_name, "EOF", 3) == 0 ){
			active_section = 0;
			break;
		}

        if(strncmp(par_name, "EDGE_WEIGHT_TYPE", 16) == 0 ){
            token1 = strtok(NULL, sep);
            if (strncmp(token1, "EUC_2D", 6) == 0) inst->weight_type = EUC_2D;
            if (strncmp(token1, "MAX_2D", 6) == 0) inst->weight_type = MAX_2D;
            if (strncmp(token1, "MAN_2D", 6) == 0) inst->weight_type = MAN_2D;
            if (strncmp(token1, "CEIL_2D", 7) == 0) inst->weight_type = CEIL_2D;
            if (strncmp(token1, "GEO", 3) == 0) inst->weight_type = GEO;
            if (strncmp(token1, "ATT", 3) == 0) inst->weight_type = ATT;
            if (strncmp(token1, "EXPLICIT", 8) == 0) print_error("Wrong edge weight type, this program resolve only 2D TSP case with coordinate type.");
            active_section = 0;  
            continue;
		}

        if (strncmp(par_name, "NODE_COORD_SECTION", 18) == 0){
            active_section = 1;
            continue;
        }

        if (strncmp(par_name, "EDGE_WEIGHT_SECTION", 19) == 0){
            active_section = 2;
            continue;
        }

        // NODE_COORD_SECTION
        if(active_section == 1){ 
            int i = atoi(par_name) - 1; // Nodes in problem's file start from index 1
			if ( i < 0 || i >= inst->num_nodes) print_error(" ... unknown node in NODE_COORD_SECTION section");     
			token1 = strtok(NULL, sep);
			token2 = strtok(NULL, sep);
            point p = {atof(token1), atof(token2)};
			inst->nodes[i] = p;
            continue;
        }
        
        // EDGE_WEIGHT_SECTION
        if (active_section == 2) { // Are we going to use this??

            continue;
        }
    }

    // close file
    fclose(fp);
}

void print_instance(instance inst) {
    if (inst.params.verbose >= 1) {
        if (inst.params.verbose >= 2) {
            printf("\n");
            printf("Type: %d\n", inst.params.type);
            printf("Time Limit: %d\n", inst.params.time_limit);
            printf("Threads: %d\n", inst.params.num_threads);
            printf("Verbose: %d\n", inst.params.verbose);
            printf("File path: %s\n", inst.params.file_path);
            printf("\n");
        }
        
        printf("\n");
        printf("name: %s\n", inst.name);
        printf("n° nodes: %d\n", inst.num_nodes);
        const char* weight;

        switch (inst.weight_type) {
        case EUC_2D:
            weight = "EUC_2D";
            break;
        case MAX_2D:
            weight = "MAX_2D";
            break;
        case MAN_2D:
            weight = "MAN_2D";
            break;
        case CEIL_2D:
            weight = "CEIL_2D";
            break;
        case GEO:
            weight = "GEO";
            break;
        case ATT:
            weight = "ATT";
            break;
        default:
            weight = "UNKNOWN";
            break;
        }
        printf("weight type: %s\n", weight);
        if (inst.params.verbose >= 3) {
            for (int i = 0; i < inst.num_nodes; i++) {
                point node = inst.nodes[i];
                printf("node %d: %0.2f, %0.2f\n", i+1, node.x, node.y);
            }
        }
        
    
        printf("\n");
    }
}


/*
/////// FILE FORMAT ///////////
SPECIFICATION PART:
- NAME : <string>
- TYPE : <string>
    - TSP Data for a symmetric traveling salesman problem
    - ATSP Data for an asymmetric traveling salesman problem
    - SOP Data for a sequential ordering problem
    - HCP Hamiltonian cycle problem data
    - CVRP Capacitated vehicle routing problem data
    - TOUR A collection of tours
- COMMENT : <string>
- DIMENSION : <integer>   number of its nodes
- CAPACITY : <integer>   for CVRP problem NOT in TSP
- EDGE WEIGHT TYPE : <string>
  Specifies how the edge weights (or distances) are given. The values are
    - EXPLICIT  Weights are listed explicitly in the corresponding section
    - EUC 2D    Weights are Euclidean distances in 2-D
    - EUC 3D    Weights are Euclidean distances in 3-D
    - MAX 2D    Weights are maximum distances in 2-D
    - MAX 3D    Weights are maximum distances in 3-D
    - MAN 2D    Weights are Manhattan distances in 2-D
    - MAN 3D    Weights are Manhattan distances in 3-D
    - CEIL 2D   Weights are Euclidean distances in 2-D rounded up
    - GEO       Weights are geographical distances
    - ATT       Special distance function for problems att48 and att532
    - XRAY1     Special distance function for crystallography problems (Version 1)
    - XRAY2     Special distance function for crystallography problems (Version 2)
    - SPECIAL   There is a special distance function documented elsewhere
- EDGE WEIGHT FORMAT : <string>
  Describes the format of the edge weights if they are given explicitly. The values are
    - FUNCTION Weights are given by a function (see above)
    - FULL MATRIX Weights are given by a full matrix
    - UPPER ROW Upper triangular matrix (row-wise without diagonal entries)
    - LOWER ROW Lower triangular matrix (row-wise without diagonal entries)
    - UPPER DIAG ROW Upper triangular matrix (row-wise including diagonal entries)
    - LOWER DIAG ROW Lower triangular matrix (row-wise including diagonal entries)
    - UPPER COL Upper triangular matrix (column-wise without diagonal entries)
    - LOWER COL Lower triangular matrix (column-wise without diagonal entries)
    - UPPER DIAG COL Upper triangular matrix (column-wise including diagonal entries)
    - LOWER DIAG COL Lower triangular matrix (column-wise including diagonal entries)
- EDGE DATA FORMAT : <string>
  Describes the format in which the edges of a graph are given, if the graph is not complete.
    - EDGE LIST The graph is given by an edge list
    - ADJ LIST The graph is given as an adjacency list
- NODE COORD TYPE : <string>
  Specifies whether coordinates are associated with each node (which, for example may be used for either graphical display or distance computations).
    - TWOD COORDS Nodes are specified by coordinates in 2-D
    - THREED COORDS Nodes are specified by coordinates in 3-D
    - NO COORDS The nodes do not have associated coordinates
  The default value is NO COORDS
- DISPLAY DATA TYPE : <string>
  Specifies how a graphical display of the nodes can be obtained.
    - COORD DISPLAY Display is generated from the node coordinates
    - TWOD DISPLAY Explicit coordinates in 2-D are given
    - NO DISPLAY No graphical display is possible
  The default value is COORD DISPLAY if node coordinates are specified and NO DISPLAY otherwise.
- EOF
  Terminates the input data. This entry is optional.

///// DATA PART /////
- NODE COORD SECTION
    - if NODE COORD TYPE is TWOD COORDS: <integer> <real> <real>
    - if NODE COORD TYPE is THREED COORDS: <integer> <real> <real> <real>
    The integers give the number of the respective nodes.
    The real numbers give the associated coordinates.
- .........
*/

#endif