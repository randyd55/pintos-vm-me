/*
Authors:
Anthony Moeller
Tim Gan
Randy Donaldson
Chineye Emeghara

Date:
28 March 2018


This program completes several actions that create, load, exec,
and exit processes, as well as creating the stack
*/


#include "userprog/process.h"
#include "userprog/syscall.h"
#include <debug.h>
#include <inttypes.h>
#include <round.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "userprog/gdt.h"
#include "userprog/pagedir.h"
#include "userprog/tss.h"
#include "filesys/directory.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/flags.h"
#include "threads/init.h"
#include "threads/interrupt.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "vm/frame.h"
#include "vm/page.h"
#include "threads/loader.h"
#include "vm/swap.h"




static thread_func start_process NO_RETURN;
static bool load (const char *cmdline, void (**eip) (void), void **esp);


/* Starts a new thread running a user program loaded from
   FILENAME.  The new thread may be scheduled (and may even exit)
   before process_execute( ) returns.  Returns the new process's
   thread id, or TID_ERROR if the thread cannot be created. */
tid_t
process_execute (const char *file_name)
{
  //printf("%s\n", "inside of process_execute");
  char *fn_copy = NULL;
  tid_t tid;
  char *save_ptr = NULL;
  char *fn = NULL;
  char *fn_temp = NULL;
  struct thread* t = thread_current();

  /* Make a copy of FILE_NAME.
     Otherwise there's a race between the caller and load(). */
  fn_copy = palloc_get_page (PAL_ZERO);
  if (fn_copy == NULL){
    return TID_ERROR;
  }
  strlcpy (fn_copy, file_name, PGSIZE);

//Randy Driving

/* Make a copy of file name, to tokenize the first token of the line*/
  fn_temp = palloc_get_page(PAL_ZERO);
  if(fn_temp == NULL){
    return TID_ERROR;
  }
  strlcpy(fn_temp, file_name, PGSIZE);
  fn = strtok_r(fn_temp, " ", &save_ptr);

  /* Create a new thread to execute FILE_NAME. */
  //printf("%s\n", "creating child thread");
  tid = thread_create (fn, PRI_DEFAULT, start_process, fn_copy);


  sema_down(&(t->exec_sema)); /*block whilst load child thread*/

  if(t->load_status == false){ /*check if the child loaded properly*/
    return -1; //failed load
  }
  /* add the child thread to the list of child threads for this list*/
  list_push_front(&(t->children), &(getThreadByTID(tid)->child_elem));

  //Randy Done Driving

  if (tid == TID_ERROR)
    palloc_free_page (fn_copy);

  palloc_free_page(fn_temp);

  return tid;
}

/* A thread function that loads a user process and starts it
   running. */
static void
start_process (void *file_name_)
{
  char *file_name = file_name_;
  struct intr_frame if_;
  bool success;
  /* Initialize interrupt frame and load executable. */
  memset (&if_, 0, sizeof if_);
  if_.gs = if_.fs = if_.es = if_.ds = if_.ss = SEL_UDSEG;
  if_.cs = SEL_UCSEG;
  if_.eflags = FLAG_IF | FLAG_MBS;
  //printf("inside of start_process\n");
  hash_init(&(thread_current()->spt), page_hash, page_less, NULL);
  success = load (file_name, &if_.eip, &if_.esp);
    //printf("Im alive\n\n");


  /* If load failed, quit. */
  palloc_free_page (file_name);

  //Tim Driving

  struct thread *parent_thread = thread_current()->parent;

  /*determine if child thread loaded properly*/
  if (success){
    parent_thread->load_status = true;
    sema_up(&(parent_thread->exec_sema));
    //printf("succeeded loading, starting process\n\n");

  } else {
      //  printf("Im alive\n\n");
    //printf("failed loading, exiting now\n\n\n");

    parent_thread->load_status = false;
    sema_up(&(parent_thread->exec_sema));
    sema_up(&(thread_current()->parent_wait_sema)); //still failing multi-oom, not sure if we care, but if we do, try immediately waiting on this thread
                                                    //after it fails loading
    //printf("Test yeye \n\n\n");

    thread_exit (); /*if load has failed, exit the thread*/
  }

  //Tim Done Driving

  /* Start the user process by simulating a return from an
     interrupt, implemented by intr_exit (in
     threads/intr-stubs.S).  Because intr_exit takes all of its
     arguments on the stack in the form of a `struct intr_frame',
     we just point the stack pointer (%esp) to our stack frame
     and jump to it. */
  asm volatile ("movl %0, %%esp; jmp intr_exit" : : "g" (&if_) : "memory");
  NOT_REACHED ();
}

