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

    if (inst.params.method.use_cplex) {
        TSP_opt(&inst);
    } else {
        TSP_heuc(&inst);
    }
    
    
    free_instance(&inst);
    return 0;
}
