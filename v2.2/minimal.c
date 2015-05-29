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
#define MAX_MENUTITLE_NUMBER 3

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
	struct Bloc * parent;
} Bloc;

typedef struct Image {
	GLuint texture;
	Bloc rect;
} Image;

typedef struct Character {
	char * name;
	Bloc * bloc;
	Vector vel, acc;
	Vector posToReach;
	Color color;
	float jumpPower;
	int isSelected;
	int isOnGround;
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
	int tab;
	int escape, enter;
} Controler;

typedef struct Camera {
	Vector pos;
	Vector posToReach;
	float zoom;
} Camera;

typedef struct Menu {
	int active;
	int selectedLevel;
	Image * images;
} Menu;

static const Uint32 FRAMERATE_MILLISECONDS = 1000 / 60;

void easeValue( float * base, float target, float step ) {
	if ( *base < target )
		*base += (pow( target - *base, step ));
	else if ( *base > target )
		*base -= (pow( *base - target, step ));
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

Vector vector( float x, float y ) {
	Vector v = { x, y };
	return v;
}

Bloc bloc( float x, float y, float a, float b ) {
	Bloc bloc;
	bloc.pos = vector( x, y );
	bloc.size = vector( a, b );
	return bloc;
}

Color color( char R, char G, char B ) {
	Color color = { R, G, B };
	return color;
}

Scene * makeScene() {
	Scene * scene = (Scene*) calloc( 1, sizeof(Scene));
	
	scene->blocs = (Bloc*) calloc( MAX_BLOC_NUMBER, sizeof(Bloc) );
	scene->characters = (Character*) calloc( MAX_CHARACTER_NUMBER, sizeof(Character) );
	
	return scene;
}

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

GLuint makeTexture( const char * filename ) {
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
	
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0, format, GL_UNSIGNED_BYTE, image->pixels );
	glBindTexture( GL_TEXTURE_2D, 0 );
	SDL_FreeSurface( image );
	
	return textureId;
}

Image * makeImage( const char * filename, Bloc rect ) {
	Image * image = (Image *) calloc( 1, sizeof(Image) );
	
	image->texture = makeTexture( filename );
	image->rect = rect;
	
	return image;
}

Camera * makeCamera() {
	Camera * camera = (Camera *) calloc( 1, sizeof(Camera) );
	
	camera->pos = vector( 0.0, 0.0 );
	camera->posToReach = camera->pos;
	camera->zoom = 1.0;
	
	return camera;
}

Menu * makeMenu( const char * filename0, const char * filename1, const char * filename2 ) {
	Menu * menu = (Menu *) calloc( 1, sizeof(Menu) );
	
	menu->active = TRUE;
	menu->selectedLevel = 0;
	menu->images = (Image *) calloc( MAX_MENUTITLE_NUMBER, sizeof(Image) );
	
	Bloc rect = bloc( -40, -5, 40.0, 10.0 );
	
	menu->images[0] = * makeImage( filename0, rect );
	menu->images[1] = * makeImage( filename1, rect );
	menu->images[2] = * makeImage( filename2, rect );
	
	return menu;
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

void drawRectShape( Bloc bloc ) {
	glPushMatrix();
		glTranslatef( bloc.pos.x, bloc.pos.y, 0.0 );
		glBegin( GL_LINE_LOOP );
			glVertex2f( 0, 0 );
			glVertex2f( bloc.size.x, 0 );
			glVertex2f( bloc.size.x, bloc.size.y );
			glVertex2f( 0, bloc.size.y );
		glEnd();
	glPopMatrix();
}

void drawImage( Image * image ) {
	glBindTexture( GL_TEXTURE_2D, image->texture );
	glBegin( GL_QUADS );
		glColor3f( 1., 1., 1. );
		
		glTexCoord2f( 0., 1. );
		glVertex2f( image->rect.pos.x, image->rect.pos.y );
		
		glTexCoord2f( 1., 1. );
		glVertex2f( image->rect.pos.x + image->rect.size.x, image->rect.pos.y );
		
		glTexCoord2f( 1., 0. );
		glVertex2f( image->rect.pos.x + image->rect.size.x, image->rect.pos.y + image->rect.size.y );
		
		glTexCoord2f( 0., 0. );
		glVertex2f( image->rect.pos.x, image->rect.pos.y + image->rect.size.y );
	glEnd();
	glBindTexture( GL_TEXTURE_2D, 0 );
}

void drawImageParallax( Image * image, Camera * camera, float coef ) {
	image->rect.pos.x = -image->rect.size.x/2 - coef * camera->zoom * camera->pos.x;
	image->rect.pos.y = -image->rect.size.y/2 - coef * camera->zoom * camera->pos.y;
	drawImage( image );
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
			scene->blocs[currentIndex].parent = NULL;
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
				
				case 5:
				scene->characters[currentIndex].posToReach.x = atof(tmp);
				break;
				
				case 6:
				scene->characters[currentIndex].posToReach.y = atof(tmp);
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
	scene->maxDimension = vector( -100000, -100000 );
	scene->minDimension = vector( 100000, 100000 );
	
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

void handleCharactersControls( Scene * scene, Controler * controler ) {
	int i, j;
	Character * C = scene->characters;
	Bloc * B = scene->blocs;
	
	for ( i=0; i<scene->numberOfCharacters; i++ ) {
		
		if ( controler->tab == i )
			C[i].isSelected = TRUE;
		else
			C[i].isSelected = FALSE;
		
		if ( C[i].isSelected ) {
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
			
			if ( controler->up && C[i].isOnGround ) {
				C[i].isOnGround = FALSE;
				C[i].vel.y = C[i].jumpPower;
			}
		}
		
		C[i].acc.y = -GRAVITY;
		C[i].vel.x += C[i].acc.x;			C[i].vel.y += C[i].acc.y;
		C[i].bloc->pos.x += C[i].vel.x;		C[i].bloc->pos.y += C[i].vel.y;
		
		j = 0;
		C[i].isOnGround = FALSE;
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
				C[i].bloc->parent = &B[j];
			}
		
			j++;
		}
	
		if ( !C[i].isOnGround )
			C[i].bloc->parent = NULL;
	}
	
}