/* Waits for thread TID to die and returns its exit status.  If
   it was terminated by the kernel (i.e. killed due to an
   exception), returns -1.  If TID is invalid or if it was not a
   child of the calling process, or if process_wait() has already
   been successfully called for the given TID, returns -1
   immediately, without waiting.

   This function will be implemented in problem 2-2.  For now, it
   does nothing. */
int
process_wait (tid_t child_tid UNUSED)
{
  //Anthony Driving

  struct thread* child;
  int status;

  child = getChildByPID(child_tid);
  if(child == NULL){
    return -1; //leave immediately
  }

  //Anthony Driving
  //Chineye Driving

  sema_down(&(child->child_exit_sema)); /*waits on child to call exit*/
  status = child->exit_status;
  //Tells child it has collected exit status
  list_remove(&(child->child_elem));
  sema_up(&(child-> parent_wait_sema));


  //Chineye Done

  return status;
}

/* Free the current process's resources. */
void
process_exit (void)
{
  struct thread *cur = thread_current ();
  uint32_t *pd;

  /* Destroy the current process's page directory and switch back
     to the kernel-only page directory. */
  pd = cur->pagedir;
  if (pd != NULL)
    {
      /* Correct ordering here is crucial.  We must set
         cur->pagedir to NULL before switching page directories,
         so that a timer interrupt can't switch back to the
         process page directory.  We must activate the base page
         directory before destroying the process's page
         directory, or our active page directory will be one
         that's been freed (and cleared). */
      cur->pagedir = NULL;
      pagedir_activate (NULL);
      pagedir_destroy (pd);
    }
}

/* Sets up the CPU for running user code in the current
   thread.
   This function is called on every context switch. */
void
process_activate (void)
{
  struct thread *t = thread_current ();

  /* Activate thread's page tables. */
  pagedir_activate (t->pagedir);

  /* Set thread's kernel stack for use in processing
     interrupts. */
  tss_update ();
}

/* We load ELF binaries.  The following definitions are taken
   from the ELF specification, [ELF1], more-or-less verbatim.  */

/* ELF types.  See [ELF1] 1-2. */
typedef uint32_t Elf32_Word, Elf32_Addr, Elf32_Off;
typedef uint16_t Elf32_Half;

/* For use with ELF types in printf(). */
#define PE32Wx PRIx32   /* Print Elf32_Word in hexadecimal. */
#define PE32Ax PRIx32   /* Print Elf32_Addr in hexadecimal. */
#define PE32Ox PRIx32   /* Print Elf32_Off in hexadecimal. */
#define PE32Hx PRIx16   /* Print Elf32_Half in hexadecimal. */

/* Executable header.  See [ELF1] 1-4 to 1-8.
   This appears at the very beginning of an ELF binary. */
struct Elf32_Ehdr
  {
    unsigned char e_ident[16];
    Elf32_Half    e_type;
    Elf32_Half    e_machine;
    Elf32_Word    e_version;
    Elf32_Addr    e_entry;
    Elf32_Off     e_phoff;
    Elf32_Off     e_shoff;
    Elf32_Word    e_flags;
    Elf32_Half    e_ehsize;
    Elf32_Half    e_phentsize;
    Elf32_Half    e_phnum;
    Elf32_Half    e_shentsize;
    Elf32_Half    e_shnum;
    Elf32_Half    e_shstrndx;
  };

/* Program header.  See [ELF1] 2-2 to 2-4.
   There are e_phnum of these, starting at file offset e_phoff
   (see [ELF1] 1-6). */
struct Elf32_Phdr
  {
    Elf32_Word p_type;
    Elf32_Off  p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
  };

/* Values for p_type.  See [ELF1] 2-3. */
#define PT_NULL    0            /* Ignore. */
#define PT_LOAD    1            /* Loadable segment. */
#define PT_DYNAMIC 2            /* Dynamic linking info. */
#define PT_INTERP  3            /* Name of dynamic loader. */
#define PT_NOTE    4            /* Auxiliary info. */
#define PT_SHLIB   5            /* Reserved. */
#define PT_PHDR    6            /* Program header table. */
#define PT_STACK   0x6474e551   /* Stack segment. */

