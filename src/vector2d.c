#include "vector2d.h"
#include <stdio.h>

static PlaydateAPI* pd = NULL;

static inline float square(float v)
{
    return v * v;
}

void vector2D_normalize(Vector2D *v)
{
    float len = vector2D_length(*v);
	if (len == 0)
		return;

    v->x /= len;
    v->y /= len;
}

// assuming coordinate space where y=0 is top-left and y=+ is bottom-left
void vector2D_leftNormal(Vector2D *target, Vector2D src)
{
    target->x = src.y;
    target->y = -src.x;
}

// assuming coordinate space where y=0 is top-left and y=+ is bottom-left
void vector2D_rightNormal(Vector2D *target, Vector2D src)
{
    target->x = -src.y;
    target->y = src.x;
}

void vector2D_dirNormalized(Vector2D *target, Vector2D pa, Vector2D pb)
{
    float len = sqrtf(square(pa.x - pb.x) + square(pa.y - pb.y));
	if (len == 0)
		len = 1.0f;
    target->x = (pa.x - pb.x) / len;
    target->y = (pa.y - pb.y) / len;
}

void vector2D_addVecScaled(Vector2D *v, Vector2D other, float otherScale)
{
    v->x += other.x * otherScale;
    v->y += other.y * otherScale;
}

float vector2D_length(Vector2D v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

float vector2D_lengthSquared(Vector2D v)
{
    return v.x * v.x + v.y * v.y;
}

float vector2D_dotProduct(Vector2D a, Vector2D b)
{
    return a.x * b.x + a.y * b.y;
}

size_t vector2D_print(char* dest, size_t len, Vector2D v)
{
	//int bytesWritten = snprintf(dest, len, "(%.2f, %.2f)", v.x, v.y);
	//if (len < bytesWritten)
	//	return len;
	//if (bytesWritten < 0)
	//	return 0;
	//return bytesWritten;
	return 0;
}

// --- LUA HOOKS ---

static int lua_vector2d_new(lua_State *L)
{
	Vector2D* v = pd->system->realloc(NULL, sizeof(Vector2D));

	if (pd->lua->getArgCount() == 1)
	{
		Vector2D* other = pd->lua->getArgObject(1, VECTOR_TYPE_NAME, NULL);
		*v = *other;
	}
	else
	{
		v->x = pd->lua->getArgFloat(1);
		v->y = pd->lua->getArgFloat(2);
	}

	pd->lua->pushObject(v, VECTOR_TYPE_NAME, 0);
	return 1;
}

static int lua_vector2d_free(lua_State *L)
{
	Vector2D* v = pd->lua->getArgObject(1, VECTOR_TYPE_NAME, NULL);
	if (v != NULL)
		pd->system->realloc(v, 0);
	return 0;
}

static int lua_vector2d_index(lua_State *L)
{
	// https://sdk.play.date/2.3.1/Inside%20Playdate%20with%20C.html#f-lua.indexMetatable
	// Maybe we don't need this?
	if (pd->lua->indexMetatable())
		return 1;

	Vector2D* v = pd->lua->getArgObject(1, VECTOR_TYPE_NAME, NULL);
	const char* arg = pd->lua->getArgString(2);
	
	if (strcmp(arg, "x") == 0)
		pd->lua->pushFloat(v->x);
	else if (strcmp(arg, "y") == 0)
		pd->lua->pushFloat(v->y);
	else
		pd->lua->pushNil();
	
	return 1;
}

static int lua_vector2d_newindex(lua_State *L)
{
	Vector2D* v = pd->lua->getArgObject(1, VECTOR_TYPE_NAME, NULL);
	const char* arg = pd->lua->getArgString(2);
	
	if (strcmp(arg, "x") == 0)
		v->x = pd->lua->getArgFloat(3);
	else if (strcmp(arg, "y") == 0)
		v->y = pd->lua->getArgFloat(3);
	
	return 0;
}

static int lua_vector2d_print(lua_State *L)
{
 //   Vector2D* v = pd->lua->getArgObject(1, VECTOR_TYPE_NAME, NULL);

 //   char result[64] = "";
 //   vector2D_print(result, sizeof(result), *v);

 //   pd->lua->pushString(result);
	//return 1;
	return 0;
}

static int lua_vector2d_unpack(lua_State *L)
{
	Vector2D* v = pd->lua->getArgObject(1, VECTOR_TYPE_NAME, NULL);

	pd->lua->pushFloat(v->x);
	pd->lua->pushFloat(v->y);
	return 2;
}

static int lua_vector2d_addScaled(lua_State *L)
{
	int argc = pd->lua->getArgCount();
	if ((argc - 1) % 2 != 0)
	{
		pd->system->error("%s:%i: Invalid arguments for vector2d:addScaled()! Must be "
			"alternating list of vector2d and scaling factors (float).",
			__FILE__, __LINE__);
		return 0;
	}

	Vector2D* v = pd->lua->getArgObject(1, VECTOR_TYPE_NAME, NULL);
	for (int i = 2; i < argc; i += 2)
	{
		Vector2D* other = pd->lua->getArgObject(i, VECTOR_TYPE_NAME, NULL);
		float scale = pd->lua->getArgFloat(i + 1);

		vector2D_addVecScaled(v, *other, scale);
	}

	return 0;
}

static int lua_vector2d_normalize(lua_State *L)
{
	Vector2D* v = pd->lua->getArgObject(1, VECTOR_TYPE_NAME, NULL);

	vector2D_normalize(v);

	return 0;
}

static int lua_vector2d_dotProduct(lua_State *L)
{
	Vector2D* a = pd->lua->getArgObject(1, VECTOR_TYPE_NAME, NULL);
	Vector2D* b = pd->lua->getArgObject(2, VECTOR_TYPE_NAME, NULL);

	float res = vector2D_dotProduct(*a, *b);

	pd->lua->pushFloat(res);
	return 1;
}

// --- END LUA HOOKS ---

static const lua_reg vector2Dlib[] =
{
	{ "new", 		lua_vector2d_new },
	{ "__gc",		lua_vector2d_free },
	{ "__index", 	lua_vector2d_index },
	{ "__newindex",	lua_vector2d_newindex },
	{ "__tostring",	lua_vector2d_print },
	{ "unpack",		lua_vector2d_unpack },
	{ "addScaled",	lua_vector2d_addScaled },
	{ "normalize",	lua_vector2d_normalize },
	{ "dotProduct",	lua_vector2d_dotProduct },
	{ NULL, NULL }
};

void registerVector2D(PlaydateAPI* playdate)
{
	pd = playdate;
	
	const char* err;
	
	if (!pd->lua->registerClass(VECTOR_TYPE_NAME, vector2Dlib, NULL, 0, &err))
		pd->system->error("%s:%i: registerClass failed, %s", __FILE__, __LINE__, err);
}