void handleCharacterParenting_Recursive( Bloc * blocRef, Scene * scene ) {
	Bloc * B = scene->blocs;
	
	if ( blocRef->parent!=NULL ) {
		handleCharacterParenting_Recursive( blocRef->parent, scene );
		
		blocRef->pos.x += ( blocRef->parent->pos.x - blocRef->parent->previousPos.x );
		blocRef->parent = NULL;
		
		int j = 0;
		int collided = FALSE;
		while ( !collided && j<scene->numberOfBlocs ) {
			if ( blocRef!=&B[j] && ( checkCollision( LEFT, *blocRef, B[j] ) || checkCollision( RIGHT, *blocRef, B[j] ) )) {
				collided = TRUE;
				blocRef->pos.x = blocRef->previousPos.x;
			}
			j++;
		}
		
	}
}

void handleCharactersParenting( Scene * scene ) {
	int i;
	for ( i=0; i<scene->numberOfCharacters; i++ )
		handleCharacterParenting_Recursive( scene->characters[i].bloc, scene );
}

void displayCharacters( Scene * scene ) {
	int i;
	Character * C = scene->characters;
	Bloc * B = scene->blocs;
	
	for ( i=0; i<scene->numberOfCharacters; i++ ) {
		glColor3ub( 255, 255, 255 );
		drawRectShape( bloc( C[i].posToReach.x, C[i].posToReach.y, C[i].bloc->size.x, C[i].bloc->size.y ) );
		if ( dist(scene->characters[i].bloc->pos, scene->characters[i].posToReach )>1 )
			glColor3ub( C[i].color.R, C[i].color.G, C[i].color.B );
		drawRect( *(C[i].bloc) );
	}
	
	for ( i=0; i<scene->numberOfBlocs; i++ )
		B[i].previousPos = B[i].pos;
	
}

void displayCharactersSelector( Scene * scene ) {
	int i;
	Character * C = scene->characters;
	
	for ( i=0; i<scene->numberOfCharacters; i++ ) {
		if ( C[i].isSelected ) {
			glColor3ub( 230, 30, 30 );
			Bloc selector = bloc( C[i].bloc->pos.x + C[i].bloc->size.x/2 - 0.5, C[i].bloc->pos.y + C[i].bloc->size.y + 1.0, 1.0, 1.0 );
			drawRect( selector );
			//printf( "pos du perso selectionne: %f, %f\n", C[i].bloc->pos.x, C[i].bloc->pos.y );
		}
	}
}

