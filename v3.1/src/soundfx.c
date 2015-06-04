#include "soundfx.h"

SoundFX * makeSoundFX() {
	SoundFX * soundFX = (SoundFX *) calloc( 1, sizeof(SoundFX) );
	
	soundFX->loop = Mix_LoadWAV( "sound/FlamingDeserts.wav" );
	soundFX->jump = Mix_LoadWAV( "sound/jump.wav" );
	soundFX->menuChange = Mix_LoadWAV( "sound/MenuChange.wav" );
	soundFX->menuSelection = Mix_LoadWAV( "sound/MenuSelection.wav" );
	
	return soundFX;
}