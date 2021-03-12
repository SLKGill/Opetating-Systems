#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h> /*POSIX*/
/*threads require less overhead than "forking" or spawning a new process because the system does not initialize a new system virtual memory space and environment for the process*/
#define n 4 /* number of rows and columns for each 2D array (matrix)*/

pthread_mutex_t mutex; /*pthread_mutex_t IS the type, mutex is a programming flag used to grab and release an object*/
pthread_cond_t cv; /*initializes the condition variable*/

int input[n][n]; /*array that needs to be sorted*/
int count = 0;


/*getting data to fill the array from the input.txt file*/
void scanMatrix(void){
    FILE * inputFile = fopen("input.txt", "r");

    for (int i=0; i<n; i++){
        for (int j=0; j<n; j++){
            fscanf(inputFile, "%d ", &input[i][j]);
        }
    }
    fclose(inputFile);
}

/*printing the matrix*/
void printMatrix(void){
    for (int i=0; i<n; i++){
        for (int j=0; j<n; j++){
            printf("%d ",input[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

/*following 3 functions are all for bubble sort implementation*/
/*reference to geeksforgeeks.org/bubble-sort/*/
void swap (int *x, int *y){ /*switching the values to be in order*/
    int temp = *x;
    *x = *y;
    *y = temp;
}

/*seperate bubble sort functions for row and column for means of shearsort which alternatively sorts rows and columns*/
void rowBubbleSort (int row){
    for (int i=0; i<n-1; i++){
        for (int j=0; j<n-i-1; j++){
            if (row%2==0){  /*for even row instances*/
                if (input[row][j]>input[row][j+1]){
                    swap(&input[row][j],&input[row][j+1]);
                }
            }
            else if (input[row][j] < input[row][j+1]){ /*for odd row instances*/
                swap(&input[row][j],&input[row][j+1]);
            }
        }
    }
}

void columnBubbleSort (int column){
    for (int j=0; j<n-1; j++){
        for (int i=0; i<n-j-1; i++){
            if (input[i][column]>input [i+1][column]){
                swap(&input[i][column],&input[i+1][column]);
            }
        }
    }
}

 void *shearSort(void *threadID){

     int id = (int) threadID;
     int maxPhase = (log(n*n)/log(2))+1; /*max phases for shearsort to complete in, converted to base 10 log*/
     int phase;
     for (phase=1; phase<=maxPhase; phase++){ /*alternating between sorting by row and column*/
         
         if (phase%2==0){/*in shearsort if even phase # column sort turn*/
             columnBubbleSort(id);
         }
         else if (phase%2!=0){ /*in shearsort if odd phase # row sort turn*/
             rowBubbleSort(id);
         }
         
         pthread_mutex_lock(&mutex); /*lock the mutix before leaving the critical section of code*/
         /* stuff that cannot be done in parallel belongs in the critical section */
         count++; /*global varibale, unreliable*/
         
         if(count!=n){
             pthread_cond_wait(&cv,&mutex); /*gives cancellation point*/
         }
         else{
             count=0;
             printf("Phase: %d\n", phase);
             printMatrix();
             pthread_cond_broadcast(&cv); /*wake up all threads blocked*/
         }
         pthread_mutex_unlock(&mutex);  /*with mutex the process that locks it=0 must unlock it=1!**/
             /*data unlocks when the data is no longer needed or routine is finished**/
     }
     pthread_exit(NULL);
 }

int main(void) {
    scanMatrix();
    printf("Input Matrix:\n");
    printMatrix();

    /*creating mutex and condition variable objects*/
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cv, NULL);
    
    int rc;
    pthread_t threadID[n]; /*threads ID*/
    for (int i=0; i<n; i++){
        rc = pthread_create(&threadID[i], NULL, shearSort, (void*)(size_t) i); /* create a new thread that will execute shearsort */
    }

    /*reference to villanova.edu/~mdamian/threads/posixthreads.html*/
    if (rc){  /*if thread could not be created*/
       printf("\n ERROR: return code from pthread_create() is %d\n", rc);
       exit(1);
    }
    
    pthread_exit(NULL);
    /*destroying*/
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cv);
}


/*
 Condition Variable Notes
 -mechanism allows threads to suspend execution and relinquish the processor until some condition is true
 -must always be associated with a mutex to avoid a race condition created by one thread preparing to wait and another thread which may signal the condition before the first thread actually waits on it resulting in a deadlock
 Mutexes
 -only allow one thread access to a section of memory at any one time
 */

