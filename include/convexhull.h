#ifndef CONVEX_HULL_H
#define CONVEX_HULL_H

#include "utility.h"
#include "convexhull.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static int comparePoints(const void *lhs, const void *rhs);

static bool ccw(const point *a, const point *b, const point *c);

point* convexHull(point *p, int len, int* hsize);

#endif