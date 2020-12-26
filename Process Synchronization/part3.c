#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include "part3.h"
#include "main.h"

/*
	- DO NOT USE SLEEP FUNCTION
	- Define every helper function in .h file
	- Use Semaphores for synchronization purposes
*/


/**
* Declare semaphores here so that they are available to all functions.
*/

sem_t synchronizingCars ;
// rest of the semaphores are part of struct defined in part3.h and will be available globally to all functions


myLane *myGlobalLanes ;
int trafficLights[4] ;
char *directions[] = {"NORTH", "SOUTH", "EAST", "WEST"} ;     
char *lanes[] = {"LEFT", "RIGHT"} ;
int turnToLeft ; // should car turn left now or not

void allocateSemaphores() {

	int counter = 0 ;
	while (counter < 4) {

		myGlobalLanes[counter].SemaphoreRightLane = (sem_t*)malloc(sizeof(sem_t)) ;
		myGlobalLanes[counter].SemaphoreLeftLane = (sem_t*)malloc(sizeof(sem_t)) ;
		myGlobalLanes[counter].SemaphoreCarsLeft = (sem_t*)malloc(sizeof(sem_t)) ;
		myGlobalLanes[counter].SemaphoreRightLaneCounter = (sem_t*)malloc(sizeof(sem_t)) ;
		myGlobalLanes[counter].SemaphoreLeftLaneCounter = (sem_t*)malloc(sizeof(sem_t)) ;
		myGlobalLanes[counter].LeftListSemaphore = (sem_t*)malloc(sizeof(sem_t)) ;
		myGlobalLanes[counter].RightListSemaphore = (sem_t*)malloc(sizeof(sem_t)) ; 
		counter += 1 ;
	}
}

void initializeSemaphores(int i) {
	sem_init(myGlobalLanes[i].SemaphoreCarsLeft, 0, 1);
	sem_init(myGlobalLanes[i].SemaphoreRightLane, 0, 0);
	sem_init(myGlobalLanes[i].SemaphoreLeftLane, 0, 0);
	sem_init(myGlobalLanes[i].SemaphoreRightLaneCounter, 0, 1);
	sem_init(myGlobalLanes[i].SemaphoreLeftLaneCounter, 0, 1);
	sem_init(myGlobalLanes[i].LeftListSemaphore, 0, 1);
	sem_init(myGlobalLanes[i].RightListSemaphore, 0, 1);
	sem_init(&synchronizingCars, 0, 1);
}

void reAssign() {
	int temp = 0 ;
	turnToLeft = temp ;
}

void initializeLanes(int i) {
	myGlobalLanes[i].carsLeft = (struct myBucket*)malloc(sizeof(struct myBucket)) ;
	myGlobalLanes[i].carsLeft -> starting = NULL ;
	myGlobalLanes[i].carsLeft -> capacity = 0 ;
	myGlobalLanes[i].left = (struct myBucket*)malloc(sizeof(struct myBucket)) ;
	myGlobalLanes[i].left -> starting = NULL ;
	myGlobalLanes[i].left -> capacity = 0 ;
	myGlobalLanes[i].right = (struct myBucket*)malloc(sizeof(struct myBucket)) ;
	myGlobalLanes[i].right -> starting = NULL ;
	myGlobalLanes[i].right -> capacity = 0 ;
}

void printer() {

	int i = 0 ;
	while (i < 4) {

		int flag = trafficLights[i] ;
		struct myVertex* itr = myGlobalLanes[flag].left -> starting ;
		
		while(itr)
		{
			printf("%s %s %s\n", directions[itr -> data.from], directions[itr -> data.to], lanes[itr -> data.lane]) ;
			itr = itr -> successor ;
		}

		itr = myGlobalLanes[flag].right -> starting ;

		while(itr)
		{
			printf("%s %s %s\n", directions[itr -> data.from], directions[itr -> data.to], lanes[itr -> data.lane]) ;
			itr = itr -> successor ;
		}

		i += 1 ;
	}
}

/**
 * Do any initial setup work in this function. You might want to 
 * initialize your semaphores here.
 */
void initializeP3() {

	// allocating space to lanes
	myGlobalLanes = (myLane*) malloc(sizeof(myLane) * 4) ;

	// initializing traffic signals
	for (int i = 0 ; i < 4 ; i++) {
		if (i == 0)
			trafficLights[i] = i ;
		else if (i == 1)
			trafficLights[i] = i + 1 ;
		else if (i == 2)
			trafficLights[i] = i - 1 ;
		else if (i == 3)
			trafficLights[i] = i ;
	}

	// allocating and initializing all four lanes
	int counter = 0 ;
	allocateSemaphores() ;
	while (counter < 4) {
		initializeSemaphores(counter) ;
		counter += 1 ;
	}

	// initializing other members
	counter = 0 ;
	while (counter < 4) {

		myGlobalLanes[counter].rightLaneCounter = 0;
		myGlobalLanes[counter].leftLaneCounter = 0;
		counter += 1 ;
	}

	// initializing vertex and bucket i.e. the lanes
	counter = 0 ;
	while (counter < 4) {

		initializeLanes(counter) ;
		counter += 1 ;
	}

	turnToLeft = 0 ;
}


