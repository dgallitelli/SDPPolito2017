## E02 - Synchronization with semaphores (lab 2.2)

Implement a concurrent program in C language, using Pthreads, which generates n threads th\_A executing code A, and 2\*n threads th\_B executing code B.

The main thread takes argument n from the command line, and terminates without waiting the termination of the threads it has created.

Both threads simply print their corresponding character, followed by their order of creation number (from 0 to n-1 for th\_A, from 0 to 2*n-1 for th\_B). After two Bs and an A characters have been printed, in whichever order, the last printing thread must print the ’\\n’ character.

``` 
Example: BBA 4
A1 B2 B3
B4 B5 A0
B1 A2 B0
A3 B7 B6
```