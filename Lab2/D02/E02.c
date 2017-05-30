#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define testPrintActivator 0    //Set to 1 if debug is wanted

void *tFA (void *);
void *tFB (void *);

sem_t *printA, *printB, *print, *start;
int contA, contB;

int main(int argc, char* argv[]){
    
    if (argc != 2){
		fprintf(stderr, "Error: wrong # of arguments. Usage: %s n \n", argv[0]);
		return -1;
	}

    int rc, n;
    long int creat_number = 0;
    contA = 0;
    contB = 0;

    n = atoi(argv[1]);
    pthread_t *tidA = malloc(n*sizeof(pthread_t));
    pthread_t *tidB = malloc(2*n*sizeof(pthread_t));
    
    printA = (sem_t*) malloc(sizeof(sem_t));
    printB = (sem_t*) malloc(sizeof(sem_t));
    print = (sem_t*) malloc(sizeof(sem_t));
    start = (sem_t*) malloc(sizeof(sem_t));
    sem_init(start, 0, 0);
    sem_init(print, 0, 1);
    sem_init(printA, 0, 1);
    sem_init(printB, 0, 2);

    if(testPrintActivator > 0) printf ("DEBUG - TEST before pthread_create of thread A\n");
    for (int i=0; i<n; i++){
        if(testPrintActivator > 0) printf ("DEBUG - TEST during pthread_create of thread A\n");
        rc = pthread_create (&tidA[i], NULL, tFA, (void*) creat_number);
        if(rc) {
            if(testPrintActivator > 0) printf ("DEBUG - ERROR during thread A creation \n");
            fprintf(stderr,"ERROR: pthread_create() of thread %d with code %d\n", i, rc );
            exit(-1);
        }
        creat_number++;
    }
    creat_number = 0;
    if(testPrintActivator > 0) printf ("DEBUG - TEST before pthread_create of thread B\n");
    for (int i=0; i<2*n; i++){
        if(testPrintActivator > 0) printf ("DEBUG - TEST during pthread_create of thread B\n");
        rc = pthread_create (&tidB[i], NULL, tFB, (void*) creat_number);
        if(rc) {
            if(testPrintActivator > 0) printf ("DEBUG - ERROR during thread A creation \n");
            fprintf(stderr,"ERROR: pthread_create() of thread %d with code %d\n", i, rc );
            exit(-1);
        }
        creat_number++;
    }
    /* Signal all threads to run after all of them have been created */
    for (int i = 0; i < 3*n; i++)
        sem_post(start);
        
    pthread_exit(NULL);
    return 0;
}

void *tFA(void* a){
    sem_wait(start);
    if(testPrintActivator > 0) printf ("DEBUG - Loaded thread A \n");
    char myChar = 'A';
    long int myNum = (long int) a;

    sem_wait(printA);
    if(testPrintActivator > 0) printf ("DEBUG - Acquiring lock on printA (from tfA %ld) \n", myNum);

    sem_wait(print);
    if(testPrintActivator > 0) printf ("DEBUG - Acquiring lock on print (from tfA %ld) \n", myNum);
    printf ("%c%ld ", myChar, myNum);
    contA ++;
    if (contB == 2 && contA == 1){
        printf ("\n");
        contA = 0;
        contB = 0;
        sem_post (printB);
        sem_post (printA);
        sem_post (printB);
    }
    if(testPrintActivator > 0) printf ("DEBUG - Releasing lock on print (from tfA %ld) \n", myNum);
    sem_post (print);

    pthread_exit(NULL);
}

void *tFB(void* b){
    sem_wait(start);
    char myChar = 'B';
    long int myNum = (long int) b;
    if(testPrintActivator > 0) printf ("DEBUG - Loaded thread B%ld \n", myNum);

    sem_wait(printB);
    if(testPrintActivator > 0) printf ("DEBUG - Acquiring lock on printB (from tfB %ld) - contB = %d\n", myNum, contB);
        
    sem_wait(print);
    if(testPrintActivator > 0) printf ("DEBUG - HERE - contB count = %d for thread B%ld\n", contB, myNum);
    if(testPrintActivator > 0) printf ("DEBUG - Acquiring lock on print (from tfB %ld) \n", myNum);
    printf ("%c%ld ", myChar, myNum);
    contB ++;
    if (contA == 1 && contB == 2){
        printf ("\n");
        contA = 0;
        contB = 0;
        sem_post (printB);
        sem_post (printA);
        sem_post (printB);
    }
    if(testPrintActivator > 0) printf ("DEBUG - Releasing lock on print (from tfB %ld) \n", myNum);
    sem_post (print);

    pthread_exit(NULL);
}