/* Flags for p_flags.  See [ELF3] 2-3 and 2-4. */
#define PF_X 1          /* Executable. */
#define PF_W 2          /* Writable. */
#define PF_R 4          /* Readable. */

static bool setup_stack (void **esp, const char *file_name);
static bool validate_segment (const struct Elf32_Phdr *, struct file *);
//static bool load_segment (struct file *file, off_t ofs, uint8_t *upage,
//                          uint32_t read_bytes, uint32_t zero_bytes,
//                          bool writable);

/* Loads an ELF executable from FILE_NAME into the current thread.
   Stores the executable's entry point into *EIP
   and its initial stack pointer into *ESP.
   Returns true if successful, false otherwise. */
bool
load (const char *file_name, void (**eip) (void), void **esp)
{

  //printf("top of load\n");
  //Chineye Driving
  struct thread *t = thread_current ();
  struct Elf32_Ehdr ehdr;
  struct file *file = NULL;
  off_t file_ofs;
  bool success = false;
  int i;
  /* Allocate and activate page directory. */
  t->pagedir = pagedir_create ();
  if (t->pagedir == NULL){
    //printf("failing bc pagedir is null\n");
    goto done;
  }
  process_activate ();

  /* Open executable file. */
  char *fn_temp = NULL;
  fn_temp = palloc_get_page(PAL_ZERO | PAL_USER);
  if(fn_temp == NULL)
    return TID_ERROR;
  strlcpy(fn_temp, file_name, PGSIZE);

  char * save_ptr = NULL;
  char * fn = strtok_r(fn_temp, " ", &save_ptr);

  lock_acquire(&filesys_lock); /* ensure mutex for files*/

  file = filesys_open (fn);
  if (file == NULL)
    {
      printf ("load: %s: open failed\n", file_name);
      goto done;
    }

  //Chineye Done Driving
  //Anthony Driving

  /*deny writes to the executable*/

  t->executable = file;
  file_deny_write(file);

  /* Read and verify executable header. */
  if (file_read (file, &ehdr, sizeof ehdr) != sizeof ehdr
      || memcmp (ehdr.e_ident, "\177ELF\1\1\1", 7)
      || ehdr.e_type != 2
      || ehdr.e_machine != 3
      || ehdr.e_version != 1
      || ehdr.e_phentsize != sizeof (struct Elf32_Phdr)
      || ehdr.e_phnum > 1024)
    {
      printf ("load: %s: error loading executable\n", file_name);
      goto done;
    }

  /* Read program headers. */
  file_ofs = ehdr.e_phoff;
  for (i = 0; i < ehdr.e_phnum; i++)
    {
      struct Elf32_Phdr phdr;

      if (file_ofs < 0 || file_ofs > file_length (file)){
        //printf("failing bc offset is less than 0/greater than file length\n");
        goto done;
      }
      file_seek (file, file_ofs);

      if (file_read (file, &phdr, sizeof phdr) != sizeof phdr){
        //printf("failing bc fileread\n");
        goto done;
      }
      file_ofs += sizeof phdr;
      switch (phdr.p_type)
        {
        case PT_NULL:
        case PT_NOTE:
        case PT_PHDR:
        case PT_STACK:
        default:
          /* Ignore this segment. */
          break;
        case PT_DYNAMIC:
        case PT_INTERP:
        case PT_SHLIB:
          //printf("failing bc idk\n");
          goto done;
        case PT_LOAD:
          if (validate_segment (&phdr, file))
            {
              bool writable = (phdr.p_flags & PF_W) != 0;
              uint32_t file_page = phdr.p_offset & ~PGMASK;
              uint32_t mem_page = phdr.p_vaddr & ~PGMASK;
              uint32_t page_offset = phdr.p_vaddr & PGMASK;
              uint32_t read_bytes, zero_bytes;
              if (phdr.p_filesz > 0)
                {
                  /* Normal segment.
                     Read initial part from disk and zero the rest. */
                  read_bytes = page_offset + phdr.p_filesz;
                  zero_bytes = (ROUND_UP (page_offset + phdr.p_memsz, PGSIZE)
                                - read_bytes);
                }
              else
                {
                  /* Entirely zero.
                     Don't read anything from disk. */
                  read_bytes = 0;
                  zero_bytes = ROUND_UP (page_offset + phdr.p_memsz, PGSIZE);
                }

              if (!load_segment (file, file_page, (void *) mem_page,
                                 read_bytes, zero_bytes, writable)){
                  //printf("failed loading segment\n");
                  goto done;
              }
            }
          else{
            //printf("failed validating segment \n");
            goto done;
          }
          break;
        }
    }

  //Anthony Done
  //Chineye Driving

  /* Set up stack. */
  if (!setup_stack (esp, file_name)){
    //printf("(%s)\n", "rip stack");
    goto done;
  }
  /* Start address. */
  *eip = (void (*) (void)) ehdr.e_entry;
  //printf("%s\n", "getting to success = true");
  success = true;

 done:
  /* We arrive here whether the load is successful or not. */
  palloc_free_page(fn_temp);
  lock_release(&filesys_lock);
  //printf("success: %d\n",success);
  return success;

  //Chineye Done
}

