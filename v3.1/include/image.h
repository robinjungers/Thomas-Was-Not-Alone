#ifndef IMAGE_H_
#define IMAGE_H_

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "basics.h"
#include "bloc.h"
#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

typedef struct Image {
	GLuint texture;
	Bloc rect;
} Image;

GLuint makeTexture( const char * filename );

Image * makeImage( const char * filename, Bloc rect );

void drawImage( Image * image );

#endif