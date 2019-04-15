#include "swap.h"
struct bitmap *data;

size_t init_swap(size_t bit_cnt){
	data = NULL;
	data = bitmap_create(bit_cnt);
	if(data == NULL)
		return NULL;
	return 1;
}
