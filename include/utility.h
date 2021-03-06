/**
 *  Utility functions and data types 
 */
#ifndef UTILITY_H

#define UTILITY_H

#include <string.h>

#define VERSION "0.1"

// ==================== STRUCTS ==========================

// Struct which stores the data from input that are useful for the model
typedef struct {
    int model_type; // Is it useful?
    int num_threads; // Is going to be useful in future?
    char *model_path;
} input_data;

// Definition of Point
typedef struct {
    double x;
    double y;
} point;

// Model instance data where all the information of the model is stored
typedef struct {
    const char *name;
    const char *comment;
    int num_nodes;
    point *nodes;
} model;

// ===================== FUNCTIONS =============================
void parse_comand_line(int argc, const char *argv[], input_data *data) {

    data->model_type = 0;
    data->model_path = (char *) malloc(1); // This malloc could lead to a memory leak. 
    
    for (int i = 1; i < argc; i++) {
        if (strcmp("-f", argv[i]) == 0) {
            strcpy(data->model_path, argv[++i]);
            continue;
        }
        if (strcmp("--v", argv[i]) == 0 || strcmp("--version", argv[i]) == 0) { 
            printf("Version %s\n", VERSION); 
            continue;
        }
    }
}

void parse_model_instance(input_data input) {
    FILE *file = fopen(input.model_path, "r");
    if (file == NULL) {
        fprintf(stderr, "File not found! Please provide a valid file path\n");
        exit(1);
    }

    fclose(file);
}

#endif