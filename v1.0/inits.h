void initLand( Bloc * myLand, size_t n ) {
	int i;
	//Remplissage du terrain
	i = 0;
	myLand[i].pos.x = -5.0;		myLand[i].pos.y = -3.0;
	myLand[i].size.x = 10.0;	myLand[i].size.y = 2.0;
	
	i = 1;
	myLand[i].pos.x = 2.0;		myLand[i].pos.y = -2.0;
	myLand[i].size.x = 2.0;		myLand[i].size.y = 4.0;
}

void initCrew( Character * myCrew, size_t n ) {
	int i;
	//Remplissage de l'équipage
	i = 0;
	myCrew[i].name = "Louise";
	myCrew[i].pos.x = 0.0;		myCrew[i].pos.y = 3.0;
	myCrew[i].vel.x = 0.0;		myCrew[i].vel.y = 0.0;
	myCrew[i].acc.x = 0.0;		myCrew[i].acc.y = 0.0;
	myCrew[i].size.x = 0.2;		myCrew[i].size.y = 1.0;
	myCrew[i].R = 200;			myCrew[i].G = 150;			myCrew[i].B = 150;
	myCrew[i].isSelected = 1;
	myCrew[i].jumpPower = 0.15;
	myCrew[i].isOnGround = 0;
	myCrew[i].onTop = NULL;
	myCrew[i].canJump = 1;
	
	i = 1;
	myCrew[i].name = "Nino";
	myCrew[i].pos.x = -3.0;		myCrew[i].pos.y = 3.0;
	myCrew[i].vel.x = 0.0;		myCrew[i].vel.y = 0.0;
	myCrew[i].acc.x = 0.0;		myCrew[i].acc.y = 0.0;
	myCrew[i].size.x = 0.6;		myCrew[i].size.y = 0.6;
	myCrew[i].R = 150;			myCrew[i].G = 110;			myCrew[i].B = 110;
	myCrew[i].isSelected = 0;
	myCrew[i].jumpPower = 0.07;
	myCrew[i].isOnGround = 0;
	myCrew[i].onTop = NULL;
	myCrew[i].canJump = 1;
	
	i = 2;
	myCrew[i].name = "Timour";
	myCrew[i].pos.x = 3.0;		myCrew[i].pos.y = 3.0;
	myCrew[i].vel.x = 0.0;		myCrew[i].vel.y = 0.0;
	myCrew[i].acc.x = 0.0;		myCrew[i].acc.y = 0.0;
	myCrew[i].size.x = 0.4;		myCrew[i].size.y = 0.8;
	myCrew[i].R = 220;			myCrew[i].G = 190;			myCrew[i].B = 190;
	myCrew[i].isSelected = 0;
	myCrew[i].jumpPower = 0.1;
	myCrew[i].isOnGround = 0;
	myCrew[i].onTop = NULL;
	myCrew[i].canJump = 1;
}