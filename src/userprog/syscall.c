#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
//#include "no-vm/multi-oom.c"
//#include "filesys/file.c"
#include "filesys/filesys.c"
//#include "vaddr.h"

static void syscall_handler (struct intr_frame *);
 int *PHYS_BASE = (int *)0xC0000000;
int fd_count = 1;
struct lock *filesys_lock;

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&filesys_lock);
}

static void
syscall_handler (struct intr_frame *f UNUSED)
{ 
 
  switch(*((uint32_t *) (f->esp))){ 
    case SYS_HALT : 
        halt(); 
        break; 
    case SYS_EXIT : 
        exit(*((uint32_t *) (f->esp + 4) ));
        break;

}
  hex_dump(*((uint32_t *) f->esp), *((uint32_t *) f->esp), PHYS_BASE - *((uint32_t *) f->esp),  1);
  printf ("system call!\n");
}

/*  */
void halt(void){

  shutdown_power_off();

}

void exit(int status){

//create thread struct and assign variable to the status
//call thread exit
thread_exit();

}
//int wait(pid_t pid){

//}
int open(const char *file){

    struct file* f_open=NULL;
    int open_spot;
    lock_acquire(&filesys_lock);
    thread_current()->fd++;
    open_spot=getFd();
    if(open_spot!=-1){
      thread_current()->files[open_spot] = file;
      f_open = file_open(file) -> inode; //this is just wrong i think?
    }
   // const char* temp_file = thread_current()->files[fd];
   lock_release(&filesys_lock);
   
    if(f_open == NULL){
      return -1;
    } else {
      return open_spot+2;
    }
}
int getFd(){
  int i;
  for(i=0; i<128; i++){
    if(thread_current()->files[i]==NULL)
      return i;
  }
  return -1;
}
int read(int fd, const void *buffer, unsigned size){
  int bytes_read=0;
  lock_acquire(&filesys_lock);
  struct thread* t =thread_current();
  //Shits a little whack, buffer expand input????
  //input_getc doooffff????????
  if(fd==0){
    buffer=input_getc(buffer,size);
    bytes_read+=1;
  }
  else if(t->files[fd-2]!=NULL){
    bytes_read=file_read(t->files[fd-2],buffer,size);
  }
  lock_release(&filesys_lock);
  return bytes_read;
}
bool create(const char* file, unsigned initial_size){
  return filesys_create(file, initial_size);
}
bool remove(const char* file){
  return filesys_remove(file);
}
int write(int fd, const void *buffer, unsigned size){

  int written=0;
  lock_acquire(&filesys_lock);
  struct thread* t =thread_current();
  if(fd==1){
    putbuf(buffer,size);
    written = size;
  }
  else if(t->files[fd-2]!=NULL){
    written=file_write(t->files[fd-2],buffer,size);
  }
  lock_release(&filesys_lock);
  return written;
}

void close(int fd){
  struct thread* t=thread_current();
  lock_acquire(&filesys_lock);
  if(t->files[fd-2]!=NULL){
    filesys_remove(t->files[fd-2]->inode);
    t->files[fd-2]=NULL;
  }
  lock_release(&filesys_lock);
}
