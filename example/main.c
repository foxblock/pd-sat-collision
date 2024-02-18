#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "pd_api.h"
#include "../src/vector2d.h"
#include "../src/polygon.h"
#include "../src/collision.h"

static PlaydateAPI* pd = NULL;

#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(PlaydateAPI* playdate, PDSystemEvent event, uint32_t arg)
{
	(void)arg;

	if (event == kEventInitLua)
	{
		pd = playdate;
		// NOTE: Collision has to be registered first, since class "collision" would overwrite "collision.vector2d" (or "collision.*" in general)
		registerCollision(pd);
		registerVector2D(pd);
		registerPoly(pd);
	}

	return 0;
}
