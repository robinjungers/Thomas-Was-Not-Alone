#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

static const Uint32 FRAMERATE_MILLISECONDS = 1000 / 60;

#include "basics.h"
#include "bloc.h"
#include "camera.h"
#include "character.h"
#include "controler.h"
#include "image.h"
#include "scene.h"
#include "soundfx.h"
#include "menu.h"


int main() {
	SDL_Init( SDL_INIT_VIDEO );
	
	//Variables Techniques
	unsigned int windowWidth  = 800;
	unsigned int windowHeight = 500;
	
	//Gestion Anti aliasing
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 6 );
	
	SDL_SetVideoMode( windowWidth, windowHeight, 32, SDL_OPENGL );
	SDL_WM_SetCaption("Thomas Was Alone", NULL);
	
	//Initilisation son
	if ( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024 ) == -1 ) printf( "%s", Mix_GetError() );
	Mix_AllocateChannels( 3 );
	
	//Variables Gameplay
	Uint8 * key = SDL_GetKeyState(NULL);
	Controler * controler = makeControler();
	Controler * previousControler = makeControler();
	Scene * scene = makeScene();
	Camera * camera = makeCamera();
	Menu * menu = makeMenu( "img/menu0.png", "img/menu1.png", "img/menu2.png" );
	SoundFX * soundFX = makeSoundFX();
	Mix_PlayChannel( 0, soundFX->loop, -1 );
	
	//Initialisation des textures
	Image * background0 = makeImage( "img/backBrun0.png", bloc( -100.0, -100.0, 256.0, 160.0 ) );
	Image * background1 = makeImage( "img/backBrun1.png", bloc( -100.0, -100.0, 256.0, 160.0 ) );
	Image * background2 = makeImage( "img/backBrun2.png", bloc( -100.0, -100.0, 256.0, 160.0 ) );
	
	//Gestion matrice d'affichage
	glViewport( 0, 0, windowWidth, windowHeight );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D( -80.0, 80.0, -50.0, 50.0);
	glPointSize( 4 ); 
	
	int loop = 1;
	while (loop) {
		time++;
		Uint32 startTime = SDL_GetTicks();
		key = SDL_GetKeyState(NULL);
		
		glClearColor( R, G, B, A );
		glClear( GL_COLOR_BUFFER_BIT );
		glEnable( GL_TEXTURE_2D );
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();
		
		/////////////////////////////////////* DEBUT DESSIN *///////////////////////////////////////

		updateControler( controler, previousControler, key, scene->numberOfCharacters );
		if ( controler->escape  ) {
			if ( !menu->active ) menu->active = TRUE;
			else menu->active = FALSE;
		}
		
		updateCamera( camera, scene );
		
		glPushMatrix();
			drawImageParallax( background0, camera, 0.2 );
			drawImageParallax( background1, camera, 0.4 );
			drawImageParallax( background2, camera, 0.6 );
			
			glRotatef( sin( 0.01*time ), 0.0, 0.0, 1.0 );
			glScalef( camera->zoom, camera->zoom, camera->zoom );
			glTranslatef( -camera->pos.x, -camera->pos.y, 0.0 );
			
			displayPlatforms( scene );
			
			if ( !menu->active ) {
				handleCharactersControls( scene, controler, soundFX );
				handleCharactersParenting( scene );
			}

			displayCharacters( scene );
			displayCharactersSelector( scene );
		glPopMatrix();
		
		if ( checkGameEnding( scene ) )
			menu->active = TRUE;
		
		if ( menu->active ) {
			handleMenuControls( menu, controler, scene, soundFX, background0, background1, background2 );
			displayMenu( menu );
		}
		
		//////////////////////////////////////* FIN DESSIN *////////////////////////////////////////
		
		glDisable( GL_TEXTURE_2D );
		SDL_GL_SwapBuffers();
		
		SDL_Event e;
		while( SDL_PollEvent(&e) ) {
			if( e.type == SDL_QUIT ) {
				loop = 0;
				break;
			}
		}
		
	    Uint32 elapsedTime = SDL_GetTicks() - startTime;
	    if(elapsedTime < FRAMERATE_MILLISECONDS)
	      SDL_Delay(FRAMERATE_MILLISECONDS - elapsedTime);
	}
	
	printf( "\n----- Closing Game -----\n\n" );
	
	//Fermeture des textures
	glDeleteTextures( 1, &(background0->texture) );
	glDeleteTextures( 1, &(background1->texture) );
	glDeleteTextures( 1, &(background2->texture) );
	
	//Fermeture du son
	Mix_FreeChunk( soundFX->loop );
	Mix_FreeChunk( soundFX->menuChange );
	Mix_FreeChunk( soundFX->menuSelection );
	Mix_CloseAudio();
	
	//Fermeture des variables GamePlay
	free( scene->blocs );
	free( scene->characters );
	free( scene );
	free( camera );
	free( controler );
	free( previousControler );
	free( menu->images );
	free( menu );
	free( soundFX );
	
	//Fermeture des variables Techniques
	SDL_Quit();
	return EXIT_SUCCESS;
}



