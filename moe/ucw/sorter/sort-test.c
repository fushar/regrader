/*
 *	UCW Library -- Testing the Sorter
 *
 *	(c) 2007 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/getopt.h"
#include "ucw/conf.h"
#include "ucw/fastbuf.h"
#include "ucw/ff-binary.h"
#include "ucw/hashfunc.h"
#include "ucw/md5.h"
#include "ucw/string.h"
#include "ucw/prime.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

/*** A hack for overriding radix-sorter configuration ***/

#ifdef FORCE_RADIX_BITS
#undef CONFIG_UCW_RADIX_SORTER_BITS
#define CONFIG_UCW_RADIX_SORTER_BITS FORCE_RADIX_BITS
#endif

/*** Time measurement ***/

static timestamp_t timer;
static uns test_id;

static void
start(void)
{
  sync();
  init_timer(&timer);
}

static void
stop(void)
{
  sync();
  msg(L_INFO, "Test %d took %.3fs", test_id, get_timer(&timer) / 1000.);
}

/*** Simple 4-byte integer keys ***/

struct key1 {
  u32 x;
};

#define SORT_KEY_REGULAR struct key1
#define SORT_PREFIX(x) s1_##x
#define SORT_INPUT_FB
#define SORT_OUTPUT_FB
#define SORT_UNIQUE
#define SORT_INT(k) (k).x
#define SORT_DELETE_INPUT 0

#include "ucw/sorter/sorter.h"

static void
test_int(int mode, u64 size)
{
  uns N = size ? nextprime(MIN(size/4, 0xffff0000)) : 0;
  uns K = N/4*3;
  msg(L_INFO, ">>> Integers (%s, N=%u)", ((char *[]) { "increasing", "decreasing", "random" })[mode], N);

  struct fastbuf *f = bopen_tmp(65536);
  for (uns i=0; i<N; i++)
    bputl(f, (mode==0) ? i : (mode==1) ? N-1-i : ((u64)i * K + 17) % N);
  brewind(f);

  start();
  f = s1_sort(f, NULL, N-1);
  stop();

  SORT_XTRACE(2, "Verifying");
  for (uns i=0; i<N; i++)
    {
      uns j = bgetl(f);
      if (i != j)
	die("Discrepancy: %u instead of %u", j, i);
    }
  bclose(f);
}

/*** Integers with merging, but no data ***/

struct key2 {
  u32 x;
  u32 cnt;
};

static inline void s2_write_merged(struct fastbuf *f, struct key2 **k, void **d UNUSED, uns n, void *buf UNUSED)
{
  for (uns i=1; i<n; i++)
    k[0]->cnt += k[i]->cnt;
  bwrite(f, k[0], sizeof(struct key2));
}

#define SORT_KEY_REGULAR struct key2
#define SORT_PREFIX(x) s2_##x
#define SORT_INPUT_FB
#define SORT_OUTPUT_FB
#define SORT_UNIFY
#define SORT_INT(k) (k).x

#include "ucw/sorter/sorter.h"

static void
test_counted(int mode, u64 size)
{
  u64 items = size / sizeof(struct key2);
  uns mult = 2;
  while (items/(2*mult) > 0xffff0000)
    mult++;
  uns N = items ? nextprime(items/(2*mult)) : 0;
  uns K = N/4*3;
  msg(L_INFO, ">>> Counted integers (%s, N=%u, mult=%u)", ((char *[]) { "increasing", "decreasing", "random" })[mode], N, mult);

  struct fastbuf *f = bopen_tmp(65536);
  for (uns m=0; m<mult; m++)
    for (uns i=0; i<N; i++)
      for (uns j=0; j<2; j++)
	{
	  bputl(f, (mode==0) ? (i%N) : (mode==1) ? N-1-(i%N) : ((u64)i * K + 17) % N);
	  bputl(f, 1);
	}
  brewind(f);

  start();
  f = s2_sort(f, NULL, N-1);
  stop();

  SORT_XTRACE(2, "Verifying");
  for (uns i=0; i<N; i++)
    {
      uns j = bgetl(f);
      if (i != j)
	die("Discrepancy: %u instead of %u", j, i);
      uns k = bgetl(f);
      if (k != 2*mult)
	die("Discrepancy: %u has count %u instead of %u", j, k, 2*mult);
    }
  bclose(f);
}

