#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

int wait_with_timeout(sem_t *S, int tmax);
void* TF1 (void*);
void* TF2 ();
void nano_sleep(long  ns);
void ALRM_handler();
sem_t *S;
int flag;

int main(int argc, char **argv) {
	if (argc != 2){
		fprintf(stderr, "Error: wrong # of arguments. Usage: %s <tmax> \n", argv[0]);
		return -1;
	}
	int tmax = atoi(argv[1]);
	signal(SIGALRM, ALRM_handler);
	S = (sem_t *) malloc(sizeof(sem_t)); 
	sem_init(S,0,0);
	pthread_t th1, th2;
	pthread_create (&th1, NULL, TF1, (void*) &tmax);
	pthread_create (&th2, NULL, TF2, NULL);
	
	pthread_join(th1, NULL);
	pthread_join(th2, NULL);
	
	return 0;
}

int wait_with_timeout(sem_t *S, int tmax) {
	flag=1;
	if (tmax < 1000)
	    ualarm(tmax*1000,0);
    else
	    alarm((float) tmax/1000);
	sem_wait(S);
	alarm(0);
	return flag;
};

void ALRM_handler() {
    /* Called when SIG_ALRM is received */
	flag = 0;
	sem_post(S);
};


int millisleep(long miliseconds) {
    // Req = values requested for the sleep | Rem = remaining time if the sleep is awaken
	struct timespec req, rem;
	if(miliseconds > 999) {
		req.tv_sec = (int)(miliseconds / 1000);
		req.tv_nsec = (miliseconds - ((long)req.tv_sec * 1000)) * 1000000;
	}
	else {
		req.tv_sec = 0;
		req.tv_nsec = miliseconds * 1000000;
	}
	return nanosleep(&req, &rem);
};

void* TF1 (void* max) {
	int t, tmax, result;
	int *a;
	a= (int*) max;
	tmax= *a;
	t= rand() % 5 + 1;
	millisleep(t);
	printf("Waiting on semaphore after %d milliseconds\n", t);
	result = wait_with_timeout(S, tmax);
	if (result == 1)
		printf("Wait returned normally. \n");
	else
		printf("Wait on semaphore S returned for timeout. \n");
	pthread_exit(NULL);
	return NULL;
}

void* TF2 () {
	int t= rand() % 9001 + 1000;
	millisleep(t);
	printf("Performing signal on sempahore s after %d milliseconds\n", t);
	sem_post(S);
	pthread_exit(NULL);
	return NULL;
}