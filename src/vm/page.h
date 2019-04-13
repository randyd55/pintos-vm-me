#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "devices/block.h"

struct sup_page{
	void *k_frame;
	block_sector_t swap_location;
	block_sector_t file_location;
};
#endif 