int checkGameEnding( Scene * scene ) {
	int i;
	for ( i=0; i<scene->numberOfCharacters; i++ ) {
		if ( dist(scene->characters[i].bloc->pos, scene->characters[i].posToReach )>1 )
			return FALSE;
	}
	return TRUE;
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

void updateCamera( Camera * camera, Scene * scene ) {
	int i;
	camera->zoom = 0.8;
	for ( i=0; i<scene->numberOfCharacters; i++ ) {
		if ( scene->characters[i].isSelected ) {
			camera->pos.x = mapValue( scene->characters[i].bloc->pos.x,
				 					  scene->minDimension.x, scene->maxDimension.x,
									  scene->minDimension.x + 70/camera->zoom, scene->maxDimension.x - 70/camera->zoom );
			camera->pos.y = mapValue( scene->characters[i].bloc->pos.y,
								 	  scene->minDimension.y, scene->maxDimension.y,
								 	  scene->minDimension.y + 40/camera->zoom, scene->maxDimension.y - 40/camera->zoom );
		}
	}
}

void handleMenuControls( Menu * menu, Controler * controler, Scene * scene ) {
	if ( controler->up && menu->selectedLevel>0 )
		menu->selectedLevel--;
	if ( controler->down && menu->selectedLevel<2 )
		menu->selectedLevel++;
	
	if ( controler->enter ) {
		switch( menu->selectedLevel ) {
			case 0:
			initBlocsFromFile( scene, "data/blocs0.txt" );
			initCharactersFromFile( scene, "data/characters0.txt" );
			break;
			
			case 1:
			initBlocsFromFile( scene, "data/blocs1.txt" );
			initCharactersFromFile( scene, "data/characters1.txt" );
			break;
			
			case 2:
			initBlocsFromFile( scene, "data/blocs2.txt" );
			initCharactersFromFile( scene, "data/characters2.txt" );
			break;
		}
		getSceneDimensions( scene );
		menu->active = FALSE;
	}
}

void displayMenu( Menu * menu, Controler * controler ) {
	int i;
	
	glColor4ub( 255, 255, 255, 250 );
	drawRect( bloc( -80, -50, 160, 100) );
	
	glColor3ub( 230, 30, 30 );
	for ( i=0; i<MAX_MENUTITLE_NUMBER; i++ )
		drawRect( bloc( 32, -1 - 10*i + menu->selectedLevel*10, 2, 2 ) );
	
	glColor4ub( 240, 240, 240, 200 );
	drawRect( bloc( -40, -5, 80, 10 ) );
	
	drawImage( &(menu->images[ menu->selectedLevel ]) );
	
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
	SDL_WM_SetCaption("Thomas Was Alone", NULL);
	
	//Variables Gameplay
	Uint8 * key = SDL_GetKeyState(NULL);
	Controler * controler = makeControler();
	Controler * previousControler = makeControler();
	Scene * scene = makeScene();
	Camera * camera = makeCamera();
	Menu * menu = makeMenu( "img/menu0.png", "img/menu1.png", "img/menu2.png" );
	
	//Initialisation des textures
	Image * background0 = makeImage( "img/back_mono0.png", bloc( -100, -100, 256, 160 ) );
	Image * background1 = makeImage( "img/back_mono1.png", bloc( -100, -100, 256, 160 ) );
	Image * background2 = makeImage( "img/back_mono2.png", bloc( -100, -100, 256, 160 ) );
	
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
			
			glRotatef( -1, 0.0, 0.0, 1.0 );
			glScalef( camera->zoom, camera->zoom, camera->zoom );
			glTranslatef( -camera->pos.x, -camera->pos.y, 0.0 );
			
			displayPlatforms( scene );
			
			if ( !menu->active ) {
				handleCharactersControls( scene, controler );
				handleCharactersParenting( scene );
			}

			displayCharacters( scene );
			displayCharactersSelector( scene );
		glPopMatrix();
		
		if ( checkGameEnding( scene ) )
			menu->active = TRUE;
		
		if ( menu->active ) {
			handleMenuControls( menu, controler, scene );
			displayMenu( menu, controler );
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
	
	//Fermeture des textures
	//glDeleteTextures( 1, background );
	
	//Fermeture des variables GamePlay
	free( scene->blocs );
	free( scene->characters );
	free( scene );
	free( camera );
	free( controler );
	free( previousControler );
	free( menu->images );
	free( menu );
	
	//Fermeture des variables Techniques
	SDL_Quit();
	return EXIT_SUCCESS;
}



