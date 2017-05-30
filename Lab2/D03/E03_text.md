## E03 - Synchronization with sempahores (lab 2.3)

Implement a concurrent program in C language, using Pthreads, which generates two threads th\_1 and th\_2.

The main thread takes an argument n, and waits the termination of the threads it has created.
Thread th\_1 loops n times printing A1 then B1 in sequence on the same line.
Thread th\_2 loops n times printing A2 then B2 in sequence on the same line.
However, B1 and B2 must be printed only after A1 and A2, and the last thread must also print the newline character

```
Example: AB12 4
A1 A2 B2 B1
A2 A1 B2 B1
A1 A2 B1 B2
A1 A2 B2 B1
```
