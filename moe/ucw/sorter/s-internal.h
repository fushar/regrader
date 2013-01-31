/*
 *	UCW Library -- Universal Sorter: Internal Sorting Module
 *
 *	(c) 2007 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/stkstring.h"

#ifdef SORT_INTERNAL_RADIX
/* Keep copies of the items' hashes to save cache misses */
#define SORT_COPY_HASH
#endif

typedef struct {
  P(key) *key;
#ifdef SORT_COPY_HASH
  P(hash_t) hash;
#endif
} P(internal_item_t);

#define ASORT_PREFIX(x) SORT_PREFIX(array_##x)
#define ASORT_KEY_TYPE P(internal_item_t)
#ifdef SORT_COPY_HASH
#  ifdef SORT_INT
#    define ASORT_LT(x,y) ((x).hash < (y).hash)		// In this mode, the hash is the value
#  else
#    define ASORT_LT(x,y) ((x).hash < (y).hash || (x).hash == (y).hash && P(compare)((x).key, (y).key) < 0)
#  endif
#else
#  define ASORT_LT(x,y) (P(compare)((x).key, (y).key) < 0)
#endif
#ifdef SORT_INTERNAL_RADIX
#    ifdef SORT_COPY_HASH
#      define ASORT_HASH(x) (x).hash
#    else
#      define ASORT_HASH(x) P(hash)((x).key)
#    endif
#    ifdef SORT_LONG_HASH
#      define ASORT_LONG_HASH
#    endif
#endif
#include "ucw/sorter/array.h"

/*
 *  The big_buf has the following layout:
 *
 *	+-------------------------------------------------------------------------------+
 *	| array of internal_item's							|
 *	+-------------------------------------------------------------------------------+
 *	| padding to make the following part page-aligned				|
 *	+--------------------------------+----------------------------------------------+
 *	| shadow copy of item array      | array of pointers to data for write_merged() |
 *	| used if radix-sorting          +----------------------------------------------+
 *	|                                | workspace for write_merged()			|
 *	+--------------------------------+----------------------------------------------+
 *	|	       +---------+							|
 *	|              | key     |							|
 *	|              +---------+							|
 *	| sequence of  | padding |							|
 *	| items        +---------+							|
 *	|              | data    |							|
 *	|              +---------+							|
 *	|              | padding |							|
 *	|              +---------+							|
 *	+-------------------------------------------------------------------------------+
 *
 *  (the data which are in different columns are never accessed simultaneously,
 *   so we use a single buffer for both)
 */

static inline void *P(internal_get_data)(P(key) *key)
{
  uns ksize = SORT_KEY_SIZE(*key);
#ifdef SORT_UNIFY
  ksize = ALIGN_TO(ksize, CPU_STRUCT_ALIGN);
#endif
  return (byte *) key + ksize;
}

static inline size_t P(internal_workspace)(P(key) *key UNUSED)
{
  size_t ws = 0;
#ifdef SORT_UNIFY
  ws += sizeof(void *);
#endif
#ifdef SORT_UNIFY_WORKSPACE
  ws += SORT_UNIFY_WORKSPACE(*key);
#endif
#ifdef SORT_INTERNAL_RADIX
  ws = MAX(ws, sizeof(P(internal_item_t)));
#endif
  return ws;
}

