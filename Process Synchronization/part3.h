#ifndef PART_3
#define PART_3
#include <semaphore.h>

enum DIRECTION {
    NORTH,
    SOUTH,
    EAST,
    WEST
} ;

enum LANE {
    LEFT,
    RIGHT
} ;

struct myargumentP3 // taken from main.h
{
    enum DIRECTION from ;
    enum DIRECTION to ;
    enum LANE lane ;
    int user_id ;
} ;


struct myVertex // car data
{
	struct myVertex *successor ;
	struct myargumentP3 data ;
} ;

struct myBucket // basically a bucket containing nodes similar to linked list to keep track of cars
{
	struct myVertex *starting ; // start of bucket
	struct myVertex *ending ; // end of bucket
	int capacity ; // basically the total number of cars
} ; 

typedef struct // lane consist of bucket and vertex defined above
{
	int rightLaneCounter ;
    int leftLaneCounter ;

    sem_t *SemaphoreLeftLane ;
    sem_t *SemaphoreRightLane ;

	struct myBucket *carsLeft;
	sem_t *SemaphoreCarsLeft;

    sem_t *SemaphoreRightLaneCounter ;
    sem_t *SemaphoreLeftLaneCounter ;

    struct myBucket *left ;
    struct myBucket *right ;

    sem_t *LeftListSemaphore ;
    sem_t *RightListSemaphore ;
}myLane ;

void initializeP3() ;

void allocateSemaphores() ;
void initializeSemaphores(int i) ;
void initializeLanes(int i) ;
void printer() ;
void reAssign() ;

/**
 * If there is a car going from SOUTH to NORTH, from lane LEFT,
 * print 
 * SOUTH NORTH LEFT
 * Also, if two cars can simulateneously travel in the two lanes,
 * first print all the cars in the LEFT lane, followed by all the
 * cars in the right lane
 */
void *goingFromToP3(void * argument) ;
void startP3() ;
#endif