/**
 * This is the function called for each car thread. You can check
 * how these functions are used by going over the test3 function
 * in main.c
 * If there is a car going from SOUTH to NORTH, from lane LEFT,
 * print 
 * SOUTH NORTH LEFT
 * Also, if two cars can simulateneously travel in the two lanes,
 * first print all the cars in the LEFT lane, followed by all the
 * cars in the right lane
 *
 * Input: *argu is of type struct argumentP3 defined in main.h
 */

void * goingFromToP3(void *argu){
	// Some code to help in understanding argu
    // struct argumentP3* car = (struct argumentP3*) argu;
    // enum DIRECTION from = car->from;
    // ...

	sem_wait(&synchronizingCars) ;
	struct myargumentP3* car = (struct myargumentP3*) argu ;

	reAssign() ;
	int to = car -> to ;

	if ((car -> from == 0 && to == 2) || (car -> from == 1 && to == 3) || (car -> from == 2 && to == 1) || (car -> from == 3 && to == 0)) {

		to = 0 ;
	}

	else
		to = 1 ;	

	// if not allowed to turn left
	if(car -> lane != turnToLeft) { 

		sem_wait(myGlobalLanes[car -> from].SemaphoreRightLaneCounter) ;
			myGlobalLanes[car -> from].rightLaneCounter++ ;
		sem_post(myGlobalLanes[car -> from].SemaphoreRightLaneCounter) ;

		sem_wait(myGlobalLanes[car -> from].RightListSemaphore) ;
			// insert car inside the bucket starts
			struct myVertex *to_add = (struct myVertex*)malloc(sizeof(struct myVertex)) ;
			to_add -> data = *car ;
			to_add -> successor = NULL ;
			if(myGlobalLanes[car -> from].right -> starting == NULL)
			{
				myGlobalLanes[car -> from].right -> starting = to_add ;
				myGlobalLanes[car -> from].right -> ending = to_add ;
				goto jump ;
			}
			myGlobalLanes[car -> from].right -> ending -> successor = to_add ;
			myGlobalLanes[car -> from].right -> ending = to_add ;
			myGlobalLanes[car -> from].right -> capacity++ ;
			// insert car inside the bucket ends
		jump: sem_post(myGlobalLanes[car -> from].RightListSemaphore) ;

		sem_post(&synchronizingCars) ;

		sem_wait(myGlobalLanes[car -> from].SemaphoreRightLane) ;

		sem_wait(myGlobalLanes[car -> from].SemaphoreRightLaneCounter) ;
			myGlobalLanes[car -> from].rightLaneCounter-- ;
		sem_post(myGlobalLanes[car -> from].SemaphoreRightLaneCounter) ;
	}

	// if allowed
	else {

		sem_wait(myGlobalLanes[car -> from].SemaphoreCarsLeft) ;
			// insert car inside the bucket starts
			struct myVertex *to_add = (struct myVertex*)malloc(sizeof(struct myVertex)) ;
			to_add -> data = *car ;
			to_add -> successor = NULL ;
			if(myGlobalLanes[car -> from].carsLeft -> starting == NULL)
			{
				myGlobalLanes[car -> from].carsLeft -> starting = to_add ;
				myGlobalLanes[car -> from].carsLeft -> ending = to_add ;
				goto jump2 ;
			}
			myGlobalLanes[car -> from].carsLeft -> ending -> successor = to_add ;
			myGlobalLanes[car -> from].carsLeft -> ending = to_add ;
			myGlobalLanes[car -> from].carsLeft -> capacity++ ;
			// insert car inside the bucket ends
		jump2:sem_post(myGlobalLanes[car -> from].SemaphoreCarsLeft) ;

		sem_wait(myGlobalLanes[car -> from].LeftListSemaphore) ;
			// insert car inside the bucket starts
			struct myVertex *to_add2 = (struct myVertex*)malloc(sizeof(struct myVertex)) ;
			to_add2 -> data = *car ;
			to_add2 -> successor = NULL ;
			if(myGlobalLanes[car -> from].left -> starting == NULL)
			{
				myGlobalLanes[car -> from].left -> starting = to_add2 ;
				myGlobalLanes[car -> from].left -> ending = to_add2 ;
				goto jump3 ;
			}
			myGlobalLanes[car -> from].left -> ending -> successor = to_add2 ;
			myGlobalLanes[car -> from].left -> ending = to_add2 ;
			myGlobalLanes[car -> from].left -> capacity++ ;
			// insert car inside the bucket ends
		jump3:sem_post(myGlobalLanes[car -> from].LeftListSemaphore) ;

		sem_wait(myGlobalLanes[car -> from].SemaphoreLeftLaneCounter) ;
			myGlobalLanes[car -> from].leftLaneCounter++ ;
		sem_post(myGlobalLanes[car -> from].SemaphoreLeftLaneCounter) ;

		sem_post(&synchronizingCars) ;

		if(to == turnToLeft)
			while(myGlobalLanes[car -> from].carsLeft -> starting -> data.user_id != car -> user_id) ;
		
		else
			sem_wait(myGlobalLanes[car -> from].SemaphoreLeftLane) ;

		sem_wait(myGlobalLanes[car -> from].SemaphoreCarsLeft) ;
			// delete car inside the bucket starts
			struct myVertex *prev, *itr ;
			itr = myGlobalLanes[car -> from].carsLeft -> starting ; 
			prev  = NULL ;
			while (itr) {

				if(itr -> data.user_id == car -> user_id) {

					if (prev == NULL) {
						myGlobalLanes[car -> from].carsLeft -> starting = itr -> successor ;
					}

					else {
						struct myVertex *node_next = itr -> successor ;
						prev -> successor = node_next ;
					}

					if((itr -> successor) == NULL) {
						myGlobalLanes[car -> from].carsLeft -> ending = prev ;
					}

					myGlobalLanes[car -> from].carsLeft -> capacity-- ;
					free(itr) ;
					break ;
				}

				prev = itr ;
				itr = itr -> successor ;
			}
			// delete car inside the bucket ends
		sem_post(myGlobalLanes[car -> from].SemaphoreCarsLeft) ;

		sem_wait(myGlobalLanes[car -> from].SemaphoreLeftLaneCounter) ;
			myGlobalLanes[car -> from].leftLaneCounter-- ; 
		sem_post(myGlobalLanes[car -> from].SemaphoreLeftLaneCounter) ;
	}
}


