# Collision checking library for Playdate
![example](https://github.com/foxblock/pd-sat-collision/blob/main/media/playdate-20240218-144341.gif)

[Separating Axis Theorem (SAT)](https://www.youtube.com/watch?v=Zgf1DYrmSnk&list=PLSlpr6o9vURwq3oxVZSimY8iC-cdd3kIs&index=7)-based 2D collision checking library for the [Playdate](https://play.date/).

Code is written in C with performance in mind. You can use it in Lua using the provided hooks. 

## Get started
There is not much documentation right now, please take a look at the example project, to see how the code is used.

You can use this library in a C- or Lua-based project. If you want to call into this library from Lua, some classes need to be registered. See main.c in the example project on how to do this.

To build follow the steps in the original guide to compile the C code: [Inside Playdate with C](https://sdk.play.date/2.6.2/Inside%20Playdate%20with%20C.html#_prerequisites)

Windows users, who alreay have GCC, CMake and VisualStudio installed, may use the provided `setup_build.bat` to generate the VS solution file and build directories. [More info on that script.](https://github.com/foxblock/playdate-build-scripts)

## Overview
Right now the library supports the following types of collision checks:

- Circle - Circle (boolean result)
- Circle - Circle (Collision normal and overlap distance result)
- Polygon - Polygon (boolean result)
- Polygon - Polygon (Collision normal and overlap distance result)
- Circle - Polygon (boolean result)
- Circle - Polygon (Collision normal and overlap distance result)

With these basically any other type of collision can be abstracted (e.g. a rectangle or line can be modeled with a polygon). However there are more efficient algorithms for other shapes, which are not implemented right now.

This library implements its own Vector2D struct in vector2d.h ("collision.vector2D" in Lua) with operators for in-memory operations (trying to minimize work for the garbage collector in Lua). Right now this is not a full drop-in replacement for playdate.geometry.vector2D, since it does not provide some operators (like +/-/magnitude/etc.). It does work in some contexts like gfx.drawCircleAtPoint (since it implements access to .x and .y and :unpack()).

Additionally there is the Polygon class defined in polygon.h.

The main entry into this library is through collision.h or using the Lua hooks through the "collision" table. Again see example project for usage.

## Performance
Using the example project as a baseline: The Playdate can handle about 20 moving objects (and one static polygon) all colliding with each other at 50fps, using about 50% CPU just for collision checking.

The example project does broad-phase boolean checking and a narrow-phase calculating the actual collision result. Other methods to speed this up further (like quadtrees) were not tested (and are not in score for this library right now).

The main focus was to create as little temporary objects as possible. This resulted in the current selection of operators of the Vector2D and Polygon classes.

## Lua
The lua folder contains the previous version of this code written in Lua. It works, but I do not recommend using it for performance reasons. It will use ~50% CPU with only a handful of objects colliding.

The upsides of the lua module are compatibility to the playdate.geometry.vector2D class and not having to compile C code (i.e. single debugger, no C toolchain, etc.)

## Contributing
This library is in active development, so things might change/shift/improve. Please feel welcome to add issues or pull requests.