/*
 *	UCW Library -- Universal Sorter
 *
 *	(c) 2001--2007 Martin Mares <mj@ucw.cz>
 *	(c) 2004 Robert Spalek <robert@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

/*
 *  This is not a normal header file, but a generator of sorting
 *  routines.  Each time you include it with parameters set in the
 *  corresponding preprocessor macros, it generates a file sorter
 *  with the parameters given.
 *
 *  The sorter operates on fastbufs containing sequences of items. Each item
 *  consists of a key, optionally followed by data. The keys are represented
 *  by fixed-size structures of type SORT_KEY internally, if this format differs
 *  from the on-disk format, explicit reading and writing routines can be provided.
 *  The data are always copied verbatim, unless the sorter is in the merging
 *  mode in which it calls callbacks for merging of items with equal keys.
 *
 *  All callbacks must be thread-safe.
 *
 *  Basic parameters and callbacks:
 *
 *  SORT_PREFIX(x)      add a name prefix (used on all global names defined by the sorter)
 *
 *  SORT_KEY		data type capable of holding a single key in memory (the on-disk
 *			representation can be different). Alternatively, you can use:
 *  SORT_KEY_REGULAR	data type holding a single key both in memory and on disk;
 *			in this case, bread() and bwrite() is used to read/write keys
 *			and it's also assumed that the keys are not very long.
 *  int PREFIX_compare(SORT_KEY *a, SORT_KEY *b)
 *			compares two keys, returns result like strcmp(). Mandatory.
 *  int PREFIX_read_key(struct fastbuf *f, SORT_KEY *k)
 *			reads a key from a fastbuf, returns nonzero=ok, 0=EOF.
 *			Mandatory unless SORT_KEY_REGULAR is defined.
 *  void PREFIX_write_key(struct fastbuf *f, SORT_KEY *k)
 *			writes a key to a fastbuf. Mandatory unless SORT_KEY_REGULAR.
 *
 *  SORT_KEY_SIZE(key)	returns the real size of a key (a SORT_KEY type in memory
 *			can be truncated to this number of bytes without any harm;
 *			used to save memory when the keys have variable sizes).
 *			Default: always store the whole SORT_KEY.
 *  SORT_DATA_SIZE(key)	gets a key and returns the amount of data following it.
 *			Default: records consist of keys only.
 *
 *  Integer sorting:
 *
 *  SORT_INT(key)	we are sorting by an integer value returned by this macro.
 *			In this mode, PREFIX_compare is supplied automatically and the sorting
 *			function gets an extra parameter specifying the range of the integers.
 *			The better the range fits, the faster we sort.
 *			Sets up SORT_HASH_xxx automatically.
 *  SORT_INT64(key)	the same for 64-bit integers.
 *
 *  Hashing (optional, but it can speed sorting up):
 *
 *  SORT_HASH_BITS	signals that a monotone hashing function returning a given number of
 *			bits is available. A monotone hash is a function f from keys to integers
 *			such that f(x) < f(y) implies x < y, which is approximately uniformly
 *			distributed. It should be declared as:
 *  uns PREFIX_hash(SORT_KEY *a)
 *
 *  Unification:
 *
 *  SORT_UNIFY		merge items with identical keys. It requires the following functions:
 *  void PREFIX_write_merged(struct fastbuf *f, SORT_KEY **keys, void **data, uns n, void *buf)
 *			takes n records in memory with keys which compare equal and writes
 *			a single record to the given fastbuf. `buf' points to a buffer which
 *			is guaranteed to hold the sum of workspace requirements (see below)
 *			over all given records. The function is allowed to modify all its inputs.
 *  void PREFIX_copy_merged(SORT_KEY **keys, struct fastbuf **data, uns n, struct fastbuf *dest)
 *			takes n records with keys in memory and data in fastbufs and writes
 *			a single record. Used only if SORT_DATA_SIZE or SORT_UNIFY_WORKSPACE
 *			is defined.
 *  SORT_UNIFY_WORKSPACE(key)
 *			gets a key and returns the amount of workspace required when merging
 *			the given record. Defaults to 0.
 *
 *  Input (choose one of these):
 *
 *  SORT_INPUT_FILE	file of a given name
 *  SORT_INPUT_FB	seekable fastbuf stream
 *  SORT_INPUT_PIPE	non-seekable fastbuf stream
 *  SORT_INPUT_PRESORT	custom presorter. Calls function
 *  int PREFIX_presort(struct fastbuf *dest, void *buf, size_t bufsize)
 *			to get successive batches of pre-sorted data.
 *			The function is passed a page-aligned presorting buffer.
 *			It returns 1 on success or 0 on EOF.
 *  SORT_DELETE_INPUT	A C expression, if true, then the input files are deleted
 *			as soon as possible.
 *
 *  Output (chose one of these):
 *
 *  SORT_OUTPUT_FILE	file of a given name
 *  SORT_OUTPUT_FB	temporary fastbuf stream
 *  SORT_OUTPUT_THIS_FB	a given fastbuf stream which can already contain some data
 *
 *  Other switches:
 *
 *  SORT_UNIQUE		all items have distinct keys (checked in debug mode)
 *
 *  The function generated:
 *
 *  <outfb> PREFIX_sort(<in>, <out> [,<range>]), where:
 *			<in> = input file name/fastbuf or NULL
 *			<out> = output file name/fastbuf or NULL
 *			<range> = maximum integer value for the SORT_INT mode
 *
 *  After including this file, all parameter macros are automatically
 *  undef'd.
 */

