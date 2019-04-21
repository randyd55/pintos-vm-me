#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

bool load_segment (struct file *file, off_t ofs, uint8_t *upage,
                          uint32_t read_bytes, uint32_t zero_bytes,
                          bool writable);

bool
install_page (void *upage, void *kpage, bool writable);

void
insert_from_filesys(uint8_t* kpage, struct sup_page* p);

void
insert_from_swap(uint8_t* kpage, struct sup_page* p);

void
insert_into_stack(uint8_t* kpage);

void
evict_to_swap();
#endif /* userprog/process.h */
