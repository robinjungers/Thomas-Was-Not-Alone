#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
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
#define GRAVITY 0.06
#define STEP 1.0
#define TRUE 1
#define FALSE 0

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

typedef struct Vector {
	float x;
	float y;
} Vector;

typedef struct Color {
	char R, G, B;
} Color;

typedef struct Bloc {
	Vector pos;
	Vector previousPos;
	Vector size;
} Bloc;

typedef struct Character {
	char * name;
	Bloc * bloc;
	Vector vel, acc;
	Color color;
	float jumpPower;
	int isSelected;
	int isOnGround;
	Bloc * parent;
	int canJump;
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

float dist( Vector A, Vector B ) {
	return sqrt( pow( A.x-B.x, 2 ) + pow( A.y-B.y, 2 ) );
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

Scene * makeScene( int nbPlatforms, int nbCharacters ) {
	Scene * scene = (Scene*) calloc( 1, sizeof(Scene));
	
	scene->numberOfBlocs = nbCharacters + nbPlatforms;
	scene->numberOfCharacters = nbCharacters;
	scene->numberOfPlatforms = nbPlatforms;
	
	scene->blocs = (Bloc*) calloc( scene->numberOfBlocs, sizeof(Bloc) );
	scene->characters = (Character*) calloc( scene->numberOfCharacters, sizeof(Character) );
	
	return scene;
}

GLuint makeImage( const char * filename ) {
	SDL_Surface * image = IMG_Load( filename );
	if ( image==NULL ) {
		printf("Error loading texture.\n");
		return EXIT_FAILURE;
	}
	GLuint textureId;
	glGenTextures( 1, &textureId );
	glBindTexture( GL_TEXTURE_2D, textureId );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	
	GLenum format;
	switch(image->format->BytesPerPixel) {
		case 1:
			format = GL_RED;
			break;
		case 3:
			format = GL_BGR;
			break;
		case 4:
			format = GL_BGRA;
			break;
		default:
			fprintf(stderr, "Format des pixels de l'image non pris en charge\n");
		return EXIT_FAILURE;
	}
	
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, image->w, image->h, 0, format, GL_UNSIGNED_BYTE, image->pixels );
	glBindTexture( GL_TEXTURE_2D, 0 );
	SDL_FreeSurface( image );
	
	return textureId;
}

int checkCollision( int side, Bloc obj, Bloc ref ) {
	switch (side) {
		case UP:
		if ( ( obj.pos.x + obj.size.x > ref.pos.x ) && ( obj.pos.x < ref.pos.x + ref.size.x ) && ( obj.pos.y + obj.size.y > ref.pos.y ) && ( obj.pos.y < ref.pos.y ) )
			return TRUE;
		break;
		
		case RIGHT:
		if ( ( obj.pos.y + obj.size.y > ref.pos.y ) && ( obj.pos.y < ref.pos.y + ref.size.y ) && ( obj.pos.x + obj.size.x > ref.pos.x ) && ( obj.pos.x < ref.pos.x ) )
			return TRUE;
		break;
		
		case DOWN:
		if ( ( obj.pos.x + obj.size.x > ref.pos.x ) && ( obj.pos.x < ref.pos.x + ref.size.x ) && ( obj.pos.y < ref.pos.y + ref.size.y ) && ( obj.pos.y > ref.pos.y ) )
			return TRUE;
		break;
		
		case LEFT:
		if ( ( obj.pos.y + obj.size.y > ref.pos.y ) && ( obj.pos.y < ref.pos.y + ref.size.y ) && ( obj.pos.x < ref.pos.x + ref.size.x ) && ( obj.pos.x > ref.pos.x ) )
			return TRUE;
		break;
	}
	
	return FALSE;
}

void initBlocs( Scene * scene ) {
	int i;
	//Remplissage des personnages
	i = 0;
	scene->blocs[i].pos = (Vector) { 0.0, 30.0 };
	scene->blocs[i].previousPos = scene->blocs[i].pos;
	scene->blocs[i].size = (Vector) { 2.0, 10.0 };
	
	i = 1;
	scene->blocs[i].pos = (Vector) { -30.0, 30.0 };
	scene->blocs[i].previousPos = scene->blocs[i].pos;
	scene->blocs[i].size = (Vector) { 6.0, 6.0 };
	
	i = 2;
	scene->blocs[i].pos = (Vector) { 30.0, 30.0 };
	scene->blocs[i].previousPos = scene->blocs[i].pos;
	scene->blocs[i].size= (Vector) { 4.0, 8.0 };
	
	//Remplissage du terrain
	i = 3;
	scene->blocs[i].pos = (Vector) { -50.0, -30.0 };
	scene->blocs[i].previousPos = scene->blocs[i].pos;
	scene->blocs[i].size = (Vector) { 100.0, 20.0 };
	
	i = 4;
	scene->blocs[i].pos = (Vector) { 20.0, -20.0 };
	scene->blocs[i].previousPos = scene->blocs[i].pos;
	scene->blocs[i].size = (Vector) { 20.0, 40.0 };
	
	i = 5;
	scene->blocs[i].pos = (Vector) { -30.0, 0.0 };
	scene->blocs[i].previousPos = scene->blocs[i].pos;
	scene->blocs[i].size = (Vector) { 20.0, 20.0 };
}

void initCharacters( Scene * scene ) {
	int i;
	//Remplissage de l'équipage
	i = 0;
	scene->characters[i].name = "Louise";
	scene->characters[i].bloc = &scene->blocs[i];
	scene->characters[i].vel = (Vector) { 0.0, 0.0 };
	scene->characters[i].acc = (Vector) { 0.0, 0.0 };
	scene->characters[i].color = (Color) { 200, 150, 150 };
	scene->characters[i].isSelected = TRUE;
	scene->characters[i].jumpPower = 2.0;
	scene->characters[i].isOnGround = FALSE;
	scene->characters[i].parent = NULL;
	scene->characters[i].canJump = TRUE;
	
	i = 1;
	scene->characters[i].name = "Nino";
	scene->characters[i].bloc = &scene->blocs[i];
	scene->characters[i].vel = (Vector) { 0.0, 0.0 };
	scene->characters[i].acc = (Vector) { 0.0, 0.0 };
	scene->characters[i].color = (Color) { 150, 110, 110 };
	scene->characters[i].isSelected = FALSE;
	scene->characters[i].jumpPower = 1.0;
	scene->characters[i].isOnGround = FALSE;
	scene->characters[i].parent = NULL;
	scene->characters[i].canJump = TRUE;
	
	i = 2;
	scene->characters[i].name = "Timour";
	scene->characters[i].bloc = &scene->blocs[i];
	scene->characters[i].vel = (Vector) { 0.0, 0.0 };
	scene->characters[i].acc = (Vector) { 0.0, 0.0 };
	scene->characters[i].color = (Color) { 220, 190, 190 };
	scene->characters[i].isSelected = FALSE;
	scene->characters[i].jumpPower = 1.5;
	scene->characters[i].isOnGround = FALSE;
	scene->characters[i].parent = NULL;
	scene->characters[i].canJump = TRUE;
}

void displayPlatforms( Scene * scene ) {
	int i;
	//Dessin du tableau
	glColor3ub( 0, 0, 0 );
	for ( i=scene->numberOfCharacters; i<scene->numberOfBlocs; i++ )
		drawRect( scene->blocs[i] );
}

void displayCharacters( Scene * scene, Controler * controler ) {
	
// ********************************************** INITIALISATION *********************************************
	int i, j;
	Character * C = scene->characters;
	Bloc * B = scene->blocs;
	
	// Début de boucle sur tous les personnages
	for ( i=0; i<scene->numberOfCharacters; i++ ) {
		
// ******************************************** GESTION SELECTEUR ********************************************
		if ( controler->tab == i )
			C[i].isSelected = TRUE;
		else
			C[i].isSelected = FALSE;
		
		// Si un personnage est sélectionné
		if ( C[i].isSelected ) {

// ******************************************** GESTION COMMANDES ********************************************
			if ( controler->right )
				C[i].bloc->pos.x += STEP;
			if ( controler->left )
				C[i].bloc->pos.x -= STEP;	
			
			j = 0;
			int collided = FALSE;
			while ( !collided && j<scene->numberOfBlocs ) {
				if ( j!=i && ( checkCollision( RIGHT, *(C[i].bloc), B[j] ) || checkCollision( LEFT, *(C[i].bloc), B[j] ) )) {
					collided = TRUE;
					C[i].bloc->pos.x = C[i].bloc->previousPos.x;
				}
				j++;
			}
			
			
// ****************************************** GESTION COMMANDE HAUT ******************************************
			if ( controler->up && C[i].isOnGround ) {
				C[i].isOnGround = FALSE;
				C[i].vel.y = C[i].jumpPower;
			}
		}
		
		
// ***************************************** GESTION MOTEUR PHYSIQUE *****************************************
		
		C[i].vel.x += C[i].acc.x;			C[i].vel.y += C[i].acc.y;
		C[i].bloc->pos.x += C[i].vel.x;		C[i].bloc->pos.y += C[i].vel.y;
		
		
// ********************************************* GESTION GRAVITE *********************************************
		
		j = 0;
		C[i].isOnGround = FALSE;
		C[i].parent = NULL;
		while ( !C[i].isOnGround && j<scene->numberOfBlocs ) {
			
			// Si collision par le haut
			if ( j!=i && checkCollision( UP, *(C[i].bloc), B[j] ) ) {
				C[i].acc.y = 0;
				C[i].vel.y = 0;
				C[i].bloc->pos.y = B[j].pos.y - C[i].bloc->size.y;
			}
			
			// Si collision par le bas
			else if ( j!=i && checkCollision( DOWN, *(C[i].bloc), B[j] ) ) {
				C[i].isOnGround = TRUE;
				C[i].acc.y = 0;
				C[i].vel.y = 0;
				C[i].bloc->pos.y = B[j].pos.y + B[j].size.y;
				
				// Création d'un lien de parenté
				C[i].parent = &B[j];
			}
			
			// Sinon, chute
			else
				C[i].acc.y = -GRAVITY;
			
			j++;
		}
		
// ****************************************** DESSIN DU PERSONNAGE ******************************************
		
		glColor3ub( C[i].color.R, C[i].color.G, C[i].color.B );
		drawRect( *(C[i].bloc) );
	}
	//printf( "\nPrevious:%f, Current:%f\n", scene->blocs[0].previousPos.x, scene->blocs[0].pos.x );
	for ( i=0; i<scene->numberOfCharacters; i++ ) {
		if ( C[i].isSelected ) {
			glColor3ub( 230, 30, 30 );
			Bloc selector = { 
				(Vector) { C[i].bloc->pos.x + C[i].bloc->size.x/2 - 0.5, C[i].bloc->pos.y + C[i].bloc->size.y + 1.0 },
				(Vector) { 0, 0 },
				(Vector) { 1.0, 1.0 }
			};
			drawRect( selector );
		}
		
		if ( C[i].parent!=NULL ) {
			C[i].bloc->pos.x += ( C[i].parent->pos.x - C[i].parent->previousPos.x );
			j = 0;
			int collided = FALSE;
			while ( !collided && j<scene->numberOfBlocs ) {
				if ( j!=i && ( checkCollision( LEFT, *(C[i].bloc), B[j] ) || checkCollision( RIGHT, *(C[i].bloc), B[j] ) )) {
					collided = TRUE;
					C[i].bloc->pos.x = C[i].bloc->previousPos.x;
				}
				j++;
			}
		}
	}
	
	
// *************************************** MàJ DE L'HISTORIQUE DE POS ***************************************
	
	for ( i=0; i<scene->numberOfBlocs; i++ )
		B[i].previousPos = B[i].pos;
	
}

void displayImage( GLuint textureId, Bloc screenRect ) {
	glBindTexture( GL_TEXTURE_2D, textureId );
	glBegin( GL_QUADS );
		glColor3f( 1., 1., 1. );
		
		glTexCoord2f( 0., 1. );
		glVertex2f( screenRect.pos.x, screenRect.pos.y );
		
		glTexCoord2f( 1., 1. );
		glVertex2f( screenRect.pos.x + screenRect.size.x, screenRect.pos.y );
		
		glTexCoord2f( 1., 0. );
		glVertex2f( screenRect.pos.x + screenRect.size.x, screenRect.pos.y + screenRect.size.y );
		
		glTexCoord2f( 0., 0. );
		glVertex2f( screenRect.pos.x, screenRect.pos.y + screenRect.size.y );
	glEnd();
	glBindTexture( GL_TEXTURE_2D, 0 );
}

int main() {
	
	SDL_Init( SDL_INIT_VIDEO );
	
	//Variables Techniques
	unsigned int windowWidth  = 800;
	unsigned int windowHeight = 500;
	
	//Gestion Anti aliasing
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 6 );
	
	SDL_SetVideoMode( windowWidth, windowHeight, 32, SDL_OPENGL );
	SDL_WM_SetCaption("Thomas Was Alone V2.0", NULL);
	
	//Variables Gameplay
	Uint8 * key = SDL_GetKeyState(NULL);
	Controler controler = { 0, 0, 0, 0, 0, 0 };
	Controler controlerPast = { 0, 0, 0, 0, 0, 0 };
	Scene * scene = makeScene( 3, 3 );
	
	//Initialisation de la scène
	initBlocs( scene );
	initCharacters( scene );
	
	//Initialisation des textures
	GLuint background = makeImage( "img/back1.jpg" );
	
	//Gestion matrice d'affichage
	glViewport( 0, 0, windowWidth, windowHeight );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D( -80.0, 80.0, -50.0, 50.0);
	glPointSize( 4 ); 
	
	int loop = 1;
	while (loop) {
		Uint32 startTime = SDL_GetTicks();
		key = SDL_GetKeyState(NULL);
		
		glClearColor( 0.95f, 0.85f, 0.85f, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT );
		glEnable( GL_TEXTURE_2D );
		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();
		
		/////////////////////////////////////* DEBUT DESSIN *///////////////////////////////////////
		displayImage( background, (Bloc) {
				(Vector) { -80, -50 },
				(Vector) { 0, 0 },
				(Vector) { 160, 100 }
			});
		
		glRotatef( -1, 0.0, 0.0, 1.0 );
		
		getControls( &controler, &controlerPast, key, scene->numberOfCharacters );
		displayPlatforms( scene );
		displayCharacters( scene, &controler );
		
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
	
	//Fermeture des textures
	glDeleteTextures( 1, &background );
	
	//Fermeture des variables GamePlay
	free( scene->blocs );
	free( scene->characters );
	free( scene );
	
	//Fermeture des variables Techniques
	SDL_Quit();
	return EXIT_SUCCESS;
}



