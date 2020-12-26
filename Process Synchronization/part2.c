#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include "part2.h"
#include "main.h"

/*
	- DO NOT USE SLEEP FUNCTION
	- Define every helper function in .h file
	- Use Semaphores for synchronization purposes
 */


/**
* Declare semaphores here so that they are available to all functions.
*/
// sem_t* example_semaphore;

const int INTER_ARRIVAL_TIME = 5;
const int NUM_TRAINS = 5;

struct Train* trains = NULL;
sem_t* mutex;


/**
 * Do any initial setup work in this function. You might want to 
 * initialize your semaphores here. Remember this is C and uses Malloc for memory allocation.
 *
 * numStations: Total number of stations. Will be >= 5. Assume that initially
 * the first train is at station 1, the second at 2 and so on.
 * maxNumPeople: The maximum number of people in a train
 */
void initializeP2(int numStations, int maxNumPeople) {
	// example_semaphore = (sem_t*) malloc(sizeof(sem_t)); 
	// Initialising the trains
	trains = (struct Train*) malloc(sizeof(struct Train) * NUM_TRAINS);
	mutex = malloc(sizeof(sem_t));
	sem_init(mutex,0,1);
	int i;
	for(i = 0; i < NUM_TRAINS; i++) {
		trains[i].ID = i; // train ID
		trains[i].numStations = numStations; // max number of stations
		trains[i].maxNumPeople = maxNumPeople; // max capacity
		trains[i].count = 0; // number of people on the train

		trains[i].countOff = malloc(sizeof(int) * numStations);
		trains[i].countOn = malloc(sizeof(int) * numStations);
		int j;
		for(j = 0; j < numStations; j++) {
			trains[i].countOff[j] = 0; // number of people who want to get off the train at a station
			trains[i].countOn[j] = 0; // number of people who want to get on the train at a station
		}

		trains[i].station = i; // current station

		trains[i].waitingOff = (sem_t**) malloc(sizeof(sem_t*) * numStations);
		trains[i].waitingOn = (sem_t**) malloc(sizeof(sem_t*) * numStations);
		int k;
		for(k = 0; k < numStations; k++) {
trains[i].waitingOff[k] = malloc(sizeof(sem_t));
trains[i].waitingOn[k] = malloc(sizeof(sem_t));
			sem_init(trains[i].waitingOff[k],0,0); // semaphore to signal people who want to get off the train at a station
			sem_init(trains[i].waitingOn[k],0,0); // semaphore to signal people who want to get on the train at a station
		}
	}
}

/**
	This function is called by each user.

 * Print in the following format:
 * If a user borads on train 0, from station 0 to station 1, and another boards
 * train 2 from station 2 to station 4, then the output will be
 * 0 0 1
 * 2 2 4
 */
void * goingFromToP2(void * user_data) {
	// Source and Destination of Passenger
	struct argument *passenger = (struct argument *) user_data;
	int src = passenger->from;
	int dest = passenger->to;
	// Deciding which train to board
	int* diffToStations = malloc(sizeof(int) * NUM_TRAINS);
	int chosen = -1;
	int i;
	for(i = 0; i < NUM_TRAINS; i++) {
		if(src == trains[i].station) {
			chosen = i;
			diffToStations[i] = -1;
		}
		diffToStations[i] = abs(trains[i].station - src);
	}
	if (chosen == -1) {
		int mini = diffToStations[0];
		chosen = 0;
		int j;
		for(j = 1; j < NUM_TRAINS; j++){
			if(mini > diffToStations[j])
			{
				mini = diffToStations[j];
				chosen = j;
			}
		}
	}
	sem_wait(mutex);
	trains[chosen].count = trains[chosen].count + 1;
	trains[chosen].countOff[dest] = trains[chosen].countOff[dest] + 1;
	trains[chosen].countOn[src] = trains[chosen].countOn[src] + 1;
	sem_post(mutex);
	sem_wait(trains[chosen].waitingOff[dest]);
	sem_wait(trains[chosen].waitingOn[src]);
	printf("%d %d %d\n", chosen, src, dest);
	return NULL;
}

void* TrainFunc(void* args)
{
	struct Train* Train = (struct Train*)args;
	int temp = Train -> station;
	int i;
	for(i = temp; i < Train -> numStations; i++) {
		// Update current station
		Train -> station = i;
		// Remove people who want to get off the train at a station
		while(Train -> countOff[i] > 0){
			sem_post(Train -> waitingOff[i]);
			sem_wait(mutex);
			Train -> countOff[i] = Train -> countOff[i] - 1;
			trains[i].count = trains[i].count - 1;
			sem_post(mutex);
		}
		// Add people who want to get on the train at a station
		while(Train -> count < Train -> maxNumPeople && Train -> countOn[i] > 0){
			sem_post(Train -> waitingOn[i]);
			sem_wait(mutex);
			Train -> countOn[i] = Train -> countOn[i] - 1;
			trains[i].count = trains[i].count - 1;
			sem_post(mutex);
		}
		if(i == Train -> numStations - 1) {
			i = 0;
		}
	}
} 

/* Use this function to start threads for your trains */
void * startP2(){
	sleep(1); // This is the only place where you are allowed to use sleep
	int i;
	for(i = 0; i < NUM_TRAINS; i++) {
		pthread_t train;
		pthread_create(&train, NULL, TrainFunc, (void *) &trains[i]);
	}
}
