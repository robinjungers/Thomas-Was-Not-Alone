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
#include <string.h>

//Constantes
#define GRAVITY 0.07
#define STEP 1.0
#define TRUE 1
#define FALSE 0

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

#define MAX_BLOC_NUMBER 50
#define MAX_CHARACTER_NUMBER 3

typedef struct Vector {
	float x;
	float y;
} Vector;

typedef struct Color {
	unsigned char R, G, B;
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
	Vector minDimension, maxDimension;
} Scene;

typedef struct Controler {
	int right, left, up, down;
	int click;
	int tab;
} Controler;

typedef struct Camera {
	Vector pos;
	Vector posToReach;
	float zoom;
} Camera;

static const Uint32 FRAMERATE_MILLISECONDS = 1000 / 60;

void easeValue( float * base, float target, float step ) {
	if ( *base < target )
		*base += floor(pow( target - *base, step ));
	else if ( *base > target )
		*base -= floor(pow( *base - target, step ));
}

void easeVector( Vector * base, Vector target, float step ) {
	easeValue( &(base->x), target.x, step );
	easeValue( &(base->y), target.y, step );
}

float mapValue( float value, float a, float b, float c, float d ) {
	return c + (d - c) * (value - a)/(b - a);
}

float dist( Vector A, Vector B ) {
	return sqrt( pow( A.x-B.x, 2 ) + pow( A.y-B.y, 2 ) );
}

