#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
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

//Librairie
#include "structs.h"
#include "functions.h"

//Initialisations
#include "inits.h"

//Constantes
#define GRAVITY 0.006
#define MOVE 0.05

static const Uint32 FRAMERATE_MILLISECONDS = 1000 / 60;

void displayLand( Bloc * myLand, size_t n ) {
	int i;
	//Dessin du tableau
	glColor3ub( 0, 0, 0 );
	for ( i=0; i<n; i++ )
		drawRect( myLand[i].pos, myLand[i].size );
}

void displayCrew( Controler * controler, Character * myCrew, size_t sizeCrew, Bloc * myLand, size_t sizeLand ) {
	int i, j;
	for ( i=0; i<sizeCrew; i++ ) {
		if ( controler->tab == i )
			myCrew[i].isSelected = 1;
		else
			myCrew[i].isSelected = 0;
		
		if ( myCrew[i].isSelected ) {

			//Gestion commande droite
			if ( controler->right ) {
				myCrew[i].pos.x += MOVE;
				j = 0;
				char collided = 0;
				while ( !collided && j<sizeLand ) {
					if ( checkCollision( 1, myCrew[i].pos, myCrew[i].size, myLand[j].pos, myLand[j].size ) ) {
						collided = 1;
						myCrew[i].pos.x -= MOVE;
					}
					j++;
				}
			}
			
			//Gestion commande gauche
			if ( controler->left ) {
				myCrew[i].pos.x -= MOVE;
				j = 0;
				char collided = 0;
				while ( !collided && j<sizeLand ) {
					if ( checkCollision( 3, myCrew[i].pos, myCrew[i].size, myLand[j].pos, myLand[j].size ) ) {
						collided = 1;
						myCrew[i].pos.x += MOVE;
					}
					j++;
				}
			}
			
			//Gestion commande haut
			if ( controler->up && myCrew[i].isOnGround ) {
				myCrew[i].isOnGround = 0;
				myCrew[i].vel.y = myCrew[i].jumpPower;
			}
		}
		
		//Gestion vitesse
		myCrew[i].vel.x += myCrew[i].acc.x;		myCrew[i].vel.y += myCrew[i].acc.y;
		myCrew[i].pos.x += myCrew[i].vel.x;		myCrew[i].pos.y += myCrew[i].vel.y;
		
		//Gestion gravité
		j = 0;
		myCrew[i].isOnGround = 0;
		while ( !myCrew[i].isOnGround && j<sizeLand ) {
			if ( checkCollision( 2, myCrew[i].pos, myCrew[i].size, myLand[j].pos, myLand[j].size ) ) {
				myCrew[i].isOnGround = 1;
				myCrew[i].acc.y = 0;
				myCrew[i].vel.y = 0;
				myCrew[i].pos.y = myLand[j].pos.y + myLand[j].size.y;
			} else {
				myCrew[i].acc.y = -GRAVITY;
			}
			j++;
		}
		
		//Dessin du personnage
		if ( myCrew[i].isSelected ) {
			glColor3ub( 0, 0, 0 );
			Bloc selector = { myCrew[i].pos.x + myCrew[i].size.x/2 - 0.05, myCrew[i].pos.y + myCrew[i].size.y + 0.1, 0.1, 0.1 };
			drawRect( selector.pos, selector.size );
		}
		glColor3ub( myCrew[i].R, myCrew[i].G, myCrew[i].B );
		drawRect( myCrew[i].pos, myCrew[i].size );
	}
}

int main(int argc, char** argv) {
	
	SDL_Init( SDL_INIT_VIDEO );
	TTF_Init();
	
	//Variables Techniques
	unsigned int windowWidth  = 800;
	unsigned int windowHeight = 500;
	TTF_Font * font = NULL;
	SDL_Color fontColor = { 0, 0, 0 };
	SDL_Surface * textSurface = NULL, * mainScreen = NULL;
	SDL_Rect textRect, textRect_copy;
	Uint8 * key = SDL_GetKeyState(NULL);
	
	//Variables Gameplay
	Controler controler = { 0, 0, 0, 0, 0, 0 };
	Controler controlerPast = { 0, 0, 0, 0, 0, 0 };
	size_t sizeLand = 2;
	Bloc * myLand = (Bloc*) calloc( sizeLand, sizeof(Bloc) );
	size_t sizeCrew = 3;
	Character * myCrew = (Character*) calloc( sizeCrew, sizeof(Character) );
	
	//Initialisation des variables Techniques
	font = TTF_OpenFont( "sources/lekton_regular.ttf", 16 );
	textSurface = TTF_RenderText_Blended( font, "Thomas Was Alone, The Game", fontColor );
	textRect.x = windowWidth / 2;	textRect.y = windowHeight /2;
	textRect.w = textSurface->w;	textRect.h = textSurface->h;
	
	//Initialisation des variables Gamplay
	initLand( myLand, sizeLand );
	initCrew( myCrew, sizeCrew );
	
	//Gestion Anti aliasing
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 6);
	
	mainScreen = SDL_SetVideoMode( windowWidth, windowHeight, 32, SDL_OPENGL );
	SDL_WM_SetCaption("Thomas Was Alone V1.0", NULL);
	
	//Gestion matrice d'affichage
	glViewport( 0, 0, windowWidth, windowHeight );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D( -8.0, 8.0, -5.0, 5.0);
	glPointSize(4); 
	
	int loop = 1;
	while (loop) {
		Uint32 startTime = SDL_GetTicks();
		key = SDL_GetKeyState(NULL);
		
		glClearColor( 0.95f, 0.85f, 0.85f, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT );
		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();
		
		/////////////////////////////////////* DEBUT DESSIN *///////////////////////////////////////
		
		glRotatef( -1, 0.0, 0.0, 1.0 );
		
		getControls( &controler, &controlerPast, key, sizeCrew );
		displayLand( myLand, sizeLand );
		displayCrew( &controler, myCrew, sizeCrew, myLand, sizeLand );


		/* NON FONCTIONNEL 
		textRect_copy = textRect;
		printf( "X:%d, Y:%d, W:%d, H:%d\n", textRect_copy.x, textRect_copy.y, textRect_copy.w, textRect_copy.h );
		int error = SDL_BlitSurface( textSurface, NULL, mainScreen, &textRect_copy );
		*/
		
		
		
		//////////////////////////////////////* FIN DESSIN *////////////////////////////////////////
		
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
	
	//Fermeture des variables GamePlay
	free( myLand );
	free( myCrew );
	
	//Fermeture des variables Techniques
	SDL_FreeSurface( textSurface );
	TTF_CloseFont( font );
	TTF_Quit();
	SDL_Quit();
	return EXIT_SUCCESS;
}



