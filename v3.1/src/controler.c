#include "controler.h"

Controler * makeControler() {
	Controler * controler = (Controler *) calloc( 1, sizeof(Controler) );
	
	controler->right = FALSE;
	controler->left = FALSE;
	controler->up = FALSE;
	controler->down = FALSE;
	
	controler->tab = 0;
	
	controler->escape = FALSE;
	controler->enter = FALSE;
	
	return controler;
}

void updateControler( Controler * controler, Controler * previous, Uint8 * key, size_t numberOfCharacters ) {
	controler->up = FALSE;
	controler->down = FALSE;
	controler->escape = FALSE;
	controler->enter = FALSE;

	controler->right = key[SDLK_RIGHT];
	controler->left = key[SDLK_LEFT];
	
	if ( key[SDLK_DOWN] && !previous->down ) {
		controler->down = TRUE;
		previous->down = TRUE;
	}
	if ( key[SDLK_UP] && !previous->up ) {
		controler->up = TRUE;
		previous->up = TRUE;
	}
	if ( key[SDLK_ESCAPE] && !previous->escape ) {
		controler->escape = TRUE;
		previous->escape = TRUE;
	}
	if ( key[SDLK_RETURN] && !previous->enter ) {
		controler->enter = TRUE;
		previous->enter = TRUE;
	}
	
	if ( key[SDLK_TAB] &&  controler->tab==previous->tab )
		controler->tab = (controler->tab + 1) % numberOfCharacters;
	
	if ( !key[SDLK_TAB] )
		previous->tab = controler->tab;
	if ( !key[SDLK_DOWN] )
		previous->down = controler->down;
	if ( !key[SDLK_UP] )
		previous->up = controler->up;
	if ( !key[SDLK_ESCAPE] )
		previous->escape = controler->escape;
	if ( !key[SDLK_RETURN] )
		previous->enter = controler->enter;
}