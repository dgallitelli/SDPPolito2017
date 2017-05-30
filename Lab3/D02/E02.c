#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define debugger_flag 0

typedef struct {
    int row;
    int column;
} Product;

sem_t *S;
int thread_number;
int **m_prod, **m1, **m2, nr1, nc1, nr2, nc2, thread_number;

void* TF_prod(void*);
void* TF_p(void*);

int main(int argc, char *argv[]){
    if (argc!=5) {
        fprintf (stderr, "Bad syntax. Usage: %s <nr1> <nc1> <nr2> <nc2>. \n", argv[0]);
        return -1;
    }
    
    nr1 = atoi(argv[1]);
    nc1 = atoi(argv[2]);
    nr2 = atoi(argv[3]);
    nc2 = atoi(argv[4]);
    
    if (nc1 != nr2){
        fprintf (stderr, "Bad values. nc1 should be equal to nr2. \n");
        return -2;
    }

    if (debugger_flag > 0) printf("Ho letto gli argomenti. \n");
    
    S = (sem_t*) malloc(sizeof(sem_t));
    sem_init(S,0,0);
    
    m1 = (int**) malloc(nr1*sizeof(int*));
    for (int i = 0; i<nr1; i++){
        m1[i] = (int*) malloc(nc1*sizeof(int));
    }

    if (debugger_flag > 0) printf("Ho allocato la matrice 1. \n");
    
    m2 = (int**) malloc(nr2*sizeof(int*));
    for (int i = 0; i<nr2; i++){
        m2[i] = (int*) malloc(nc2*sizeof(int));
    }

    if (debugger_flag > 0) printf("Ho allocato la matrice 2. \n");
    
    m_prod = (int**) malloc(nr1*sizeof(int*));
    for (int i = 0; i<nr2; i++){
        m_prod[i] = (int*) malloc(nc2*sizeof(int));
    }

    if (debugger_flag > 0) printf("Ho allocato la m_prod. \n");
    
    int k;
    k = 0;
    for (int i = 0; i<nr1; i++){
        for (int j = 0; j<nc1; j++){
            m1[i][j] = k;
            k++;
        }
    }

    if (debugger_flag > 0) printf("Ho riempito la matrice 1. \n");

    k = 0;
    for (int i = 0; i<nr2; i++){
        for (int j = 0; j<nc2; j++){
            m2[i][j] = k;
            k++;
        }
    }

    if (debugger_flag > 0) printf("Ho riempito la matrice 2. \n");
    
    
    thread_number = nr1*nc2;
    Product *prod_struct;
    prod_struct = (Product*) malloc(thread_number*sizeof(Product));

    if (debugger_flag > 0) printf("Ho allocato la matrice dei Product. \n");
    
    
    pthread_t *prod_th, p_th;
    prod_th = (pthread_t*) malloc(thread_number*sizeof(pthread_t));

    if (debugger_flag > 0) printf("Ho allocato i thread. \n");
    
    k=0;
    for (int i=0; i<nr1; i++){
        for (int j=0; j<nc2; j++){

            if (debugger_flag > 0) printf("Assegno al Product %d la coppia (%d,%d) \n", k, i, j);
            prod_struct[k].row = i; 
            prod_struct[k].column = j; 
            if (debugger_flag > 0) printf("Coppia Assegnata. \n");
            pthread_create(&prod_th[k], NULL, TF_prod, (void*) &prod_struct[k]);
            if (debugger_flag > 0) printf("Thread %d creato. \n", k);
            k++;
        }
    };

    if (debugger_flag > 0) printf("Ho creato i thread. \n");
    
    pthread_create(&p_th, NULL, TF_p, NULL);
    
    pthread_exit(NULL);
    
    return 1;
}

void* TF_prod(void* prod){
    
    Product* myStruct = (Product*) prod;

    m_prod[myStruct->row][myStruct->column] = 0;
    for (int k=0; k<nc1; k++)
         m_prod[myStruct->row][myStruct->column] += m1[myStruct->row][k]*m2[k][myStruct->column];
    
    sem_post(S);
    pthread_exit(NULL);
}

void* TF_p(void* p){
    for (int i=0; i<thread_number; i++)
        sem_wait(S);
        
    for (int i=0; i<nr1; i++){
        for (int j=0; j<nc2; j++)
            printf ("%d \t", m_prod[i][j]);
        printf ("\n");
    }
    
    pthread_exit(NULL);
}