/*
 *	UCW Library -- Optimized Array Sorter
 *
 *	(c) 2003--2007 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

/*
 *  This is a generator of routines for sorting huge arrays, similar to the one
 *  in ucw/sorter/array-simple.h. It cannot handle discontiguous arrays, but it is able
 *  to employ radix-sorting if a monotone hash function is available and also
 *  use several threads in parallel on SMP systems (this assumes that all
 *  callbacks you provide are thread-safe).
 *
 *  It is usually called internally by the generic shorter machinery, but
 *  you are free to use it explicitly if you need.
 *
 *  So much for advocacy, there are the parameters (those marked with [*]
 *  are mandatory):
 *
 *  ASORT_PREFIX(x) [*]	add a name prefix (used on all global names
 *			defined by the sorter)
 *  ASORT_KEY_TYPE  [*]	data type of a single array entry key
 *  ASORT_LT(x,y)	x < y for ASORT_KEY_TYPE (default: "x<y")
 *  ASORT_HASH(x)	a monotone hash function (safisfying hash(x) < hash(y) => x<y)
 *  ASORT_LONG_HASH	hashes are 64-bit numbers (default is 32 bits)
 *
 *  Fine-tuning parameters: (if you really insist)
 *
 *  ASORT_THRESHOLD	threshold for switching between quicksort and insertsort
 *  ASORT_RADIX_BITS	how many bits of the hash functions are to be used at once for
 *			radix-sorting.
 *
 *  After including this file, a function
 * 	ASORT_KEY_TYPE *ASORT_PREFIX(sort)(ASORT_KEY_TYPE *array, uns num_elts [, ASORT_KEY_TYPE *buf, uns hash_bits])
 *  is declared and all parameter macros are automatically undef'd. Here `buf' is an
 *  auxiliary buffer of the same size as the input array, required whenever radix
 *  sorting should be used, and `hash_bits' is the number of significant bits returned
 *  by the hash function. If the buffer is specified, the sorting function returns either
 *  a pointer to the input array or to the buffer, depending on where the result is stored.
 *  If you do not use hashing, these parameters should be omitted.
 */

#include "ucw/sorter/common.h"

#define Q(x) ASORT_PREFIX(x)

typedef ASORT_KEY_TYPE Q(key);

#ifndef ASORT_LT
#define ASORT_LT(x,y) ((x) < (y))
#endif

#ifndef ASORT_SWAP
#define ASORT_SWAP(i,j) do { Q(key) tmp = array[i]; array[i]=array[j]; array[j]=tmp; } while (0)
#endif

#ifndef ASORT_THRESHOLD
#define ASORT_THRESHOLD 8		/* Guesswork and experimentation */
#endif

#ifndef ASORT_RADIX_BITS
#define ASORT_RADIX_BITS CONFIG_UCW_RADIX_SORTER_BITS
#endif
#define ASORT_RADIX_MASK ((1 << (ASORT_RADIX_BITS)) - 1)

/* QuickSort with optimizations a'la Sedgewick, inspired by qsort() from GNU libc. */