/*** Longer records with hashes (similar to Shepherd's index records) ***/

struct key3 {
  u32 hash[4];
  u32 i;
  u32 payload[3];
};

static inline int s3_compare(struct key3 *x, struct key3 *y)
{
  COMPARE(x->hash[0], y->hash[0]);
  COMPARE(x->hash[1], y->hash[1]);
  COMPARE(x->hash[2], y->hash[2]);
  COMPARE(x->hash[3], y->hash[3]);
  return 0;
}

static inline uns s3_hash(struct key3 *x)
{
  return x->hash[0];
}

#define SORT_KEY_REGULAR struct key3
#define SORT_PREFIX(x) s3_##x
#define SORT_INPUT_FB
#define SORT_OUTPUT_FB
#define SORT_HASH_BITS 32

#include "ucw/sorter/sorter.h"

static void
gen_hash_key(int mode, struct key3 *k, uns i)
{
  k->i = i;
  k->payload[0] = 7*i + 13;
  k->payload[1] = 13*i + 19;
  k->payload[2] = 19*i + 7;
  switch (mode)
    {
    case 0:
      k->hash[0] = i;
      k->hash[1] = k->payload[0];
      k->hash[2] = k->payload[1];
      k->hash[3] = k->payload[2];
      break;
    case 1:
      k->hash[0] = ~i;
      k->hash[1] = k->payload[0];
      k->hash[2] = k->payload[1];
      k->hash[3] = k->payload[2];
      break;
    default: ;
      md5_hash_buffer((byte *) &k->hash, (byte *) &k->i, 4);
      break;
    }
}

static void
test_hashes(int mode, u64 size)
{
  uns N = MIN(size / sizeof(struct key3), 0xffffffff);
  msg(L_INFO, ">>> Hashes (%s, N=%u)", ((char *[]) { "increasing", "decreasing", "random" })[mode], N);
  struct key3 k, lastk;

  struct fastbuf *f = bopen_tmp(65536);
  uns hash_sum = 0;
  for (uns i=0; i<N; i++)
    {
      gen_hash_key(mode, &k, i);
      hash_sum += k.hash[3];
      bwrite(f, &k, sizeof(k));
    }
  brewind(f);

  start();
  f = s3_sort(f, NULL);
  stop();

  SORT_XTRACE(2, "Verifying");
  for (uns i=0; i<N; i++)
    {
      int ok = breadb(f, &k, sizeof(k));
      ASSERT(ok);
      if (i && s3_compare(&k, &lastk) <= 0)
	ASSERT(0);
      gen_hash_key(mode, &lastk, k.i);
      if (memcmp(&k, &lastk, sizeof(k)))
	ASSERT(0);
      hash_sum -= k.hash[3];
    }
  ASSERT(!hash_sum);
  bclose(f);
}

/*** Variable-length records (strings) with and without var-length data ***/

#define KEY4_MAX 256

struct key4 {
  uns len;
  byte s[KEY4_MAX];
};

static inline int s4_compare(struct key4 *x, struct key4 *y)
{
  uns l = MIN(x->len, y->len);
  int c = memcmp(x->s, y->s, l);
  if (c)
    return c;
  COMPARE(x->len, y->len);
  return 0;
}

static inline int s4_read_key(struct fastbuf *f, struct key4 *x)
{
  x->len = bgetl(f);
  if (x->len == 0xffffffff)
    return 0;
  ASSERT(x->len < KEY4_MAX);
  breadb(f, x->s, x->len);
  return 1;
}

static inline void s4_write_key(struct fastbuf *f, struct key4 *x)
{
  ASSERT(x->len < KEY4_MAX);
  bputl(f, x->len);
  bwrite(f, x->s, x->len);
}

