#ifndef VM_FRAME_H
#define VM_FRAME_H


#include "threads/palloc.h"
#include <stdio.h>
#include <string.h>
#include <bitmap.h>
#include <debug.h>
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "threads/pte.h"
#include "userprog/process.h"
#include "userprog/exception.h"
#include "threads/init.h"
#include "userprog/pagedir.h"
#define NUM_FRAMES 1024
struct frame
{
	void* phys_address; //starting address of the frame that this struct represents
	struct thread* owner; //process that owns the memory in this frame, if any
	struct sup_page* resident; //page that lives in this frame, if any
};

static struct frame *frame_table[NUM_FRAMES];
void init_frame_table();
void set_frame(struct frame* f,void* kpage);
int get_open_frame();
/*void set_frame(struct frame *f, void *p_addr, struct thread* new_owner, struct sup_page* res){	
	f->phys_address = p_addr;
	f->owner = new_owner;
	f->resident = res;	
}*/

/*
//init frame table
frame_table[init_ram];
for(init_ram)
struct frame * f = NULL;
set_frame(f, 0, NULL, NULL);
frame_table[i] = f;

//can mix up where we parts of the update_sup_page
//page fault, stuff isn't in swap
check_if_need_to_panic();
get_frame_to_evict();

copy_to_swap();
update_sup_page();

set_frame(frame_to_evict, new_frame, new_frame , new_frame);
update_new_sup_page();

//page fault, stuff is in swap
check_if_need_to_panic();
get_frame_to_evict();

copy_to_swap();
update_sup_page(inside swap);

set_frame(frame_to_evict, new_frame, new_frame , new_frame);
update_sup_page(outside of memory);
update_new_sup_page(inside memory);

remove_page_from_swap();
update_new_sup_page(outside of swap);
*/

#endif 
