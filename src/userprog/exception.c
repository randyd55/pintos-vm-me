#include "userprog/exception.h"
#include <inttypes.h>
#include <stdio.h>
#include "userprog/gdt.h"
#include "threads/vaddr.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/palloc.h"
#include "userprog/process.h"
#include "vm/frame.h"
#include "vm/swap.h"
#include "userprog/syscall.h"

/* Number of page faults processed. */
static long long page_fault_cnt;

static void kill (struct intr_frame *);
static void page_fault (struct intr_frame *);
int asdf = 0;

/* Registers handlers for interrupts that can be caused by user
   programs.

   In a real Unix-like OS, most of these interrupts would be
   passed along to the user process in the form of signals, as
   described in [SV-386] 3-24 and 3-25, but we don't implement
   signals.  Instead, we'll make them simply kill the user
   process.

   Page faults are an exception.  Here they are treated the same
   way as other exceptions, but this will need to change to
   implement virtual memory.

   Refer to [IA32-v3a] section 5.15 "Exception and Interrupt
   Reference" for a description of each of these exceptions. */
void
exception_init (void) 
{
  /* These exceptions can be raised explicitly by a user program,
     e.g. via the INT, INT3, INTO, and BOUND instructions.  Thus,
     we set DPL==3, meaning that user programs are allowed to
     invoke them via these instructions. */
  intr_register_int (3, 3, INTR_ON, kill, "#BP Breakpoint Exception");
  intr_register_int (4, 3, INTR_ON, kill, "#OF Overflow Exception");
  intr_register_int (5, 3, INTR_ON, kill,
                     "#BR BOUND Range Exceeded Exception");

  /* These exceptions have DPL==0, preventing user processes from
     invoking them via the INT instruction.  They can still be
     caused indirectly, e.g. #DE can be caused by dividing by
     0.  */
  intr_register_int (0, 0, INTR_ON, kill, "#DE Divide Error");
  intr_register_int (1, 0, INTR_ON, kill, "#DB Debug Exception");
  intr_register_int (6, 0, INTR_ON, kill, "#UD Invalid Opcode Exception");
  intr_register_int (7, 0, INTR_ON, kill,
                     "#NM Device Not Available Exception");
  intr_register_int (11, 0, INTR_ON, kill, "#NP Segment Not Present");
  intr_register_int (12, 0, INTR_ON, kill, "#SS Stack Fault Exception");
  intr_register_int (13, 0, INTR_ON, kill, "#GP General Protection Exception");
  intr_register_int (16, 0, INTR_ON, kill, "#MF x87 FPU Floating-Point Error");
  intr_register_int (19, 0, INTR_ON, kill,
                     "#XF SIMD Floating-Point Exception");

  /* Most exceptions can be handled with interrupts turned on.
     We need to disable interrupts for page faults because the
     fault address is stored in CR2 and needs to be preserved. */
  intr_register_int (14, 0, INTR_OFF, page_fault, "#PF Page-Fault Exception");
}

/* Prints exception statistics. */
void
exception_print_stats (void) 
{
  printf ("Exception: %lld page faults\n", page_fault_cnt);
}

/* Handler for an exception (probably) caused by a user process. */
static void
kill (struct intr_frame *f) 
{
  /* This interrupt is one (probably) caused by a user process.
     For example, the process might have tried to access unmapped
     virtual memory (a page fault).  For now, we simply kill the
     user process.  Later, we'll want to handle page faults in
     the kernel.  Real Unix-like operating systems pass most
     exceptions back to the process via signals, but we don't
     implement them. */
     
  /* The interrupt frame's code segment value tells us where the
     exception originated. */
  switch (f->cs)
    {
    case SEL_UCSEG:
      /* User's code segment, so it's a user exception, as we
         expected.  Kill the user process.  */
      printf ("%s: dying due to interrupt %#04x (%s).\n",
              thread_name (), f->vec_no, intr_name (f->vec_no));
      intr_dump_frame (f);
      thread_exit (); 

    case SEL_KCSEG:
      /* Kernel's code segment, which indicates a kernel bug.
         Kernel code shouldn't throw exceptions.  (Page faults
         may cause kernel exceptions--but they shouldn't arrive
         here.)  Panic the kernel to make the point.  */
      intr_dump_frame (f);
      PANIC ("Kernel bug - unexpected interrupt in kernel"); 

    default:
      /* Some other code segment?  Shouldn't happen.  Panic the
         kernel. */
      printf ("Interrupt %#04x (%s) in unknown segment %04x\n",
             f->vec_no, intr_name (f->vec_no), f->cs);
      thread_exit ();
    }
}