#define SORT_KEY struct key4
#define SORT_PREFIX(x) s4_##x
#define SORT_KEY_SIZE(x) (sizeof(struct key4) - KEY4_MAX + (x).len)
#define SORT_INPUT_FB
#define SORT_OUTPUT_FB

#include "ucw/sorter/sorter.h"

#define s4b_compare s4_compare
#define s4b_read_key s4_read_key
#define s4b_write_key s4_write_key

static inline uns s4_data_size(struct key4 *x)
{
  return x->len ? (x->s[0] ^ 0xad) : 0;
}

#define SORT_KEY struct key4
#define SORT_PREFIX(x) s4b_##x
#define SORT_KEY_SIZE(x) (sizeof(struct key4) - KEY4_MAX + (x).len)
#define SORT_DATA_SIZE(x) s4_data_size(&(x))
#define SORT_INPUT_FB
#define SORT_OUTPUT_FB

#include "ucw/sorter/sorter.h"

static void
gen_key4(struct key4 *k)
{
  k->len = random_max(KEY4_MAX);
  for (uns i=0; i<k->len; i++)
    k->s[i] = random();
}

static void
gen_data4(byte *buf, uns len, uns h)
{
  while (len--)
    {
      *buf++ = h >> 24;
      h = h*259309 + 17;
    }
}

static void
test_strings(uns mode, u64 size)
{
  uns avg_item_size = KEY4_MAX/2 + 4 + (mode ? 128 : 0);
  uns N = MIN(size / avg_item_size, 0xffffffff);
  msg(L_INFO, ">>> Strings %s(N=%u)", (mode ? "with data " : ""), N);
  srand(1);

  struct key4 k, lastk;
  byte buf[256], buf2[256];
  uns sum = 0;

  struct fastbuf *f = bopen_tmp(65536);
  for (uns i=0; i<N; i++)
    {
      gen_key4(&k);
      s4_write_key(f, &k);
      uns h = hash_block(k.s, k.len);
      sum += h;
      if (mode)
	{
	  gen_data4(buf, s4_data_size(&k), h);
	  bwrite(f, buf, s4_data_size(&k));
	}
    }
  brewind(f);

  start();
  f = (mode ? s4b_sort : s4_sort)(f, NULL);
  stop();

  SORT_XTRACE(2, "Verifying");
  for (uns i=0; i<N; i++)
    {
      int ok = s4_read_key(f, &k);
      ASSERT(ok);
      uns h = hash_block(k.s, k.len);
      if (mode && s4_data_size(&k))
	{
	  ok = breadb(f, buf, s4_data_size(&k));
	  ASSERT(ok);
	  gen_data4(buf2, s4_data_size(&k), h);
	  ASSERT(!memcmp(buf, buf2, s4_data_size(&k)));
	}
      if (i && s4_compare(&k, &lastk) < 0)
	ASSERT(0);
      sum -= h;
      lastk = k;
    }
  ASSERT(!sum);
  bclose(f);
}

/*** Graph-like structure with custom presorting ***/

struct key5 {
  u32 x;
  u32 cnt;
};

static uns s5_N, s5_K, s5_L, s5_i, s5_j;

struct s5_pair {
  uns x, y;
};

static int s5_gen(struct s5_pair *p)
{
  if (s5_j >= s5_N)
    {
      if (!s5_N || s5_i >= s5_N-1)
	return 0;
      s5_j = 0;
      s5_i++;
    }
  p->x = ((u64)s5_j * s5_K) % s5_N;
  p->y = ((u64)(s5_i + s5_j) * s5_L) % s5_N;
  s5_j++;
  return 1;
}

#define ASORT_PREFIX(x) s5m_##x
#define ASORT_KEY_TYPE u32
#include "ucw/sorter/array-simple.h"

static void s5_write_merged(struct fastbuf *f, struct key5 **keys, void **data, uns n, void *buf)
{
  u32 *a = buf;
  uns m = 0;
  for (uns i=0; i<n; i++)
    {
      memcpy(&a[m], data[i], 4*keys[i]->cnt);
      m += keys[i]->cnt;
    }
  s5m_sort(a, m);
  keys[0]->cnt = m;
  bwrite(f, keys[0], sizeof(struct key5));
  bwrite(f, a, 4*m);
}

