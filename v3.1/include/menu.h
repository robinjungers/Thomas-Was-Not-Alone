#ifndef MENU_H_
#define MENU_H_

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "bloc.h"
#include "basics.h"
#include "image.h"
#include "scene.h"
#include "controler.h"
#include "soundfx.h"

#define MAX_MENUTITLE_NUMBER 3

typedef struct Menu {
	int active;
	int selectedLevel;
	Image * images;
} Menu;

Menu * makeMenu( const char * filename0, const char * filename1, const char * filename2 );

void displayMenu( Menu * menu );

void handleMenuControls( Menu * menu, Controler * controler, Scene * scene, SoundFX * soundFX, Image * background0, Image * background1, Image * background2 );

#endif