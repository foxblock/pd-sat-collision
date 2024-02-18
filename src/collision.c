#include "collision.h"

#ifndef FLT_MAX
#define FLT_MAX 3.402823466e+38F
#endif

static PlaydateAPI* pd = NULL;

// -- HELPER ---

static void polyEdge(Vector2D *target, Polygon p, int index)
{
    int index2 = (index + 1) % p.count;
    target->x = p.verts[index2].x - p.verts[index].x;
    target->y = p.verts[index2].y - p.verts[index].y;
}

static inline float square(float v)
{
    return v * v;
}

static void projectPoly(float *outMin, float *outMax, Polygon poly, Vector2D axis)
{
    *outMin = FLT_MAX;
    *outMax = -FLT_MAX;
    for (int i = 0; i < poly.count; ++i)
    {
        float val = vector2D_dotProduct(axis, poly.verts[i]);
        if (val < *outMin)
            *outMin = val;
        if (val > *outMax)
            *outMax = val;
    }
}

// note axis is assumed to be normalized
static void projectCircle(float *outMin, float *outMax, Vector2D center, float radius, Vector2D axis)
{
    *outMin = FLT_MAX;
    *outMax = -FLT_MAX;
    Vector2D temp = { .x = center.x - axis.x * radius, .y = center.y - axis.y * radius };
    *outMin = vector2D_dotProduct(axis, temp);
    temp.x = center.x + axis.x * radius;
    temp.y = center.y + axis.y * radius;
    *outMax = vector2D_dotProduct(axis, temp);

    if (*outMin > *outMax)
    {
        float temp = *outMin;
        *outMin = *outMax;
        *outMax = temp;
    }
}

static int findClosestVertexIndex(Vector2D target, Polygon poly)
{
    float distSqr = FLT_MAX;
    int result = 0;
    for (int i = 0; i < poly.count; ++i)
    {
        float val = square(poly.verts[i].x - target.x) + square(poly.verts[i].y - target.y);
        if (val < distSqr)
        {
            distSqr = val;
            result = i;
        }
    }
    return result;
}

// --- COLLISION ---

int collision_circleCircle_check(Vector2D centerA, float radiusA, Vector2D centerB, float radiusB)
{
    float distSqr = square(centerA.x - centerB.x) + square(centerA.y - centerB.y);
    if (distSqr >= square(radiusA + radiusB))
        return 0;
    return 1;
}

int collision_circleCircle(Vector2D *resolveDir, float *depth, Vector2D centerA, float radiusA, Vector2D centerB, float radiusB)
{
    *depth = square(centerA.x - centerB.x) + square(centerA.y - centerB.y);
    if (*depth >= square(radiusA + radiusB))
        return 0;

    *depth = sqrtf(*depth);
    resolveDir->x = (centerB.x - centerA.x) / *depth;
    resolveDir->y = (centerB.y - centerA.y) / *depth;
    return 1;
}


int collision_polyPoly_check(Polygon polyA, Polygon polyB)
{
    float minA, minB, maxA, maxB;
    Vector2D edge;
    Vector2D axis;

    for (int i = 0; i < polyA.count; ++i)
    {
        polyEdge(&edge, polyA, i);

        if (edge.x == 0 && edge.y == 0)
            continue;

        vector2D_leftNormal(&axis, edge);
        
        projectPoly(&minA, &maxA, polyA, axis);
        projectPoly(&minB, &maxB, polyB, axis);

        if (maxA < minB || maxB < minA)
            return 0;
    }
    for (int i = 0; i < polyB.count; ++i)
    {
        polyEdge(&edge, polyB, i);

        if (edge.x == 0 && edge.y == 0)
            continue;

        vector2D_leftNormal(&axis, edge);
        
        projectPoly(&minA, &maxA, polyA, axis);
        projectPoly(&minB, &maxB, polyB, axis);

        if (maxA < minB || maxB < minA)
            return 0;
    }

    return 1;
}

