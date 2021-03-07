//
//  main.c
//  TSP_Optimization
//

#include <stdio.h>
#include <cplex.h>
#include "utility.h"



//////////////////////////////////////////////////////
//////////// FUNCTIONS SIGNATURE /////////////////////
//////////////////////////////////////////////////////

//////////////////////////////////////////////////////
//////////// MAIN ////////////////////////////////////
//////////////////////////////////////////////////////
int main(int argc, const char *argv[])
{
    input_data input_data;
    parse_comand_line(argc, argv, &input_data);
    model inst;
    parse_model_instance(input_data, &inst);

    printf("name: %s\n", inst.name);
    printf("n° nodes %d\n", inst.num_nodes);
    printf("weight type %d\n", inst.weight_type);
    for (int i = 0; i < inst.num_nodes; i++)
    {
        point node = inst.nodes[i];
        printf("node %d: %f, %f\n", i, node.x, node.y);
    }
    
    printf("");
    

    free_input_data(&input_data);
    free_model(&inst);
    return 0;
}