/* load() helpers. */

//static bool install_page (void *upage, void *kpage, bool writable);

/* Checks whether PHDR describes a valid, loadable segment in
   FILE and returns true if so, false otherwise. */
static bool
validate_segment (const struct Elf32_Phdr *phdr, struct file *file)
{
  /* p_offset and p_vaddr must have the same page offset. */
  if ((phdr->p_offset & PGMASK) != (phdr->p_vaddr & PGMASK))
    return false;

  /* p_offset must point within FILE. */
  if (phdr->p_offset > (Elf32_Off) file_length (file))
    return false;

  /* p_memsz must be at least as big as p_filesz. */
  if (phdr->p_memsz < phdr->p_filesz)
    return false;

  /* The segment must not be empty. */
  if (phdr->p_memsz == 0)
    return false;

  /* The virtual memory region must both start and end within the
     user address space range. */
  if (!is_user_vaddr ((void *) phdr->p_vaddr))
    return false;
  if (!is_user_vaddr ((void *) (phdr->p_vaddr + phdr->p_memsz)))
    return false;

  /* The region cannot "wrap around" across the kernel virtual
     address space. */
  if (phdr->p_vaddr + phdr->p_memsz < phdr->p_vaddr)
    return false;

  /* Disallow mapping page 0.
     Not only is it a bad idea to map page 0, but if we allowed
     it then user code that passed a null pointer to system calls
     could quite likely panic the kernel by way of null pointer
     assertions in memcpy(), etc. */
  if (phdr->p_vaddr < PGSIZE)
    return false;

  /* It's okay. */
  return true;
}

/* Loads a segment starting at offset OFS in FILE at address
   UPAGE.  In total, READ_BYTES + ZERO_BYTES bytes of virtual
   memory are initialized, as follows:

        - READ_BYTES bytes at UPAGE must be read from FILE
          starting at offset OFS.

        - ZERO_BYTES bytes at UPAGE + READ_BYTES must be zeroed.

   The pages initialized by this function must be writable by the
   user process if WRITABLE is true, read-only otherwise.

   Return true if successful, false if a memory allocation error
   or disk read error occurs. */
