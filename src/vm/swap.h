#ifndef VM_SWAP_H
#define VM_SWAP_H

/*
    swap.h initalizes the variables and functions utilized for 
    page eviction to swap

    Authors: Chineye, Randy, Anthony, Tim

    Date: April 19th 2019
*/

//Chineye is driving
#include "bitmap.h"
#include "devices/block.h"
#define SECTORS_PER_PAGE 8
struct bitmap* swap_spots; //bitmap to indicate open spots in the swap
struct block* swap_partition; //swap itself
size_t init_swap(size_t bit_cnt); //initializes the swap
int get_open_swap_slot(struct bitmap *bm); //gets first open spot in the swap

#endif
