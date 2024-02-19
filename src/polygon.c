#include "polygon.h"
#include <stdio.h>

static PlaydateAPI* pd = NULL;

static inline float square(float v)
{
    return v * v;
}

static void polygon_middle(Vector2D *dest, Polygon p)
{
    float sumX = 0.0f;
    float sumY = 0.0f;
    for (int i=0; i < p.count; ++i)
    {
        sumX += p.verts[i].x;
        sumY += p.verts[i].y;
    }
    dest->x = sumX / p.count;
    dest->y = sumY / p.count;
}

// --- LUA HOOKS ---

static int lua_polygon_new(lua_State *L)
{
    int argc = pd->lua->getArgCount();
    Polygon *p;
    if (argc == 1)
    {
        int count = pd->lua->getArgInt(1);
        p = pd->system->realloc(NULL, sizeof(Polygon));
        p->count = count;
        p->verts = pd->system->realloc(NULL, sizeof(Vector2D) * p->count);
        memset(p->verts, 0, sizeof(Vector2D) * p->count);
    }
    else
    {
        if (argc % 2 != 0)
        {
            pd->system->error("%s:%i: creating new poly failed with invalid arguments "
                    "(needs to be either: [vertex count] or [x1,y1,x2,y2,...])", __FILE__, __LINE__);
            return 0;
        }

        p = pd->system->realloc(NULL, sizeof(Polygon));
        p->count = argc / 2;
        p->verts = pd->system->realloc(NULL, sizeof(Vector2D) * p->count);
        for (int i = 1; i <= argc; i += 2)
        {
            Vector2D *v = (Vector2D*)p->verts + (i-1)/2;
            v->x = pd->lua->getArgFloat(i);
            v->y = pd->lua->getArgFloat(i+1);
        }
    }

	pd->lua->pushObject(p, POLY_TYPE_NAME, 0);
	return 1;
}

static int lua_polygon_free(lua_State *L)
{
	Polygon* p = pd->lua->getArgObject(1, POLY_TYPE_NAME, NULL);
    pd->system->realloc(p->verts, 0);
	pd->system->realloc(p, 0);
	return 0;
}

static int lua_polygon_index(lua_State *L)
{
    // https://sdk.play.date/2.3.1/Inside%20Playdate%20with%20C.html#f-lua.indexMetatable
    // Maybe we don't need this?
    if (pd->lua->indexMetatable())
        return 1;

    Polygon* p = pd->lua->getArgObject(1, POLY_TYPE_NAME, NULL);
    int i = pd->lua->getArgInt(2) - 1;

    if (i >= p->count || i < 0)
        return 0;

    // Need to create a new object for now, since we don't have reference counting implemented in Vector2D
    Vector2D* v = pd->system->realloc(NULL, sizeof(Vector2D));
    *v = p->verts[i];
    pd->lua->pushObject(v, VECTOR_TYPE_NAME, 0);
    return 1;
}

static int lua_polygon_len(lua_State *L)
{
    Polygon* p = pd->lua->getArgObject(1, POLY_TYPE_NAME, NULL);

    pd->lua->pushInt(p->count);
    return 1;
}

static int lua_polygon_print(lua_State *L)
{
    //Polygon* p = pd->lua->getArgObject(1, POLY_TYPE_NAME, NULL);

    //size_t size = 256;
    //char *result = pd->system->realloc(NULL, size);
    //result[0] = '(';
    //result[1] = 0;
    //size_t pos = 1;
    //for (int i = 0; i < p->count; ++i)
    //{
    //    if (pos + 64 >= size)
    //    {
    //        size *= 2;
    //        result = pd->system->realloc(result, size);
    //    }
    //    pos += vector2D_print(result + pos, size - pos, p->verts[i]);
    //    strncat(result, ", ", size - pos - 1);
    //    pos += 2;
    //}
    //result[pos - 2] = ')';
    //result[pos - 1] = 0;

    //pd->lua->pushString(result);
    //return 1;
    return 0;
}

static int lua_polygon_set(lua_State *L)
{
	Polygon* p = pd->lua->getArgObject(1, POLY_TYPE_NAME, NULL);
    int argc = pd->lua->getArgCount();
    if ((argc - 1) != p->count * 2)
    {
        pd->system->error("%s:%i: Invalid arguments for poly:set()! Must be list of points as "
                "[x1,y1,x2,y2,...] equal to the amount of vertices in poly: %d", 
                __FILE__, __LINE__, p->count);
        return 0;
    }

    for (int i = 2; i < argc; i += 2)
    {
        Vector2D *v = &(p->verts[(i-2)/2]);
        v->x = pd->lua->getArgFloat(i);
        v->y = pd->lua->getArgFloat(i+1);
    }

    return 0;
}

static int lua_polygon_addScaled(lua_State *L)
{
    Polygon* p = pd->lua->getArgObject(1, POLY_TYPE_NAME, NULL);
    Vector2D* v = pd->lua->getArgObject(2, VECTOR_TYPE_NAME, NULL);
    float scale = pd->lua->getArgFloat(3);

    for (int i = 0; i < p->count; ++i)
    {
        vector2D_addVecScaled(&p->verts[i], *v, scale);
    }

    return 0;
}

static int lua_polygon_boundingCircle(lua_State *L)
{
    Polygon* p = pd->lua->getArgObject(1, POLY_TYPE_NAME, NULL);

    Vector2D *middle = pd->system->realloc(NULL, sizeof(Vector2D));

    polygon_middle(middle, *p);
    float minDist = 0;
    for (int i = 0; i < p->count; ++i)
    {
        float dist = square(p->verts[i].x - middle->x) + square(p->verts[i].y - middle->y);
        if (dist > minDist)
            minDist = dist;
    }
    minDist = sqrtf(minDist);

	pd->lua->pushObject(middle, VECTOR_TYPE_NAME, 0);
    pd->lua->pushFloat(minDist);
    return 2;
}

static const lua_reg polylib[] =
{
	{ "new", 		lua_polygon_new },
	{ "__gc",		lua_polygon_free },
    { "__index",	lua_polygon_index },
	{ "__len",		lua_polygon_len },
    { "__tostring",	lua_polygon_print },
	{ "set",		lua_polygon_set },
    { "addScaled",	lua_polygon_addScaled },
    { "getBoundingCircle", lua_polygon_boundingCircle },
	{ NULL, NULL }
};

void registerPoly(PlaydateAPI* playdate)
{
	pd = playdate;
	
	const char* err;
	
	if (!pd->lua->registerClass(POLY_TYPE_NAME, polylib, NULL, 0, &err))
		pd->system->error("%s:%i: registerClass failed, %s", __FILE__, __LINE__, err);
}