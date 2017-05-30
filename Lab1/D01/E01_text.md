## E01 - C program for file generation

Implement a C program that:

-   takes from the command line two integer numbers n1, n2,
-   allocates two vectors v1 and v2, of dimensions n1 and n2, respectively,
-   fills v1 with n1 random even integer numbers between 10-100,
-   fills v2 with n2 random odd integer numbers between 21-101,
-   sort v1 and v2 (increasing values),
-   save the content of vectors v1 and v2 in two text files fv1.txt and
fv2.txt, respectively,
-   save the content of vectors v1 and v2 in two binary files fv1.b and
fv2.b, respectively,
Use command _od_ for verifying the content of files v1, v2.

_Standard ANSI C_
```
FILE *fpw; // file pointer
if ((fpw = fopen (filename, "wb")) == NULL){ // w for write,
b for binary
fprinf(stderr," error open %s\n", filename);
return(1);
}
fwrite(buffer,sizeof(buffer),1, fpw); // write sizeof(buffer)
bytes from buffer
```
_Using Unix system calls_
```
#include <unistd.h>
#include <fcntl.h>
int fdo; // file descriptor
if ((fdo = open(filename, O_WRONLY)) < 0){
fprinf(stderr," error open %s\n", filename);
return 1;
}
write(fdo, buffer, sizeof(buffer));
```