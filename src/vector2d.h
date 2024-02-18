#ifndef _vector2d_H
#define _vector2d_H

#include "pd_api.h"

#define VECTOR_TYPE_NAME "collision.vector2D"

typedef struct
{
    float x;
    float y;
} Vector2D;

void vector2D_normalize(Vector2D *v);
void vector2D_leftNormal(Vector2D *target, Vector2D src);
void vector2D_rightNormal(Vector2D *target, Vector2D src);
void vector2D_dirNormalized(Vector2D *v, Vector2D pa, Vector2D pb);
void vector2D_addVecScaled(Vector2D *v, Vector2D other, float otherScale);
size_t vector2D_print(char* dest, size_t len, Vector2D v);

float vector2D_length(Vector2D v);
float vector2D_lengthSquared(Vector2D v);
float vector2D_dotProduct(Vector2D a, Vector2D b);

void registerVector2D(PlaydateAPI *playdate);

//void Vector2D_print(char *dest, int len, Vector2D *v);

#endif // _vector2d_H