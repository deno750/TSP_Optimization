#ifndef DISTUTIL_H
#define DISTUTIL_H

#include "utility.h"

#define PI 3.141592
#define EARTH_RAD 6378.388

/**
 * Rounds to the nearest integer value.
 * For negative numbers it may create some problems.
 * 
 * Some example behaviors:
 * 0.4 -> 0.0
 * 0.51 -> 1.0
 * -0.3 -> 0.0
 * -0.6 -> 0.0
 * -1.2 -> 0.0
 * -1.6 -> 1.0
 */
static double nint(double x);

/**
 * Rounds the number such as round function.
 * 
 * Use this instead of nint when you work with negative numbers.
 * 
 * Some examples:
 * 0.4 -> 0.0
 * 0.51 -> 1.0
 * -0.3 -> 0.0
 * -0.6 -> -1.0
 */
static double e_round(double x);

/**
 * Calculates the euclidean 2d distance between two points.
 * 
 * Integer param is passed when an integer distance have to be
 * computed.
 */
double calc_euc2d(point p1, point p2, int integer);

/**
 * Calculates the pseudo euclidean 2d distance between two points.
 * 
 * Integer param is passed when an integer distance have to be
 * computed.
 */
double calc_pseudo_euc(point p1, point p2, int integer);

/**
 * Calculates the manhattan 2d distance between two points.
 * 
 * Integer param is passed when an integer distance have to be
 * computed.
 */
double calc_man2d(point p1, point p2, int integer);

/**
 * Calculates the maximum 2d distance between two points.
 * 
 * Integer param is passed when an integer distance have to be
 * computed.
 */
double calc_max2d(point p1, point p2, int integer);

/**
 * Calculates the ceiling of euclidean 2d distance between two points.
 * 
 * Integer param is passed when an integer distance have to be
 * computed.
 */
double calc_ceil2d(point p1, point p2);  //Returns always an integer value


/**
 * Calculates the latitude and longitude of a given point.
 * 
 */
static void calc_lat_lon(point p, double *lat, double *lon);

/**
 * Calculates the geographical distance between two points.
 * 
 * Integer param is passed when an integer distance have to be
 * computed.
 */
double calc_geo(point p1, point p2, int integer);

#endif