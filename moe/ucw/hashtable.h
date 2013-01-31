/*
 *	UCW Library -- Universal Hash Table
 *
 *	(c) 2002--2004 Martin Mares <mj@ucw.cz>
 *	(c) 2002--2005 Robert Spalek <robert@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

/*
 *  This is not a normal header file, it's a generator of hash tables.
 *  Each time you include it with parameters set in the corresponding
 *  preprocessor macros, it generates a hash table with the parameters
 *  given.
 *
 *  You need to specify:
 *
 *  HASH_NODE		data type where a node dwells (usually a struct).
 *  HASH_PREFIX(x)	macro to add a name prefix (used on all global names
 *			defined by the hash table generator).
 *
 *  Then decide on type of keys:
 *
 *  HASH_KEY_ATOMIC=f	use node->f as a key of an atomic type (i.e.,
 *			a type which can be compared using `==')
 *			HASH_ATOMIC_TYPE (defaults to int).
 *  | HASH_KEY_STRING=f	use node->f as a string key, allocated
 *			separately from the rest of the node.
 *  | HASH_KEY_ENDSTRING=f use node->f as a string key, allocated
 *			automatically at the end of the node struct
 *			(to be declared as "char f[1]" at the end).
 *  | HASH_KEY_COMPLEX	use a multi-component key; as the name suggests,
 *			the passing of parameters is a bit complex then.
 *			The HASH_KEY_COMPLEX(x) macro should expand to
 *			`x k1, x k2, ... x kn' and you should also define:
 *    HASH_KEY_DECL	declaration of function parameters in which key
 *			should be passed to all hash table operations.
 *			That is, `type1 k1, type2 k2, ... typen kn'.
 *			With complex keys, HASH_GIVE_HASHFN and HASH_GIVE_EQ
 *			are mandatory.
 *  | HASH_KEY_MEMORY=f	use node->f as a raw data key, compared using
 *  			memcmp
 *    HASH_KEY_SIZE	the length of the key block
 *
 *  Then specify what operations you request (all names are automatically
 *  prefixed by calling HASH_PREFIX):
 *
 *  <always defined>	init() -- initialize the hash table.
 *  HASH_WANT_CLEANUP	cleanup() -- deallocate the hash table.
 *  HASH_WANT_FIND	node *find(key) -- find first node with the specified
 *			key, return NULL if no such node exists.
 *  HASH_WANT_FIND_NEXT	node *find(node *start) -- find next node with the
 *			specified key, return NULL if no such node exists.
 *  HASH_WANT_NEW	node *new(key) -- create new node with given key.
 *			Doesn't check whether it already exists.
 *  HASH_WANT_LOOKUP	node *lookup(key) -- find node with given key,
 *			if it doesn't exist, create it. Defining
 *			HASH_GIVE_INIT_DATA is strongly recommended.
 *  HASH_WANT_DELETE	int delete(key) -- delete and deallocate node
 *			with given key. Returns success.
 *  HASH_WANT_REMOVE	remove(node *) -- delete and deallocate given node.
 *
 *  You can also supply several functions:
 *
 *  HASH_GIVE_HASHFN	unsigned int hash(key) -- calculate hash value of key.
 *			We have sensible default hash functions for strings
 *			and integers.
 *  HASH_GIVE_EQ	int eq(key1, key2) -- return whether keys are equal.
 *			By default, we use == for atomic types and either
 *			strcmp or strcasecmp for strings.
 *  HASH_GIVE_EXTRA_SIZE int extra_size(key) -- returns how many bytes after the
 *			node should be allocated for dynamic data. Default=0
 *			or length of the string with HASH_KEY_ENDSTRING.
 *  HASH_GIVE_INIT_KEY	void init_key(node *,key) -- initialize key in a newly
 *			created node. Defaults: assignment for atomic keys
 *			and static strings, strcpy for end-allocated strings.
 *  HASH_GIVE_INIT_DATA	void init_data(node *) -- initialize data fields in a
 *			newly created node. Very useful for lookup operations.
 *  HASH_GIVE_ALLOC	void *alloc(unsigned int size) -- allocate space for
 *			a node. Default is xmalloc() or pooled allocation, depending
 *			on HASH_USE_POOL and HASH_AUTO_POOL switches.
 *			void free(void *) -- the converse.
 *
 *  ... and a couple of extra parameters:
 *
 *  HASH_NOCASE		String comparisons should be case-insensitive.
 *  HASH_DEFAULT_SIZE=n	Initially, use hash table of approx. `n' entries.
 *  HASH_CONSERVE_SPACE	Use as little space as possible.
 *  HASH_FN_BITS=n	The hash function gives only `n' significant bits.
 *  HASH_ATOMIC_TYPE=t	Atomic values are of type `t' instead of int.
 *  HASH_USE_POOL=pool	Allocate all nodes from given mempool. Note, however, that
 *			deallocation is not supported by mempools, so delete/remove
 *			will leak pool memory.
 *  HASH_AUTO_POOL=size	Create a pool of the given block size automatically.
 *  HASH_ZERO_FILL	New entries should be initialized to all zeroes.
 *  HASH_TABLE_ALLOC	The hash table itself will be allocated and freed using
 *			the same allocation functions as the nodes instead of
 *			the default xmalloc().
 *  HASH_TABLE_DYNAMIC	Support multiple hash tables; the first parameter of all
 *			hash table operations is struct HASH_PREFIX(table) *.
 *
 *  You also get a iterator macro at no extra charge:
 *
 *  HASH_FOR_ALL(hash_prefix, variable)
 *    {
 *      // node *variable gets declared automatically
 *      do_something_with_node(variable);
 *      // use HASH_BREAK and HASH_CONTINUE instead of break and continue
 *	// you must not alter contents of the hash table here
 *    }
 *  HASH_END_FOR;
 *
 *  (For dynamic tables, use HASH_FOR_ALL_DYNAMIC(hash_prefix, hash_table, variable) instead.)
 *
 *  Then include "ucw/hashtable.h" and voila, you have a hash table
 *  suiting all your needs (at least those which you've revealed :) ).
 *
 *  After including this file, all parameter macros are automatically
 *  undef'd.
 */

