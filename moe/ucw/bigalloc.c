/*
 *	UCW Library -- Allocation of Large Aligned Buffers
 *
 *	(c) 2006--2007 Martin Mares <mj@ucw.cz>
 *	(c) 2007 Pavel Charvat <char@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"

#include <sys/mman.h>
#include <string.h>
#include <limits.h>

void *
page_alloc(u64 len)
{
  if (!len)
    return NULL;
  if (len > SIZE_MAX)
    die("page_alloc: Size %llu is too large for the current architecture", (long long) len);
  ASSERT(!(len & (CPU_PAGE_SIZE-1)));
  byte *p = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
  if (p == (byte*) MAP_FAILED)
    die("Cannot mmap %llu bytes of memory: %m", (long long)len);
  return p;
}

void *
page_alloc_zero(u64 len)
{
  void *p = page_alloc(len);
  bzero(p, len);
  return p;
}

void
page_free(void *start, u64 len)
{
  ASSERT(!(len & (CPU_PAGE_SIZE-1)));
  ASSERT(!((uintptr_t) start & (CPU_PAGE_SIZE-1)));
  munmap(start, len);
}

void *
page_realloc(void *start, u64 old_len, u64 new_len)
{
  void *p = page_alloc(new_len);
  memcpy(p, start, MIN(old_len, new_len));
  page_free(start, old_len);
  return p;
}

static u64
big_round(u64 len)
{
  return ALIGN_TO(len, (u64)CPU_PAGE_SIZE);
}

void *
big_alloc(u64 len)
{
  u64 l = big_round(len);
  if (l > SIZE_MAX - 2*CPU_PAGE_SIZE)
    die("big_alloc: Size %llu is too large for the current architecture", (long long) len);
#ifdef CONFIG_DEBUG
  l += 2*CPU_PAGE_SIZE;
#endif
  byte *p = page_alloc(l);
#ifdef CONFIG_DEBUG
  *(u64*)p = len;
  mprotect(p, CPU_PAGE_SIZE, PROT_NONE);
  mprotect(p+l-CPU_PAGE_SIZE, CPU_PAGE_SIZE, PROT_NONE);
  p += CPU_PAGE_SIZE;
#endif
  return p;
}

void *
big_alloc_zero(u64 len)
{
  void *p = big_alloc(len);
  bzero(p, big_round(len));
  return p;
}

void
big_free(void *start, u64 len)
{
  byte *p = start;
  u64 l = big_round(len);
#ifdef CONFIG_DEBUG
  p -= CPU_PAGE_SIZE;
  mprotect(p, CPU_PAGE_SIZE, PROT_READ);
  ASSERT(*(u64*)p == len);
  l += 2*CPU_PAGE_SIZE;
#endif
  page_free(p, l);
}

#ifdef TEST

int main(void)
{
  byte *p = big_alloc(123456);
  // p[-1] = 1;
  big_free(p, 123456);
  return 0;
}

#endif
