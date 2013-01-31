/*
 *	UCW Library -- Prefetch
 *
 *	(c) 1997--2006 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _UCW_PREFETCH_H
#define _UCW_PREFETCH_H

#if defined(__k6)
  /* K6 doesn't have prefetches */

#elif defined(__athlon) || defined(__k8) || \
      defined(__i686) || \
      defined(__pentium4) || defined(__prescott) || defined(__nocona)

#define HAVE_PREFETCH
static inline void prefetch(void *addr)
{
  asm volatile ("prefetcht0 %0" : : "m" (*(byte*)addr));
}

#else
#warning "Don't know how to prefetch on your CPU. Please fix ucw/prefetch.h."
#endif

#ifndef HAVE_PREFETCH
static inline void prefetch(void *addr UNUSED)
{
}
#endif

#endif
