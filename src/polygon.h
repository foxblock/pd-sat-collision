#ifndef _POLY_H
#define _POLY_H

#include "pd_api.h"
#include "vector2d.h"

#define POLY_TYPE_NAME "collision.polygon"

typedef struct
{
    int count;
    Vector2D *verts;
} Polygon;


void registerPoly(PlaydateAPI *playdate);

#endif // _POLY_H