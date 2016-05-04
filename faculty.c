//@author: Xingfan Xia
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <errno.h>

//Global counter
int numCS_lounge = 0; //number of CS prof in 
int numMath_lounge = 0; //number of Math prof
int numCS_wait = 0;
int numMath_wait= 0;
int sign = 0; //1 for Math, 2 for CS
int totalnumber;

pthread_mutex_t avail_lock;
pthread_mutex_t leave_lock;
pthread_cond_t CSwait;
pthread_cond_t Mathwait;
pthread_cond_t CSgo;
pthread_cond_t Mathgo;


sem_t* CS_Here;
sem_t* Math_Here;
sem_t* startThread_sem; //sem to start all the threads
sem_t* waitThread_sem; //sem to check if all thread are created

void* mathProfArrive(void*);
void* csProfArrive(void*);
void* csProfLeave(void*);
void* mathProfLeave(void*);
void* Math(void*);
void* CS(void*);
void initSync();
void closeSync();
void delay(int);
void shuffle(int*, int);


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
		// for (i=0; i<maths+css; i++) {
		// 	sem_wait(waitThread_sem);
		// }

		// for (i=0; i<maths+css; i++) {
		// 	sem_post(startThread_sem);
		// }

		printf("111111\n");
		// join all threads before letting main exit
		for (i=0; i<maths+css; i++) {
			pthread_join(profs[i], NULL);
			printf("how many phreads closed: %d\n", i);
		}
		printf("22222\n");
  //       pthread_t MathP[MathPthread];  //declare pthreads
  //      	pthread_t CSP[CSPthread];

  //       for( int a = 1; a <= MathPthread; a = a + 1 ){ //init Math phreads
	 //      pthread_t MathP[a];
	 //      pthread_create(&MathP[a], NULL, Math, NULL);
	 //      //pthread_join(MathP[a], NULL);
		// }

		// for( int c = 1; c <= CSPthread; c = c + 1 ){//init CS phreads
	 //      pthread_t CSP[c];
	 //      pthread_create(&CSP[c], NULL, CS, NULL);
	 //      //pthread_join(CSP[c], NULL);
		// }

		
		// for( int a = 1; a <= MathPthread; a = a + 1 ){ //wait for all Maths thread finished creating
	 //      sem_wait(waitThread_sem);
		// }

		// for( int c = 1; c <= CSPthread; c = c + 1 ){ //wait for all CS thread finished creating
	 //      sem_wait(waitThread_sem);
		// }

		
		// for( int a = 1; a <= MathPthread; a = a + 1 ){ //start all Math thread when all thread created
	 //      sem_post(startThread_sem);
		// }

		// for( int c = 1; c <= CSPthread; c = c + 1 ){ //start all CS thread when all thread created
	 //      sem_post(startThread_sem);
		// }


		// int totalnumber = numCS_wait + numMath_wait;
		// while (totalnumber == 0) {
		// 	for( int a = 1; a <= MathPthread; a = a + 1 ){ //Terminate Math threads when rowing finished
		      
		//       pthread_join(MathP[a], NULL);
		// 	}
	
		// 	for( int c = 1; c <= CSPthread; c = c + 1 ){//Terminate CS threads when rowing finished
		//       pthread_join(CSP[c], NULL);
		// 	}
		// }
	}
	closeSync();
	return 0;
}

void* Math(void* args) {
	// sem_post(waitThread_sem); //tell main it is created
	// sem_wait(startThread_sem); //wait for main to tell it to start
	mathProfArrive(args);
	mathProfLeave(args);
	return 0;
}

void* CS(void* args) {
	// sem_post(waitThread_sem); //tell main it is created
	// sem_wait(startThread_sem); //wait for main to tell it to start
	csProfArrive(args);
	csProfLeave(args);
	return 0;
}

