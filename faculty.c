//@author: Xingfan Xia
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <errno.h>

//Global counter
int numCS_lounge = 0; //number of CS prof in lounge
int numMath_lounge = 0; //number of Math prof in lounge

//debug counter
int left = 0;
int enters = 0;
int left1 = 0;
int enters1 = 0; 

int sign = 0; //1 for Math, 2 for CS, 0 for empty

//Sync stuffs
pthread_mutex_t avail_lock;
pthread_cond_t CSwait; 
pthread_cond_t Mathwait;

//main solution function
void* mathProfArrive(void*);
void* csProfArrive(void*);
void* csProfLeave(void*);
void* mathProfLeave(void*);

//test helper
void* Math(void*);
void* CS(void*);

//sync init/termination
void initSync();
void closeSync();

//helper function for testing
void shuffle(int*, int);
void delay(int);

//test main
int main(int argc, char *argv[]) { 
	int maths; //var to hold commandline inputs
	int css;
	initSync(); //init Syncs

	
	if ( argc != 3 ) /* argc should be 2 for correct execution */
    {
        printf( "2 arguments are needed as input for the number of MathProf and CSProf \n" );
    	fflush(stdout);
    } else {
        maths = atoi(argv[1]); //convert input to int
        css = atoi(argv[2]); //convert input to int

        // create shuffled list of 1's and 2's
		int order[maths + css];
		int i;
		for (i=0; i<maths; i++) {
			order[i] = 1;
		}
		for (; i<maths+css; i++) {
			order[i] = 2;
		}
		// order now has as many 1's as PyDE students and as many 2's as Phylo 
		// students, so just need to shuffle  
		shuffle(order, maths+css);

		// now create threads in order indicated by shuffled order array
		pthread_t profs[maths+css];
		for (i=0; i<maths+css; i++) {
			if (order[i]==1) pthread_create(&profs[i], NULL, Math, NULL);
			else if (order[i]==2) pthread_create(&profs[i], NULL, CS, NULL);
			else printf("something went horribly wrong!!!\n");
	        // create threads in batches, so that test case where all current
	        // math & cs profs enter and exit lounge, then some more show up
			// if (i%5==4) sleep(2);
		}
		// join all threads before letting main exit
		for (i=0; i<maths+css; i++) {
			pthread_join(profs[i], NULL);
		}
	}
	closeSync();
	return 0;
}

void* Math(void* args) {
	mathProfArrive(args);
	delay(rand()%5+4);
	mathProfLeave(args);
	return 0;
}

void* CS(void* args) {
	csProfArrive(args);
	delay(rand()%5+4);
	csProfLeave(args);
	return 0;
}

void* mathProfArrive(void* args) {
	printf("Math Prof arrives\n"); 
	fflush(stdout);
	pthread_mutex_lock(&avail_lock); //lock Critical section
	while (sign == 2) {//wait only if the sign is 2(CS)
		pthread_cond_wait(&Mathwait, &avail_lock);
	}

	//otherwise go in!
	printf("Math Prof enters lounge\n"); 
	fflush(stdout);
	enters++;//debug counter
	numMath_lounge++;//to track last men in lounge
	sign = 1;//This is a Math Lounge now

	pthread_mutex_unlock(&avail_lock);//unlock Critical section
}


void* csProfArrive(void* args) {
	printf("CS Prof arrives\n"); 
	fflush(stdout);
	pthread_mutex_lock(&avail_lock);//lock Critical section
	while (sign == 1) { //wait only if the sign is 1(Math)
		pthread_cond_wait(&CSwait, &avail_lock);
	}
	printf("CS Prof enters lounge\n");
	fflush(stdout);
	enters1++;//debug counter
	numCS_lounge++;//to track last men in lounge
	sign = 2;//This is a CS Lounge now

	pthread_mutex_unlock(&avail_lock);//unlock Critical section
}

void* mathProfLeave(void* args) {
	pthread_mutex_lock(&avail_lock); //lock Critical section
	printf("Math Prof leaves Lounge\n");
	fflush(stdout);
	numMath_lounge--;
	left++;//debug tracker
	if (numMath_lounge == 0) { //if last men leaving
		sign = 0; //change the sign to empty
		pthread_cond_broadcast(&CSwait); //wake up all the CS prof waiting!
	}
	pthread_mutex_unlock(&avail_lock);//unlock Critical section

	//debug information
	printf("Math prof enter/left: %d / %d\n", enters, left);
	fflush(stdout);
}

void* csProfLeave(void* args) {
	pthread_mutex_lock(&avail_lock);//lock Critical section
	printf("CS Prof leaves Lounge\n");
	fflush(stdout);
	numCS_lounge--;
	left1++;//debug tracker
	if (numCS_lounge == 0) {//if last men leaving
		sign = 0; //change the sign to empty
		pthread_cond_broadcast(&Mathwait);//wake up all the Math prof waiting!
	}
	pthread_mutex_unlock(&avail_lock);//unlock Critical section

	//debug information
	printf("CS prof enter/left: %d / %d\n", enters1, left1);
	fflush(stdout);
		
}

void initSync() { //init all sync stuff
	pthread_mutex_init(&avail_lock, NULL);
	pthread_cond_init(&CSwait, NULL);
	pthread_cond_init(&Mathwait, NULL);
	
}

void closeSync() { //destroy all syncs
	pthread_mutex_destroy(&avail_lock);
	pthread_cond_destroy(&CSwait);
	pthread_cond_destroy(&Mathwait);

}

// function to randomize list of integers
void shuffle(int* intArray, int arrayLen) {
  int i=0;
  for (i=0; i<arrayLen; i++) {
    int r = rand()%arrayLen;
    int temp = intArray[i];
    intArray[i] = intArray[r];
    intArray[r] = temp;
  }
}

/*
 * NOP function to simply use up CPU time
 * arg limit is number of times to run each loop, so runs limit^2 total loops
 */
void delay( int limit )
{
  int j, k;

  for( j=1; j < limit*1000; j++ )
    {
      for( k=1; k < limit*1000; k++ )
        {
            int x = j*k/(k+j);
            int y = x/j + k*x - (j+5)/k + (x*j*k);
        }
    }
}