#include "ucw/sorter/common.h"
#include "ucw/fastbuf.h"

#include <fcntl.h>

#define P(x) SORT_PREFIX(x)

#ifdef SORT_KEY_REGULAR
typedef SORT_KEY_REGULAR P(key);
static inline int P(read_key) (struct fastbuf *f, P(key) *k)
{
  return breadb(f, k, sizeof(P(key)));
}
static inline void P(write_key) (struct fastbuf *f, P(key) *k)
{
  bwrite(f, k, sizeof(P(key)));
}
#elif defined(SORT_KEY)
typedef SORT_KEY P(key);
#else
#error Missing definition of sorting key.
#endif

#ifdef SORT_INT64
typedef u64 P(hash_t);
#define SORT_INT SORT_INT64
#define SORT_LONG_HASH
#else
typedef uns P(hash_t);
#endif

#ifdef SORT_INT
static inline int P(compare) (P(key) *x, P(key) *y)
{
  if (SORT_INT(*x) < SORT_INT(*y))
    return -1;
  if (SORT_INT(*x) > SORT_INT(*y))
    return 1;
  return 0;
}

#ifndef SORT_HASH_BITS
static inline P(hash_t) P(hash) (P(key) *x)
{
  return SORT_INT((*x));
}
#endif
#endif

#ifdef SORT_UNIFY
#define LESS <
#else
#define LESS <=
#endif
#define SWAP(x,y,z) do { z=x; x=y; y=z; } while(0)

#if defined(SORT_UNIQUE) && defined(DEBUG_ASSERTS)
#define SORT_ASSERT_UNIQUE
#endif

#ifdef SORT_KEY_SIZE
#define SORT_VAR_KEY
#else
#define SORT_KEY_SIZE(key) sizeof(key)
#endif

#ifdef SORT_DATA_SIZE
#define SORT_VAR_DATA
#else
#define SORT_DATA_SIZE(key) 0
#endif

static inline void P(copy_data)(P(key) *key, struct fastbuf *in, struct fastbuf *out)
{
  P(write_key)(out, key);
#ifdef SORT_VAR_DATA
  bbcopy(in, out, SORT_DATA_SIZE(*key));
#else
  (void) in;
#endif
}

#if defined(SORT_UNIFY) && !defined(SORT_VAR_DATA) && !defined(SORT_UNIFY_WORKSPACE)
static inline void P(copy_merged)(P(key) **keys, struct fastbuf **data UNUSED, uns n, struct fastbuf *dest)
{
  P(write_merged)(dest, keys, NULL, n, NULL);
}
#endif

