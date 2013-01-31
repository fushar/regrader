/*
 *	UCW Library -- Universal Sorter: Fixed-Size Internal Sorting Module
 *
 *	(c) 2007 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/stkstring.h"

#define ASORT_PREFIX(x) SORT_PREFIX(array_##x)
#define ASORT_KEY_TYPE P(key)
#define ASORT_LT(x,y) (P(compare)(&(x), &(y)) < 0)
#ifdef SORT_INTERNAL_RADIX
#  define ASORT_HASH(x) P(hash)(&(x))
#    ifdef SORT_LONG_HASH
#      define ASORT_LONG_HASH
#    endif
#endif
#include "ucw/sorter/array.h"

/*
 *  This is a more efficient implementation of the internal sorter,
 *  which runs under the following assumptions:
 *
 *     - the keys have fixed (and small) size
 *     - no data are present after the key
 *     - unification does not require any workspace
 */

static size_t P(internal_workspace)(void)
{
  size_t workspace = 0;
#ifdef SORT_UNIFY
  workspace = sizeof(P(key) *);
#endif
#ifdef SORT_INTERNAL_RADIX
  workspace = MAX(workspace, sizeof(P(key)));
#endif
  return workspace;
}

static uns P(internal_num_keys)(struct sort_context *ctx)
{
  size_t bufsize = ctx->big_buf_size;
  size_t workspace = P(internal_workspace)();
  if (workspace)
    bufsize -= CPU_PAGE_SIZE;
  u64 maxkeys = bufsize / (sizeof(P(key)) + workspace);
  return MIN(maxkeys, ~0U);					// The number of records must fit in uns
}

static int P(internal)(struct sort_context *ctx, struct sort_bucket *bin, struct sort_bucket *bout, struct sort_bucket *bout_only)
{
  sorter_alloc_buf(ctx);
  struct fastbuf *in = sbuck_read(bin);
  P(key) *buf = ctx->big_buf;
  uns maxkeys = P(internal_num_keys)(ctx);

  SORT_XTRACE(5, "s-fixint: Reading (maxkeys=%u, hash_bits=%d)", maxkeys, bin->hash_bits);
  uns n = 0;
  while (n < maxkeys && P(read_key)(in, &buf[n]))
    n++;
  if (!n)
    return 0;
  void *workspace UNUSED = ALIGN_PTR(&buf[n], CPU_PAGE_SIZE);

  SORT_XTRACE(4, "s-fixint: Sorting %u items (%s items, %s workspace)",
	n,
	stk_fsize(n * sizeof(P(key))),
	stk_fsize(n * P(internal_workspace)()));
  timestamp_t timer;
  init_timer(&timer);
  buf = P(array_sort)(buf, n
#ifdef SORT_INTERNAL_RADIX
    , workspace, bin->hash_bits
#endif
    );
  if ((void *)buf != ctx->big_buf)
    workspace = ctx->big_buf;
  ctx->total_int_time += get_timer(&timer);

  SORT_XTRACE(5, "s-fixint: Writing");
  if (n < maxkeys)
    bout = bout_only;
  struct fastbuf *out = sbuck_write(bout);
  bout->runs++;
  uns merged UNUSED = 0;
  for (uns i=0; i<n; i++)
    {
#ifdef SORT_UNIFY
      if (i < n-1 && !P(compare)(&buf[i], &buf[i+1]))
	{
	  P(key) **keys = workspace;
	  uns n = 2;
	  keys[0] = &buf[i];
	  keys[1] = &buf[i+1];
	  while (!P(compare)(&buf[i], &buf[i+n]))
	    {
	      keys[n] = &buf[i+n];
	      n++;
	    }
	  P(write_merged)(out, keys, NULL, n, NULL);
	  merged += n - 1;
	  i += n - 1;
	  continue;
	}
#endif
#ifdef SORT_ASSERT_UNIQUE
      ASSERT(i == n-1 || P(compare)(&buf[i], &buf[i+1]) < 0);
#endif
      P(write_key)(out, &buf[i]);
    }
#ifdef SORT_UNIFY
  SORT_XTRACE(4, "Merging reduced %u records", merged);
#endif

  return (n == maxkeys);
}

static u64
P(internal_estimate)(struct sort_context *ctx, struct sort_bucket *b UNUSED)
{
  return P(internal_num_keys)(ctx) * sizeof(P(key)) - 1;	// -1 since if the buffer is full, we don't recognize EOF
}
