#ifndef PART_2
#define PART_2

#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

/**
 * Do any initial setup work in this function.
 * numStations: Total number of stations. Will be >= 5
 * maxNumPeople: The maximum number of people in a train
 */
struct Train {
	int ID; // train ID
	int numStations; // max number of stations
	int maxNumPeople; // max capacity
	int count; // number of people on the train
	int* countOff; // number of people who want to get off the train at a station
	int* countOn; // number of people who want to get on the train at a station
	int station; // current station
	sem_t** waitingOff; // semaphore to signal people who want to get off the train at a station
	sem_t** waitingOn; // semaphore to signal people who want to get on the train at a station
};

void initializeP2(int numTrains, int numStations);
/**
 * Print data in the format described in part 5
 */
void * goingFromToP2(void * user_data);


void * startP2();
#endif