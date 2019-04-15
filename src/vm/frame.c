#include "frame.h"

void
init_frame_table(){
  int i;
  for(i=0; i<NUM_FRAMES; i++){
    frame_table[i] = NULL;
  }
  lock_init(&frame_lock);
}


void
set_frame(struct frame* f,void* kpage, struct sup_page* s){
  int open_spot;
  if(kpage == NULL){
    printf("%s\n", "asdf");
    exit(-1);
  }
  f->phys_address=kpage;
  f->owner=thread_current();
  f->resident=s; //Fix supplemental page table and implement

  open_spot=get_open_frame();
  if(open_spot==-1){
    printf("AHHHHHHyolo\n\n");
    exit(-1);
  }

  frame_table[open_spot]=f;
  

}

int
get_open_frame(){
  int i;
  for(i=0; i<NUM_FRAMES; i++){
    if(frame_table[i]==NULL)
      return i;
  }
  return -1;
}
