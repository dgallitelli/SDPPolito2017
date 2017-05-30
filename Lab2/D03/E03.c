#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define testPrintActivator 1    //Set to 1 if debug is wanted

typedef struct {
    int countA;
    int countB;
    pthread_mutex_t mutex;
} Counter;

void* TF1(void*);
void* TF2(void*);

sem_t *b1, *b2, *print;

Counter *c;

int main(int argc, char **argv) {
	if (argc != 2)
	{
		fprintf(stderr, "Wrong # of arguments");
		return -1;
	}
	
	b1 = (sem_t *) malloc(sizeof(sem_t));
	b2 = (sem_t *) malloc(sizeof(sem_t));
	print = (sem_t *)malloc(sizeof(sem_t));
	sem_init(b1, 0, 0);
	sem_init(b2, 0, 0);
	sem_init(print, 0, 1);
	
	c = (Counter *) malloc (sizeof(Counter));
	c->countA = 0;
	c->countB = 0;
	pthread_mutex_init(&c->mutex, NULL);
	
	int n = atoi(argv[1]);
    if(testPrintActivator > 0) printf ("n is %d\n", n);
	
	pthread_t th_1;
	pthread_t th_2;
	pthread_create(&th_1, NULL, TF1, (void*) &n);
	pthread_create(&th_2, NULL, TF2, (void*) &n);
	
	pthread_join(th_1, NULL); //waiting the termination
	pthread_join(th_2, NULL);
	
	return 0;
}

void* TF1(void* num)
{
	int *a;
	int n;
	a = (int*) num;
	n= *a;
	
	for(int i=0; i<n; i++)
	{
		sem_wait(print);
		printf("A1 ");
		sem_post(print);
		pthread_mutex_lock(&c->mutex);
		c->countA++;
		if(c->countA == 2) {
			sem_post(b1);
			sem_post(b1);
			c->countA = 0;
		}
		pthread_mutex_unlock(&c->mutex);
		sem_wait(b1);
		sem_wait(print);
		printf(" B1");
		sem_post(print);
		pthread_mutex_lock(&c->mutex);
		c->countB++;
		if(c->countB == 2) {
			sem_post(b2);
			sem_post(b2);
			c->countB = 0;
			sem_wait(print);
			printf("\n");
			sem_post(print);
		}
		pthread_mutex_unlock(&c->mutex);
		sem_wait(b2);
	}
	pthread_exit(NULL);
	return(NULL);
	
}

void* TF2(void* num)
{
	int *a;
	int n;
	a = (int*) num;
	n= *a;
	
	for(int i=0; i<n; i++)
	{
		sem_wait(print);
		printf("A2 ");
		sem_post(print);
		pthread_mutex_lock(&c->mutex);
		c->countA++;
		if(c->countA == 2)
		{
			sem_post(b1);
			sem_post(b1);
			c->countA = 0;
		}
		pthread_mutex_unlock(&c->mutex);
		sem_wait(b1);
		sem_wait(print);
		printf(" B2");
		sem_post(print);
		pthread_mutex_lock(&c->mutex);
		c->countB++;
		if(c->countB == 2)
		{
			sem_post(b2);
			sem_post(b2);
			c->countB = 0;
			sem_wait(print);
			printf("\n");
			sem_post(print);
		}
		pthread_mutex_unlock(&c->mutex);
		sem_wait(b2);
	}
	pthread_exit(NULL);
	return(NULL);
	
}