#ifndef _UCW_HASHFUNC_H
#include "ucw/hashfunc.h"
#endif

#include "ucw/prime.h"

#include <string.h>

/* Initial setup of parameters */

#if !defined(HASH_NODE) || !defined(HASH_PREFIX)
#error Some of the mandatory configuration macros are missing.
#endif

#if defined(HASH_KEY_ATOMIC) && !defined(HASH_CONSERVE_SPACE)
#define HASH_CONSERVE_SPACE
#endif

#define P(x) HASH_PREFIX(x)

/* Declare buckets and the hash table */

typedef HASH_NODE P(node);

typedef struct P(bucket) {
  struct P(bucket) *next;
#ifndef HASH_CONSERVE_SPACE
  uns hash;
#endif
  P(node) n;
} P(bucket);

struct P(table) {
  uns hash_size;
  uns hash_count, hash_max, hash_min, hash_hard_max;
  P(bucket) **ht;
#ifdef HASH_AUTO_POOL
  struct mempool *pool;
#endif
};

#ifdef HASH_TABLE_DYNAMIC
#define T (*table)
#define TA struct P(table) *table
#define TAC TA,
#define TAU TA UNUSED
#define TAUC TA UNUSED,
#define TT table
#define TTC table,
#else
struct P(table) P(table);
#define T P(table)
#define TA void
#define TAC
#define TAU void
#define TAUC
#define TT
#define TTC
#endif

/* Preset parameters */

#if defined(HASH_KEY_ATOMIC)

#define HASH_KEY(x) x HASH_KEY_ATOMIC

#ifndef HASH_ATOMIC_TYPE
#  define HASH_ATOMIC_TYPE int
#endif
#define HASH_KEY_DECL HASH_ATOMIC_TYPE HASH_KEY( )

#ifndef HASH_GIVE_HASHFN
#  define HASH_GIVE_HASHFN
   static inline int P(hash) (TAUC HASH_ATOMIC_TYPE x)
   { return ((sizeof(x) <= 4) ? hash_u32(x) : hash_u64(x)); }
#endif

#ifndef HASH_GIVE_EQ
#  define HASH_GIVE_EQ
   static inline int P(eq) (TAUC HASH_ATOMIC_TYPE x, HASH_ATOMIC_TYPE y)
   { return x == y; }
#endif

#ifndef HASH_GIVE_INIT_KEY
#  define HASH_GIVE_INIT_KEY
   static inline void P(init_key) (TAUC P(node) *n, HASH_ATOMIC_TYPE k)
   { HASH_KEY(n->) = k; }
