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

//Constantes
#define GRAVITY 0.006
#define MOVE 0.05
#define TRUE 1
#define FALSE 0

typedef struct Vector {
	float x;
	float y;
} Vector;

typedef struct Color {
	char R, G, B;
} Color;

typedef struct Bloc {
	Vector pos;
	Vector size;
} Bloc;

typedef struct Character {
	char * name;
	Bloc * bloc;
	Vector vel, acc;
	Color color;
	float jumpPower;
	char isSelected;
	char isOnGround;
	struct Character * onTop;
	char canJump;
} Character;

typedef struct Scene {
	Bloc * blocs;
	Character * characters;
	size_t numberOfBlocs;
	size_t numberOfCharacters;
	size_t numberOfPlatforms;
} Scene;

typedef struct Controler {
	char right, left, up, down;
	char click;
	char tab;
} Controler;

static const Uint32 FRAMERATE_MILLISECONDS = 1000 / 60;

void getControls( Controler * controler, Controler * controlerPast, Uint8 * key, size_t numberOfCharacters ) {
	controler->right = key[SDLK_RIGHT];
	controler->left = key[SDLK_LEFT];
	controler->up = key[SDLK_UP];
	if ( key[SDLK_TAB] &&  controler->tab==controlerPast->tab )
		controler->tab = (controler->tab + 1) % numberOfCharacters;
	
	if ( !key[SDLK_TAB] )
		*controlerPast = *controler;
}

void ease( float * base, float target, float step ) {
	if ( *base < target )
		*base += pow( *base - target, step );
	else if ( *base < target )
		*base -= pow( *base - target, step );
}

void drawRect( Bloc bloc ) {
	glPushMatrix();
		glTranslatef( bloc.pos.x, bloc.pos.y, 0.0 );
		glBegin( GL_QUADS );
			glVertex2f( 0, 0 );
			glVertex2f( bloc.size.x, 0 );
			glVertex2f( bloc.size.x, bloc.size.y );
			glVertex2f( 0, bloc.size.y );
		glEnd();
	glPopMatrix();
}

Scene * makeScene( void ) {
	Scene * scene = NULL;
	
	scene->numberOfPlatforms = 5;
	scene->numberOfCharacters = 3;
	scene->numberOfPlatforms = 2;
	
	scene->blocs = (Bloc*) calloc( 5, sizeof(Bloc) );
	scene->characters = (Character*) calloc( 3, sizeof(Character) );
	
	return scene;
}

int checkCollision( int side, Vector posObj, Vector sizeObj, Vector posRef, Vector sizeRef ) {
	switch (side) {
		case 0: // Haut
		
		break;
		
		case 1: // Droite
		if ( ( posObj.y + sizeObj.y > posRef.y ) && ( posObj.y < posRef.y + sizeRef.y ) && ( posObj.x + sizeObj.x > posRef.x ) && ( posObj.x < posRef.x ) )
			return TRUE;
		break;
		
		case 2: // Bas
		if ( ( posObj.x + sizeObj.x > posRef.x ) && ( posObj.x < posRef.x + sizeRef.x ) && ( posObj.y < posRef.y + sizeRef.y ) && ( posObj.y > posRef.y ) )
			return TRUE;
		break;
		
		case 3: // Gauche
		if ( ( posObj.y + sizeObj.y > posRef.y ) && ( posObj.y < posRef.y + sizeRef.y ) && ( posObj.x < posRef.x + sizeRef.x ) && ( posObj.x > posRef.x ) )
			return TRUE;
		break;
	}
	
	return FALSE;
}

void initBlocs( Scene * scene ) {
	int i;
	//Remplissage des personnages
	i = 0;
	scene->blocs[i].pos = { 0.0, 3.0 };
	scene->blocs[i].size = { 0.2, 1.0 };
	
	i = 1;
	scene->blocs[i].pos = { -3.0, 3.0 };
	scene->blocs[i].size = { 0.6, 0.6 };
	
	i = 2;
	scene->blocs[i].pos = { 3.0, 3.0 };
	scene->blocs[i].size= { 0.4, 0.8 };
	
	//Remplissage du terrain
	i = 3;
	scene->blocs[i].pos = { -5.0, -3.0 };
	scene->blocs[i].size = { 10.0, 2.0 };
	
	i = 4;
	scene->blocs[i].pos = { 2.0, -2.0 };
	scene->blocs[i].size = { 2.0, 4.0 };
}

