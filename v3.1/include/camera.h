#ifndef CAMERA_H_
#define CAMERA_H_

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "image.h"
#include "scene.h"
#include "bloc.h"
#include "basics.h"

typedef struct Camera {
	Vector pos;
	Vector posToReach;
	float zoom;
} Camera;

Camera * makeCamera();

void drawImageParallax( Image * image, Camera * camera, float coef );

void updateCamera( Camera * camera, Scene * scene );

#endif