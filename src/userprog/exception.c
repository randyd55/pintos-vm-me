#include "userprog/exception.h"
#include <inttypes.h>
#include <stdio.h>
#include "userprog/gdt.h"
#include "threads/vaddr.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/palloc.h"
#include "threads/loader.h"
#include "userprog/process.h"
#include "vm/frame.h"
#include "userprog/syscall.h"


/* Number of page faults processed. */
static long long page_fault_cnt;

static void kill (struct intr_frame *);
static void page_fault (struct intr_frame *);

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
  bool bad_read_below_esp = !write
  && (((uint32_t) fault_addr) > ((uint32_t) f->esp));

  // if(!write && ((uint32_t) fault_addr) > ((uint32_t) f->esp)){
  //   exit(-1);
  // }

  //in the case of a page fault and these conditions, we simply exit with -1
  if(fault_addr == NULL|| bad_read_below_esp|| is_kernel_vaddr(fault_addr) ||
  !is_user_vaddr(fault_addr)){
    //printf("Died due to bad address %x\n\n", fault_addr);
    exit(-1);
  }
  /* To implement virtual memory, delete the rest of the function
     body, and replace it with code that brings in the page to
     which fault_addr refers. */
  struct thread* t=thread_current();
  struct sup_page *p;
  struct sup_page *sp;
  bool success;
  uint32_t addr;
  uint8_t *kpage;
  p = page_lookup(fault_addr);
  //printf("Fault address: %x\n\n", fault_addr);
  //printf("P NULL?????? %d\n\n", p==NULL);
  if(p!=NULL)
    //printf("Sup page stuff: writable: %d, upage: %x, k_frame: %x, all_zeros: %d\n\n",p->writable, p->upage, p->k_frame, p->all_zeros);
  //uninstall();
  //remove_from_frame();
  if(get_open_frame()==-1 && p!=NULL){
     //printf("%s\n", "page faulting, eviction");
     replace_page(evict_this_frame_in_particular(),p);
  }
  else if(p!=NULL && p->all_zeros == true){
      //printf("%s\n", "page faulting, adding 0pg");
      kpage = palloc_get_page (PAL_USER | PAL_ZERO);
      //printf("after palloc_get_page\n");
      if (kpage == NULL){
        //printf("%s\n", "kpage is null\n");
        exit(-1);
      }
      if (!install_page (p->upage, kpage, p->writable, p))
          {
            //printf("%s\n", "failed installing the 0page");
            palloc_free_page (kpage);
            return false;
      }
      //printf("%s\n", "finished adding 0pgs");
  } 
  else if(write&&fault_addr>file_length(t->executable)*3000-(uint8_t)PHYS_BASE&&t->stack_pages<STACK_LIMIT){
    //printf("page faulting, growing the stack\n");

    bool success;
    addr=((uint32_t)f->esp&(~PGMASK));
    kpage = palloc_get_page (PAL_USER);
    sp = palloc_get_page(PAL_USER);
    if (kpage != NULL && sp != NULL){
      success = install_page (((uint8_t *)PHYS_BASE-(t->stack_pages+1)*PGSIZE), kpage, true, sp);
      if(success)
        t->stack_pages++;
    }

  } else{
      //printf("Big if, write: %d, heuristic: %d, stack size: %d\n\n",write,fault_addr>file_length(t->executable)*3000-(uint8_t)PHYS_BASE,t->stack_pages<STACK_LIMIT);
    //we gotta do LRU & use timer.c to track the
//time stamps of each frame an stuff
//its 1:36am send help plz in stalling on another
//project plz send help

/*when we evict we gotta: take out the entry From the
frame table so,  clear the page directory page

maybe dont take it out of spt, bc the process shouldnt
knwo we took it out...transparency and stuff??

search swap for open spot, then write it
maybe add a member to page, to know which
sector in swap we moved it to, so when we need it list_push_back
we can just go to the index and write it back to the frame (reclaim it)

//we need to add a swap bit to the spt or frame table i think, to track which ones are in swap

other notes: check the fault address if it is from a page we moved to swap or if it straight
up dont exist, then just evict again i guess?????
put the data from block back to frame_lock, then free that block bc we dont need the spot annymore

its 2am help/ this will prob all be incoherent tmrw and thats fine


    // printf("Im a page faults bastard child\n\n");
      /*if(lock_held_by_current_thread(&filesys_lock))
	       lock_release(&filesys_lock);
      if(lock_held_by_current_thread(&frame_lock))
         lock_release(&frame_lock);*/
    //printf("page faulting, exiting\n");

      if(lock_held_by_current_thread(&filesys_lock)){
         lock_release(&filesys_lock);
      }
      if(lock_held_by_current_thread(&frame_lock)){
         lock_release(&frame_lock);
      }
      //printf("I hate myself\n\n");
      exit(-1);
  }


  //printf("Did that stuff\n\n");
  /*printf ("Page fault at %p: %s error %s page in %s context.\n",
          fault_addr,
          not_present ? "not present" : "rights violation",
          write ? "writing" : "reading",

          user ? "user" : "kernel");

  printf("There is no crying in Pintos!\n");*/


  //kill (f);
}
struct frame*
evict_this_frame_in_particular(){
   //printf("%s\n", "evicting");
   return &frame_table[page_fault_cnt%NUM_FRAMES];
}