static int P(internal)(struct sort_context *ctx, struct sort_bucket *bin, struct sort_bucket *bout, struct sort_bucket *bout_only)
{
  sorter_alloc_buf(ctx);
  struct fastbuf *in = sbuck_read(bin);

  P(key) key, *keybuf = ctx->key_buf;
  if (!keybuf)
    keybuf = ctx->key_buf = sorter_alloc(ctx, sizeof(key));
  if (ctx->more_keys)
    {
      key = *keybuf;
      ctx->more_keys = 0;
    }
  else if (!P(read_key)(in, &key))
    return 0;

  size_t bufsize = ctx->big_buf_size;
#ifdef SORT_VAR_DATA
  if (sizeof(key) + 2*CPU_PAGE_SIZE + SORT_DATA_SIZE(key) + P(internal_workspace)(&key) > bufsize)
    {
      SORT_XTRACE(4, "s-internal: Generating a giant run");
      struct fastbuf *out = sbuck_write(bout);
      P(copy_data)(&key, in, out);
      bout->runs++;
      return 1;				// We don't know, but 1 is always safe
    }
#endif

  SORT_XTRACE(5, "s-internal: Reading");
  P(internal_item_t) *item_array = ctx->big_buf, *item = item_array, *last_item;
  byte *end = (byte *) ctx->big_buf + bufsize;
  size_t remains = bufsize - CPU_PAGE_SIZE;
  do
    {
      uns ksize = SORT_KEY_SIZE(key);
#ifdef SORT_UNIFY
      uns ksize_aligned = ALIGN_TO(ksize, CPU_STRUCT_ALIGN);
#else
      uns ksize_aligned = ksize;
#endif
      uns dsize = SORT_DATA_SIZE(key);
      uns recsize = ALIGN_TO(ksize_aligned + dsize, CPU_STRUCT_ALIGN);
      size_t totalsize = recsize + sizeof(P(internal_item_t)) + P(internal_workspace)(&key);
      if (unlikely(totalsize > remains
#ifdef CPU_64BIT_POINTERS
		   || item >= item_array + ~0U		// The number of items must fit in an uns
#endif
	 ))
	{
	  ctx->more_keys = 1;
	  *keybuf = key;
	  break;
	}
      remains -= totalsize;
      end -= recsize;
      memcpy(end, &key, ksize);
#ifdef SORT_VAR_DATA
      breadb(in, end + ksize_aligned, dsize);
#endif
      item->key = (P(key)*) end;
#ifdef SORT_COPY_HASH
      item->hash = P(hash)(item->key);
#endif
      item++;
    }
  while (P(read_key)(in, &key));
  last_item = item;

  uns count = last_item - item_array;
  void *workspace UNUSED = ALIGN_PTR(last_item, CPU_PAGE_SIZE);
  SORT_XTRACE(4, "s-internal: Read %u items (%s items, %s workspace, %s data)",
	count,
	stk_fsize((byte*)last_item - (byte*)item_array),
	stk_fsize(end - (byte*)last_item - remains),
	stk_fsize((byte*)ctx->big_buf + bufsize - end));
  timestamp_t timer;
  init_timer(&timer);
  item_array = P(array_sort)(item_array, count
#ifdef SORT_INTERNAL_RADIX
    , workspace, bin->hash_bits
#endif
    );
  if ((void *)item_array != ctx->big_buf)
    workspace = ctx->big_buf;
  last_item = item_array + count;
  ctx->total_int_time += get_timer(&timer);

  SORT_XTRACE(5, "s-internal: Writing");
  if (!ctx->more_keys)
    bout = bout_only;
  struct fastbuf *out = sbuck_write(bout);
  bout->runs++;
  uns merged UNUSED = 0;
  for (item = item_array; item < last_item; item++)
    {
#ifdef SORT_UNIFY
      if (item < last_item - 1 && !P(compare)(item->key, item[1].key))
	{
	  // Rewrite the item structures with just pointers to keys and place
	  // pointers to data in the workspace.
	  P(key) **key_array = (void *) item;
	  void **data_array = workspace;
	  key_array[0] = item[0].key;
	  data_array[0] = P(internal_get_data)(key_array[0]);
	  uns cnt;
	  for (cnt=1; item+cnt < last_item && !P(compare)(key_array[0], item[cnt].key); cnt++)
	    {
	      key_array[cnt] = item[cnt].key;
	      data_array[cnt] = P(internal_get_data)(key_array[cnt]);
	    }
	  P(write_merged)(out, key_array, data_array, cnt, data_array+cnt);
	  item += cnt - 1;
	  merged += cnt - 1;
	  continue;
	}
#endif
#ifdef SORT_ASSERT_UNIQUE
      ASSERT(item == last_item-1 || P(compare)(item->key, item[1].key) < 0);
#endif
      P(write_key)(out, item->key);
#ifdef SORT_VAR_DATA
      bwrite(out, P(internal_get_data)(item->key), SORT_DATA_SIZE(*item->key));
#endif
    }
#ifdef SORT_UNIFY
  SORT_XTRACE(4, "Merging reduced %u records", merged);
#endif

  return ctx->more_keys;
}

static u64
P(internal_estimate)(struct sort_context *ctx, struct sort_bucket *b UNUSED)
{
  // Most of this is just wild guesses
#ifdef SORT_VAR_KEY
  uns avg = ALIGN_TO(sizeof(P(key))/4, CPU_STRUCT_ALIGN);
#else
  uns avg = ALIGN_TO(sizeof(P(key)), CPU_STRUCT_ALIGN);
#endif
  uns ws = 0;
#ifdef SORT_UNIFY
  ws += sizeof(void *);
#endif
#ifdef SORT_UNIFY_WORKSPACE
  ws += avg;
#endif
#ifdef SORT_INTERNAL_RADIX
  ws = MAX(ws, sizeof(P(internal_item_t)));
#endif
  // We ignore the data part of records, it probably won't make the estimate much worse
  return (ctx->big_buf_size / (avg + ws + sizeof(P(internal_item_t))) * avg);
}

#undef SORT_COPY_HASH
