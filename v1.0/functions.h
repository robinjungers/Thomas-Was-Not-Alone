void getControls( Controler * controler, Controler * controlerPast, Uint8 * key, size_t sizeCrew ) {
	controler->right = key[SDLK_RIGHT];
	controler->left = key[SDLK_LEFT];
	controler->up = key[SDLK_UP];
	if ( key[SDLK_TAB] &&  controler->tab==controlerPast->tab )
		controler->tab = (controler->tab + 1) % sizeCrew;
	
	if ( !key[SDLK_TAB] )
		*controlerPast = *controler;
}

void ease( float * base, float target, float step ) {
	if ( *base < target )
		*base += pow( *base - target, step );
	else if ( *base < target )
		*base -= pow( *base - target, step );
}

void drawRect( Vector pos, Vector size  ) {
	glPushMatrix();
		glTranslatef( pos.x, pos.y, 0.0 );
		glBegin( GL_QUADS );
			glVertex2f( 0, 0 );
			glVertex2f( size.x, 0 );
			glVertex2f( size.x, size.y );
			glVertex2f( 0, size.y );
		glEnd();
	glPopMatrix();
}

char checkCollision( char side, Vector posObj, Vector sizeObj, Vector posRef, Vector sizeRef ) {
	switch (side) {
		case 0: // Haut
		
		break;
		
		case 1: // Droite
		if ( ( posObj.y + sizeObj.y > posRef.y ) && ( posObj.y < posRef.y + sizeRef.y ) && ( posObj.x + sizeObj.x > posRef.x ) && ( posObj.x < posRef.x ) )
			return 1;
		break;
		
		case 2: // Bas
		if ( ( posObj.x + sizeObj.x > posRef.x ) && ( posObj.x < posRef.x + sizeRef.x ) && ( posObj.y < posRef.y + sizeRef.y ) && ( posObj.y > posRef.y ) )
			return 1;
		break;
		
		case 3: // Gauche
		if ( ( posObj.y + sizeObj.y > posRef.y ) && ( posObj.y < posRef.y + sizeRef.y ) && ( posObj.x < posRef.x + sizeRef.x ) && ( posObj.x > posRef.x ) )
			return 1;
		break;
	}
	
	return 0;
}



