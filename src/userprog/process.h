#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#include "vm/frame.h"
#include "vm/swap.h"
tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

bool load_segment (struct file *file, off_t ofs, uint8_t *upage,
                          uint32_t read_bytes, uint32_t zero_bytes,
                          bool writable);

bool
install_page (void *upage, void *kpage, bool writable);

bool
replace_page (struct frame *f, struct sup_page *new_sup_page);

#define STACK_LIMIT 1000
#endif /* userprog/process.h */