Scene * makeScene() {
	Scene * scene = (Scene*) calloc( 1, sizeof(Scene));
	
	scene->blocs = (Bloc*) calloc( MAX_BLOC_NUMBER, sizeof(Bloc) );
	scene->characters = (Character*) calloc( MAX_CHARACTER_NUMBER, sizeof(Character) );
	
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

Camera * makeCamera() {
	Camera * camera = (Camera *) calloc( 1, sizeof(Camera) );
	
	camera->pos = (Vector) { 0.0, 0.0 };
	camera->posToReach = camera->pos;
	camera->zoom = 1.0;
	
	return camera;
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

void drawImage( GLuint textureId, Bloc screenRect ) {
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

int initBlocsFromFile( Scene * scene, const char * filename ) {
	//Remplissage des personnages
    FILE * file = fopen ( filename, "r" );
    char tmp[10];
    int currentTmp = 0;
	int currentParameter = 0, currentIndex = 0;
    int c;

    if ( file==NULL )
		return EXIT_FAILURE;
	
	scene->numberOfBlocs = 0;
	
	printf( "\n----- Reading Blocs File : -----\n\n" );
	
    do {
		// Récupération du prochain caractère du fichier dans la mémoire tempon
		c = fgetc( file );
		if ( c == ',' || c == ';' ) {
			// Ajout du paramètre courant
			switch (currentParameter) {
				case 0:
				scene->blocs[currentIndex].pos.x = atof(tmp);
				break;
				
				case 1:
				scene->blocs[currentIndex].pos.y = atof(tmp);
				break;
				
				case 2:
				scene->blocs[currentIndex].size.x = atof(tmp);
				break;
				
				case 3:
				scene->blocs[currentIndex].size.y = atof(tmp);
				break;
			}
			currentParameter++;
			
			// RàZ du tampon
			currentTmp = 0;
			memset( tmp, 0, sizeof tmp );
		} else if( (c == '\n') || (c == EOF) ) {
			// Finalisation du bloc courant
			scene->numberOfBlocs++;
			scene->blocs[currentIndex].previousPos = scene->blocs[currentIndex].pos;
			printf( "New bloc: pos( %f, %f ), size( %f, %f )\n",
					scene->blocs[currentIndex].pos.x, scene->blocs[currentIndex].pos.y,
					scene->blocs[currentIndex].size.x, scene->blocs[currentIndex].size.y );
			
			// Retour au premier paramètre
			currentParameter = 0;
			currentIndex++;
			
			// RàZ du tampon
			currentTmp = 0;
			memset( tmp, 0, sizeof tmp );
		} else {
			if ( currentTmp > 8 ) // Pour ne pas dépasser la taille maximale du tampon
				return EXIT_FAILURE;
			tmp[currentTmp++] = c;
		}
    } while ( c != EOF && currentIndex<MAX_BLOC_NUMBER );
	
	fclose( file );
	return EXIT_SUCCESS;
}

int initCharactersFromFile( Scene * scene, const char * filename ) {
	//Remplissage des personnages
    FILE * file = fopen ( filename, "r" );
    char tmp[10];
    int currentTmp = 0;
	int currentParameter = 0, currentIndex = 0;
    int c;

    if ( file==NULL )
		return EXIT_FAILURE;
	
	scene->numberOfCharacters = 0;
	
	printf( "\n----- Reading Characters File : -----\n\n" );
	
    do {
		// Récupération du prochain caractère du fichier dans la mémoire tempon
		c = fgetc( file );
		if ( c == ',' || c == ';' ) {
			// Ajout du paramètre courant
			//printf( "Found value: %f\n", atof(tmp) );
			switch (currentParameter) {
				case 0:
				scene->characters[currentIndex].name = tmp;
				break;
				
				case 1:
				scene->characters[currentIndex].color.R = atoi(tmp);
				break;
				
				case 2:
				scene->characters[currentIndex].color.G = atoi(tmp);
				break;
				
				case 3:
				scene->characters[currentIndex].color.B = atoi(tmp);
				break;
				
				case 4:
				scene->characters[currentIndex].jumpPower = atof(tmp);
				break;
			}
			currentParameter++;
			
			// RàZ du tampon
			currentTmp = 0;
			memset( tmp, 0, sizeof tmp );
		} else if( (c == '\n') || (c == EOF) ) {
			// Finalisation du bloc courant
			scene->numberOfCharacters++;
			scene->characters[currentIndex].bloc = &(scene->blocs[currentIndex]);
			scene->characters[currentIndex].vel = (Vector) { 0.0, 0.0 };
			scene->characters[currentIndex].acc = (Vector) { 0.0, 0.0 };
			scene->characters[currentIndex].isSelected = FALSE;
			scene->characters[currentIndex].parent = NULL;
			scene->characters[currentIndex].canJump = TRUE;
			printf( "New Character: name( %s ), color( %u, %u, %u ), jump( %.2f )\n",
					scene->characters[currentIndex].name,
					scene->characters[currentIndex].color.R, scene->characters[currentIndex].color.G, scene->characters[currentIndex].color.B,
					scene->characters[currentIndex].jumpPower );
			
			// Retour au premier paramètre
			currentParameter = 0;
			currentIndex++;
			
			// RàZ du tampon
			currentTmp = 0;
			memset( tmp, 0, sizeof tmp );
		} else {
			if ( currentTmp > 8 ) // Pour ne pas dépasser la taille maximale du tampon
				return EXIT_FAILURE;
			tmp[currentTmp++] = c;
		}
    } while ( c != EOF && currentIndex<MAX_CHARACTER_NUMBER );
	
	scene->characters[0].isSelected = TRUE;
	
	fclose( file );
	return EXIT_SUCCESS;
}

void getSceneDimensions( Scene * scene ) {
	int i;
	scene->maxDimension = (Vector) { -100000, -100000 };
	scene->minDimension = (Vector) { 100000, 100000 };
	
	for ( i=0; i<scene->numberOfBlocs; i++ ) {
		if (scene->blocs[i].pos.x + scene->blocs[i].size.x > scene->maxDimension.x)
			scene->maxDimension.x = scene->blocs[i].pos.x + scene->blocs[i].size.x;
		if (scene->blocs[i].pos.y + scene->blocs[i].size.y > scene->maxDimension.y)
			scene->maxDimension.y = scene->blocs[i].pos.y + scene->blocs[i].size.y;
		if (scene->blocs[i].pos.x < scene->minDimension.x)
			scene->minDimension.x = scene->blocs[i].pos.x;
		if (scene->blocs[i].pos.y < scene->minDimension.y)
			scene->minDimension.y = scene->blocs[i].pos.y;
	}
	
	printf( "\nScene dimensions: %f, %f\n", scene->maxDimension.x - scene->minDimension.x, scene->maxDimension.y - scene->minDimension.y );
}

void displayPlatforms( Scene * scene ) {
	int i;
	//Dessin du tableau
	glColor3ub( 0, 0, 0 );
	for ( i=scene->numberOfCharacters; i<scene->numberOfBlocs; i++ )
		drawRect( scene->blocs[i] );
}

void displayCharacters( Scene * scene, Controler controler ) {
	
// ********************************************** INITIALISATION *********************************************
	int i, j;
	Character * C = scene->characters;
	Bloc * B = scene->blocs;
	
	// Début de boucle sur tous les personnages
	for ( i=0; i<scene->numberOfCharacters; i++ ) {
		
// ******************************************** GESTION SELECTEUR ********************************************
		if ( controler.tab == i )
			C[i].isSelected = TRUE;
		else
			C[i].isSelected = FALSE;
		
		// Si un personnage est sélectionné
		if ( C[i].isSelected ) {

// ******************************************** GESTION COMMANDES ********************************************
			if ( controler.right )
				C[i].bloc->pos.x += STEP;
			if ( controler.left )
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
			if ( controler.up && C[i].isOnGround ) {
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

void getControls( Controler * controler, Controler * controlerPast, Uint8 * key, size_t numberOfCharacters ) {
	controler->right = key[SDLK_RIGHT];
	controler->left = key[SDLK_LEFT];
	controler->up = key[SDLK_UP];
	if ( key[SDLK_TAB] &&  controler->tab==controlerPast->tab )
		controler->tab = (controler->tab + 1) % numberOfCharacters;
	
	if ( !key[SDLK_TAB] )
		*controlerPast = *controler;
}

void updateCamera( Camera * camera, Scene * scene ) {
	int i;
	for ( i=0; i<scene->numberOfCharacters; i++ ) {
		if ( scene->characters[i].isSelected ) {
			camera->pos.x = mapValue( scene->characters[i].bloc->pos.x,
				 					  scene->minDimension.x, scene->maxDimension.x,
									  scene->minDimension.x + 70, scene->maxDimension.x - 70 );
			camera->pos.y = mapValue( scene->characters[i].bloc->pos.y,
								 	  scene->minDimension.y, scene->maxDimension.y,
								 	  scene->minDimension.y + 40, scene->maxDimension.y - 40 );
		}
	}
	camera->zoom = 0.9;
	//easeVector( &(camera->pos), camera->posToReach, 0.2 );
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
	Scene * scene = makeScene();
	Camera * camera = makeCamera();
	
	//Initialisation de la scène
	initBlocsFromFile( scene, "data/blocs2.txt" );
	initCharactersFromFile( scene, "data/characters.txt" );
	getSceneDimensions( scene );
	
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
		
		drawImage( background, (Bloc) {
				(Vector) { -80, -50 },
				(Vector) { 0, 0 },
				(Vector) { 160, 100 }
			});
		
		updateCamera( camera, scene );
		
		glRotatef( -1, 0.0, 0.0, 1.0 );
		glScalef( camera->zoom, camera->zoom, camera->zoom );
		glTranslatef( -camera->pos.x, -camera->pos.y, 0.0 );
		
		getControls( &controler, &controlerPast, key, scene->numberOfCharacters );
		displayPlatforms( scene );
		displayCharacters( scene, controler );
		
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



