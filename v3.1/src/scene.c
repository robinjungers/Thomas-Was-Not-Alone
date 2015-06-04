#include "scene.h"

Scene * makeScene() {
	Scene * scene = (Scene*) calloc( 1, sizeof(Scene));
	
	scene->blocs = (Bloc*) calloc( MAX_BLOC_NUMBER, sizeof(Bloc) );
	scene->characters = (Character*) calloc( MAX_CHARACTER_NUMBER, sizeof(Character) );
	
	return scene;
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
			printf( "New bloc: pos( %.1f, %.1f ), size( %.1f, %.1f )\n",
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
			scene->characters[currentIndex].vel = vector( 0.0, 0.0 );
			scene->characters[currentIndex].acc = vector( 0.0, 0.0 );
			scene->characters[currentIndex].isSelected = FALSE;
			printf( "New Character: name( %s ), color( %u, %u, %u ), jump( %.1f )\n",
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
	
	printf( "\nScene dimensions: %.2f, %.2f\n", scene->maxDimension.x - scene->minDimension.x, scene->maxDimension.y - scene->minDimension.y );
}

void displayPlatforms( Scene * scene ) {
	int i;
	//Dessin du tableau
	glColor3ub( 0, 0, 0 );
	for ( i=scene->numberOfCharacters; i<scene->numberOfBlocs; i++ )
		drawRect( scene->blocs[i] );
}

void handleCharactersControls( Scene * scene, Controler * controler, SoundFX * soundFX ) {
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
				Mix_PlayChannel( 2, soundFX->jump, 0 );
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