void initCharacters( Scene * scene ) {
	int i;
	//Remplissage de l'équipage
	i = 0;
	scene->characters[i].name = "Louise";
	scene->characters[i].(*bloc) = &blocs[i];
	scene->characters[i].vel = { 0.0, 0.0 };
	scene->characters[i].acc = { 0.0, 0.0 };
	scene->characters[i].color = { 200, 150, 150 };
	scene->characters[i].isSelected = TRUE;
	scene->characters[i].jumpPower = 0.15;
	scene->characters[i].isOnGround = 0;
	scene->characters[i].onTop = NULL;
	scene->characters[i].canJump = 1;
	
	i = 1;
	scene->characters[i].name = "Nino";
	scene->characters[i].(*bloc) = &blocs[i];
	scene->characters[i].vel.x = { 0.0, 0.0 };
	scene->characters[i].acc.x = { 0.0, 0.0 };
	scene->characters[i].color = { 150, 110, 110 };
	scene->characters[i].isSelected = FALSE;
	scene->characters[i].jumpPower = 0.07;
	scene->characters[i].isOnGround = 0;
	scene->characters[i].onTop = NULL;
	scene->characters[i].canJump = 1;
	
	i = 2;
	scene->characters[i].name = "Timour";
	scene->characters[i].(*bloc) = &blocs[i];
	scene->characters[i].vel.x = { 0.0, 0.0 };
	scene->characters[i].acc.x = { 0.0, 0.0 };
	scene->characters[i].color = { 220, 190, 190 };
	scene->characters[i].isSelected = FALSE;
	scene->characters[i].jumpPower = 0.1;
	scene->characters[i].isOnGround = 0;
	scene->characters[i].onTop = NULL;
	scene->characters[i].canJump = 1;
}

void displayPlatforms( Scene * scene ) {
	int i;
	//Dessin du tableau
	glColor3ub( 0, 0, 0 );
	for ( i=scene->numberOfCharacters-1; i<scene->numberOfBlocs; i++ )
		drawRect( scene->blocs[i] );
}

void displayCharacters( Scene * scene, Controler * controler ) {
	int i, j;
	Character * C = scene->characters;
	Blocs * B = scene->blocs;
	for ( i=0; i<numberOfCharacters; i++ ) {
		if ( controler->tab == i )
			C[i].isSelected = 1;
		else
			C[i].isSelected = 0;
		
		if ( C[i].isSelected ) {

			//Gestion commande droite
			if ( controler->right ) {
				C[i].pos.x += MOVE;
				j = 0;
				int collided = 0;
				while ( !collided && j<sizeScene ) {
					if ( checkCollision( 1, C[i].pos, C[i].size, B[j].pos, B[j].size ) ) {
						collided = 1;
						C[i].pos.x -= MOVE;
					}
					j++;
				}
			}
			
			//Gestion commande gauche
			if ( controler->left ) {
				C[i].pos.x -= MOVE;
				j = 0;
				int collided = 0;
				while ( !collided && j<sizeScene ) {
					if ( checkCollision( 3, C[i].pos, C[i].size, B[j].pos, B[j].size ) ) {
						collided = 1;
						C[i].pos.x += MOVE;
					}
					j++;
				}
			}
			
			//Gestion commande haut
			if ( controler->up && C[i].isOnGround ) {
				C[i].isOnGround = 0;
				C[i].vel.y = C[i].jumpPower;
			}
		}
		
		//Gestion vitesse
		C[i].vel.x += characters[i].acc.x;		C[i].vel.y += C[i].acc.y;
		C[i].pos.x += characters[i].vel.x;		C[i].pos.y += C[i].vel.y;
		
		//Gestion gravité
		j = 0;
		C[i].isOnGround = 0;
		while ( !C[i].isOnGround && j<sizeScene ) {
			if ( checkCollision( 2, C[i].pos, C[i].size, B[j].pos, B[j].size ) ) {
				C[i].isOnGround = 1;
				C[i].acc.y = 0;
				C[i].vel.y = 0;
				C[i].pos.y = B[j].pos.y + B[j].size.y;
			} else
				C[i].acc.y = -GRAVITY;
			
			j++;
		}
		
		//Dessin du personnage
		if ( C[i].isSelected ) {
			glColor3ub( 0, 0, 0 );
			Bloc selector = { C[i].pos.x + C[i].size.x/2 - 0.05, C[i].pos.y + C[i].size.y + 0.1, 0.1, 0.1 };
			drawRect( selector );
		}
		glColor3ub( C[i].color.R, C[i].color.G, C[i].color.B );
		drawRect( C[i] );
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
	Scene * scene = makeScene();
	
	//Initialisation des variables Techniques
	font = TTF_OpenFont( "sources/lekton_regular.ttf", 16 );
	textSurface = TTF_RenderText_Blended( font, "Thomas Was Alone, The Game", fontColor );
	textRect.x = windowWidth / 2;	textRect.y = windowHeight /2;
	textRect.w = textSurface->w;	textRect.h = textSurface->h;
	
	//Initialisation des variables Gamplay
	initBlocs( scene );
	initCharacters( scene );
	
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
		
		getControls( &controler, &controlerPast, key, numberOfCharacters );
		displayPlatforms( blocs, sizeScene );
		displayCharacters( scene, &controler );


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
	free( blocs );
	free( characters );
	
	//Fermeture des variables Techniques
	SDL_FreeSurface( textSurface );
	TTF_CloseFont( font );
	TTF_Quit();
	SDL_Quit();
	return EXIT_SUCCESS;
}



