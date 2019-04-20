

#ifndef VM_FRAME_H
#define VM_FRAME_H

#include "threads/thread.h"
#include "threads/synch.h"

/* frame.h declares the variabel and methods for the frame table and entires*/

#define NUM_FRAMES 1024
struct frame
{
	void* phys_address; //starting address of the kpage 
						//that this frame represents
	struct thread* owner; //process that owns the memory in this frame, if any
	struct sup_page* resident; //page that lives in this frame, if any
};

struct frame *frame_table[NUM_FRAMES]; //array that represents the frame_table
struct lock frame_lock; //lock when accessing frame table
void init_frame_table(); //initializes the frame table
void set_frame(struct frame* f,void* kpage); //installs a kpage into a frame
int get_open_frame(); //returns the first open frame in the frame table, if any

#endif 