#if defined(SORT_HASH_BITS) || defined(SORT_INT)
#define SORT_INTERNAL_RADIX
#include "ucw/sorter/s-radix.h"
#endif

#if defined(SORT_VAR_KEY) || defined(SORT_VAR_DATA) || defined(SORT_UNIFY_WORKSPACE)
#include "ucw/sorter/s-internal.h"
#else
#include "ucw/sorter/s-fixint.h"
#endif

#include "ucw/sorter/s-twoway.h"
#include "ucw/sorter/s-multiway.h"

static struct fastbuf *P(sort)(
#ifdef SORT_INPUT_FILE
			       byte *in,
#else
			       struct fastbuf *in,
#endif
#ifdef SORT_OUTPUT_FILE
			       byte *out
#else
			       struct fastbuf *out
#endif
#ifdef SORT_INT
			       , u64 int_range
#endif
			       )
{
  struct sort_context ctx;
  bzero(&ctx, sizeof(ctx));

#ifdef SORT_INPUT_FILE
  ctx.in_fb = bopen_file(in, O_RDONLY, &sorter_fb_params);
  ctx.in_size = bfilesize(ctx.in_fb);
#elif defined(SORT_INPUT_FB)
  ctx.in_fb = in;
  ctx.in_size = bfilesize(in);
#elif defined(SORT_INPUT_PIPE)
  ctx.in_fb = in;
  ctx.in_size = ~(u64)0;
#elif defined(SORT_INPUT_PRESORT)
  ASSERT(!in);
  ctx.custom_presort = P(presort);
  ctx.in_size = ~(u64)0;
#else
#error No input given.
#endif
#ifdef SORT_DELETE_INPUT
  if (SORT_DELETE_INPUT)
    bconfig(ctx.in_fb, BCONFIG_IS_TEMP_FILE, 1);
#endif

#ifdef SORT_OUTPUT_FB
  ASSERT(!out);
#elif defined(SORT_OUTPUT_THIS_FB)
  ctx.out_fb = out;
#elif defined(SORT_OUTPUT_FILE)
  /* Just assume fastbuf output and rename the fastbuf later */
#else
#error No output given.
#endif

#ifdef SORT_HASH_BITS
  ctx.hash_bits = SORT_HASH_BITS;
  ctx.radix_split = P(radix_split);
#elif defined(SORT_INT)
  ctx.hash_bits = 0;
  while (ctx.hash_bits < 64 && (int_range >> ctx.hash_bits))
    ctx.hash_bits++;
  ctx.radix_split = P(radix_split);
#endif

  ctx.internal_sort = P(internal);
  ctx.internal_estimate = P(internal_estimate);
  ctx.twoway_merge = P(twoway_merge);
  ctx.multiway_merge = P(multiway_merge);

  sorter_run(&ctx);

#ifdef SORT_OUTPUT_FILE
  bfix_tmp_file(ctx.out_fb, out);
  ctx.out_fb = NULL;
#endif
  return ctx.out_fb;
}

#undef SORT_ASSERT_UNIQUE
#undef SORT_DATA_SIZE
#undef SORT_DELETE_INPUT
#undef SORT_HASH_BITS
#undef SORT_INPUT_FB
#undef SORT_INPUT_FILE
#undef SORT_INPUT_PIPE
#undef SORT_INPUT_PRESORT
#undef SORT_INT
#undef SORT_INT64
#undef SORT_INTERNAL_RADIX
#undef SORT_KEY
#undef SORT_KEY_REGULAR
#undef SORT_KEY_SIZE
#undef SORT_LONG_HASH
#undef SORT_OUTPUT_FB
#undef SORT_OUTPUT_FILE
#undef SORT_OUTPUT_THIS_FB
#undef SORT_PREFIX
#undef SORT_UNIFY
#undef SORT_UNIFY_WORKSPACE
#undef SORT_UNIQUE
#undef SORT_VAR_DATA
#undef SORT_VAR_KEY
#undef SWAP
#undef LESS
#undef P
