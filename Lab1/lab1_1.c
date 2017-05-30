#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>

int cmpfunc (const void*, const void*);

int main(int argc, char* argv[]){
	if (argc != 3){
		fprintf(stderr, "Error: wrong # of arguments");
		return -1;
	}
	
	srand(time(NULL));
	// Take from command line two integer numbers - as parameters
	int n1 = atoi(argv[1]);
	int n2 = atoi(argv[2]);
	
	// Allocate two vectors v1 and v2
	int *v1 = malloc(n1*sizeof(int));
	int *v2 = malloc(n2*sizeof(int));

	// Fill v1 with n1 random numbers between 10 and 100 - need random lib
	for (int i=0; i<n1; i++){
		do {
			v1[i] = rand() % 90;
			v1[i] += 10;
		} while (v1[i]%2!=0);
	}
	// Fill v2 with n2 random numbers between 21 and 101 - need random lib
	for (int i=0; i<n2; i++){
		do {
			v2[i] = rand() % 81;
			v2[i] += 21;
		} while (v2[i]%2==0);
	}
	// Printing arrays before Sorting
	printf("Array before sort:\n");
	printf("Array 1:\n");
	for (int i=0; i<n1; i++) printf ("%d\t", v1[i]);
	printf("\n");
	printf("Array 2:\n");
	for (int i=0; i<n2; i++) printf ("%d\t", v2[i]);
	printf("\n");

	//Sort v1 and v2
	qsort(v1, n1, sizeof(int), cmpfunc);
	qsort(v2, n2, sizeof(int), cmpfunc);

	// Print after sort (to check)
	printf("Array after sort:\n");
	printf("Array 1:\n");
	for (int i=0; i<n1; i++) printf ("%d\t", v1[i]);
	printf("\n");
	printf("Array 2:\n");
	for (int i=0; i<n2; i++) printf ("%d\t", v2[i]);
	printf("\n");

	// Print the first text file
	int fv1, fv2;
	if ((fv1 = open("fv1.txt", O_WRONLY | O_CREAT, 0777))<0){
		return -1;
	}
	char myBuff1[n1*4 + 4];
	int j = 0;
	for (int i=0; i<n1; i++) j+=sprintf(&myBuff1[j], "%d ", v1[i]);
	write(fv1, myBuff1, sizeof(char)*j);
	close(fv1);
	printf("File v1 written. \n");

	// Print the second text file
	if ((fv2 = open("fv2.txt", O_WRONLY | O_CREAT, 0777))<0){
		return -1;
	}
	char myBuff2[n2*4 + 4];
	j = 0;
	for (int i=0; i<n2; i++) j+=sprintf(&myBuff2[j], "%d ", v2[i]);
	write(fv2, myBuff2, sizeof(char)*j);
	close(fv2);
	printf("File v2 written. \n");

	// Print v1 in fv1.b
	if ((fv1 = open("fv1.b", O_WRONLY | O_CREAT, 0777))<0){
		return -1;
	}
	write(fv1, v1, sizeof(int)*n1);
	// Print v2 in fv2.b
	if ((fv2 = open("fv2.b", O_WRONLY | O_CREAT, 0777))<0){
		return -1;
	}
	write(fv2, v2, sizeof(int)*n2);

	exit(1);
}

int cmpfunc (const void * a, const void * b){
	return ( *(int*)a - *(int*)b );
}