static void s5_copy_merged(struct key5 **keys, struct fastbuf **data, uns n, struct fastbuf *dest)
{
  u32 k[n];
  uns m = 0;
  for (uns i=0; i<n; i++)
    {
      k[i] = bgetl(data[i]);
      m += keys[i]->cnt;
    }
  struct key5 key = { .x = keys[0]->x, .cnt = m };
  bwrite(dest, &key, sizeof(key));
  while (key.cnt--)
    {
      uns b = 0;
      for (uns i=1; i<n; i++)
	if (k[i] < k[b])
	  b = i;
      bputl(dest, k[b]);
      if (--keys[b]->cnt)
	k[b] = bgetl(data[b]);
      else
	k[b] = ~0U;
    }
}

static inline int s5p_lt(struct s5_pair x, struct s5_pair y)
{
  COMPARE_LT(x.x, y.x);
  COMPARE_LT(x.y, y.y);
  return 0;
}

#define ASORT_PREFIX(x) s5p_##x
#define ASORT_KEY_TYPE struct s5_pair
#define ASORT_LT(x,y) s5p_lt(x,y)
#include "ucw/sorter/array.h"

static int s5_presort(struct fastbuf *dest, void *buf, size_t bufsize)
{
  uns max = MIN(bufsize/sizeof(struct s5_pair), 0xffffffff);
  struct s5_pair *a = buf;
  uns n = 0;
  while (n<max && s5_gen(&a[n]))
    n++;
  if (!n)
    return 0;
  s5p_sort(a, n);
  uns i = 0;
  while (i < n)
    {
      uns j = i;
      while (i < n && a[i].x == a[j].x)
	i++;
      struct key5 k = { .x = a[j].x, .cnt = i-j };
      bwrite(dest, &k, sizeof(k));
      while (j < i)
	bputl(dest, a[j++].y);
    }
  return 1;
}

#define SORT_KEY_REGULAR struct key5
#define SORT_PREFIX(x) s5_##x
#define SORT_DATA_SIZE(k) (4*(k).cnt)
#define SORT_UNIFY
#define SORT_UNIFY_WORKSPACE(k) SORT_DATA_SIZE(k)
#define SORT_INPUT_PRESORT
#define SORT_OUTPUT_THIS_FB
#define SORT_INT(k) (k).x

#include "ucw/sorter/sorter.h"

#define SORT_KEY_REGULAR struct key5
#define SORT_PREFIX(x) s5b_##x
#define SORT_DATA_SIZE(k) (4*(k).cnt)
#define SORT_UNIFY
#define SORT_UNIFY_WORKSPACE(k) SORT_DATA_SIZE(k)
#define SORT_INPUT_FB
#define SORT_OUTPUT_THIS_FB
#define SORT_INT(k) (k).x
#define s5b_write_merged s5_write_merged
#define s5b_copy_merged s5_copy_merged

#include "ucw/sorter/sorter.h"

static void
test_graph(uns mode, u64 size)
{
  uns N = 3;
  while ((u64)N*(N+2)*4 < size)
    N = nextprime(N);
  if (!size)
    N = 0;
  msg(L_INFO, ">>> Graph%s (N=%u)", (mode ? "" : " with custom presorting"), N);
  s5_N = N;
  s5_K = N/4*3;
  s5_L = N/3*2;
  s5_i = s5_j = 0;

  struct fastbuf *in = NULL;
  if (mode)
    {
      struct s5_pair p;
      in = bopen_tmp(65536);
      while (s5_gen(&p))
	{
	  struct key5 k = { .x = p.x, .cnt = 1 };
	  bwrite(in, &k, sizeof(k));
	  bputl(in, p.y);
	}
      brewind(in);
    }

  start();
  struct fastbuf *f = bopen_tmp(65536);
  bputl(f, 0xfeedcafe);
  struct fastbuf *g = (mode ? s5b_sort(in, f, s5_N-1) : s5_sort(NULL, f, s5_N-1));
  ASSERT(f == g);
  stop();

  SORT_XTRACE(2, "Verifying");
  uns c = bgetl(f);
  ASSERT(c == 0xfeedcafe);
  for (uns i=0; i<N; i++)
    {
      struct key5 k;
      int ok = breadb(f, &k, sizeof(k));
      ASSERT(ok);
      ASSERT(k.x == i);
      ASSERT(k.cnt == N);
      for (uns j=0; j<N; j++)
	{
	  uns y = bgetl(f);
	  ASSERT(y == j);
	}
    }
  bclose(f);
}

