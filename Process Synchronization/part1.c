#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "part1.h"
#include "main.h"

/*
	- DO NOT USE SLEEP FUNCTION
	- Define every helper function in .h file
	- Use Semaphores for synchronization purposes
*/


/**
* Declare semaphores here so that they are available to all functions.
*/

const int MAX_NUM_FLOORS = 20 ;

sem_t* Outelevator ; // semaphore for users waiting outside elevator
sem_t* Inelevator ; // semaphore for users waiting inside elevator
sem_t* Updater ; // semaphore to ensure mutual exclusion when modifying the elevator
sem_t* Printer ; // semaphore to synchronize printing between different threads
sem_t* Mutex ; // semaphore for signalling and waiting the elevator after a user has went out of the elevator

struct Elevator* globalElevator ; // struct defined in part1.h

// some other gloabl variables to keep track of people inside and outside and people on each floor
int peopleInside ; // number of people inside the elevator
int peopleOutside ; // number of people outside the elevator
int *sourceArray ; // array to keep track of number of people at particular source floor
int *destinationArray ; // array to keep track of number of people at particular destination floor


/**
 * Do any initial setup work in this function. You might want to 
 * initialize your semaphores here. Remember this is C and uses Malloc for memory allocation.
 *
 * numFloors: Total number of floors elevator can go to. numFloors will be smaller or equal to MAX_NUM_FLOORS
 * maxNumPeople: The maximum capacity of the elevator
 *
 */

void initializeP1(int numFloors, int maxNumPeople) { 

	// Memory allocation
	// total floors == numFloors and there would be 2 semaphores per floor
	Outelevator = (sem_t*) malloc(sizeof(sem_t)* numFloors) ;
	Inelevator = (sem_t*) malloc(sizeof(sem_t)* numFloors) ;
	Updater = malloc(sizeof(sem_t)) ;
	Printer = malloc(sizeof(sem_t)) ;
	Mutex = malloc(sizeof(sem_t)) ;

	// Each floor has 2 semaphores and there are "numFloors" such floors elevator can go to so initializing all of them
	for (int i = 0 ; i < numFloors ; i++) {
		sem_init(&Outelevator[i], 0, 0) ;
		sem_init(&Inelevator[i], 0, 0) ;
	}

	// Initialization of semaphores	(name of semaphore, threads, initial Value)
	sem_init(Updater, 0, 1) ;
	sem_init(Printer, 0, 1) ;
	sem_init(Mutex, 0, 0) ;

	// Initializing Elevator and other variables
	globalElevator = malloc(sizeof(struct Elevator)) ;
	sourceArray = calloc(numFloors, sizeof(int)) ;
	destinationArray = calloc(numFloors, sizeof(int)) ;
	globalElevator -> ID = 1;
	globalElevator -> numFLoors = numFloors ;
	globalElevator -> maxNumPeople = maxNumPeople ;

	// initially elevator is empty and no one is waiting for elevator
	peopleInside = 0 ;
	peopleOutside = 0 ;

	return ;
}


/**
 * Every passenger will call this function when 
 * he/she wants to take the elevator. (Already
 * called in main.c)
 * 
 * This function should print info "id from to" without quotes,
 * where:
 * 	id = id of the user (would be 0 for the first user)
 * 	from = source floor (from where the passenger is taking the elevator)
 * 	to = destination floor (floor where the passenger is going)
 * 
 * info of a user x_1 getting off the elevator before a user x_2
 * should be printed before.
 * 
 * Suppose a user 1 from floor 1 wants to go to floor 4 and
 * a user 2 from floor 2 wants to go to floor 3 then the final print statements
 * will be 
 * 2 2 3
 * 1 1 4
 *
 */

