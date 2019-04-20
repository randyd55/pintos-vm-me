#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "devices/block.h"
#include "lib/kernel/hash.h"
#include "threads/thread.h"
struct sup_page{
	void *k_frame; 	//points to the frame the page is loaded in, 
					//NULL if not loaded in
	void *upage;
	block_sector_t swap_location; //location in swap, -1 if not 
	struct file * file; //location in file_system, -1 if not
	off_t file_offset; //offset in file
	int writable; //indicates if page is writable
	struct hash_elem hash_elem; //elem for hash table
	int page_read_bytes; 
	bool allocated; //true if allocated, false otherwise
	
};


//Taken from Pintos Reference section A.8.5
/* Returns a hash value for page p. */
unsigned
page_hash (const struct hash_elem *p_, void *aux);


/* Returns true if page a precedes page b. */
bool
page_less (const struct hash_elem *a_, const struct hash_elem *b_,
           void *aux);
struct sup_page *
page_lookup (const void *address);

#endif 

