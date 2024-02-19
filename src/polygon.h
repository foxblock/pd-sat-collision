#ifndef _POLY_H
#define _POLY_H

#include "pd_api.h"
#include "vector2d.h"

#define POLY_TYPE_NAME "collision.polygon"

// TODO: Add C API
// TODO: Cache normals of edges (cacheNormals, clearCache functions)

typedef struct
{
    int count;
    Vector2D *verts;
} Polygon;


void registerPoly(PlaydateAPI *playdate);

#endif // _POLY_H