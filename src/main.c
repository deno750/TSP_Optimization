//
//  main.c
//  TSP_Optimization
//
//

#include <stdio.h>
#include <cplex.h>
#include "utility.h"

int main(int argc, const char * argv[]) {
    printf("Hello World!\n");
    
    input_data input_data;
    parse_comand_line(argc, argv, &input_data);
    parse_model_instance(input_data);
    return 0;
}
