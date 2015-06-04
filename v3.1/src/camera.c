#include "camera.h"

Camera * makeCamera() {
	Camera * camera = (Camera *) calloc( 1, sizeof(Camera) );
	
	camera->pos = vector( 0.0, 0.0 );
	camera->posToReach = camera->pos;
	camera->zoom = 1.0;
	
	return camera;
}

void drawImageParallax( Image * image, Camera * camera, float coef ) {
	image->rect.pos.x = -image->rect.size.x/2 - coef * camera->zoom * camera->pos.x;
	image->rect.pos.y = -image->rect.size.y/2 - coef * camera->zoom * camera->pos.y;
	drawImage( image );
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