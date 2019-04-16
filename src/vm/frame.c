#include "frame.h"
//#include "threads/malloc.c"


void
init_frame_table(){
  int i;
  for(i=0; i<NUM_FRAMES; i++){
    frame_table[i] = NULL;
  }
  lock_init(&frame_lock);
}


void
set_frame(struct frame* f,void* kpage, struct sup_page* sp){
  int open_spot;
  if(kpage == NULL){
    printf("%s\n", "asdf");
    exit(-1);
  }

  // if(*sp == NULL){
  //   printf("AHHHHHHHHHH\n\n\n");
  // }

  f->phys_address=kpage;
  f->owner=thread_current();
  f->resident=sp; //Fix supplemental page table and implement

  open_spot=get_open_frame();
  if(open_spot==-1){
  //  printf("AHHHHHHyolo\n\n");
    exit(-1);
  }

  frame_table[open_spot]=f;
  sp = malloc(sizeof(struct sup_page));


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
