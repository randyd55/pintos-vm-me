#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "threads/interrupt.h"

//int *PHYS_BASE = (int *)0xC0000000; //CHANGE LATER

void syscall_init (void);
 /* terminates Pintos by calling shutdown_power_off()*/
void halt(void);
/*terminates the current user program*/
void exit(int status);
/*runs the executable whos name is given by cmd_line*/
//pid_t exec(const char *cmd_line);

// int wait(pid_t pid);

// bool create(const char *file, unsigned intial_size);

// bool remove(const char *file);

int open(const char *file);

// int filesize(int fd);

// int read(int fd, void *buffer, unsigned size);

int write(int fd, const void *buffer, unsigned size);

// void seek(int fd, unsigned position);

// unsigned tell(int fd);

// void close(int fd);*/

#endif /* userprog/syscall.h */