#endif

#elif defined(HASH_KEY_MEMORY)

#define HASH_KEY(x) x HASH_KEY_MEMORY

#define HASH_KEY_DECL byte HASH_KEY( )[HASH_KEY_SIZE]

#ifndef HASH_GIVE_HASHFN
#  define HASH_GIVE_HASHFN
   static inline int P(hash) (TAUC byte *x)
   { return hash_block(x, HASH_KEY_SIZE); }
#endif

#ifndef HASH_GIVE_EQ
#  define HASH_GIVE_EQ
   static inline int P(eq) (TAUC byte *x, byte *y)
   { return !memcmp(x, y, HASH_KEY_SIZE); }
#endif

#ifndef HASH_GIVE_INIT_KEY
#  define HASH_GIVE_INIT_KEY
   static inline void P(init_key) (TAUC P(node) *n, byte *k)
   { memcpy(HASH_KEY(n->), k, HASH_KEY_SIZE); }
#endif

#elif defined(HASH_KEY_STRING) || defined(HASH_KEY_ENDSTRING)

#ifdef HASH_KEY_STRING
#  define HASH_KEY(x) x HASH_KEY_STRING
#  ifndef HASH_GIVE_INIT_KEY
#    define HASH_GIVE_INIT_KEY
     static inline void P(init_key) (TAUC P(node) *n, char *k)
     { HASH_KEY(n->) = k; }
#  endif
#else
#  define HASH_KEY(x) x HASH_KEY_ENDSTRING
#  define HASH_GIVE_EXTRA_SIZE
   static inline int P(extra_size) (TAUC char *k)
   { return strlen(k); }
#  ifndef HASH_GIVE_INIT_KEY
#    define HASH_GIVE_INIT_KEY
     static inline void P(init_key) (TAUC P(node) *n, char *k)
     { strcpy(HASH_KEY(n->), k); }
#  endif
#endif
#define HASH_KEY_DECL char *HASH_KEY( )

#ifndef HASH_GIVE_HASHFN
#define HASH_GIVE_HASHFN
  static inline uns P(hash) (TAUC char *k)
   {
#    ifdef HASH_NOCASE
       return hash_string_nocase(k);
#    else
       return hash_string(k);
#    endif
   }
#endif

#ifndef HASH_GIVE_EQ
#  define HASH_GIVE_EQ
   static inline int P(eq) (TAUC char *x, char *y)
   {
#    ifdef HASH_NOCASE
       return !strcasecmp(x,y);
#    else
       return !strcmp(x,y);
#    endif
   }
#endif

#elif defined(HASH_KEY_COMPLEX)

#define HASH_KEY(x) HASH_KEY_COMPLEX(x)

#else
#error You forgot to set the hash key type.
#endif

/* Defaults for missing parameters */

#ifndef HASH_GIVE_HASHFN
#error Unable to determine which hash function to use.
#endif

#ifndef HASH_GIVE_EQ
#error Unable to determine how to compare two keys.
#endif

#ifdef HASH_GIVE_EXTRA_SIZE
/* This trickery is needed to avoid `unused parameter' warnings */
#define HASH_EXTRA_SIZE(x) P(extra_size)(TTC x)
#else
/*
 *  Beware, C macros are expanded iteratively, not recursively,
 *  hence we get only a _single_ argument, although the expansion
 *  of HASH_KEY contains commas.
 */
#define HASH_EXTRA_SIZE(x) 0
#endif

#ifndef HASH_GIVE_INIT_KEY
#error Unable to determine how to initialize keys.
#endif

#ifndef HASH_GIVE_INIT_DATA
static inline void P(init_data) (TAUC P(node) *n UNUSED)
{
}
#endif

#ifdef HASH_GIVE_ALLOC
/* If the caller has requested to use his own allocation functions, do so */
static inline void P(init_alloc) (TAU) { }
static inline void P(cleanup_alloc) (TAU) { }

#elif defined(HASH_USE_POOL)
/* If the caller has requested to use his mempool, do so */
#include "ucw/mempool.h"
static inline void * P(alloc) (TAUC unsigned int size) { return mp_alloc_fast(HASH_USE_POOL, size); }
static inline void P(free) (TAUC void *x UNUSED) { }
static inline void P(init_alloc) (TAU) { }
static inline void P(cleanup_alloc) (TAU) { }

