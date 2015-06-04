#ifndef BLOC_H_
#define BLOC_H_

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "basics.h"
#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

typedef struct Bloc {
	Vector pos;
	Vector previousPos;
	Vector size;
	struct Bloc * parent;
} Bloc;

Bloc bloc( float x, float y, float a, float b );

void drawRect( Bloc bloc );

void drawRectShape( Bloc bloc );

int checkCollision( int side, Bloc obj, Bloc ref );

#endif