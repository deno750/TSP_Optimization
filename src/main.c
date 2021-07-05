//
//  main.c
//  TSP_Optimization
//

#include <stdio.h>
#include <cplex.h>
#include "utility.h"    //Structs and function used globally.
#include "solver.h"

//////////////////////////////////////////////////////
//////////// MAIN ////////////////////////////////////
//////////////////////////////////////////////////////
int main(int argc, const char *argv[])
{
    instance inst;                          // create an empty tsp istance
    parse_comand_line(argc, argv, &inst);   // Read the user commands
    parse_instance(&inst);                  // Read the TSP istance
    
    print_instance(inst);                   // Show the istance

    if (inst.params.method.use_cplex) {     // Solve using cplex
        TSP_opt(&inst);
    } else {                                // Solve using our heuristic methods
        TSP_heuc(&inst);
    }
    
    
    free_instance(&inst);
    return 0;
}
