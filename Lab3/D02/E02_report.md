# SDP 2016/17 - Lab 3 - Davide Gallitelli S241521

### E02 - Report

In order to solve this exercise, a semaphore has been used to synchronize the threads _prod\_th_ which compute an element of the final matrix. After each computation, each thread performs a _sem\_post_; the main thread _p\_th_, which has been waiting on the same semaphore, is unlocked only once all computations have been performed, so that the matrix can be printed. 

Because each thread _prod\_th_ computes the product of the i-th row of matrix m1 and the j-th column of matrix m2, a struct is used to provide the indices to the threads. The threads are the one responsible for reading the values from the i-th row of matrix m1 and the j-th column of matrix m2 to compute the result. The values are read concurrently from the matrices, however read is not an exclusive operation therefore no semaphore to manage concurrent access is needed.