int collision_polyPoly(Vector2D *resolveDir, float *depth, Polygon polyA, Polygon polyB)
{
    float minA, minB, maxA, maxB;
    *depth = FLT_MAX;
    int invertResult = 0;
    Vector2D edge;
    Vector2D axis;

    for (int i = 0; i < polyA.count; ++i)
    {
        polyEdge(&edge, polyA, i);

        if (edge.x == 0 && edge.y == 0)
            continue;

        vector2D_leftNormal(&axis, edge);
        vector2D_normalize(&axis);
        
        projectPoly(&minA, &maxA, polyA, axis);
        projectPoly(&minB, &maxB, polyB, axis);

        if (maxA < minB || maxB < minA)
            return 0;

        float axisDepth = fminf(maxA - minB, maxB - minA);
        if (axisDepth < *depth)
        {
            *depth = axisDepth;
            *resolveDir = axis;
            invertResult = maxB - minA < maxA - minB;
        }
    }
    for (int i = 0; i < polyB.count; ++i)
    {
        polyEdge(&edge, polyB, i);

        if (edge.x == 0 && edge.y == 0)
            continue;

        vector2D_leftNormal(&axis, edge);
        vector2D_normalize(&axis);
        
        projectPoly(&minA, &maxA, polyA, axis);
        projectPoly(&minB, &maxB, polyB, axis);

        if (maxA < minB || maxB < minA)
            return 0;

        float axisDepth = fminf(maxA - minB, maxB - minA);
        if (axisDepth < *depth)
        {
            *depth = axisDepth;
            *resolveDir = axis;
            invertResult = maxB - minA < maxA - minB;
        }
    }

    if (invertResult)
    {
        resolveDir->x *= -1;
        resolveDir->y *= -1;
    }

    return 1;
}


int collision_circlePoly_check(Vector2D center, float radius, Polygon poly)
{
    float minA, minB, maxA, maxB;
    Vector2D edge;
    Vector2D axis;

    int index = findClosestVertexIndex(center, poly);
    vector2D_dirNormalized(&axis, poly.verts[index], center);

    projectCircle(&minA, &maxA, center, radius, axis);
    projectPoly(&minB, &maxB, poly, axis);

    if (maxA < minB || maxB < minA)
        return 0;

    for (int i = 0; i < poly.count; ++i)
    {
        polyEdge(&edge, poly, i);

        if (edge.x == 0 && edge.y == 0)
            continue;

        vector2D_leftNormal(&axis, edge);
        
        projectCircle(&minA, &maxA, center, radius, axis);
        projectPoly(&minB, &maxB, poly, axis);

        if (maxA < minB || maxB < minA)
            return 0;
    }

    return 1;
}

int collision_circlePoly(Vector2D *resolveDir, float *depth, Vector2D center, float radius, Polygon poly)
{
    float minA, minB, maxA, maxB;
    *depth = FLT_MAX;
    int invertResult = 0;
    Vector2D edge;
    Vector2D axis;

    int index = findClosestVertexIndex(center, poly);
    vector2D_dirNormalized(&axis, poly.verts[index], center);

    projectCircle(&minA, &maxA, center, radius, axis);
    projectPoly(&minB, &maxB, poly, axis);

    if (maxA < minB || maxB < minA)
        return 0;

    float axisDepth = fminf(maxA - minB, maxB - minA);
    if (axisDepth < *depth)
    {
        *depth = axisDepth;
        *resolveDir = axis;
        invertResult = maxB - minA < maxA - minB;
    }

    for (int i = 0; i < poly.count; ++i)
    {
        polyEdge(&edge, poly, i);

        if (edge.x == 0 && edge.y == 0)
            continue;

        vector2D_leftNormal(&axis, edge);
        vector2D_normalize(&axis);
        
        projectCircle(&minA, &maxA, center, radius, axis);
        projectPoly(&minB, &maxB, poly, axis);

        if (maxA < minB || maxB < minA)
            return 0;

        float axisDepth = fminf(maxA - minB, maxB - minA);
        if (axisDepth < *depth)
        {
            *depth = axisDepth;
            *resolveDir = axis;
            invertResult = maxB - minA < maxA - minB;
        }
    }

    if (invertResult)
    {
        resolveDir->x *= -1;
        resolveDir->y *= -1;
    }

    return 1;
}

// --- LUA HOOKS ---

static int lua_collision_circleCircle_check(lua_State *L)
{
	Vector2D *centerA = pd->lua->getArgObject(1, VECTOR_TYPE_NAME, NULL);
	float radiusA = pd->lua->getArgFloat(2);
	Vector2D *centerB = pd->lua->getArgObject(3, VECTOR_TYPE_NAME, NULL);
	float radiusB = pd->lua->getArgFloat(4);

    int collides = collision_circleCircle_check(*centerA, radiusA, *centerB, radiusB);

    pd->lua->pushBool(collides);
    return 1;
}

static int lua_collision_circleCircle(lua_State *L)
{
	Vector2D *centerA = pd->lua->getArgObject(1, VECTOR_TYPE_NAME, NULL);
	float radiusA = pd->lua->getArgFloat(2);
	Vector2D *centerB = pd->lua->getArgObject(3, VECTOR_TYPE_NAME, NULL);
	float radiusB = pd->lua->getArgFloat(4);

    float dist = square(centerA->x - centerB->x) + square(centerA->y - centerB->y);
    if (dist >= square(radiusA + radiusB))
        return 0;

    Vector2D *resolveDir = pd->system->realloc(NULL, sizeof(Vector2D));

    dist = sqrtf(dist);
    resolveDir->x = (centerB->x - centerA->x) / dist;
    resolveDir->y = (centerB->y - centerA->y) / dist;
    
    pd->lua->pushObject(resolveDir, VECTOR_TYPE_NAME, 0);
    pd->lua->pushFloat(radiusA + radiusB - dist);
	return 2;
}

