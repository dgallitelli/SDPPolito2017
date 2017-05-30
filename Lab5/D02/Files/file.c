//
// File descriptors
//

#include "types.h"
#include "defs.h"
#include "param.h"
#include "fs.h"
#include "file.h"
#include "spinlock.h"

struct devsw devsw[NDEV];
struct {
  struct spinlock lock;
  struct file file[NFILE];
} ftable;

struct semaphore {
	struct spinlock lock;
	int value;
	int valuelocked;
};

struct {
	struct spinlock lock;
	struct semaphore my_sems[MAX_SEMAPHORES];
} sem_table;

void
fileinit(void)
{
  initlock(&ftable.lock, "ftable");
}

void semaphoreInit(void){
	initlock(&sem_table.lock, "sem_table");
}

// Allocate a file structure.
struct file*
filealloc(void)
{
  struct file *f;

  acquire(&ftable.lock);
  for(f = ftable.file; f < ftable.file + NFILE; f++){
    if(f->ref == 0){
      f->ref = 1;
      release(&ftable.lock);
      return f;
    }
  }
  release(&ftable.lock);
  return 0;
}

int sem_alloc(void){
    struct semaphore *s;
	int k = 0;
    acquire(&sem_table.lock);
    for(s = sem_table.my_sems; s < sem_table.my_sems + MAX_SEMAPHORES; s+=sizeof(struct semaphore)){
        if(s->value == 0){
            s->value = 1;
            release(&sem_table.lock);
            return k;
        }
		k++;
    }
    release(&sem_table.lock);
    return -1;
}

void sem_init(int sem, int count){
    struct semaphore *s;
	s = &sem_table.my_sems[sem];
	
    acquire(&s->lock);
    s->value = count;
	s->valuelocked = 0;
    release(&s->lock);
}

void sem_destroy(int sem){
    struct semaphore *s;
	s = &sem_table.my_sems[sem];
	
    acquire(&s->lock);
    s->value = 0;
	s->valuelocked = 0;
    release(&s->lock);
}

void sem_wait(int sem){
    struct semaphore *s;
	s = &sem_table.my_sems[sem];
	
    acquire(&s->lock);
    s->value--;
	if (s->value<0)
		s->valuelocked--;
    for(;;){
        if(s->value >= 0)
        	break;
		else{
			sleep(s, &s->lock);
			if(s->value!=s->valuelocked){
				s->valuelocked++;
				break;
			}
		}
    }
    release(&s->lock);
}

void sem_post(int sem){
    struct semaphore *s;
	s = &sem_table.my_sems[sem];
	
    acquire(&s->lock);
    s->value++;
	if(s->value <= 0)
		wakeup(s);
    release(&s->lock);
}

// Increment ref count for file f.
struct file*
filedup(struct file *f)
{
  acquire(&ftable.lock);
  if(f->ref < 1)
    panic("filedup");
  f->ref++;
  release(&ftable.lock);
  return f;
}

// Close file f.  (Decrement ref count, close when reaches 0.)
void
fileclose(struct file *f)
{
  struct file ff;

  acquire(&ftable.lock);
  if(f->ref < 1)
    panic("fileclose");
  if(--f->ref > 0){
    release(&ftable.lock);
    return;
  }
  ff = *f;
  f->ref = 0;
  f->type = FD_NONE;
  release(&ftable.lock);
  
  if(ff.type == FD_PIPE)
    pipeclose(ff.pipe, ff.writable);
  else if(ff.type == FD_INODE){
    begin_trans();
    iput(ff.ip);
    commit_trans();
  }
}

// Get metadata about file f.
int
filestat(struct file *f, struct stat *st)
{
  if(f->type == FD_INODE){
    ilock(f->ip);
    stati(f->ip, st);
    iunlock(f->ip);
    return 0;
  }
  return -1;
}

// Read from file f.
int
fileread(struct file *f, char *addr, int n)
{
  int r;

  if(f->readable == 0)
    return -1;
  if(f->type == FD_PIPE)
    return piperead(f->pipe, addr, n);
  if(f->type == FD_INODE){
    ilock(f->ip);
    if((r = readi(f->ip, addr, f->off, n)) > 0)
      f->off += r;
    iunlock(f->ip);
    return r;
  }
  panic("fileread");
}

//PAGEBREAK!
// Write to file f.
int
filewrite(struct file *f, char *addr, int n)
{
  int r;

  if(f->writable == 0)
    return -1;
  if(f->type == FD_PIPE)
    return pipewrite(f->pipe, addr, n);
  if(f->type == FD_INODE){
    // write a few blocks at a time to avoid exceeding
    // the maximum log transaction size, including
    // i-node, indirect block, allocation blocks,
    // and 2 blocks of slop for non-aligned writes.
    // this really belongs lower down, since writei()
    // might be writing a device like the console.
    int max = ((LOGSIZE-1-1-2) / 2) * 512;
    int i = 0;
    while(i < n){
      int n1 = n - i;
      if(n1 > max)
        n1 = max;

      begin_trans();
      ilock(f->ip);
      if ((r = writei(f->ip, addr + i, f->off, n1)) > 0)
        f->off += r;
      iunlock(f->ip);
      commit_trans();

      if(r < 0)
        break;
      if(r != n1)
        panic("short filewrite");
      i += r;
    }
    return i == n ? n : -1;
  }
  panic("filewrite");
}