#elif defined(HASH_AUTO_POOL)
/* Use our own pools */
#include "ucw/mempool.h"
static inline void * P(alloc) (TAUC unsigned int size) { return mp_alloc_fast(T.pool, size); }
static inline void P(free) (TAUC void *x UNUSED) { }
static inline void P(init_alloc) (TAU) { T.pool = mp_new(HASH_AUTO_POOL); }
static inline void P(cleanup_alloc) (TAU) { mp_delete(T.pool); }
#define HASH_USE_POOL

#else
/* The default allocation method */
static inline void * P(alloc) (TAUC unsigned int size) { return xmalloc(size); }
static inline void P(free) (TAUC void *x) { xfree(x); }
static inline void P(init_alloc) (TAU) { }
static inline void P(cleanup_alloc) (TAU) { }

#endif

#ifdef HASH_TABLE_ALLOC
static inline void * P(table_alloc) (TAUC unsigned int size) { return P(alloc)(TTC size); }
static inline void P(table_free) (TAUC void *x) { P(free)(TTC x); }
#else
static inline void * P(table_alloc) (TAUC unsigned int size) { return xmalloc(size); }
static inline void P(table_free) (TAUC void *x) { xfree(x); }
#endif

#ifndef HASH_DEFAULT_SIZE
#define HASH_DEFAULT_SIZE 32
#endif

#ifndef HASH_FN_BITS
#define HASH_FN_BITS 32
#endif

#ifdef HASH_ZERO_FILL
static inline void * P(new_bucket)(TAUC uns size)
{
  byte *buck = P(alloc)(TTC size);
  bzero(buck, size);
  return buck;
}
#else
static inline void * P(new_bucket)(TAUC uns size) { return P(alloc)(TTC size); }
#endif

/* Now the operations */

static void P(alloc_table) (TAU)
{
  T.hash_size = next_table_prime(T.hash_size);
  T.ht = P(table_alloc)(TTC sizeof(void *) * T.hash_size);
  bzero(T.ht, sizeof(void *) * T.hash_size);
  if (2*T.hash_size < T.hash_hard_max)
    T.hash_max = 2*T.hash_size;
  else
    T.hash_max = ~0U;
  if (T.hash_size/2 > HASH_DEFAULT_SIZE)
    T.hash_min = T.hash_size/4;
  else
    T.hash_min = 0;
}

/**
 * Initializes the hash table.
 * This one is available no matter what `HASH_WANT_` macros you defined or not.
 **/
static void HASH_PREFIX(init)(TA)
{
  T.hash_count = 0;
  T.hash_size = HASH_DEFAULT_SIZE;
#if HASH_FN_BITS < 28
  T.hash_hard_max = 1 << HASH_FN_BITS;
#else
  T.hash_hard_max = 1 << 28;
#endif
  P(init_alloc)(TT);
  P(alloc_table)(TT);
}

#ifdef HASH_WANT_CLEANUP
/**
 * Deallocates the hash table, including the nodes.
 * It is available if you defined <<want_cleanup,`HASH_WANT_CLEANUP`>>.
 **/
static void HASH_PREFIX(cleanup)(TA)
{
#ifndef HASH_USE_POOL
  uns i;
  P(bucket) *b, *bb;

  for (i=0; i<T.hash_size; i++)
    for (b=T.ht[i]; b; b=bb)
      {
	bb = b->next;
	P(free)(TTC b);
      }
#endif
  P(cleanup_alloc)(TT);
  P(table_free)(TTC T.ht);
}
#endif

static inline uns P(bucket_hash) (TAUC P(bucket) *b)
{
#ifdef HASH_CONSERVE_SPACE
  return P(hash)(TTC HASH_KEY(b->n.));
#else
  return b->hash;
#endif
}

static void P(rehash) (TAC uns size)
{
  P(bucket) *b, *nb;
  P(bucket) **oldt = T.ht, **newt;
  uns oldsize = T.hash_size;
  uns i, h;

  DBG("Rehashing %d->%d at count %d", oldsize, size, T.hash_count);
  T.hash_size = size;
  P(alloc_table)(TT);
  newt = T.ht;
  for (i=0; i<oldsize; i++)
    {
      b = oldt[i];
      while (b)
	{
	  nb = b->next;
	  h = P(bucket_hash)(TTC b) % T.hash_size;
	  b->next = newt[h];
	  newt[h] = b;
	  b = nb;
	}
    }
  P(table_free)(TTC oldt);
}