bool
load_segment (struct file *file, off_t ofs, uint8_t *upage,
              uint32_t read_bytes, uint32_t zero_bytes, bool writable)
{
  //printf("Im a upage going into memory: %x\n\n", upage);
  ASSERT ((read_bytes + zero_bytes) % PGSIZE == 0);
  ASSERT (pg_ofs (upage) == 0);
  ASSERT (ofs % PGSIZE == 0);

  file_seek (file, ofs);
  while (read_bytes > 0 || zero_bytes > 0)
    {
      /* Calculate how to fill this page.
         We will read PAGE_READ_BYTES bytes from FILE
         and zero the final PAGE_ZERO_BYTES bytes. */
      struct sup_page *sp = palloc_get_page(PAL_USER | PAL_ZERO);

      if(sp == NULL){
          //printf("Bad heap shit\n\n");
          exit(-1);
      }

      size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
      size_t page_zero_bytes = PGSIZE - page_read_bytes;
      struct thread *t = thread_current ();
      /* Get a page of memory. */
      if (page_zero_bytes == PGSIZE){
        //printf("%s\n", "0 page added");
        

        //check if already mapped 
        if(pagedir_get_page (t->pagedir, upage) != NULL){
          palloc_free_page (sp);
          return false;
        }
        
        lock_acquire(&frame_lock);
        sp->all_zeros = true;
        sp->swap_location = -1;
        sp->file_location = -1;
        sp->writable = writable;
        sp->upage = upage;

        lock_release(&frame_lock);
      }

      //not all zeroes

      else{

        uint8_t *kpage = palloc_get_page (PAL_USER | PAL_ZERO);

        if (kpage == NULL){
          //printf("%s\n", "kpage is null\n");
          palloc_free_page (sp);
          return false;
        }

        /* Load this page. */
        if (file_read (file, kpage, page_read_bytes) != (int) page_read_bytes)
          {
          palloc_free_page (kpage);
          palloc_free_page (sp);
          //printf("%s\n", "rip file read");
          return false;
          }

        memset (kpage + page_read_bytes, 0, page_zero_bytes);

        /* Add the page to the process's address space. */
        if (!install_page (upage, kpage, writable, sp))
          {
            //printf("%s\n", "rip install page");
            palloc_free_page (kpage);
            palloc_free_page (sp);
            return false;
          }

        /* Advance. */
      }

      read_bytes -= page_read_bytes;
      zero_bytes -= page_zero_bytes;
      upage += PGSIZE;

      if (&(t->spt) == NULL){

        //printf("SPT null \n\n\n\n");
      }


      hash_insert (&(t->spt), &(sp->hash_elem)); //page should be allocated by here
      //printf("%d\n", writable);
    }
  return true;
}

/* Create a minimal stack by mapping a zeroed page at the top of
   user virtual memory.

   Parameters:
   - esp: stask pointer
   - file_name: the command line for the executable

   Return value: return true if the intallation of the page was successful
*/
static bool
setup_stack (void **esp, const char *file_name)
{

  uint8_t *kpage;
  bool success = false;
  char* temp_fn, *token, *save_ptr, *my_esp, *argv_ptr;
  char** argv;
  int argc, i;

  //Anthony Driving
  kpage = palloc_get_page (PAL_USER | PAL_ZERO);
  struct sup_page *sp = palloc_get_page (PAL_USER | PAL_ZERO);
  if (sp == NULL)
    exit(-1);
  if (kpage != NULL){

    success = install_page (((uint8_t *) PHYS_BASE) - PGSIZE, kpage, true, sp);

    if (success){
      *esp = PHYS_BASE;

      /*create deep copy of file_name*/
      temp_fn = palloc_get_page(PAL_ZERO);

  //Anthony Done
  //Randy Driving

      if(temp_fn == NULL)
        return TID_ERROR;

      /*create deep copy of file name for tokenizing*/
      strlcpy(temp_fn, file_name, strlen(file_name) + 1);

      argv = palloc_get_page(PAL_ZERO);
      if(argv == NULL)
        return TID_ERROR;
      my_esp = (char *) *esp;
      argc = 0;

      for(token = strtok_r (temp_fn, " ", &save_ptr); token != NULL;
        token = strtok_r (NULL, " ", &save_ptr)){
        argv[argc]=token;
        argc++;
      }

      i = argc - 1;
    //Randy Done
    //Tim Driving

      /*Copy argv into memory and store pointers of my_esp into argv*/
      for (; i >= 0; i--){

        my_esp -= strlen(argv[i]) + 1;

        /*check for overflow*/
        if((int) my_esp < PHYS_BASE - 4096){
          palloc_free_page (kpage);
          palloc_free_page(temp_fn);
          palloc_free_page(argv);
          return false; //failed
        }

        memcpy(my_esp, argv[i], strlen(argv[i]) + 1);
        argv[i] = my_esp;
      }

      //Add padding after strings stored to stack
      while((int) my_esp % 4 != 0){
        my_esp -= 1;
        /*check for overflow*/
        if((int) my_esp < PHYS_BASE - 4096){
          palloc_free_page (kpage);
          palloc_free_page(temp_fn);
          palloc_free_page(argv);
          return false; //failed
        }
      }

      i = argc - 1;
      my_esp -= sizeof(char*); //null sentinel

      /*check for overflow*/
      if((int) my_esp < PHYS_BASE - 4096){
        palloc_free_page (kpage);
        palloc_free_page(temp_fn);
        palloc_free_page(argv);
        return false; //failed
      }

      //pushing addresses of strings onto stack
      for(; i >= 0; i--){
        my_esp -= sizeof(char *);
        /*check for overflow*/
        if((int) my_esp < PHYS_BASE - 4096){
          palloc_free_page (kpage);
          palloc_free_page(temp_fn);
          palloc_free_page(argv);
          return false; //failed
        }
        memcpy(my_esp, &argv[i], sizeof(char *));
      }

      argv_ptr = my_esp;
      //Push address of argv onto stack
      my_esp -= sizeof(char **);
      /*check for overflow*/
      if((int) my_esp < PHYS_BASE - 4096){
        palloc_free_page (kpage);
        palloc_free_page(temp_fn);
        palloc_free_page(argv);
        return false; //failed
      }

      memcpy(my_esp, &argv_ptr, sizeof(char*));

      //Push argc onto stack
      my_esp -= sizeof(int);

      /*check for overflow*/
      if((int) my_esp < PHYS_BASE - 4096){
        palloc_free_page (kpage);
        palloc_free_page(temp_fn);
        palloc_free_page(argv);
        return false; //failed
      }

      *(int *)my_esp = argc;
      //Push void* onto stack for return address
      my_esp -= sizeof(int);

      /*check for overflow*/
      if((int) my_esp < PHYS_BASE - 4096){
        palloc_free_page (kpage);
        palloc_free_page(temp_fn);
        palloc_free_page(argv);
        return false; //failed
      }

      *(int *)my_esp = 0;
      *esp = my_esp;
      palloc_free_page(temp_fn);
      palloc_free_page(argv);
      palloc_free_page(token);
  }
  else
    palloc_free_page (kpage);
  }
  if(success)
    thread_current()->stack_pages=1;
  //Tim Done
  //printf("Stack setup!\n\n");
  return success;
}

