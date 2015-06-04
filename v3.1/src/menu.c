#include "menu.h"

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

void displayMenu( Menu * menu ) {
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

void handleMenuControls( Menu * menu, Controler * controler, Scene * scene, SoundFX * soundFX, Image * background0, Image * background1, Image * background2 ) {
	if ( controler->up && menu->selectedLevel>0 ) {
		Mix_PlayChannel( 1, soundFX->menuChange, 0 );
		menu->selectedLevel--;
	}
	if ( controler->down && menu->selectedLevel<2 ) {
		Mix_PlayChannel( 1, soundFX->menuChange, 0 );
		menu->selectedLevel++;
	}
	
	if ( controler->enter ) {
		Mix_PlayChannel( 1, soundFX->menuSelection, 0 );
		switch( menu->selectedLevel ) {
			case 0:
			initBlocsFromFile( scene, "data/blocs0.txt" );
			initCharactersFromFile( scene, "data/characters0.txt" );
			background0->texture = makeTexture( "img/backBrun0.png" );
			background1->texture = makeTexture( "img/backBrun1.png" );
			background2->texture = makeTexture( "img/backBrun2.png" );
			R = 0.98;	G = 0.9;	B = 0.9;
			break;
			
			case 1:
			initBlocsFromFile( scene, "data/blocs1.txt" );
			initCharactersFromFile( scene, "data/characters1.txt" );
			background0->texture = makeTexture( "img/backBleu0.png" );
			background1->texture = makeTexture( "img/backBleu1.png" );
			background2->texture = makeTexture( "img/backBleu2.png" );
			R = 0.9;	G = 0.93;	B = 0.98;
			break;
			
			case 2:
			initBlocsFromFile( scene, "data/blocs2.txt" );
			initCharactersFromFile( scene, "data/characters2.txt" );
			background0->texture = makeTexture( "img/backVert0.png" );
			background1->texture = makeTexture( "img/backVert1.png" );
			background2->texture = makeTexture( "img/backVert2.png" );
			R = 0.9;	G = 0.98;	B = 0.9;
			break;
		}
		getSceneDimensions( scene );
		menu->active = FALSE;
	}
}