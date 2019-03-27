#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "threads/thread.h"
#include "threads/interrupt.h"
#include "threads/synch.h"
#include "devices/shutdown.h"


int wait(pid_t pid);

void syscall_init (void);
 /* terminates Pintos by calling shutdown_power_off()*/
void halt(void);
/*terminates the current user program*/
void exit(int status);
/*runs the executable whos name is given by cmd_line*/
pid_t exec(const char *cmd_line);

// int wait(pid_t pid);

bool create(const char *file, unsigned intial_size);

bool remove(const char *file);

int open(const char *file);

int filesize(int fd);

int read(int fd, const void *buffer, unsigned size);

int write(int fd, const void *buffer, unsigned size);

void seek(int fd, unsigned position);

unsigned tell(int fd);

void close(int fd);

int getFd(void);

bool check_pointer(uint32_t * stack_ptr);
#endif /* userprog/syscall.h */
