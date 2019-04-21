#ifndef VM_FRAME_H
#define VM_FRAME_H

/*
#include "threads/palloc.h"
#include <stdio.h>
#include <string.h>
#include <bitmap.h>
#include <debug.h>
#include "threads/vaddr.h"

#include "threads/pte.h"
#include "userprog/process.h"
#include "userprog/exception.h"
#include "threads/init.h"
#include "userprog/pagedir.h"
*/
#include "threads/thread.h"
#include "threads/synch.h"


#define NUM_FRAMES 366
#define HEURISTIC 3000

struct frame
{
	void* kpage; //starting address of the kpage 
						//that this frame represents
	struct thread* owner; //process that owns the memory in this frame, if any
	struct sup_page* resident; //page that lives in this frame, if any

};

struct frame *frame_table[NUM_FRAMES]; //array that represents the frame_table
struct lock frame_lock; //lock when accessing frame table
void init_frame_table(); //initializes the frame table
void set_frame(struct frame* f,void* kpage, struct sup_page * s); //installs a kpage into a frame
int get_open_frame(); //returns the first open frame in the frame table, if any
int evict_this_frame_in_particular();
#endif 
