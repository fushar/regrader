/*
 *	UCW Library -- Fast Allocator for Fixed-Size Elements
 *
 *	(c) 2007 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

/*
 *  This allocator is optimized for intensive allocation and freeing of small
 *  blocks of identical sizes. System memory is allocated by multiples of the
 *  page size and it is returned back only when the whole eltpool is deleted.
 *
 *  In the future, we can add returning of memory to the system and also cache
 *  coloring like in the SLAB allocator used in the Linux kernel.
 */

#undef LOCAL_DEBUG

#include "ucw/lib.h"
#include "ucw/eltpool.h"

struct eltpool *
ep_new(uns elt_size, uns elts_per_chunk)
{
  struct eltpool *pool = xmalloc_zero(sizeof(*pool));
  pool->elt_size = ALIGN_TO(MAX(elt_size, sizeof(struct eltpool_free)), CPU_STRUCT_ALIGN);
  pool->chunk_size = CPU_PAGE_SIZE;
  while (pool->elt_size * elts_per_chunk + sizeof(struct eltpool_chunk) > pool->chunk_size)
    pool->chunk_size *= 2;
  pool->elts_per_chunk = (pool->chunk_size - sizeof(struct eltpool_chunk)) / pool->elt_size;
  DBG("ep_new(): got elt_size=%d, epc=%d; used chunk_size=%d, epc=%d", elt_size, elts_per_chunk, pool->chunk_size, pool->elts_per_chunk);
  return pool;
}

void
ep_delete(struct eltpool *pool)
{
  struct eltpool_chunk *ch;
  while (ch = pool->first_chunk)
    {
      pool->first_chunk = ch->next;
      page_free(ch, pool->chunk_size);
    }
  xfree(pool);
}

void *
ep_alloc_slow(struct eltpool *pool)
{
  struct eltpool_chunk *ch = page_alloc(pool->chunk_size);
  void *p = (void *)(ch+1);
  for (uns i=1; i<pool->elts_per_chunk; i++)
    {
      struct eltpool_free *f = p;
      f->next = pool->first_free;
      pool->first_free = f;
      p += pool->elt_size;
    }
  ch->next = pool->first_chunk;
  pool->first_chunk = ch;
  pool->num_chunks++;
  return p;
}

u64
ep_total_size(struct eltpool *pool)
{
  return (u64)pool->num_chunks * pool->chunk_size + sizeof(*pool);
}

#ifdef TEST

#include <stdio.h>
#include "ucw/clists.h"

struct argh {
  cnode n;
  byte x[1];
} PACKED;

int main(void)
{
  struct eltpool *ep = ep_new(sizeof(struct argh), 64);
  clist l;
  clist_init(&l);
  for (uns i=0; i<65536; i++)
    {
      struct argh *a = ep_alloc(ep);
      if (i % 3)
	clist_add_tail(&l, &a->n);
      else
	clist_add_head(&l, &a->n);
      if (!(i % 5))
	{
	  a = clist_head(&l);
	  clist_remove(&a->n);
	  ep_free(ep, a);
	}
    }
  ep_delete(ep);
  puts("OK");
  return 0;
}

#endif