/* Adds a mapping from user virtual address UPAGE to kernel
   virtual address KPAGE to the page table.
   If WRITABLE is true, the user process may modify the page;
   otherwise, it is read-only.
   UPAGE must not already be mapped.
   KPAGE should probably be a page obtained from the user pool
   with palloc_get_page().
   Returns true on success, false if UPAGE is already mapped or
   if memory allocation fails. */
bool
install_page (void *upage, void *kpage, bool writable, struct sup_page *sp)
{

  //printf("Install Page\n\n");
  struct frame *f;
  struct thread *t = thread_current ();
  if(sp->k_frame == NULL)
    f = palloc_get_page(PAL_USER);
  else{
    f = sp->k_frame;
  }
  //struct sup_page *sp = palloc_get_page(PAL_USER);
  if(f == NULL || sp == NULL){
    printf("Bad heap shit\n\n");
    exit(-1);
  }
  if(pagedir_get_page (t->pagedir, upage) != NULL)
    return false;
  bool success;

  /* Verify that there's not already a page at that virtual
     address, then map our page there. */

  success = pagedir_set_page (t->pagedir, upage, kpage, writable);
  if(success){
    lock_acquire(&frame_lock);
    set_frame(f, kpage, sp); //page should be initalizaed/allocated here 

    
      
    sp->k_frame = f;
    //sp->swap_location = -1;
    //sp->file_location = -1;
    sp->writable = writable;
    sp->upage = upage;
    //printf("upage: %x\n\n",upage);


    lock_release(&frame_lock);
  }


  return success;
}
//check for 0page case
  //uint8_t *kpage = palloc_get_page (PAL_USER | PAL_ZERO);
bool
replace_page (struct frame *f, struct sup_page *new_sup_page){
  printf("Replace page\n\n");
  struct sup_page *old_sup_page = f->resident;

  //move to swap
  int i;
  void *to_write;
  int swap_slot = get_open_swap_slot(&swap_partition);
  for(i = 0; i < 8; i++)
    block_write(&swap_partition, (swap_slot * 8) + i, old_sup_page->upage + i * (PGSIZE/8));
  
  old_sup_page->swap_location = swap_slot;
  bitmap_mark (&swap_partition, swap_slot);
  pagedir_clear_page (f->owner->pagedir, old_sup_page->upage);
  old_sup_page->k_frame = NULL;
  ;
  //insert new page into frame
  f->owner = thread_current();
  f->resident = new_sup_page;
  new_sup_page->k_frame = f;

  return pagedir_set_page (f->owner->pagedir, new_sup_page->upage, f->kpage, new_sup_page->writable);

}