void* goingFromToP1(void *arg) {
	
	struct argument *user = (struct argument *) arg ;
	int id = user -> id ;
	int from = user -> from ;
	int to = user -> to ;

	// user comes to elevator
	sem_wait(Updater) ;
		peopleOutside += 1 ; // people waiting outside the elevator increases
		sourceArray[from]++ ; // increment people on source floor i.e. if people on floor 2 were initially 3 they will now be 4
	sem_post(Updater) ;

	sem_wait(&Outelevator[from]) ; // wait until signal to go in elevator

	// Now user has entered inside the elevator

	sem_wait(Updater) ;
		peopleOutside-- ; // decrement people waiting outside the elevator
		peopleInside++ ; // increment people inside the elevator
		sourceArray[from]-- ; // decrement people waiting on that particular floor
		destinationArray[to]++ ; // increment people on destination floor i.e. if people on floor 2 were initially 3 they will now be 4
	sem_post(Updater) ;

	sem_wait(&Inelevator[to]) ; // wait until signal to go out of the elevator

	sem_wait(Updater) ;
		destinationArray[to]-- ; // decrement destination array for that particular floor
		peopleInside-- ; // decrement people inside the elevator
	sem_post(Updater) ;

	// now printing info "id from to"
	sem_wait(Printer) ;
		printf("%d %d %d \n", id, from, to) ;
	sem_post(Printer) ;

	sem_post(Mutex) ; // signal elevator that user has successfully got out of the elevator

	return NULL ;
}

void goingUp(int numFloors, int maxNumPeople) {

	for(int i = 0 ; i < numFloors ; i++)
	{
		// signal all users in elevator to get out if their destination floor is this current floor i.e. ith index
		while(destinationArray[i] != 0)
		{
			sem_post(&Inelevator[i]) ; // signal to get out
			sem_wait(Mutex) ; // wait for user to get out
		}

		// signal all users outside elevator to get in until maxCapacity of elevator is reached (and if there is any user)
		// waiting on that particular floor i.e. ith index
		while((peopleInside != maxNumPeople) && (sourceArray[i] > 0))
		{
			sem_post(&Outelevator[i]) ; // signal to get in
		}
	}
}

void goingDown(int numFloors, int maxNumPeople) {

	for(int i = (numFloors - 1) ; i >= 0 ; i--)
	{
		// signal all users in elevator to get out if their destination floor is this current floor i.e. ith index
		while(destinationArray[i] != 0)
		{
			sem_post(&Inelevator[i]) ; // signal to get out
			sem_wait(Mutex) ; // wait for user to get out
		}

		// signal all users outside elevator to get in until maxCapacity of elevator is reached (and if there is any user)
		// waiting on that particular floor i.e. ith index
		while((peopleInside != maxNumPeople) && (sourceArray[i] > 0))
		{
			sem_post(&Outelevator[i]) ; // signal to get in
		}
	}
}

void* Elevatorfn(void* args) {

	// args contain the global Elevator struct
	struct Elevator* elevatorInsideFn = (struct Elevator*)args ;
	int numFloors = elevatorInsideFn -> numFLoors ;
	int maxNumPeople = elevatorInsideFn -> maxNumPeople ;

	while(true) {

		// going from ground floor to "numfloors"
		goingUp(numFloors, maxNumPeople) ;

		// if there is no one waiting outside and inside the elevator break the loop and return
		if( (peopleOutside == 0) && (peopleInside == 0) ) 
		{ 
			break ;
		}

		// going back from "numfloors" to ground floor
		goingDown(numFloors, maxNumPeople) ;

		// if there is no one waiting outside and inside the elevator break the loop and return
		if( (peopleOutside == 0) && (peopleInside == 0) ) 
		{ 
			break ;
		}
	}

	return NULL ;
}


/*If you see the main file, you will get to 
know that this function is called after setting every
passenger.

So use this function for starting your elevator. In 
this way, you will be sure that all passengers are already
waiting for the elevator.
*/

void startP1(){
	sleep(1) ;
	pthread_t elevatorThread ;
	pthread_create(&elevatorThread, NULL, Elevatorfn, (void *) globalElevator) ;
	return ;
}