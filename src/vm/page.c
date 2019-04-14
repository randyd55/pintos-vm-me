#include "page.h"
#include "lib/kernel/bitmap.h"

//Taken from Pintos Reference section A.8.5
/* Returns a hash value for page p. */
unsigned
page_hash (const struct hash_elem *p_, void *aux)
{
  const struct page *p = hash_entry (*p_, struct page, hash_elem);
  return hash_bytes (p->addr, sizeof p->k_page);
}

/* Returns true if page a precedes page b. */
bool
page_less (const struct hash_elem *a_, const struct hash_elem *b_,
           void *aux)
{
  const struct page *a = hash_entry (*a_, struct page, hash_elem);
  const struct page *b = hash_entry (*b_, struct page, hash_elem);

  return a->k_page < b->k_page;
}