static void Q(quicksort)(void *array_ptr, uns num_elts)
{
  Q(key) *array = array_ptr;
  struct stk { int l, r; } stack[8*sizeof(uns)];
  int l, r, left, right, m;
  uns sp = 0;
  Q(key) pivot;

  if (num_elts <= 1)
    return;

  left = 0;
  right = num_elts - 1;
  for(;;)
    {
      l = left;
      r = right;
      m = (l+r)/2;
      if (ASORT_LT(array[m], array[l]))
	ASORT_SWAP(l,m);
      if (ASORT_LT(array[r], array[m]))
	{
	  ASORT_SWAP(m,r);
	  if (ASORT_LT(array[m], array[l]))
	    ASORT_SWAP(l,m);
	}
      pivot = array[m];
      do
	{
	  while (ASORT_LT(array[l], pivot))
	    l++;
	  while (ASORT_LT(pivot, array[r]))
	    r--;
	  if (l < r)
	    {
	      ASORT_SWAP(l,r);
	      l++;
	      r--;
	    }
	  else if (l == r)
	    {
	      l++;
	      r--;
	    }
	}
      while (l <= r);
      if ((r - left) >= ASORT_THRESHOLD && (right - l) >= ASORT_THRESHOLD)
	{
	  /* Both partitions ok => push the larger one */
	  if ((r - left) > (right - l))
	    {
	      stack[sp].l = left;
	      stack[sp].r = r;
	      left = l;
	    }
	  else
	    {
	      stack[sp].l = l;
	      stack[sp].r = right;
	      right = r;
	    }
	  sp++;
	}
      else if ((r - left) >= ASORT_THRESHOLD)
	{
	  /* Left partition OK, right undersize */
	  right = r;
	}
      else if ((right - l) >= ASORT_THRESHOLD)
	{
	  /* Right partition OK, left undersize */
	  left = l;
	}
      else
	{
	  /* Both partitions undersize => pop */
	  if (!sp)
	    break;
	  sp--;
	  left = stack[sp].l;
	  right = stack[sp].r;
	}
    }

  /*
   * We have a partially sorted array, finish by insertsort. Inspired
   * by qsort() in GNU libc.
   */

  /* Find minimal element which will serve as a barrier */
  r = MIN(num_elts, ASORT_THRESHOLD);
  m = 0;
  for (l=1; l<r; l++)
    if (ASORT_LT(array[l], array[m]))
      m = l;
  ASORT_SWAP(0,m);

  /* Insertion sort */
  for (m=1; m<(int)num_elts; m++)
    {
      l=m;
      while (ASORT_LT(array[m], array[l-1]))
	l--;
      while (l < m)
	{
	  ASORT_SWAP(l,m);
	  l++;
	}
    }
}

/* Just the splitting part of QuickSort */

static void Q(quicksplit)(void *array_ptr, uns num_elts, int *leftp, int *rightp)
{
  Q(key) *array = array_ptr;
  int l, r, m;
  Q(key) pivot;

  l = 0;
  r = num_elts - 1;
  m = (l+r)/2;
  if (ASORT_LT(array[m], array[l]))
    ASORT_SWAP(l,m);
  if (ASORT_LT(array[r], array[m]))
    {
      ASORT_SWAP(m,r);
      if (ASORT_LT(array[m], array[l]))
	ASORT_SWAP(l,m);
    }
  pivot = array[m];
  do
    {
      while (ASORT_LT(array[l], pivot))
	l++;
      while (ASORT_LT(pivot, array[r]))
	r--;
      if (l < r)
	{
	  ASORT_SWAP(l,r);
	  l++;
	  r--;
	}
      else if (l == r)
	{
	  l++;
	  r--;
	}
    }
  while (l <= r);
  *leftp = l;
  *rightp = r;
}

#ifdef ASORT_HASH

static void Q(radix_count)(void *src_ptr, uns num_elts, uns *cnt, uns shift)
{
  Q(key) *src = src_ptr;
  uns i;

  switch (shift)
    {
#define RC(s) \
    case s:								\
      for (i=0; i<num_elts; i++)					\
	cnt[ (ASORT_HASH(src[i]) >> s) & ASORT_RADIX_MASK ] ++;		\
      break;								\

#ifdef ASORT_LONG_HASH
      RC(63); RC(62); RC(61); RC(60); RC(59); RC(58); RC(57); RC(56);
      RC(55); RC(54); RC(53); RC(52); RC(51); RC(50); RC(49); RC(48);
      RC(47); RC(46); RC(45); RC(44); RC(43); RC(42); RC(41); RC(40);
      RC(39); RC(38); RC(37); RC(36); RC(35); RC(34); RC(33); RC(32);
#endif
      RC(31); RC(30); RC(29); RC(28); RC(27); RC(26); RC(25); RC(24);
      RC(23); RC(22); RC(21); RC(20); RC(19); RC(18); RC(17); RC(16);
      RC(15); RC(14); RC(13); RC(12); RC(11); RC(10); RC(9); RC(8);
      RC(7); RC(6); RC(5); RC(4); RC(3); RC(2); RC(1); RC(0);
    default:
      ASSERT(0);
    }
#undef RC
}

