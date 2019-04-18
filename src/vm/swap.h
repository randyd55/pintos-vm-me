#ifndef VM_SWAP_H
#define VM_SWAP_H

#include "bitmap.h"
#include "devices/block.h"
static struct bitmap* swap_spots;
size_t init_swap(size_t bit_cnt);


#endif
