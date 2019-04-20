#include "swap.h"
//Randy is driving

/*
	swap.c: handles initialiozation and flipping of bits for the swap 
	implementation

	Authors: Chineye, Randy, Anthony, Tim

	Date: APril 19th 2019
*/



/*init_swap: initializes the BLOCK_SWAP, bitmap

  Paramters: 
  - size_t bit_cnt: number of bits to determine 
  how many swap spots we will need 

  Return: 
  - return NULL if bit_map or BLOCK_SWAP partition cannot be created
  - return 1 if the bitmap and BLOCK_SWAP were created successfully
*/

struct bitmap *data; //bitmap to track the swap slots

size_t init_swap(size_t bit_cnt){
	swap_spots = NULL;
	swap_spots = bitmap_create(bit_cnt);
	swap_partition = NULL;
	swap_partition = block_get_role(BLOCK_SWAP);
	if(swap_spots == NULL)
		return NULL;
	return 1;
}
/*
  get_open_swap_slot: retireves the first open swap spot set to true

  Paramters: 
  - bitmap *bm

  Return: 
  - the bitmap index for the first index set to false (available)
*/
int get_open_swap_slot(struct bitmap *bm){
    return bitmap_scan (bm, 0, 1, 0);
}