/*** Simple 8-byte integer keys ***/

struct key6 {
  u64 x;
};

#define SORT_KEY_REGULAR struct key6
#define SORT_PREFIX(x) s6_##x
#define SORT_INPUT_FB
#define SORT_OUTPUT_FB
#define SORT_UNIQUE
#define SORT_INT64(k) (k).x

#include "ucw/sorter/sorter.h"

static void
test_int64(int mode, u64 size)
{
  u64 N = size ? nextprime(MIN(size/8, 0xffff0000)) : 0;
  u64 K = N/4*3;
  msg(L_INFO, ">>> 64-bit integers (%s, N=%llu)", ((char *[]) { "increasing", "decreasing", "random" })[mode], (long long)N);

  struct fastbuf *f = bopen_tmp(65536);
  for (u64 i=0; i<N; i++)
    bputq(f, 777777*((mode==0) ? i : (mode==1) ? N-1-i : ((u64)i * K + 17) % N));
  brewind(f);

  start();
  f = s6_sort(f, NULL, 777777*(N-1));
  stop();

  SORT_XTRACE(2, "Verifying");
  for (u64 i=0; i<N; i++)
    {
      u64 j = bgetq(f);
      if (777777*i != j)
	die("Discrepancy: %llu instead of %llu", (long long)j, 777777*(long long)i);
    }
  bclose(f);
}

/*** Main ***/

static void
run_test(uns i, u64 size)
{
  test_id = i;
  switch (i)
    {
    case 0:
      test_int(0, size); break;
    case 1:
      test_int(1, size); break;
    case 2:
      test_int(2, size); break;
    case 3:
      test_counted(0, size); break;
    case 4:
      test_counted(1, size); break;
    case 5:
      test_counted(2, size); break;
    case 6:
      test_hashes(0, size); break;
    case 7:
      test_hashes(1, size); break;
    case 8:
      test_hashes(2, size); break;
    case 9:
      test_strings(0, size); break;
    case 10:
      test_strings(1, size); break;
    case 11:
      test_graph(0, size); break;
    case 12:
      test_graph(1, size); break;
    case 13:
      test_int64(0, size); break;
    case 14:
      test_int64(1, size); break;
    case 15:
      test_int64(2, size); break;
#define TMAX 16
    }
}

int
main(int argc, char **argv)
{
  log_init(NULL);
  int c;
  u64 size = 10000000;
  uns t = ~0;

  while ((c = cf_getopt(argc, argv, CF_SHORT_OPTS "d:s:t:v", CF_NO_LONG_OPTS, NULL)) >= 0)
    switch (c)
      {
      case 'd':
	sorter_debug = atol(optarg);
	break;
      case 's':
	if (cf_parse_u64(optarg, &size))
	  goto usage;
	break;
      case 't':
	  {
	    char *w[32];
	    int f = str_sepsplit(optarg, ',', w, ARRAY_SIZE(w));
	    if (f < 0)
	      goto usage;
	    t = 0;
	    for (int i=0; i<f; i++)
	      {
		int j = atol(w[i]);
		if (j >= TMAX)
		  goto usage;
		t |= 1 << j;
	      }
	  }
	break;
      case 'v':
	sorter_trace++;
	break;
      default:
      usage:
	fputs("Usage: sort-test [-v] [-d <debug>] [-s <size>] [-t <test>]\n", stderr);
	exit(1);
      }
  if (optind != argc)
    goto usage;

  for (uns i=0; i<TMAX; i++)
    if (t & (1 << i))
      run_test(i, size);

  return 0;
}
