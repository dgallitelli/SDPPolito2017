# SDP 2016/17 - Lab 3 - Davide Gallitelli S241521

### E02 - Text

Write a C program using Pthreads that implements the product of two matrices m1 and m2, of dimension nr1 x nc1 and nr2 x nc2, respectively, where nr1 and nc1 are the number of rows and columns of matrix m1, and nr2 and nc2 refer to matrix m2.
The dimensions of the matrices are given as arguments of the command line, and you must verify that nc1 and nr2 are equal.

The main thread allocate dynamically the two matrices, fills each matrix with increasing integer numbers, starting from 0. Then, it creates nr1*nc2 threads prod\_th, and an additional thread p\_th that is responsible for printing the product of the two matrices when all threads prod\_th have completed their work.

The main threads does not wait the termination of the prod\_th threads. Each thread prod\_th computes the product of the i-th row of matrix m1 and the j-th column of matrix m2.