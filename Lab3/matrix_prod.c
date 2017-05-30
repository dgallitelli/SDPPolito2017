#include <stdio.h>
#include <stdlib.h>
 
void ** allocaM2d (int nr, int nc, int dim);
void prodmat(double ** a,  double ** b, double ** c, int nr1, int nc1nr2,
               int nc2);

int main(void) {

double **dmat1, **dmat2, **dmatprod;
int  nr1, nc1, nr2, nc2, dim, i, j, k;


printf("Rows and Columns of the first matrix: ");
scanf ("%d%d",&nr1,  &nc1);
do {
  printf("Rows and Columns of the second matrix\n");
  printf("The number of rows must be equal to nc1 ");
  scanf ("%d%d",&nr2,  &nc2);
} while (nr2 != nc1);

//  dynamic allocation 

dim = sizeof(double);

dmat1 = (double **) allocaM2d(nr1,  nc1, dim);
dmat2 = (double **) allocaM2d(nr2,  nc2, dim);
dmatprod = (double **) allocaM2d(nr1, nc2, dim);

k=0;
for(i=0;i<nr1;i++)
  for(j=0;j<nc1;j++)
    dmat1[i][j] = k++;

for(i=0;i<nr1;i++){
  for(j=0;j<nc1;j++)
    printf("%lf ", dmat1[i][j]);
  printf("\n");
}

printf("\n");
k=0;
for(i=0;i<nr2;i++)
  for(j=0;j<nc2;j++)
    dmat2[i][j] = k++;

for(i=0;i<nr2;i++){
  for(j=0;j<nc2;j++)
    printf("%lf ", dmat2[i][j]);
  printf("\n");
}


prodmat( dmat1, dmat2, dmatprod, nr1, nc1, nc2);

printf("\n");
for(i=0;i<nr1;i++){
  for(j=0;j<nc2;j++)
    printf("%lf ", dmatprod[i][j]);
  printf("\n");
}

}

/************************************************************/

void **
allocaM2d (int nr, int nc, int dim){
int i;
void ** m;
char * vd;  //  byte pointer

m = (void **) malloc(nr * sizeof(void *));
vd = (char *) malloc(nr * nc * dim);

for(i=0;i<nr;i++)
  m[i] = &vd[i * nc * dim];

return (void **) m;	// return matrix pointer		
}

/***********************************************************************/

void 
prodmat( double ** a,  double **b, double **c, 
         int nr1, int nc1, int nc2)
{ int i,j,k;
  for (i=0; i<nr1; i++)
    for (j=0; j<nc2; j++) {
      c[i][j] = 0;
      for (k=0; k<nc1; k++)
        c[i][j] += a[i][k]*b[k][j];
    }
  return;
}

