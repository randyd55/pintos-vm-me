#include "swap.h"
struct bitmap *data;

size_t init_swap(size_t bit_cnt){
	swap_spots = NULL;
	swap_spots = bitmap_create(bit_cnt);
	swap_partition = NULL;
	swap_partition = block_get_role(BLOCK_SWAP);
	if(swap_spots == NULL)// swap_partition == NULL)
		return NULL;
	return 1;
}

int get_open_swap_slot(struct bitmap *bm){
    return bitmap_scan (bm, 0, 1, 0);
}