static int lua_collision_polyPoly_check(lua_State *L)
{
    Polygon* polyA = pd->lua->getArgObject(1, POLY_TYPE_NAME, NULL);
    Polygon* polyB = pd->lua->getArgObject(2, POLY_TYPE_NAME, NULL);

    int collides = collision_polyPoly_check(*polyA, *polyB);

    pd->lua->pushBool(collides);
    return 1;
}

static int lua_collision_polyPoly(lua_State *L)
{
	Polygon* polyA = pd->lua->getArgObject(1, POLY_TYPE_NAME, NULL);
	Polygon* polyB = pd->lua->getArgObject(2, POLY_TYPE_NAME, NULL);

    Vector2D *resolveDir = pd->system->realloc(NULL, sizeof(Vector2D));
    float depth;

    int collides = collision_polyPoly(resolveDir, &depth, *polyA, *polyB);

    if (!collides)
    {
        pd->system->realloc(resolveDir, 0);
        return 0;
    }

    pd->lua->pushObject(resolveDir, VECTOR_TYPE_NAME, 0);
    pd->lua->pushFloat(depth);
	return 2;
}

static int lua_collision_circlePoly_check(lua_State *L)
{
    Vector2D* center = pd->lua->getArgObject(1, VECTOR_TYPE_NAME, NULL);
    float radius = pd->lua->getArgFloat(2);
    Polygon* poly = pd->lua->getArgObject(3, POLY_TYPE_NAME, NULL);

    int collides = collision_circlePoly_check(*center, radius, *poly);

    pd->lua->pushBool(collides);
    return 1;
}

static int lua_collision_circlePoly(lua_State *L)
{
	Vector2D* center = pd->lua->getArgObject(1, VECTOR_TYPE_NAME, NULL);
	float radius = pd->lua->getArgFloat(2);
	Polygon* poly = pd->lua->getArgObject(3, POLY_TYPE_NAME, NULL);

    Vector2D *resolveDir = pd->system->realloc(NULL, sizeof(Vector2D));
    float depth;

    int collides = collision_circlePoly(resolveDir, &depth, *center, radius, *poly);
    
    if (!collides)
    {
        pd->system->realloc(resolveDir, 0);
        return 0;
    }

    pd->lua->pushObject(resolveDir, VECTOR_TYPE_NAME, 0);
    pd->lua->pushFloat(depth);
	return 2;
}

static int lua_collision_swordResolution(lua_State *L)
{
	Vector2D* center = pd->lua->getArgObject(1, VECTOR_TYPE_NAME, NULL);
	float radius = pd->lua->getArgFloat(2);
	Polygon* poly = pd->lua->getArgObject(3, POLY_TYPE_NAME, NULL);

    float minA, maxB;

    Vector2D *axis = pd->system->realloc(NULL, sizeof(Vector2D));
    vector2D_dirNormalized(axis, poly->verts[1], poly->verts[0]);
    vector2D_leftNormal(axis, *axis);

    // we don't need maxB from this call, but cannot pass NULL at the moment
    projectCircle(&minA, &maxB, *center, radius, *axis);
    maxB = vector2D_dotProduct(*axis, poly->verts[0]);
    axis->x *= -1;
    axis->y *= -1;
    
    pd->lua->pushObject(axis, VECTOR_TYPE_NAME, 0);
    pd->lua->pushFloat(maxB - minA);
	return 2;
}

static const lua_reg collisionlib[] =
{
	{ "circleCircle_check", lua_collision_circleCircle_check },
	{ "circlePoly_check", lua_collision_circlePoly_check },
	{ "polyPoly_check", lua_collision_polyPoly_check },
	{ "circleCircle", lua_collision_circleCircle },
	{ "circlePoly", lua_collision_circlePoly },
	{ "polyPoly", lua_collision_polyPoly },
	{ "swordRes", lua_collision_swordResolution },
	{ NULL, NULL }
};

void registerCollision(PlaydateAPI* playdate)
{
	pd = playdate;
	
	const char* err;
	
	if (!pd->lua->registerClass(COLLISION_TYPE_NAME, collisionlib, NULL, 0, &err))
		pd->system->error("%s:%i: registerClass failed, %s", __FILE__, __LINE__, err);
}