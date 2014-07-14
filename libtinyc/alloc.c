/* A really dumb dynamic memory allocator.  */

#include "stdlib.h"
#include "string.h"

//#define DEBUG

#ifdef DEBUG
#include <stdio.h>
#include <unistd.h>

char *__HEAP_RUN__, *__HEAP_SIZE__;

static void __attribute__((constructor))
init (void)
{
  void *chunk = sbrk (65536);
  __HEAP_RUN__ = chunk;
  __HEAP_SIZE__ = 65536;
}
#endif

extern char __RAM_START__;
extern char __HEAP_RUN__;
extern unsigned int __BSS_SIZE__, __RAM_SIZE__;

/* This is a little hack to stop the linker discarding the otherwise-empty
   HEAP section.  */
static char the_heap __attribute__((section("HEAP"), used));

typedef struct heapblock {
  struct heapblock *next;
  size_t size;  /* The total block size, including the header.  */
} heapblock;

#define HEADER(P) ((heapblock *) ((char *) (P) - sizeof (heapblock)))
#define BLOCK(P) (((char *) (P)) + sizeof (heapblock))

static heapblock *freelist = 0;

/* FIXME: Use a constructor for this at program start.  */

static void
init_heap (void)
{
  static int initialized = 0;
  
  if (!initialized)
    {
      freelist = (heapblock *) &__HEAP_RUN__;
      freelist->next = NULL;
      freelist->size = &__RAM_START__ + __RAM_SIZE__ - &__HEAP_RUN__;
      initialized = 1;
    }
}

#ifdef DEBUG
static void
print_freelist (void)
{
  heapblock *walk;
  
  init_heap ();
  
  for (walk = freelist; walk; walk = walk->next)
    printf ("free block: %p, next %p, size %zu\n", walk, walk->next,
            walk->size);
}
#endif

/* [HDR] [                     ...                      ]
   | <-                   origsize                   -> |

   [HDR1] [      ...     ] [HDR2] [         ...         ]
   | <- carve+hdrsize -> | | origsize - (carve+hdrsize) |
*/

static heapblock *
split_block (heapblock *hb, size_t carve)
{
  heapblock *orig_next = hb->next;
  size_t origsize = hb->size;
  size_t newplushdr = carve + sizeof (heapblock);
  heapblock *remainder = (heapblock *) (((char *) hb) + newplushdr);
  
  hb->next = remainder;
  hb->size = newplushdr;
  remainder->next = orig_next;
  remainder->size = origsize - newplushdr;
  
  return remainder;
}

/* Walk over free list until we find a block big enough for SIZE.  */

void *
malloc (size_t size)
{
  heapblock *prev = NULL, *walk;
  size_t sizeplusblk = size + sizeof (heapblock);
  
  init_heap ();
  
  for (walk = freelist; walk; walk = walk->next)
    {
      if (sizeplusblk < walk->size)
        {
	  heapblock *rem = split_block (walk, size);

	  if (prev)
	    prev->next = rem;
	  else if (walk == freelist)
	    freelist = rem;
	  
	  return BLOCK (walk);
	}
      else if (sizeplusblk == walk->size)
        {
	  /* Block fits exactly.  */
	  if (prev)
	    prev->next = walk->next;
	  else if (walk == freelist)
	    freelist = walk->next;
	  
	  return BLOCK (walk);
	}

      prev = walk;
    }
  
  return NULL;
}

/* When freeing we could tidy up by e.g. quicksorting the free list, and then
   merging adjacent blocks (maybe only when we run out of heap).  For now
   though, just add the deleted block to the free list.  */

void
free (void *ptr)
{
  heapblock *hb = HEADER (ptr);
  
#ifdef DEBUG
  printf ("free: heapblock: %p\n", hb);
#endif
  
  hb->next = freelist;
  freelist = hb;
}

void *
calloc (size_t nmemb, size_t size)
{
  size_t totsize = nmemb * size;
  void *newblk = malloc (totsize);
  memset (newblk, 0, totsize);
  return newblk;
}

void *
realloc (void *ptr, size_t size)
{
  void *newblk = malloc (size);
  memcpy (newblk, ptr, size);
  free (ptr);
  return newblk;
}

#ifdef DEBUG
int main (int argc, char *argv)
{
  char *ptr, *ptr2;
  
  print_freelist ();
  ptr = malloc (128);
  printf ("ptr: %p\n", ptr);
  print_freelist ();
  ptr2 = malloc (64);
  printf ("ptr2: %p\n", ptr2);
  print_freelist ();
  free (ptr);
  print_freelist ();
  printf ("allocate again:\n");
  ptr = malloc (64);
  printf ("new ptr: %p\n", ptr);
  print_freelist ();
  return 0;
}
#endif