/* Page fault handler.  This is a skeleton that must be filled in
   to implement virtual memory.  Some solutions to project 2 may
   also require modifying this code.

   At entry, the address that faulted is in CR2 (Control Register
   2) and information about the fault, formatted as described in
   the PF_* macros in exception.h, is in F's error_code member.  The
   example code here shows how to parse that information.  You
   can find more information about both of these in the
   description of "Interrupt 14--Page Fault Exception (#PF)" in
   [IA32-v3a] section 5.15 "Exception and Interrupt Reference". */
// Randy and Timothy drove here
static void
page_fault (struct intr_frame *f) 
{
  //printf("page fault\n\n");
  bool not_present;  /* True: not-present page, false: writing r/o page. */
  bool write;        /* True: access was write, false: access was read. */
  bool user;         /* True: access by user, false: access by kernel. */
  void *fault_addr;  /* Fault address. */

  /* Obtain faulting address, the virtual address that was
     accessed to cause the fault.  It may point to code or to
     data.  It is not necessarily the address of the instruction
     that caused the fault (that's f->eip).
     See [IA32-v2a] "MOV--Move to/from Control Registers" and
     [IA32-v3a] 5.15 "Interrupt 14--Page Fault Exception
     (#PF)". */
  
  asm ("movl %%cr2, %0" : "=r" (fault_addr));

  /* Turn interrupts back on (they were only off so that we could
     be assured of reading CR2 before it changed). */
  intr_enable ();

  /* Count page faults. */
  page_fault_cnt++;

  /* Determine cause. */
  not_present = (f->error_code & PF_P) == 0;
  write = (f->error_code & PF_W) != 0;
  user = (f->error_code & PF_U) != 0;

  //in the case of a page fault and these conditions, we simply exit with -1
  if(fault_addr==NULL||is_kernel_vaddr(fault_addr)||!is_user_vaddr(fault_addr)){

  
  struct hash_iterator i;
    exit(-1);
  }

  struct thread* t=thread_current();
  struct sup_page *p;
  int open_spot;

  //Find page for demand paging
  p = page_lookup(fault_addr);
  
  open_spot = get_open_frame();
  uint8_t *kpage = palloc_get_page (PAL_USER | PAL_ZERO);

  lock_acquire(&frame_lock);

  //Evict Page
  if(open_spot==-1 && p != NULL){
    evict_to_swap();
  }
  //Import from filesys
  else if(p != NULL && p->swap_location == -1){
    insert_from_filesys(kpage,p);
  }
  //Import from swap
  else if(p != NULL){
    insert_from_swap(kpage,p);
  }
  //Create page for stack
  //must be a write, within the heuristic, and the number of stack pages must
  //be less than the number of frames
  else if(p==NULL&&write&&
    fault_addr>file_length(t->executable)*HEURISTIC-(uint8_t)PHYS_BASE&&
    t->stack_pages<NUM_FRAMES){
    insert_into_stack(kpage);
 
  }
  //exit if the faulting address isn't valid/we don't grow the stack 
  else{
    exit(-1);
  }
  lock_release(&frame_lock);
}


