#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/times.h>
#include <limits.h>

#define LOOP 10

int
main (argc, argv)
     int argc;
     char *argv[];
{
  int fd, len, pg, i, j;
  struct stat stat_buf;
  char c, *paddr;

  if ((fd = open (argv[1], O_RDWR)) == -1)
    perror ("open");

  if (fstat (fd, &stat_buf) == -1)
    perror ("fstat");
  len = stat_buf.st_size;

  pg = getpagesize ();
  printf ("Page size %d\n", pg);
  printf ("File size in byte %ld\n", len);
  len = len + pg - (len % pg);
  printf ("File size rounded to page size %ld\n", len);

  paddr = mmap ((caddr_t) 0, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  //paddr = mmap ((caddr_t) 0, len, PROT_READ | PROT_WRITE,  MAP_PRIVATE, fd, 0);
  if (paddr == (caddr_t) - 1)
	perror ("mmap");

  close (fd);

  for (j = 0; j < LOOP; j++) {
    printf ("loop %d\n", j);
    for (i = 0; i < len - pg; i += (2 * pg)) {
      c = paddr[i];
      paddr[i] = 'Y';
      paddr[i + 1] = 'Y';
    }

    for (i = pg; i < len - pg; i += (2 * pg)) {
      c = paddr[i];
      paddr[i] = 'N';
      paddr[i + 1] = 'N';
    }
  }
  return 0;
}
