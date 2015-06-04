#ifndef BASICS_H_
#define BASICS_H_

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

#define TRUE 1
#define FALSE 0

static GLfloat R = 1.0, G = 1.0, B = 1.0, A = 1.0;
static int time = 0;

typedef struct Vector {
	float x;
	float y;
} Vector;

typedef struct Color {
	unsigned char R, G, B;
} Color;

Vector vector( float x, float y );

Color color( char R, char G, char B );

void easeValue( float * base, float target, float step );

void easeVector( Vector * base, Vector target, float step );

float mapValue( float value, float a, float b, float c, float d );

float dist( Vector A, Vector B );

#endif