void
evict_to_swap(){
  int eviction_spot;
  int swap_slot;
  struct frame* fr;
  struct sup_page *old_sup_page;
  int i;
  void*to_write;

  //Get frame you want to evict
  eviction_spot = evict_this_frame_in_particular();
  swap_slot = get_open_swap_slot(swap_spots);
  fr = frame_table[eviction_spot];
  old_sup_page = fr->resident;

  //Ensure swap isnt full
  if(swap_slot == BITMAP_ERROR){
    printf("swap full \n");
    exit(-1);
  }

  //Write page to swap
  for(i = 0; i < SECTORS_PER_PAGE; i++){
    off_t offset;
    offset = i * (BLOCK_SECTOR_SIZE);
    void * buf;
    buf = malloc(BLOCK_SECTOR_SIZE);
    memcpy(buf, fr->kpage + offset, BLOCK_SECTOR_SIZE);
    block_write(swap_partition, (swap_slot * SECTORS_PER_PAGE) + i, buf);
    free(buf);
  }

  //Update swap info
  old_sup_page -> swap_location = swap_slot;
  bitmap_mark(swap_partition, swap_slot);
  
  //Free page for process use
  palloc_free_page(fr->kpage);
  pagedir_clear_page(fr->owner->pagedir, old_sup_page->upage);
  free(frame_table[eviction_spot]);
  old_sup_page->k_frame = NULL;
  frame_table[eviction_spot] = NULL;

}


void
insert_into_stack(uint8_t* kpage){

  //allocate frame and page to insert into process stack
  struct frame *fr;
  fr = malloc(sizeof(struct frame));
  struct sup_page* p=(struct sup_page*)malloc(sizeof(struct sup_page));
  if(fr == NULL || p == NULL)
    exit(-1);

  bool success;
  struct thread* t=thread_current();

  //Set frame and install page to stack
  if (kpage != NULL)
  {
    success = install_page (((uint8_t *)PHYS_BASE-(t->stack_pages+1)*PGSIZE),
                                                            kpage, true);
    if(success)
      t->stack_pages++;
  } 
  set_frame(fr, kpage, p);
  //Set sup page properties and insert into sup page table
  p->allocated=true;
  p->k_frame = fr; 
  p->upage=((uint8_t *)PHYS_BASE-(t->stack_pages+1)*PGSIZE);
  p->writable=true;
  hash_insert(&thread_current()->spt,&p->hash_elem);
}



void
insert_from_swap(uint8_t* kpage, struct sup_page* p){
  struct frame *fr;
  fr= malloc(sizeof(struct frame));
  if (kpage == NULL)
  {
      printf("kpage null?\n\n");
      exit(-1);
  }
  //Get page from swap
  int i;
  for(i = 0; i < SECTORS_PER_PAGE; i++)
  {
    off_t offset;
    offset = i * (BLOCK_SECTOR_SIZE);
    void * buf;
    buf = malloc(BLOCK_SECTOR_SIZE);
    
    block_read(swap_partition, 
      (p->swap_location * SECTORS_PER_PAGE) + i, buf);
    memcpy(kpage + offset, buf, BLOCK_SECTOR_SIZE);
    free(buf);
  }

  //Install page to frame table and page table
  if (!install_page (p->upage, kpage, p->writable))
  {
    palloc_free_page (kpage);
    printf("install page failed \n\n");
    exit(-1);
  }
  set_frame(fr, kpage, p);
}



void
insert_from_filesys(uint8_t* kpage, struct sup_page* p){
  //Initialize frame
  struct frame *fr;
  fr = malloc(sizeof(struct frame));
  file_seek (p->file, p->file_offset);

  //Check that kpage isnt null
  if (kpage == NULL){
    exit(-1); //shouldn't get here
  }

  //Read page from file
  if (file_read (p->file, kpage, 
    p->page_read_bytes) != (int) p->page_read_bytes)
    {
      palloc_free_page (kpage);
      printf("file_read failed\n\n");
      exit(-1);
    }
  memset(kpage + p->page_read_bytes, 0, PGSIZE - p->page_read_bytes);

  //Install page
  if (!install_page (p->upage, kpage, p->writable))
    {
      palloc_free_page (kpage);
      exit(-1);
    }
  set_frame(fr, kpage, p);  
}

