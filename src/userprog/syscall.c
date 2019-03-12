#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "no-vm/multi-oom.c"
#include "filesys/file.c"
//#include "vaddr.h"

static void syscall_handler (struct intr_frame *);
int fd_count = 1;
struct lock filesys_lock;

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
        exit(*((uint32_t *) (f->esp + 4)), f);
        break;

}
  hex_dump(*((uint32_t *) f->esp), *((uint32_t *) f->esp), PHYS_BASE - *((uint32_t *) f->esp),  1);
  printf ("system call!\n");
}

/*  */
void halt(void){

  shutdown_power_off();

}

void exit(int status, struct intr_frame *f UNUSED){

//create thread struct and assign variable to the status
//call thread exit
thread_exit();

}

int open(const char *file){

    lock_acquire(&filesys_lock);
    struct file* temp_file = filesys_open (file);

    if(temp_file == NULL){ //can't be opened
        return -1;
    } else {
      fd_count++;
      return fd_count;
    }
    lock_release(&filesys_lock);


}

int write(int fd, const void *buffer, unsigned size){

  int byte_count;
  uint8_t write = 0;

  int i = 0;
  file_write()


}
