#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "devices/block.h"
#include "lib/kernel/hash.h"

struct sup_page{
	void *k_frame; 	//points to the frame the page is loaded in, 
					//NULL if not loaded in
	block_sector_t swap_location; //location in swap, -1 if not 
	block_sector_t file_location; //location in file_system, -1 if not
	int writable; //indicates if page is writable
	struct hash_elem hash_elem; //elem for hash table
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

