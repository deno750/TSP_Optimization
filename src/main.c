//
//  main.c
//  TSP_Optimization
//

#include <stdio.h>
#include <cplex.h>
#include "utility.h"
#include "solver.h"

//////////////////////////////////////////////////////
//////////// MAIN ////////////////////////////////////
//////////////////////////////////////////////////////
int main(int argc, const char *argv[])
{
    instance inst;
    parse_comand_line(argc, argv, &inst);
    parse_instance(&inst);
    print_instance(inst);

    TSP_opt(&inst);
    
    free_instance(&inst);
    return 0;
}