#ifdef HASH_WANT_FIND
/**
 * Finds a node with given key (specified in the @HAS_KEY_DECL parameter).
 * If it does not exist, NULL is returned.
 *
 * Enabled by the <<want_find,`HASH_WANT_FIND`>> macro.
 **/
static HASH_NODE* HASH_PREFIX(find)(TAC HASH_KEY_DECL)
{
  uns h0 = P(hash) (TTC HASH_KEY( ));
  uns h = h0 % T.hash_size;
  P(bucket) *b;

  for (b=T.ht[h]; b; b=b->next)
    {
      if (
#ifndef HASH_CONSERVE_SPACE
	  b->hash == h0 &&
#endif
	  P(eq)(TTC HASH_KEY( ), HASH_KEY(b->n.)))
	return &b->n;
    }
  return NULL;
}
#endif

#ifdef HASH_WANT_FIND_NEXT
/**
 * Finds next node with the same key. Returns NULL if it does not exist.
 *
 * Enabled by the <<want_find_next,`HASH_WANT_FIND_NEXT`>> macro.
 **/
static HASH_NODE* HASH_PREFIX(find_next)(TAC P(node) *start)
{
#ifndef HASH_CONSERVE_SPACE
  uns h0 = P(hash) (TTC HASH_KEY(start->));
#endif
  P(bucket) *b = SKIP_BACK(P(bucket), n, start);

  for (b=b->next; b; b=b->next)
    {
      if (
#ifndef HASH_CONSERVE_SPACE
	  b->hash == h0 &&
#endif
	  P(eq)(TTC HASH_KEY(start->), HASH_KEY(b->n.)))
	return &b->n;
    }
  return NULL;
}
#endif

#ifdef HASH_WANT_NEW
/**
 * Generates a new node with a given key.
 *
 * Enabled by the <<want_new,`HASH_WANT_NEW`>> macro.
 **/
static HASH_NODE * HASH_PREFIX(new)(TAC HASH_KEY_DECL)
{
  uns h0, h;
  P(bucket) *b;

  h0 = P(hash) (TTC HASH_KEY( ));
  h = h0 % T.hash_size;
  b = P(new_bucket) (TTC sizeof(struct P(bucket)) + HASH_EXTRA_SIZE(HASH_KEY( )));
  b->next = T.ht[h];
  T.ht[h] = b;
#ifndef HASH_CONSERVE_SPACE
  b->hash = h0;
#endif
  P(init_key)(TTC &b->n, HASH_KEY( ));
  P(init_data)(TTC &b->n);
  if (T.hash_count++ >= T.hash_max)
    P(rehash)(TTC 2*T.hash_size);
  return &b->n;
}
#endif

#ifdef HASH_WANT_LOOKUP
/**
 * Finds a node with a given key. If it does not exist, a new one is created.
 * It is strongly recommended to use <<give_init_data,`HASH_GIVE_INIT_DATA`>>.
 *
 * This one is enabled by the <<want_lookup,`HASH_WANT_LOOKUP`>> macro.
 **/
static HASH_NODE* HASH_PREFIX(lookup)(TAC HASH_KEY_DECL)
{
  uns h0 = P(hash) (TTC HASH_KEY( ));
  uns h = h0 % T.hash_size;
  P(bucket) *b;

  for (b=T.ht[h]; b; b=b->next)
    {
      if (
#ifndef HASH_CONSERVE_SPACE
	  b->hash == h0 &&
#endif
	  P(eq)(TTC HASH_KEY( ), HASH_KEY(b->n.)))
	return &b->n;
    }

  b = P(new_bucket) (TTC sizeof(struct P(bucket)) + HASH_EXTRA_SIZE(HASH_KEY( )));
  b->next = T.ht[h];
  T.ht[h] = b;
#ifndef HASH_CONSERVE_SPACE
  b->hash = h0;
#endif
  P(init_key)(TTC &b->n, HASH_KEY( ));
  P(init_data)(TTC &b->n);
  if (T.hash_count++ >= T.hash_max)
    P(rehash)(TTC 2*T.hash_size);
  return &b->n;
}
#endif