void* mathProfArrive(void* args) {
	printf("Math Prof arrives\n"); 
	fflush(stdout);
	numMath_wait++;
	// sem_post(waitThread_sem); //tell main it is created
	// sem_wait(startThread_sem); //wait for main to tell it to start
	
	while (numCS_wait + numMath_wait > 1) {
		pthread_mutex_lock(&avail_lock);
		while (sign != 1) {
			pthread_cond_wait(&Mathwait, &avail_lock);
			printf("waiting \n");
		}
		if (numMath_lounge+numCS_lounge == 0) {
			pthread_cond_signal(&Mathwait);
		}
		printf("current sign is %d\n", sign);
		printf("Math Prof enters lounge\n");
		numMath_wait--;
		numMath_lounge++;
		sign == 1;
		pthread_cond_signal(&Mathgo);
		pthread_mutex_unlock(&avail_lock);
	}

	// pthread_mutex_lock(&avail_lock);
	// // while (sign == 2) {
	// // 	pthread_cond_wait(&Mathwait, &avail_lock);
	// // 	printf("%d\n", sign);
	// // 	printf("Math waiting\n");
	// // }

	// if (sign != 2) {//Math Prof got into the lounge here
	// 	printf("Math Prof enters Lounge\n");
	// 	sign = 1;
	// 	fflush(stdout);
	// 	//pthread_cond_signal(&Mathwait);
	// 	numMath_wait--;
	// 	numMath_lounge++;
		
	// 	pthread_mutex_unlock(&avail_lock);
	// }
}

void* csProfArrive(void* args) {
	printf("CS Prof arrives\n"); 
	fflush(stdout);
	numCS_wait++;

	while (numCS_wait + numMath_wait > 1) {
		pthread_mutex_lock(&avail_lock);
		while (sign != 2) {
			pthread_cond_wait(&CSwait, &avail_lock);
		}
		if (numMath_lounge+numCS_lounge == 0) {
			pthread_cond_signal(&CSwait);
		}
		printf("how the fk u get here\n");
		printf("CS Prof enters lounge\n");
		numCS_wait--;
		numCS_lounge++;
		sign = 2;
		pthread_cond_signal(&CSgo);
		pthread_mutex_unlock(&avail_lock);
	}

	// sem_post(waitThread_sem); //tell main it is created
	// sem_wait(startThread_sem); //wait for main to tell it to start
	// pthread_mutex_lock(&avail_lock);
	// // while (sign == 1) {
	// // 	pthread_cond_wait(&CSwait, &avail_lock);
	// // 	printf("%d\n", sign);
	// // 	printf("CS waiting\n");
	// // }

	// if (sign != 1) {//CS Prof got into the lounge here
	// 	printf("CS Prof enters Lounge\n");
	// 	sign = 2;
	// 	fflush(stdout);
	// 	//pthread_cond_signal(&CSwait);
	// 	numCS_wait--;
	// 	numCS_lounge++;
	// 	// sem_post(&CSHere)
	// 	pthread_mutex_unlock(&avail_lock);

	// }
}

void* mathProfLeave(void* args) {
	pthread_mutex_lock(&leave_lock);
	while (numMath_lounge == 0) {
		pthread_cond_wait(&Mathgo,&leave_lock);
	}
	printf("num of maths prof: %d\n",numMath_lounge);
	printf("Math Prof leaves Lounge\n");
	fflush(stdout);
	numMath_lounge--;
	if (numMath_lounge == 0) {
		sign = 0;
	}
	pthread_mutex_unlock(&leave_lock);
	// totalnumberleaving++;
	// printf("#ofleft: %d\n", totalnumberleaving);
	// int numtotal = numMath_wait + numCS_wait;
	// printf("number total waiting is%d\n", numtotal);
}

void* csProfLeave(void* args) {
	pthread_mutex_lock(&leave_lock);
	while (numCS_lounge == 0) {
		pthread_cond_wait(&CSgo,&leave_lock);
	}
	printf("CS Prof leaves Lounge\n");
	fflush(stdout);
	numCS_lounge--;
	if (numCS_lounge == 0) {
		sign = 0;
	}
	pthread_mutex_unlock(&leave_lock);
	// totalnumberleaving++;
	// printf("#ofleft: %d\n", totalnumberleaving);
	// int numtotal = numMath_wait + numCS_wait;
	// printf("number total waiting is%d\n", numtotal);
}

