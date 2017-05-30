#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/times.h>
#include <limits.h>
#include "pthread.h"

int size;
typedef struct
{
	int *v, left, right;
}quicks_struct;

void quicksort (int*, int, int);

int main(int argc, char **argv)
{
	int fdsc, leng;
	caddr_t paddr;
	struct stat buf_state;
	
	if (argc !=2){
		fprintf(stderr, "Error: the number of argument is wrong");
		return -1;
	}
	size = atoi(argv[1]);
	char path[] = "file1.bin";
	if ((fdsc = open(path, O_RDWR, 0777)) <0) {
		fprintf(stderr, "opening error %s1n", path);
		return -1;
	}
	if (fstat (fdsc, &buf_state) == -1)
   		perror ("fstat");	
	leng = buf_state.st_size;
	printf ("the File size in byte is %d\n", leng);
	paddr = mmap ((caddr_t) 0, leng, PROT_READ | PROT_WRITE, MAP_SHARED, fdsc, 0); 
	if (paddr == (caddr_t) - 1)
	{
		perror ("mmap");
	}
	int left = 0;
	int right = (len/sizeof(int))-1;
	
	quicksort((int*)paddr, left, right);
	
	close(fdsc);

	pthread_exit(NULL);
}


void* quicksorthread(void* argStruct)
{
	quicks_struct *myStruct = (quicks_struct*) argStruct;
	quicksort(myStruct->v, myStruct->left, myStruct->right);
	return NULL;
	
}


void quicksort (int *v, int left, int right) {
	int i, j, x;
	if (left >= right)
	return;
	x = v[left];
	i = left - 1;
	j = right + 1;
	while (i < j) {
		while (v[--j] > x);
		while (v[++i] < x);
		if (i < j){
			int tmp;
			tmp = v[i];
			v[i] = v[j];
			v[j] = tmp;
		}
	}
	if( right - left >= size) {
		pthread_t tid[2];
		quicks_struct *QSS = (qs_struct*) malloc(sizeof(qs_struct));
		quicks_struct *QSS2 = (qs_struct*) malloc(sizeof(qs_struct));
		QSS->v = v; 
		QSS->left = left;
		QSS->right = j;
		QSS2->v = v;
		QSS2->left = j+1;
		QSS2->right = right;
		int rc;
		rc = pthread_create(&tid[0], NULL, quicksorthread, (void*) QSS);
		if(rc) {
			fprintf(stderr, "ERROR: create() of thread with code %d\n", rc);
		}
		rc = pthread_create(&tid[1], NULL, quicksorthread, (void*) QSS2);
		if(rc) {
			fprintf(stderr, "ERROR: create() of thread with code %d\n", rc);
		}
		pthread_join(tid[0], NULL);
		pthread_join(tid[1], NULL);
	} else {	
		quicksort (v, left, j);
		quicksort (v, j+1, right);
	}
	return;
}