typedef struct Vector {
	float x;
	float y;
} Vector;

typedef struct Bloc {
	Vector pos;
	Vector size;
} Bloc;

typedef struct Character {
	char * name;
	Vector pos, vel, acc;
	Vector size;
	char R, G, B;
	float jumpPower;
	char isSelected;
	char isOnGround;
	struct Character * onTop;
	char canJump;
} Character;

typedef struct Controler {
	char right, left, up, down;
	char click;
	char tab;
} Controler;