void initSync() { //init all sync stuff
	pthread_mutex_init(&avail_lock, NULL);
	pthread_cond_init(&CSwait, NULL);
	pthread_cond_init(&Mathwait, NULL);
	pthread_cond_init(&CSgo, NULL);
	pthread_cond_init(&Mathgo, NULL);
	
	// pthread_cond_init(&cWaitAt1, NULL);
	// pthread_cond_init(&cWaitToBow0, NULL);
	// pthread_cond_init(&stopRowingback, NULL);


	// CS_Here = sem_open("CSHere", O_CREAT|O_EXCL, 0466, 0);

	// Math_Here = sem_open("MathHere", O_CREAT|O_EXCL, 0466, 0);

	startThread_sem = sem_open("startSem", O_CREAT|O_EXCL, 0466, 0);

	waitThread_sem = sem_open("waitSem", O_CREAT|O_EXCL, 0466, 0);

	// while (CS_Here==SEM_FAILED) {
	//     if (errno == EEXIST) {
	//       printf("semaphore CSHere already exists, unlinking and reopening\n");
	//       fflush(stdout);
	//       sem_unlink("CSHere");
	//       CS_Here = sem_open("CSHere", O_CREAT|O_EXCL, 0466, 0);
	//     }
	//     else {
	//       printf("semaphore could not be opened, error # %d\n", errno);
	//       fflush(stdout);
	//       exit(1);
	//     }
	// }

	// while (Math_Here==SEM_FAILED) {
	//     if (errno == EEXIST) {
	//       printf("semaphore MathHere already exists, unlinking and reopening\n");
	//       fflush(stdout);
	//       sem_unlink("MathHere");
	//       Math_Here = sem_open("MathHere", O_CREAT|O_EXCL, 0466, 0);
	//     }
	//     else {
	//       printf("semaphore could not be opened, error # %d\n", errno);
	//       fflush(stdout);
	//       exit(1);
	//     }
	// }

    while (startThread_sem==SEM_FAILED) {
	    if (errno == EEXIST) {
	      printf("semaphore startSem already exists, unlinking and reopening\n");
	      fflush(stdout);
	      sem_unlink("startSem");
	      startThread_sem = sem_open("startSem", O_CREAT|O_EXCL, 0466, 0);
	    }
	    else {
	      printf("semaphore could not be opened, error # %d\n", errno);
	      fflush(stdout);
	      exit(1);
	    }
	}

    while (waitThread_sem==SEM_FAILED) {
	    if (errno == EEXIST) {
	      printf("semaphore waitSem already exists, unlinking and reopening\n");
	      fflush(stdout);
	      sem_unlink("waitSem");
	      waitThread_sem = sem_open("waitSem", O_CREAT|O_EXCL, 0466, 0);
	    }
	    else {
	      printf("semaphore could not be opened, error # %d\n", errno);
	      fflush(stdout);
	      exit(1);
	    }
	}
}

void closeSync() { //destroy all syncs
	pthread_mutex_destroy(&avail_lock);
	pthread_cond_destroy(&CSwait);
	pthread_cond_destroy(&Mathwait);
	pthread_cond_destroy(&CSgo);
	pthread_cond_destroy(&Mathgo);	
	// pthread_cond_Destroy(&cWaitAt1, NULL);
	// pthread_cond_Destroy(&cWaitToBow0, NULL);
	// pthread_cond_Destroy(&stopRowingback, NULL);

  // 	sem_close(CS_Here);
 	// sem_unlink("CSHere");

 	// sem_close(Math_Here);
 	// sem_unlink("MathHere");

 	sem_close(waitThread_sem);
 	sem_unlink("waitSem");

 	sem_close(startThread_sem);
 	sem_unlink("startSem");
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
