#include <math.h>
#include "distutil.h"

static double nint(double x) {
    return (long) (x + 0.5);
}

static double e_round(double x) {
    double r = x > 0 ? 0.5 : -0.5;
    return (long) (x + r);
}

double calc_euc2d(point p1, point p2, int integer) {
    double dx = p1.x - p2.x;
    double dy = p1.y - p2.y;
    double dist = sqrt(dx*dx + dy*dy);
    return integer ? nint(dist) : dist;
}

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

double calc_man2d(point p1, point p2, int integer) {
    double dx = fabs(p1.x - p2.x);
    double dy = fabs(p2.y - p2.y);
    return integer ? nint(dx + dy) : dx + dy;
}

double calc_max2d(point p1, point p2, int integer) {
    double dx = fabs(p1.x - p2.x);
    double dy = fabs(p2.y - p2.y);
    dx = integer ? nint(dx) : dx;
    dy = integer ? nint(dy) : dy;
    return dmax(dx, dy);
}

double calc_ceil2d(point p1, point p2) { //Returns always an integer value
    return ceil(calc_euc2d(p1, p2, 0));
}

static void calc_lat_lon(point p, double *lat, double *lon) {
    double deg = nint(p.x); 
    double min = p.x - deg;
    *lat = PI * (deg + 5.0 * min / 3.0) / 180.0;
    deg = nint(p.y); 
    min = p.y - deg;
    *lon = PI * (deg + 5.0 * min / 3.0 ) / 180.0;
}

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