/**
 * startP3 is called once all cars have been initialized. The logic of the traffic signals
 * will go here
*/

void startP3(){

	sleep(1) ; // This sleep function only waits for all threads to arrive

	// Logic of traffic signal starts

	// ------------------------------------------------------------------------------

	int flag ;
	bool isFinished = true ;
	int myCounter = 0 ;
	int waitingCars = 5 ;

	for (; ;) {

		bool truth = myCounter > 3 ;
		if (truth) {
			if (isFinished) {
				break ;
			}

			myCounter = 0 ;
			isFinished = true ;
		}

		flag = trafficLights[myCounter] ;

		if((myGlobalLanes[flag].leftLaneCounter + myGlobalLanes[flag].rightLaneCounter) != 0)
			isFinished = false ;

		else
		{
			myCounter++ ;
			continue ;
		}

		sem_wait(myGlobalLanes[flag].SemaphoreCarsLeft) ;
		struct myVertex *itr = myGlobalLanes[flag].carsLeft -> starting ;

		// rule 6
		int j = 0 ;
		int ls = 0;
		int rs = 0;
		while (j < waitingCars) {
			if(itr == NULL)
				break ;

			else {

				int fromm = itr -> data.from ;
				int too = itr -> data.to ;

				if ((fromm == 0 && too == 2) || (fromm == 1 && too == 3) || (fromm == 2 && too == 1) || (fromm == 3 && too == 0)) {

					// do nothing
				}

				else
					ls += 1 ;	
			}

			itr = itr -> successor ;

			j += 1 ;
		}

		sem_post(myGlobalLanes[flag].SemaphoreCarsLeft) ;

		j = 0 ;
		while (j < ls) {

			sem_post(myGlobalLanes[flag].SemaphoreLeftLane) ;
			j += 1 ;
		}

		if (myGlobalLanes[flag].rightLaneCounter > 5)
			rs = 5 ;
		else
			rs = myGlobalLanes[flag].rightLaneCounter ;

		j = 0 ;
		while (j < rs) {

			sem_post(myGlobalLanes[flag].SemaphoreRightLane) ;
			j += 1 ;
		}

		myCounter += 1 ;
	}

	// ------------------------------------------------------------------------------

	// Logic of traffic signal ends

	sleep(1) ; // This sleep function only waits for all threads to exit

	printer() ;
}
