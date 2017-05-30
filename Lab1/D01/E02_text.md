## E02 - Synchronization with semaphores

Implement a concurrent program in C language, using Pthreads, which creates two client
threads, then it acts as a server.

A client thread loops reading the next number from the binary file (fv1.b and fv2.b,
respectively), and storing it in a global variable g. Then, it performs a signals on a
semaphore to indicate to the server that the variable is ready, and it waits on a semaphore
a signal from the server indicating that the number has been processed (simply multiplied
by 3), finally, it prints the result and its identifier.
The server loops waiting the signal of the clients, doing the multiplication, storing the
results on the same global variable g, and signalling to the client that the string is ready to
be printed.

The main thread waits the end on the threads, and prints the total number of served
requests.