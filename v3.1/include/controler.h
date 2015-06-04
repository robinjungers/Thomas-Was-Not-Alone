#ifndef CONTROLER_H_
#define CONTROLER_H_

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include <SDL/SDL.h>

#include "basics.h"

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

typedef struct Controler {
	int right, left, up, down;
	int tab;
	int escape, enter;
} Controler;

Controler * makeControler();

void updateControler( Controler * controler, Controler * previous, Uint8 * key, size_t numberOfCharacters );

#endif