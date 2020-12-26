#ifndef PART_1
#define PART_1

struct Elevator {
	int ID; // will be 1 since we have 1 functioning elevator only
	int numFLoors; // total number of floors the elevator can go to
	int maxNumPeople; // maximum capacity of elevator i.e. how many people can it hold
};

void initializeP1(int numFloors, int maxNumPeople) ;
void *goingFromToP1(void *) ;
void startP1() ;
void *Elevatorfn(void *args) ; // elevator helper function
void goingUp(int numFloors, int maxNumPeople) ; // elevator going up
void goingDown(int numFloors, int maxNumPeople) ; // elevator coming down

#endif
