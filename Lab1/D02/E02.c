#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/syscall.h>		//For syscall()
#include <string.h>				//For STRCAT

#define testPrintActivator 0    //Set to 1 if debug is wanted
#define CHILDREN_NO 2

void *tF (void *myFDptr);

/* Section for the global variables*/
int g;
sem_t *s1, *s2, *s3;

int main(int argc, char* argv[]){
	int fd[CHILDREN_NO], requests = 0;
	// To check treads creation status
	long int rc;
	
	// allocating semaphores
	s1 = (sem_t *) malloc(sizeof(sem_t));
	s2 = (sem_t *) malloc(sizeof(sem_t));
	s3 = (sem_t *) malloc(sizeof(sem_t));

	// initializing semaphores
	if(sem_init(s1, 0, 1) == -1){
		fprintf(stderr, "ERROR init semaphore s1");
	}
	if(testPrintActivator > 0)printf("semaphore s1 initialized\n");
	if(sem_init(s2, 0, 0) == -1){
		fprintf(stderr, "ERROR init semaphore s2");
	}
	if(testPrintActivator > 0)printf("semaphore s2 initialized\n");
	if(sem_init(s3, 0, 0) == -1){
		fprintf(stderr, "ERROR init semaphore s3");
	}
	if(testPrintActivator > 0)printf("semaphore s3 initialized\n");

	pthread_t tid[CHILDREN_NO];
	//test print
	if(testPrintActivator > 0)printf ("TEST before open files\n");

	for (int i=0; i<CHILDREN_NO; i++){	
		char path[] = "fv", path2[10];
		sprintf(path2,"%d", i+1);
		strcat(path2, ".b");
		strcat(path, path2);	//Code for generating path
		if ((fd[i] = open(path, O_RDONLY)) <0) {
			fprintf(stderr, "error open %s\n", path);
			return -1;
		}
	}

	//test print
	if(testPrintActivator > 0)	printf ("TEST before pthread_create\n");
	// creating threads
	for (int i=0; i<CHILDREN_NO; i++){
		rc = pthread_create (&tid[i], NULL, tF, (void*) &fd[i]);
		if(rc) {
			fprintf(stderr,"ERROR: pthread_create() of thread %d with code %ld\n", i, rc );
			exit(-1);
		}
	}
    
    int myChildCount = 0;
	do {
		sem_wait(s2);
        if (g==-1) myChildCount++; 
        else {
            g *= 3;
            requests++; 
        }
        sem_post(s3);   
	} while (myChildCount<CHILDREN_NO);
	
	printf("The number of served request is: %d\n", requests);
	/* Exit with no errors */
	return 0;
}

void *tF (void* myFDptr){
	//test print
	if(testPrintActivator > 0)	printf ("Loaded thread %d .\n", syscall(SYS_gettid));
	int number;
	int *myFDptr2 = (int*) myFDptr;
	int myFD = *myFDptr2;
	while (read(myFD, &number, sizeof(int))){
		sem_wait(s1);
        if(testPrintActivator > 0) printf("Thread %d has read %d\n", syscall(SYS_gettid), number);
		g = number;
		sem_post(s2);

		sem_wait(s3);
		printf("Value %d from thread %d \n", g, syscall(SYS_gettid));
		sem_post(s1);
	}

	sem_wait(s1);
	g = -1;
	sem_post(s2);
    sem_wait(s3);
    sem_post(s1);
	
	//pthread_exit(NULL);
}
