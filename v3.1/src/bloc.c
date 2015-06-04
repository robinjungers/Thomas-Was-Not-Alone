#include "bloc.h"

Bloc bloc( float x, float y, float a, float b ) {
	Bloc bloc;
	bloc.pos = vector( x, y );
	bloc.size = vector( a, b );
	return bloc;
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