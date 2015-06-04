#include "basics.h"

Vector vector( float x, float y ) {
	Vector v = { x, y };
	return v;
}

Color color( char R, char G, char B ) {
	Color color = { R, G, B };
	return color;
}

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

