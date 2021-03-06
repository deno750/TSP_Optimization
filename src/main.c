//
//  main.c
//  TSP_Optimization
//

#include <stdio.h>
#include <cplex.h>
#include <stdlib.h>
#include <string.h>


//////////////////////////////////////////////////////
//////////// DATA STRUCTURES /////////////////////////
//////////////////////////////////////////////////////
typedef struct {   
	
	//input data
	int nnodes; 	    // number of nodes
    int weight_type; //type of weight between edges (maybe pointer to some function?)
    /**for symmetric travelling salesman problems:
     *  0 = EUC_2D       : weights are Euclidean distances in 2-D
     *  1 = MAX_2D       : weights are maximum distances in 2-D
     *  2 = MAN_2D       : weights are Manhattan distances in 2-D
     *  3 = CEIL_2D      : weights are Euclidean distances in 2-D rounded up
     *  4 = GEO          : weights are geographical distances
     *  5 = ATT          : special distance function for problems att48 and att532 (pseudo-Euclidean)
     */
	double *xcoord;
	double *ycoord;

    //file informations
    char *filename;
    char *name;
    
} instance;



//////////////////////////////////////////////////////
//////////// FUNCTIONS SIGNATURE /////////////////////
//////////////////////////////////////////////////////
void read_input(instance *inst);
void parse_command_line(int argc, char **argv, instance *inst);

void print_error(const char *err) { printf("\n\n ERROR: %s \n\n", err); fflush(NULL); exit(1); } 
void free_instance(instance *inst){
    free(inst->xcoord);
    free(inst->ycoord);
    // aggiungere altro??????
}

//////////////////////////////////////////////////////
//////////// MAIN ////////////////////////////////////
//////////////////////////////////////////////////////
int main(int argc, const char *argv[])
{
    printf("Hello World!\n");
    printf("prova\n");
    printf("prova2\n");

    char file_path[] = "../data/att48.tsp";

    instance inst;

    inst.filename=file_path;

    //parse_command_line(argc,argv, &inst);

    read_input(&inst);

    printf("name: %s\n",inst.name);
    printf("n nodes %d\n",inst.nnodes);
    printf("weight type %d\n",inst.weight_type);
    for (int i = 0; i < inst.nnodes; i++)
    {
        printf("node %d: %f, %f\n",i,inst.xcoord[i],inst.ycoord[i]);
    }
    
    printf("");
    

    free_instance(&inst);
    return 0;
}

//////////////////////////////////////////////////////
//////////// FUNCTIONS BODY //////////////////////////
//////////////////////////////////////////////////////

/////////// PARSE COMMAND LINE ///////////////////////
// TODO



////////// LOAD DATA /////////////////////////////////
void read_input(instance *inst){
    inst->nnodes = -1;

    // Open file
    FILE *fp = fopen(inst->filename, "r");
    if(fp == NULL) print_error("Unable to open file!");
   
    char line[128];         // 1 line of the file
    char *par_name;         // name of the parameter in the readed line
    char *token1;           // value of the parameter in the readed line
    char *token2;           // second value of the parameter in the readed line (used for reading coordinates)
    int active_section = 0; // 0=reading parameters, 1=NODE_COORD_SECTION, 2=EDGE_WEIGHT_SECTION
    char sep[] = ": ";      // separator for parsing

    // Read the file line by line
    while(fgets(line, sizeof(line), fp) != NULL) {
        if (strlen(line) <= 1 ) continue; // skip empty lines
        printf(line);

        par_name = strtok(line, sep);

        if(strncmp(par_name, "NAME", 4) == 0){
			active_section = 0;
            token1 = strtok(NULL, sep);
            inst->name=calloc(strlen(token1)-1, 1);         //-1 beacuse the "\n"
            strncpy(inst->name,token1,strlen(token1)-1);
			continue;
		}

		if(strncmp(par_name, "COMMENT", 7) == 0){
			active_section = 0;   
			token1 = strtok(NULL, sep);
            //We don't do nithing with this parameter because we don't care about the comment  
			continue;
		}   

        if(strncmp(par_name, "TYPE", 4) == 0){
            token1 = strtok(NULL, sep);  
            if(strncmp(token1, "TSP",3) != 0) print_error(" format error:  only TYPE == TSP implemented so far!!!!!!");
            active_section = 0;
            continue;
		}

        if(strncmp(par_name, "DIMENSION", 9) == 0 ){
            token1 = strtok(NULL, " :");
            inst->nnodes = atoi(token1);
            inst->xcoord = (double *) calloc(inst->nnodes, sizeof(double)); 	 
            inst->ycoord = (double *) calloc(inst->nnodes, sizeof(double));    
            active_section = 0;  
            continue;
		}

        if(strncmp(par_name, "EOF", 3) == 0 ){
			active_section = 0;
			break;
		}

        if(strncmp(par_name, "EDGE_WEIGHT_TYPE", 16) == 0 ){
            token1 = strtok(NULL, " :");
            if (strncmp(token1, "EUC_2D", 6) == 0)inst->weight_type = 0;
            if (strncmp(token1, "MAX_2D", 6) == 0)inst->weight_type = 1;
            if (strncmp(token1, "MAN_2D", 6) == 0)inst->weight_type = 2;
            if (strncmp(token1, "CEIL_2D", 7) == 0)inst->weight_type = 3;
            if (strncmp(token1, "GEO", 3) == 0)inst->weight_type = 4;
            if (strncmp(token1, "ATT", 3) == 0)inst->weight_type = 5;
            if (strncmp(token1, "EXPLICIT", 8) == 0)print_error("Wrong edge weight type, this program resolve only 2D TSP case with coordinate type.");
            active_section = 0;  
            continue;
		}

        if (strncmp(par_name, "NODE_COORD_SECTION", 18) == 0){
            active_section=1;
            continue;
        }

        // NODE_COORD_SECTION
        if(active_section == 1){ 
            int i = atoi(par_name) - 1; 
			if ( i < 0 || i >= inst->nnodes ) print_error(" ... unknown node in NODE_COORD_SECTION section");     
			token1 = strtok(NULL, sep);
			token2 = strtok(NULL, sep);
			inst->xcoord[i] = atof(token1);
			inst->ycoord[i] = atof(token2);
            continue;
        }
    }

    // close file
    fclose(fp);

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
