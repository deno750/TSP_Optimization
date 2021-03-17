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
static double nint(double x) {
    return (long) (x + 0.5);
}

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
static double e_round(double x) {
    double r = x > 0 ? 0.5 : -0.5;
    return (long) (x + r);
}

/**
 * Calculates the euclidean 2d distance between two points.
 * 
 * Integer param is passed when an integer distance have to be
 * computed.
 */
double calc_euc2d(point p1, point p2, int integer) {
    double dx = p1.x - p2.x;
    double dy = p1.y - p2.y;
    double dist = sqrt(dx*dx + dy*dy);
    return integer ? nint(dist) : dist;
}

/**
 * Calculates the pseudo euclidean 2d distance between two points.
 * 
 * Integer param is passed when an integer distance have to be
 * computed.
 */
double calc_pseudo_euc(point p1, point p2, int integer) {
    double dx = p1.x - p2.x;
    double dy = p1.y - p2.y;
    double r = sqrt((dx*dx + dy*dy) / 10.0);
    if (integer) {  
        double t = nint(r);
        double dist = t < r ? t + 1 : t;
        return dist;
    } else {
        return r;
    }
}

/**
 * Calculates the manhattan 2d distance between two points.
 * 
 * Integer param is passed when an integer distance have to be
 * computed.
 */
double calc_man2d(point p1, point p2, int integer) {
    double dx = fabs(p1.x - p2.x);
    double dy = fabs(p2.y - p2.y);
    return integer ? nint(dx + dy) : dx + dy;
}

/**
 * Calculates the maximum 2d distance between two points.
 * 
 * Integer param is passed when an integer distance have to be
 * computed.
 */
double calc_max2d(point p1, point p2, int integer) {
    double dx = fabs(p1.x - p2.x);
    double dy = fabs(p2.y - p2.y);
    dx = integer ? nint(dx) : dx;
    dy = integer ? nint(dy) : dy;
    return dmax(dx, dy);
}

/**
 * Calculates the ceiling of euclidean 2d distance between two points.
 * 
 * Integer param is passed when an integer distance have to be
 * computed.
 */
double calc_ceil2d(point p1, point p2) { //Returns always an integer value
    return ceil(calc_euc2d(p1, p2, 0));
}

/**
 * Calculates the latitude and longitude of a given point.
 * 
 */
static void calc_lat_lon(point p, double *lat, double *lon) {
    double deg = e_round(p.x); // Use instead nint as in TSP documentation?
    double min = p.x - deg;
    *lat = PI * (deg + 5.0 * min / 3.0) / 180.0;
    deg = e_round(p.y); // Use instead nint as in TSP documentation?
    min = p.y - deg;
    *lon = PI * (deg + 5.0 * min / 3.0 ) / 180.0;
}

/**
 * Calculates the geographical distance between two points.
 * 
 * Integer param is passed when an integer distance have to be
 * computed.
 */
double calc_geo(point p1, point p2, int integer) {
    double lat1, lon1; // Latitude and longitude point 1
    double lat2, lon2; // Latitude and longitude point 2
    calc_lat_lon(p1, &lat1, &lon1);
    calc_lat_lon(p2, &lat2, &lon2);

    double q1 = cos( lon1 - lon2 );
    double q2 = cos( lat1 - lat2 );
    double q3 = cos( lat1 + lat2 );
    double dist = EARTH_RAD * acos( 0.5 * ((1.0+q1)*q2 - (1.0-q1)*q3) ) + 1.0;
    return integer ? nint(dist) : dist;
}

#endif