#ifndef PLOT_H

#define PLOT_H

#include <stdio.h>
#include "utility.h"

typedef FILE* PLOT;

/**
 * Instantiates the plot pipe
 */
PLOT plot_open() {
    PLOT plot = popen("gnuplot -persistent", "w");
    return plot;
}

/**
 * Adds in the chart a new plot with specified params. 
 * For example someone needs to see two functions in the chart with different colors,
 * adds two plot params
 */
void add_plot_param(PLOT plot, const char* param) {
    if (param != NULL) {
        fprintf(plot, "%s\n", param);
    }
}

/**
 * Tells to gnuplot that the input data stream is ended
 */
void plot_end_input(PLOT plot) {
    fprintf(plot, "e");
}

/**
 * Tells the plot to draw in a file located in "plot" directory. 
 * When called, gnuplot will not show the drawing in a separated window,
 * only in the file.
 */
void plot_in_file(PLOT plot, const char* name) {
    fprintf(plot, "set terminal jpeg size 1024,768\n");
    fprintf(plot, "set output '../plot/%s.jpg'\n", name);
}

/**
 * Plot an edge between point i and point j
 */
void plot_edge(PLOT plot, point i, point j) {
    //The points should be in different blocks. An empty line between the blocks
    /**
     * x1 y1
     * x2 y2
     * 
     * x3 y3
     * x4 y4
     * 
     * x5 y5
     * x6 y6
     */

    // Plotting one block of two points
    fprintf(plot, "%lf %lf \n", i.x, i.y);
    fprintf(plot, "%lf %lf \n\n", j.x, j.y);
}

/**
 * Draws a point
 */
void plot_point(PLOT plot, point p) {
    fprintf(plot, "%lf %lf \n", p.x, p.y);
}

/**
 * Closes the plot pipe
 */
void plot_free(PLOT plot) {
    pclose(plot);
}


#endif