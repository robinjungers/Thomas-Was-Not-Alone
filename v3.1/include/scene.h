#ifndef SCENE_H_
#define SCENE_H_

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "basics.h"
#include "bloc.h"
#include "controler.h"
#include "character.h"
#include "soundfx.h"
#include "image.h"

#define GRAVITY 0.07
#define STEP 1.0

#define MAX_BLOC_NUMBER 50
#define MAX_CHARACTER_NUMBER 3

typedef struct Scene {
	Bloc * blocs;
	Character * characters;
	size_t numberOfBlocs;
	size_t numberOfCharacters;
	Vector minDimension, maxDimension;
} Scene;

Scene * makeScene();

int initBlocsFromFile( Scene * scene, const char * filename );

int initCharactersFromFile( Scene * scene, const char * filename );

void getSceneDimensions( Scene * scene );

void displayPlatforms( Scene * scene );

void handleCharactersControls( Scene * scene, Controler * controler, SoundFX * soundFX );

void handleCharacterParenting_Recursive( Bloc * blocRef, Scene * scene );

void handleCharactersParenting( Scene * scene );

void displayCharacters( Scene * scene );

void displayCharactersSelector( Scene * scene );

int checkGameEnding( Scene * scene );



#endif