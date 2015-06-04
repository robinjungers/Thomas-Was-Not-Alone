#ifndef CHARACTER_H_
#define CHARACTER_H_

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "basics.h"
#include "bloc.h"

typedef struct Character {
	char * name;
	Bloc * bloc;
	Vector vel, acc;
	Vector posToReach;
	Color color;
	float jumpPower;
	int isSelected;
	int isOnGround;
} Character;

#endif