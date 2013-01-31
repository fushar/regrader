/*
 *	UCW Library -- Memory Pools (One-Time Allocation)
 *
 *	(c) 1997--2001 Martin Mares <mj@ucw.cz>
 *	(c) 2007 Pavel Charvat <pchar@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#undef LOCAL_DEBUG

#include "ucw/lib.h"
#include "ucw/mempool.h"

#include <string.h>

#define MP_CHUNK_TAIL ALIGN_TO(sizeof(struct mempool_chunk), CPU_STRUCT_ALIGN)
#define MP_SIZE_MAX (~0U - MP_CHUNK_TAIL - CPU_PAGE_SIZE)

struct mempool_chunk {
  struct mempool_chunk *next;
  uns size;
};

static uns
mp_align_size(uns size)
{
#ifdef CONFIG_UCW_POOL_IS_MMAP
  return ALIGN_TO(size + MP_CHUNK_TAIL, CPU_PAGE_SIZE) - MP_CHUNK_TAIL;
#else
  return ALIGN_TO(size, CPU_STRUCT_ALIGN);
#endif
}

void
mp_init(struct mempool *pool, uns chunk_size)
{
  chunk_size = mp_align_size(MAX(sizeof(struct mempool), chunk_size));
  *pool = (struct mempool) {
    .chunk_size = chunk_size,
    .threshold = chunk_size >> 1,
    .last_big = &pool->last_big };
}

static void *
mp_new_big_chunk(uns size)
{
  struct mempool_chunk *chunk;
  chunk = xmalloc(size + MP_CHUNK_TAIL) + size;
  chunk->size = size;
  return chunk;
}

static void
mp_free_big_chunk(struct mempool_chunk *chunk)
{
  xfree((void *)chunk - chunk->size);
}

static void *
mp_new_chunk(uns size)
{
#ifdef CONFIG_UCW_POOL_IS_MMAP
  struct mempool_chunk *chunk;
  chunk = page_alloc(size + MP_CHUNK_TAIL) + size;
  chunk->size = size;
  return chunk;
#else
  return mp_new_big_chunk(size);
#endif
}

static void
mp_free_chunk(struct mempool_chunk *chunk)
{
#ifdef CONFIG_UCW_POOL_IS_MMAP
  page_free((void *)chunk - chunk->size, chunk->size + MP_CHUNK_TAIL);
#else
  mp_free_big_chunk(chunk);
#endif
}

struct mempool *
mp_new(uns chunk_size)
{
  chunk_size = mp_align_size(MAX(sizeof(struct mempool), chunk_size));
  struct mempool_chunk *chunk = mp_new_chunk(chunk_size);
  struct mempool *pool = (void *)chunk - chunk_size;
  DBG("Creating mempool %p with %u bytes long chunks", pool, chunk_size);
  chunk->next = NULL;
  *pool = (struct mempool) {
    .state = { .free = { chunk_size - sizeof(*pool) }, .last = { chunk } },
    .chunk_size = chunk_size,
    .threshold = chunk_size >> 1,
    .last_big = &pool->last_big };
  return pool;
}

static void
mp_free_chain(struct mempool_chunk *chunk)
{
  while (chunk)
    {
      struct mempool_chunk *next = chunk->next;
      mp_free_chunk(chunk);
      chunk = next;
    }
}

static void
mp_free_big_chain(struct mempool_chunk *chunk)
{
  while (chunk)
    {
      struct mempool_chunk *next = chunk->next;
      mp_free_big_chunk(chunk);
      chunk = next;
    }
}

void
mp_delete(struct mempool *pool)
{
  DBG("Deleting mempool %p", pool);
  mp_free_big_chain(pool->state.last[1]);
  mp_free_chain(pool->unused);
  mp_free_chain(pool->state.last[0]); // can contain the mempool structure
}

void
mp_flush(struct mempool *pool)
{
  mp_free_big_chain(pool->state.last[1]);
  struct mempool_chunk *chunk, *next;
  for (chunk = pool->state.last[0]; chunk && (void *)chunk - chunk->size != pool; chunk = next)
    {
      next = chunk->next;
      chunk->next = pool->unused;
      pool->unused = chunk;
    }
  pool->state.last[0] = chunk;
  pool->state.free[0] = chunk ? chunk->size - sizeof(*pool) : 0;
  pool->state.last[1] = NULL;
  pool->state.free[1] = 0;
  pool->state.next = NULL;
  pool->last_big = &pool->last_big;
}

static void
mp_stats_chain(struct mempool_chunk *chunk, struct mempool_stats *stats, uns idx)
{
  while (chunk)
    {
      stats->chain_size[idx] += chunk->size + sizeof(*chunk);
      stats->chain_count[idx]++;
      chunk = chunk->next;
    }
  stats->total_size += stats->chain_size[idx];
}

void
mp_stats(struct mempool *pool, struct mempool_stats *stats)
{
  bzero(stats, sizeof(*stats));
  mp_stats_chain(pool->state.last[0], stats, 0);
  mp_stats_chain(pool->state.last[1], stats, 1);
  mp_stats_chain(pool->unused, stats, 2);
}

u64
mp_total_size(struct mempool *pool)
{
  struct mempool_stats stats;
  mp_stats(pool, &stats);
  return stats.total_size;
}

void *
mp_alloc_internal(struct mempool *pool, uns size)
{
  struct mempool_chunk *chunk;
  if (size <= pool->threshold)
    {
      pool->idx = 0;
      if (pool->unused)
        {
	  chunk = pool->unused;
	  pool->unused = chunk->next;
	}
      else
	chunk = mp_new_chunk(pool->chunk_size);
      chunk->next = pool->state.last[0];
      pool->state.last[0] = chunk;
      pool->state.free[0] = pool->chunk_size - size;
      return (void *)chunk - pool->chunk_size;
    }
  else if (likely(size <= MP_SIZE_MAX))
    {
      pool->idx = 1;
      uns aligned = ALIGN_TO(size, CPU_STRUCT_ALIGN);
      chunk = mp_new_big_chunk(aligned);
      chunk->next = pool->state.last[1];
      pool->state.last[1] = chunk;
      pool->state.free[1] = aligned - size;
      return pool->last_big = (void *)chunk - aligned;
    }
  else
    die("Cannot allocate %u bytes from a mempool", size);
}

void *
mp_alloc(struct mempool *pool, uns size)
{
  return mp_alloc_fast(pool, size);
}

void *
mp_alloc_noalign(struct mempool *pool, uns size)
{
  return mp_alloc_fast_noalign(pool, size);
}

void *
mp_alloc_zero(struct mempool *pool, uns size)
{
  void *ptr = mp_alloc_fast(pool, size);
  bzero(ptr, size);
  return ptr;
}

void *
mp_start_internal(struct mempool *pool, uns size)
{
  void *ptr = mp_alloc_internal(pool, size);
  pool->state.free[pool->idx] += size;
  return ptr;
}

void *
mp_start(struct mempool *pool, uns size)
{
  return mp_start_fast(pool, size);
}

void *
mp_start_noalign(struct mempool *pool, uns size)
{
  return mp_start_fast_noalign(pool, size);
}

void *
mp_grow_internal(struct mempool *pool, uns size)
{
  if (unlikely(size > MP_SIZE_MAX))
    die("Cannot allocate %u bytes of memory", size);
  uns avail = mp_avail(pool);
  void *ptr = mp_ptr(pool);
  if (pool->idx)
    {
      uns amortized = likely(avail <= MP_SIZE_MAX / 2) ? avail * 2 : MP_SIZE_MAX;
      amortized = MAX(amortized, size);
      amortized = ALIGN_TO(amortized, CPU_STRUCT_ALIGN);
      struct mempool_chunk *chunk = pool->state.last[1], *next = chunk->next;
      ptr = xrealloc(ptr, amortized + MP_CHUNK_TAIL);
      chunk = ptr + amortized;
      chunk->next = next;
      chunk->size = amortized;
      pool->state.last[1] = chunk;
      pool->state.free[1] = amortized;
      pool->last_big = ptr;
      return ptr;
    }
  else
    {
      void *p = mp_start_internal(pool, size);
      memcpy(p, ptr, avail);
      return p;
    }
}

uns
mp_open(struct mempool *pool, void *ptr)
{
  return mp_open_fast(pool, ptr);
}

void *
mp_realloc(struct mempool *pool, void *ptr, uns size)
{
  return mp_realloc_fast(pool, ptr, size);
}

void *
mp_realloc_zero(struct mempool *pool, void *ptr, uns size)
{
  uns old_size = mp_open_fast(pool, ptr);
  ptr = mp_grow(pool, size);
  if (size > old_size)
    bzero(ptr + old_size, size - old_size);
  mp_end(pool, ptr + size);
  return ptr;
}

void *
mp_spread_internal(struct mempool *pool, void *p, uns size)
{
  void *old = mp_ptr(pool);
  void *new = mp_grow_internal(pool, p-old+size);
  return p-old+new;
}

void
mp_restore(struct mempool *pool, struct mempool_state *state)
{
  struct mempool_chunk *chunk, *next;
  struct mempool_state s = *state;
  for (chunk = pool->state.last[0]; chunk != s.last[0]; chunk = next)
    {
      next = chunk->next;
      chunk->next = pool->unused;
      pool->unused = chunk;
    }
  for (chunk = pool->state.last[1]; chunk != s.last[1]; chunk = next)
    {
      next = chunk->next;
      mp_free_big_chunk(chunk);
    }
  pool->state = s;
  pool->last_big = &pool->last_big;
}

struct mempool_state *
mp_push(struct mempool *pool)
{
  struct mempool_state state = pool->state;
  struct mempool_state *p = mp_alloc_fast(pool, sizeof(*p));
  *p = state;
  pool->state.next = p;
  return p;
}

void
mp_pop(struct mempool *pool)
{
  ASSERT(pool->state.next);
  struct mempool_state state = pool->state;
  mp_restore(pool, &state);
}

#ifdef TEST

#include "ucw/getopt.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static void
fill(byte *ptr, uns len, uns magic)
{
  while (len--)
    *ptr++ = (magic++ & 255);
}

static void
check(byte *ptr, uns len, uns magic, uns align)
{
  ASSERT(!((uintptr_t)ptr & (align - 1)));
  while (len--)
    if (*ptr++ != (magic++ & 255))
      ASSERT(0);
}

int main(int argc, char **argv)
{
  srand(time(NULL));
  log_init(argv[0]);
  cf_def_file = NULL;
  if (cf_getopt(argc, argv, CF_SHORT_OPTS, CF_NO_LONG_OPTS, NULL) >= 0 || argc != optind)
    die("Invalid usage");

  uns max = 1000, n = 0, m = 0, can_realloc = 0;
  void *ptr[max];
  struct mempool_state *state[max];
  uns len[max], num[max], align[max];
  struct mempool *mp = mp_new(128), mp_static;

  for (uns i = 0; i < 5000; i++)
    {
      for (uns j = 0; j < n; j++)
	check(ptr[j], len[j], j, align[j]);
#if 0
      DBG("free_small=%u free_big=%u idx=%u chunk_size=%u last_big=%p", mp->state.free[0], mp->state.free[1], mp->idx, mp->chunk_size, mp->last_big);
      for (struct mempool_chunk *ch = mp->state.last[0]; ch; ch = ch->next)
	DBG("small %p %p %p %d", (byte *)ch - ch->size, ch, ch + 1, ch->size);
      for (struct mempool_chunk *ch = mp->state.last[1]; ch; ch = ch->next)
	DBG("big %p %p %p %d", (byte *)ch - ch->size, ch, ch + 1, ch->size);
#endif
      int r = random_max(100);
      if ((r -= 1) < 0)
        {
	  DBG("flush");
	  mp_flush(mp);
	  n = m = 0;
	}
      else if ((r -= 1) < 0)
        {
	  DBG("delete & new");
	  mp_delete(mp);
	  if (random_max(2))
	    mp = mp_new(random_max(0x1000) + 1);
	  else
	    mp = &mp_static, mp_init(mp, random_max(512) + 1);
	  n = m = 0;
	}
      else if (n < max && (r -= 30) < 0)
        {
	  len[n] = random_max(0x2000);
	  DBG("alloc(%u)", len[n]);
	  align[n] = random_max(2) ? CPU_STRUCT_ALIGN : 1;
	  ptr[n] = (align[n] == 1) ? mp_alloc_fast_noalign(mp, len[n]) : mp_alloc_fast(mp, len[n]);
	  DBG(" -> (%p)", ptr[n]);
	  fill(ptr[n], len[n], n);
	  n++;
	  can_realloc = 1;
	}
      else if (n < max && (r -= 20) < 0)
        {
	  len[n] = random_max(0x2000);
	  DBG("start(%u)", len[n]);
	  align[n] = random_max(2) ? CPU_STRUCT_ALIGN : 1;
	  ptr[n] = (align[n] == 1) ? mp_start_fast_noalign(mp, len[n]) : mp_start_fast(mp, len[n]);
	  DBG(" -> (%p)", ptr[n]);
	  fill(ptr[n], len[n], n);
	  n++;
	  can_realloc = 1;
	  goto grow;
	}
      else if (can_realloc && n && (r -= 10) < 0)
        {
	  if (mp_open(mp, ptr[n - 1]) != len[n - 1])
	    ASSERT(0);
grow:
	  {
	    uns k = n - 1;
	    for (uns i = random_max(4); i--; )
	      {
	        uns l = len[k];
	        len[k] = random_max(0x2000);
	        DBG("grow(%u)", len[k]);
	        ptr[k] = mp_grow(mp, len[k]);
	        DBG(" -> (%p)", ptr[k]);
	        check(ptr[k], MIN(l, len[k]), k, align[k]);
	        fill(ptr[k], len[k], k);
	      }
	    mp_end(mp, ptr[k] + len[k]);
	  }
	}
      else if (can_realloc && n && (r -= 20) < 0)
        {
	  uns i = n - 1, l = len[i];
	  DBG("realloc(%p, %u)", ptr[i], len[i]);
	  ptr[i] = mp_realloc(mp, ptr[i], len[i] = random_max(0x2000));
	  DBG(" -> (%p, %u)", ptr[i], len[i]);
	  check(ptr[i],  MIN(len[i], l), i, align[i]);
	  fill(ptr[i], len[i], i);
	}
      else if (m < max && (r -= 5) < 0)
        {
	  DBG("push(%u)", m);
	  num[m] = n;
	  state[m++] = mp_push(mp);
	  can_realloc = 0;
	}
      else if (m && (r -= 2) < 0)
        {
	  m--;
	  DBG("pop(%u)", m);
	  mp_pop(mp);
	  n = num[m];
	  can_realloc = 0;
	}
      else if (m && (r -= 1) < 0)
        {
	  uns i = random_max(m);
	  DBG("restore(%u)", i);
	  mp_restore(mp, state[i]);
	  n = num[m = i];
	  can_realloc = 0;
	}
      else if (can_realloc && n && (r -= 5) < 0)
        ASSERT(mp_size(mp, ptr[n - 1]) == len[n - 1]);
    }

  mp_delete(mp);
  return 0;
}

#endif
