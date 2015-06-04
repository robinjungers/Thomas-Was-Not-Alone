#ifndef SOUNDFX_H_
#define SOUNDFX_H_

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "basics.h"

typedef struct SoundFX {
	Mix_Chunk * loop;
	Mix_Chunk * jump;
	Mix_Chunk * menuChange;
	Mix_Chunk * menuSelection;
} SoundFX;

SoundFX * makeSoundFX();

#endif