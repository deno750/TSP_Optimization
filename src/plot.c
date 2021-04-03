
#include <stdio.h>
#include <sys/stat.h>
#include "plot.h"

PLOT plot_open() {
    PLOT plot = popen("gnuplot -persistent", "w");
    return plot;
}

void add_plot_param(PLOT plot, const char* param) {
    if (param != NULL) {
        fprintf(plot, "%s\n", param);
    }
}

void plot_end_input(PLOT plot) {
    fprintf(plot, "e");
}

void plot_in_file(PLOT plot, const char* name) {
    mkdir("../plot", 0777); // Create the plot directory if does not exists
    fprintf(plot, "set terminal jpeg size 1024,768\n");
    fprintf(plot, "set output '../plot/%s.jpg'\n", name);
}

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

void plot_vector(PLOT plot, point i, point j) {
    fprintf(plot, "%lf %lf\n %lf %lf\n", i.x, i.y, j.x - i.x, j.y - i.y);
}

void plot_point(PLOT plot, point p) {
    fprintf(plot, "%lf %lf \n", p.x, p.y);
}

void plot_free(PLOT plot) {
    fflush(plot);
    pclose(plot);
}

