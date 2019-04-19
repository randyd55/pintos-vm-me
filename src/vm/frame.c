/* frame.c is a program that assist in intializing the frame table, 
adding frames, clearing frames, 
and other utilities for managing the frame table

Authors: Timothy Gan, Randy Donaldson, Anthony Mueller, Chineye Emeghara

Dates: April 19th 2019


*/

#include "frame.h"


/*
  init_frame_table: intializes the frame table

  Parameters: none
  Return values: none
*/
void
init_frame_table(){
  int i;

  //Search for an open spot
  for(i = 0; i < NUM_FRAMES; i++){
    frame_table[i] = NULL;
  }

  //initialize the lock for frame table synchronization
  lock_init(&frame_lock);
}


/*
  set_frame: sets the first available frame for kpage, into the frame table
  Parameters: 
  - struct frame *f: the pointer to the specific frame for physical address
  - void* kpage: The physical address for the frame

  Return values: none

*/
void
set_frame(struct frame* f,void* kpage){
  int open_spot;
  if(kpage == NULL){
    exit(-1);
  }
  f->phys_address = kpage;
  f->owner = thread_current();
  f->resident = NULL;

  open_spot = get_open_frame();
  if(open_spot == -1){
    exit(-1);
  }

  frame_table[open_spot] = f;
  

}

/*
get_open_frame: Searched the Frame Table for the first (if any) available
frame entry 

Parameters: none
Return values: none
*/
int
get_open_frame(){
  int i;
  for(i = 0; i < NUM_FRAMES; i++){
    if(frame_table[i] == NULL)
      return i;
  }
  return -1;
}