static void Q(radix_split)(void *src_ptr, void *dest_ptr, uns num_elts, uns *ptrs, uns shift)
{
  Q(key) *src = src_ptr, *dest = dest_ptr;
  uns i;

  switch (shift)
    {
#define RS(s) \
    case s:										\
      for (i=0; i<num_elts; i++)							\
	dest[ ptrs[ (ASORT_HASH(src[i]) >> s) & ASORT_RADIX_MASK ]++ ] = src[i];	\
      break;

#ifdef ASORT_LONG_HASH
      RS(63); RS(62); RS(61); RS(60); RS(59); RS(58); RS(57); RS(56);
      RS(55); RS(54); RS(53); RS(52); RS(51); RS(50); RS(49); RS(48);
      RS(47); RS(46); RS(45); RS(44); RS(43); RS(42); RS(41); RS(40);
      RS(39); RS(38); RS(37); RS(36); RS(35); RS(34); RS(33); RS(32);
#endif
      RS(31); RS(30); RS(29); RS(28); RS(27); RS(26); RS(25); RS(24);
      RS(23); RS(22); RS(21); RS(20); RS(19); RS(18); RS(17); RS(16);
      RS(15); RS(14); RS(13); RS(12); RS(11); RS(10); RS(9); RS(8);
      RS(7); RS(6); RS(5); RS(4); RS(3); RS(2); RS(1); RS(0);
    default:
      ASSERT(0);
    }
#undef RS
}

#endif

#ifdef ASORT_HASH
#define ASORT_HASH_ARGS , Q(key) *buffer, uns hash_bits
#else
#define ASORT_HASH_ARGS
#endif

/**
 * The generated function. The @array is the data to be sorted, @num_elts tells
 * how many elements the array has.  If you did not provide `ASORT_HASH`, then
 * the `ASORT_HASH_ARGS` is empty (there are only the two parameters in that
 * case).  When you provide it, the function gains two more parameters in the
 * `ASORT_HASH_ARGS` macro. They are `ASORT_KEY_TYPE *@buffer`, which must be a
 * memory buffer of the same size as the input array, and `uns @hash_bits`,
 * specifying how many significant bits the hash function returns.
 *
 * The function returns pointer to the sorted data, either the @array or the
 * @buffer argument.
 **/
static ASORT_KEY_TYPE *ASORT_PREFIX(sort)(ASORT_KEY_TYPE *array, uns num_elts ASORT_HASH_ARGS)
{
  struct asort_context ctx = {
    .array = array,
    .num_elts = num_elts,
    .elt_size = sizeof(Q(key)),
    .quicksort = Q(quicksort),
    .quicksplit = Q(quicksplit),
#ifdef ASORT_HASH
    .buffer = buffer,
    .hash_bits = hash_bits,
    .radix_count = Q(radix_count),
    .radix_split = Q(radix_split),
    .radix_bits = ASORT_RADIX_BITS,
#endif
  };
  asort_run(&ctx);
  return ctx.array;
}

#undef ASORT_HASH
#undef ASORT_KEY_TYPE
#undef ASORT_LONG_HASH
#undef ASORT_LT
#undef ASORT_PAGE_ALIGNED
#undef ASORT_PREFIX
#undef ASORT_RADIX_BITS
#undef ASORT_RADIX_MASK
#undef ASORT_SWAP
#undef ASORT_THRESHOLD
#undef ASORT_HASH_ARGS
#undef Q
