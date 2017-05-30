## Lab 5 Ex 2 - SDP - Davide Gallitelli S241521

The aim of this exercise is to implement semaphore in xv6, which is a kernel without threads. To realize that some files has been modified because we want to implement:

- sem\_alloc;
- sem\_init;
- sem\_destroy;
- sem\_wait;
- sem\_post.

The struct semaphore is implemented in file.c which has:

- spinlock: allow to implement the mutual exclusion;
- value: counter of the semaphore (in case it would be negative it means that the absolute value are the number of processes that are locked on that semaphore;
- valuelocked: necessary for waking up a single process and so allow us to know how many processes are waiting for a sem\_post.


#### file.c

In _file.c_ there is another struct, _sem\_table_, which has a spinlock and a vector of semaphore where the maximum number that can be instantiated is defined in another file, param.h, which is 1024. Moreover in this file there are the definitions of the functions to be implemented:

- _void semaphoreInit (void)_: initialize the lock on the semaphore table.
- _int sem\_alloc(void)_:  acquires the lock on the semaphore table, looks for the first available semaphore (not yet initialized) and returns an integer identifier _k_, then releases the lock.
- _void sem\_init(int sem, int count)_: initializes the semaphore identified by _sem_ to the value _count_. The variable _valuelocked_ is set to 0.
- _void sem\_destroy(int sem)_: resets the _count_, as well as _valuelocked_, of the semaphore _sem_ to 0.
- _void sem\_wait(int sem)_: it acquires the lock on the _sem_ semaphore; it decrements its count, and decrements _valuelocked_ only if _value_ is less than 0; then it loops infinitely, sleeping until a _wakeup_ is performed by some other process. The process which performs the _sleep_ releases temporary the lock until a _wakeup_ is called. A _wakeup_ function awakens all processes which performed a _sleep_. A process woken up while value is equal to valuelocked, which means that it cannot access its critical section, cannot leave the loop and sleeps again. Only then it releases the lock on the semaphore.
- _void sem\_post(int sem)_: increments the _value_ of the _sem_ semaphore, then performs a _wakeup_ if there is at least one process sleeping, which means that _valuelocked_ is negative.

#### sysfile.c

The _sysfile.c_ file contains the definition of the system calls, which are called when the appropriate function is called by the user. 
The main concept behind these functions is to check for correctness of the arguments, if any, and call the appropriate function defined in _file.c_.

The following example is for the _sys\_sem\_post_ function:

```
int sys_sem_post(void){
	int sem;
	if (argint(0, &sem) < 0)
		return -1;
	sem_post(sem);
	return 0;	
}
```

#### main.c

In order to initialize at boot the semaphore table, the _semaphoreInit()_ function is called in the main procedure of the _main.c_ file.

#### Other files

The other files that have been modified are: 

- defs.h
- Makefile
- param.h
- syscall.c
- syscall.h
- user.h
- usys.S
