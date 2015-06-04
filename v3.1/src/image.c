#include "image.h"

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