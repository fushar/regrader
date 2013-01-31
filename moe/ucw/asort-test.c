/*
 *	UCW Library -- Universal Array Sorter Test and Benchmark
 *
 *	(c) 2003 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"

#include <stdlib.h>
#include <stdio.h>

#define N 4000037			/* a prime */

struct elt {
  u32 key;
  u32 x, y;
};

static struct elt array[N];

#define ASORT_KEY_TYPE u32
#define ASORT_ELT(i) array[i].key
#define ASORT_SWAP(i,j) do { struct elt e=array[j]; array[j]=array[i]; array[i]=e; } while(0)

static void generate(void)
{
  uns i;
  for (i=0; i<N; i++)
#if 0
    ASORT_ELT(i) = N-i-1;
#elif 0
    ASORT_ELT(i) = i;
#else
    ASORT_ELT(i) = (i ? ASORT_ELT(i-1)+1944833754 : 3141592) % N;
#endif
}

static int errors = 0;

static void check(void)
{
  uns i;
  for (i=0; i<N; i++)
    if (ASORT_ELT(i) != i)
    {
      printf("error at pos %d: %08x != %08x\n", i, ASORT_ELT(i), i);
      errors = 1;
    }
}

static int qs_comp(const struct elt *X, const struct elt *Y)
{
  if (X->key < Y->key)
    return -1;
  else if (X->key > Y->key)
    return 1;
  else
    return 0;
}

#define ASORT_PREFIX(x) as_##x
#include "ucw/sorter/array-simple.h"

int main(void)
{
  timestamp_t timer;

  generate();
  init_timer(&timer);
  qsort(array, N, sizeof(array[0]), (int (*)(const void *, const void *)) qs_comp);
  printf("qsort: %d ms\n", get_timer(&timer));
  check();
  generate();
  init_timer(&timer);
  as_sort(N);
  printf("asort: %d ms\n", get_timer(&timer));
  check();
  return errors;
}
