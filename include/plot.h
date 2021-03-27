#ifndef PLOT_H

#define PLOT_H

#include "utility.h"

typedef FILE* PLOT;

/**
 * Instantiates the plot pipe
 *
 * @returns the an instane of plot pipe
 */
PLOT plot_open();

/**
 * Adds in the chart a new plot with specified params. 
 * For example someone needs to see two functions in the chart with different colors,
 * adds two plot params
 *
 * @param plot The plot pipe
 * @param param The gnuplot's parameter string
 */
void add_plot_param(PLOT plot, const char* param);

/**
 * Tells to gnuplot that the input data stream is ended
 *
 * @param plot The plot pipe
 */
void plot_end_input(PLOT plot);

/**
 * Tells the plot to draw in a file located in "plot" directory. 
 * When called, gnuplot will not show the drawing in a separated window,
 * only in the file.
 *
 * @param plot The plot pipe
 * @param param The file name
 */
void plot_in_file(PLOT plot, const char* name);

/**
 * Plot an edge between point i and point j
 *
 * @param plot The plot pipe
 * @param i The index of node i
 * @param j The index of node j
 */
void plot_edge(PLOT plot, point i, point j);

/**
 * Plot a vector between point i and point j
 *
 * @param plot The plot pipe
 * @param i The index of node i
 * @param j The index of node j
 */
void plot_vector(PLOT plot, point i, point j);

/**
 * Draws a point
 *
 * @param plot The plot pipe
 * @param p The point to plot
 */
void plot_point(PLOT plot, point p);

/**
 * Closes the plot pipe
 *
 * @param plot The plot pipe to close
 */
void plot_free(PLOT plot);


#endif