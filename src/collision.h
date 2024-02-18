#ifndef _COLLISION_H
#define _COLLISION_H

#include "pd_api.h"
#include "vector2d.h"
#include "polygon.h"

#define COLLISION_TYPE_NAME "collision"

int collision_circleCircle_check(Vector2D centerA, float radiusA, Vector2D centerB, float radiusB);
int collision_polyPoly_check(Polygon polyA, Polygon polyB);
int collision_circlePoly_check(Vector2D center, float radius, Polygon poly);

int collision_circleCircle(Vector2D *resolveDir, float *depth, Vector2D centerA, float radiusA, Vector2D centerB, float radiusB);
int collision_polyPoly(Vector2D *resolveDir, float *depth, Polygon polyA, Polygon polyB);
int collision_circlePoly(Vector2D *resolveDir, float *depth, Vector2D center, float radius, Polygon poly);

void registerCollision(PlaydateAPI *playdate);

#endif