#ifdef HASH_WANT_DELETE
/**
 * Removes a node with the given key from hash table and deallocates it.
 *
 * Success is returned.
 *
 * This one is enabled by <<want_delete,`HASH_WANT_DELETE`>> macro.
 **/
static int HASH_PREFIX(delete)(TAC HASH_KEY_DECL)
{
  uns h0 = P(hash) (TTC HASH_KEY( ));
  uns h = h0 % T.hash_size;
  P(bucket) *b, **bb;

  for (bb=&T.ht[h]; b=*bb; bb=&b->next)
    {
      if (
#ifndef HASH_CONSERVE_SPACE
	  b->hash == h0 &&
#endif
	  P(eq)(TTC HASH_KEY( ), HASH_KEY(b->n.)))
	{
	  *bb = b->next;
	  P(free)(TTC b);
	  if (--T.hash_count < T.hash_min)
	    P(rehash)(TTC T.hash_size/2);
	  return 1;
	}
    }
  return 0;
}
#endif

#ifdef HASH_WANT_REMOVE
/**
 * Removes a given node and deallocates it.
 * It differs from <<fun__GENERIC_LINK|HASH_PREFIX|delete,`HASH_PREFIX(delete)()`>>
 * in its type of parameter -- this one deletes a specific node, that one looks for it by a key.
 *
 * Enabled by <<want_remove,`HASH_WANT_REMOVE`>> macro.
 **/
static void HASH_PREFIX(remove)(TAC HASH_NODE *n)
{
  P(bucket) *x = SKIP_BACK(struct P(bucket), n, n);
  uns h0 = P(bucket_hash)(TTC x);
  uns h = h0 % T.hash_size;
  P(bucket) *b, **bb;

  for (bb=&T.ht[h]; (b=*bb) && b != x; bb=&b->next)
    ;
  ASSERT(b);
  *bb = b->next;
  P(free)(TTC b);
  if (--T.hash_count < T.hash_min)
    P(rehash)(TTC T.hash_size/2);
}
#endif

/* And the iterator */

#ifndef HASH_FOR_ALL

#define HASH_FOR_ALL_DYNAMIC(h_px, h_table, h_var)					\
do {											\
  uns h_slot;										\
  struct GLUE_(h_px,bucket) *h_buck;							\
  for (h_slot=0; h_slot < (h_table)->hash_size; h_slot++)				\
    for (h_buck = (h_table)->ht[h_slot]; h_buck; h_buck = h_buck->next)			\
      {											\
	GLUE_(h_px,node) *h_var = &h_buck->n;
#define HASH_FOR_ALL(h_px, h_var) HASH_FOR_ALL_DYNAMIC(h_px, &GLUE_(h_px,table), h_var)
#define HASH_END_FOR } } while(0)
#define HASH_BREAK
#define HASH_CONTINUE continue

#endif

/* Finally, undefine all the parameters */

#undef P
#undef T
#undef TA
#undef TAC
#undef TAU
#undef TAUC
#undef TT
#undef TTC

#undef HASH_ATOMIC_TYPE
#undef HASH_CONSERVE_SPACE
#undef HASH_DEFAULT_SIZE
#undef HASH_EXTRA_SIZE
#undef HASH_FN_BITS
#undef HASH_GIVE_ALLOC
#undef HASH_GIVE_EQ
#undef HASH_GIVE_EXTRA_SIZE
#undef HASH_GIVE_HASHFN
#undef HASH_GIVE_INIT_DATA
#undef HASH_GIVE_INIT_KEY
#undef HASH_KEY
#undef HASH_KEY_ATOMIC
#undef HASH_KEY_COMPLEX
#undef HASH_KEY_DECL
#undef HASH_KEY_ENDSTRING
#undef HASH_KEY_STRING
#undef HASH_KEY_MEMORY
#undef HASH_KEY_SIZE
#undef HASH_NOCASE
#undef HASH_NODE
#undef HASH_PREFIX
#undef HASH_USE_POOL
#undef HASH_AUTO_POOL
#undef HASH_WANT_CLEANUP
#undef HASH_WANT_DELETE
#undef HASH_WANT_FIND
#undef HASH_WANT_FIND_NEXT
#undef HASH_WANT_LOOKUP
#undef HASH_WANT_NEW
#undef HASH_WANT_REMOVE
#undef HASH_TABLE_ALLOC
#undef HASH_TABLE_DYNAMIC
#undef HASH_ZERO_FILL
