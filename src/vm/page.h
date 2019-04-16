#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "devices/block.h"
#include "lib/kernel/hash.h"
#include "threads/thread.h"
struct sup_page{
	void *k_frame;
	void *upage;
	block_sector_t swap_location;
	block_sector_t file_location;
	int writable;
	struct hash_elem hash_elem;
};


//Taken from Pintos Reference section A.8.5
/* Returns a hash value for page p. */
unsigned
page_hash (const struct hash_elem *p_, void *aux);


/* Returns true if page a precedes page b. */
bool
page_less (const struct hash_elem *a_, const struct hash_elem *b_,
           void *aux);

#endif
