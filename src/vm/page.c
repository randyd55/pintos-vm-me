#include "page.h"
#include "threads/vaddr.h"
#include "lib/kernel/bitmap.h"

//Taken from Pintos Reference section A.8.5
/* Returns a hash value for page p. */
unsigned
page_hash (const struct hash_elem *p_, void *aux)
{
  const struct sup_page *p = hash_entry (p_, struct sup_page, hash_elem);
  return hash_bytes (&p->upage, sizeof p->upage);
}

/* Returns true if page a precedes page b. */
bool
page_less (const struct hash_elem *a_, const struct hash_elem *b_,
           void *aux)
{
  //printf("Yolo\n\n");
  const struct sup_page *a = hash_entry (a_, struct sup_page, hash_elem);
  //printf("Yolo Swaggins\n\n");
  const struct sup_page *b = hash_entry (b_, struct sup_page, hash_elem);
  //printf("Yolo>?>>\n\n");
  // if(a->upage != NULL){
  //   printf("AHHHHH \n\n\n\n\n");
  // }
  return a->upage < b->upage;
}

/* Returns the page containing the given virtual address,
   or a null pointer if no such page exists. */
struct sup_page *
page_lookup (const void *address)
{
  struct sup_page p;
  struct hash_elem *e;


  p.upage = (uint32_t)address&(~PGMASK);
  //printf("Im a upage: %x\n\n",p.upage);
  e = hash_find (&(thread_current()->spt), &p.hash_elem);
  return e != NULL ? hash_entry (e, struct sup_page, hash_elem) : NULL;
}
