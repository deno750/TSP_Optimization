/**
 * Code taken from here: https://rosettacode.org/wiki/Convex_hull#C
 */

#include "convexhull.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static int comparePoints(const void *lhs, const void *rhs) {
    const point* lp = lhs;
    const point* rp = rhs;
    if (lp->x < rp->x)
        return -1;
    if (rp->x < lp->x)
        return 1;
    if (lp->y < rp->y)
        return -1;
    if (rp->y < lp->y)
        return 1;
    return 0;
}

static bool ccw(const point *a, const point *b, const point *c) {
    return (b->x - a->x) * (c->y - a->y)
         > (b->y - a->y) * (c->x - a->x);
}

point* convexHull(point *p, int len, int* hsize) {
    if (len == 0) {
        *hsize = 0;
        return NULL;
    }
 
    int i, size = 0, capacity = 4;
    point* hull = MALLOC(capacity, point);
 
    qsort(p, len, sizeof(point), comparePoints);
 
    /* lower hull */
    for (i = 0; i < len; ++i) {
        while (size >= 2 && !ccw(&hull[size - 2], &hull[size - 1], &p[i]))
            --size;
        if (size == capacity) {
            capacity *= 2;
            hull = REALLOC(hull, capacity , point);
        }
        assert(size >= 0 && size < capacity);
        hull[size] = p[i];
        size++;
    }
 
    /* upper hull */
    int t = size + 1;
    for (i = len - 1; i >= 0; i--) {
        while (size >= t && !ccw(&hull[size - 2], &hull[size - 1], &p[i]))
            --size;
        if (size == capacity) {
            capacity *= 2;
            hull = REALLOC(hull, capacity, point);
        }
        assert(size >= 0 && size < capacity);
        hull[size] = p[i];
        size++;
    }
    --size;
    assert(size >= 0);
    hull = REALLOC(hull, size, point);
    *hsize = size;
    return hull;
}