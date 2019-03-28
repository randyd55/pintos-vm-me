#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "filesys/filesys.h"
#include "userprog/process.h"
#include "threads/vaddr.h"
#include "devices/input.h"
#include "userprog/pagedir.h"

static void syscall_handler (struct intr_frame *);


int fd_count = 1;
//struct lock filesys_lock;

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&filesys_lock);
}

static void
syscall_handler (struct intr_frame *f UNUSED)
{


  //printf("YOU SHOULD BE IN SYSCALL HANDLER \n\n\n\n\n");
  if(!check_pointer(f->esp))
    exit(-1);

  switch(*((uint32_t *) (f->esp))){
    case SYS_EXEC :
        if (check_pointer(*(int*)(f->esp + 4))){
          f->eax=exec(((char *) *(int*)(f->esp + 4)));
        }
        else
      		exit(-1);
        break;
    case SYS_WRITE :
        if (check_pointer(f->esp +4) && check_pointer(*(int*)(f->esp + 8)) && check_pointer(f->esp + 12)){
          f->eax=write(*((uint32_t *) (f->esp + 4) ),((void*)*(int*)(f->esp + 8) ),*((unsigned *) (f->esp + 12) ));
        }
        else
      		exit(-1);
        break;
    case SYS_READ :
       if (check_pointer(f->esp + 4) && check_pointer(*(int*)(f->esp + 8)) && check_pointer(f->esp + 12))
          f->eax=read(*((uint32_t *) (f->esp + 4) ),((void*)*(int*)(f->esp + 8) ),*((unsigned *) (f->esp + 12) ));
      else
      	exit(-1);
      break;
    case SYS_OPEN :
        if (check_pointer(*(int*)(f->esp + 4)))
          f->eax=open(((char *) *(int*)(f->esp + 4) ));
      	else
      		exit(-1);
        break;
    case SYS_CLOSE :
        if (check_pointer(f->esp + 4))
          close(*((uint32_t *) (f->esp + 4) ));
        else
      		exit(-1);
        break;
    case SYS_CREATE :
		if (check_pointer(*(int*)(f->esp +4))){
          f->eax=create((char*)*(int*) (f->esp + 4),(*(int*)(f->esp + 8)));
        }
        else
      		exit(-1);
        break;
    case SYS_FILESIZE:
        f->eax= filesize(*(int*)(f->esp +4));
        break;
    case SYS_SEEK:
    //printf("SEEK\n\n\n");
      if(check_pointer((f->esp+4)))
        seek(*(int*)(f->esp+4),*(unsigned*)(f->esp+8));
      break;
    case SYS_TELL:
        f->eax=tell(*(unsigned*)f->esp+4);
        break;
    case SYS_HALT :
        halt();
        break;
    case SYS_EXIT :
        if (check_pointer(f->esp + 4))
          exit(*((uint32_t *) (f->esp + 4) ));
      	else
      		exit(-1);
        break;
    case SYS_WAIT :
       f->eax= wait(*(int*)(f->esp +4));
       break;

  }

  //printf ("system call!\n");
}

bool check_pointer(uint32_t * stack_ptr){
  if (stack_ptr == NULL || is_kernel_vaddr(stack_ptr) || !is_user_vaddr(stack_ptr) || pagedir_get_page(thread_current() -> pagedir, stack_ptr) == NULL){

    return false;
}
  return true;

}

/*  */
void halt(void){
  shutdown_power_off();
}

void exit(int status){
  struct thread *t = thread_current();
  t->exit_status = status;

  printf("%s: exit(%d)\n",t->name,t->exit_status);
  
  file_close(t->executable);
  thread_exit();
}


int wait(pid_t pid){
  return process_wait(pid);
}

pid_t exec(const char *cmd_line){

  int e = process_execute(cmd_line);
  return e;

}

int open(const char *file){

    struct file* f_open = NULL;
    int open_spot;
    lock_acquire(&filesys_lock);
    thread_current()->fd++;
    open_spot = getFd();
    if(open_spot != -1){
      f_open = filesys_open(file); //this is just wrong i think?
      thread_current()->files[open_spot] = f_open; //fix her
    }

   lock_release(&filesys_lock);


    if(f_open == NULL){
      return -1;
    } else {
      return open_spot;
    }
}

/*helper method*/
int getFd(){
  int i;
  for(i = 2; i < 130; i++){
    if(thread_current()->files[i] == NULL)
      return i;
  }
  return -1;
}
int read(int fd, const void *buffer, unsigned size){

  int bytes_read = 0, i = 0;
  lock_acquire(&filesys_lock);
  struct thread* t = thread_current();

  if(fd == 0){
    for(; i < size; i++){
      *(uint8_t *) buffer = input_getc();
      buffer++;
    }

    bytes_read = i;

  }
  else if(fd < 0 || fd > 130){
     bytes_read = -1;
  }
  else if(t->files[fd] == NULL) {
    bytes_read = -1; //file cannot be read, because it doesn't exist
  } else { //file exists
    bytes_read = file_read(t->files[fd], buffer, size);
  }

  lock_release(&filesys_lock);
  return bytes_read;
}

int filesize(int fd){

  return file_length(thread_current()->files[fd]);
}

bool create(const char* file, unsigned initial_size){
  return filesys_create(file, initial_size);
}

bool remove(const char* file){
  return filesys_remove(file);
}


int write(int fd, const void *buffer, unsigned size)
{

  if(buffer==NULL)
    exit(-1);

  int written = 0;
  lock_acquire(&filesys_lock);
  struct thread* t = thread_current();

  if(fd <= 0 || fd > 130){
    lock_release(&filesys_lock);
    exit(-1);
  }
  else if(fd == 1){
    putbuf((char*)buffer, size ); //user input
    written = size;
  }

  else {
    written = file_write(t->files[fd],buffer,size); //changed from fd-2 to fd
  }
  lock_release(&filesys_lock);
  return written;
}
void seek(int fd, unsigned position){
  struct thread* t=thread_current();
  file_seek(t->files[fd],position);
}
unsigned tell(int fd){
  struct thread* t=thread_current();
  file_tell(t->files[fd]);
}
void close(int fd){
  struct thread* t = thread_current();

  if(fd < 2 || fd>128){
    exit(-1);
  }

  lock_acquire(&filesys_lock);

  if(t->files[fd] != NULL){ //if the file exists

    file_close (t->files[fd]); //close file
    t->files[fd] = NULL; //free up spot

  }

  lock_release(&filesys_lock);
}
