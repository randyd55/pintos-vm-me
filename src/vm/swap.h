#ifndef VM_SWAP_H
#define VM_SWAP_H

#include "bitmap.h"
#include "devices/block.h"
struct bitmap* swap_spots;
struct block* swap_partition;
size_t init_swap(size_t bit_cnt);
int get_open_swap_slot(struct bitmap *bm);

#endif
