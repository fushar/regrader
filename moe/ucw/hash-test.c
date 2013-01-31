/* Tests for hash table routines */

#include "ucw/lib.h"
#include "ucw/mempool.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* TEST 1: integers */

struct node1 {
  int key;
  int data;
};

#define HASH_NODE struct node1
#define HASH_PREFIX(x) test1_##x
#define HASH_KEY_ATOMIC key
#define HASH_ATOMIC_TYPE int
#define HASH_ZERO_FILL

#define HASH_GIVE_INIT_DATA
static inline void test1_init_data(struct node1 *n)
{
  n->data = n->key + 123;
}

#define HASH_WANT_FIND
#define HASH_WANT_LOOKUP
#define HASH_WANT_REMOVE

#include "ucw/hashtable.h"

static void test1(void)
{
  int i;

  test1_init();
  for (i=0; i<1024; i++)
    {
      struct node1 *n = test1_lookup(i);
      ASSERT(n->data == i+123);
    }
  for (i=1; i<1024; i+=2)
    {
      struct node1 *n = test1_lookup(i);
      test1_remove(n);
    }
  for (i=0; i<1024; i++)
    {
      struct node1 *n = test1_find(i);
      if (!n != (i&1) || (n && n->data != i+123))
	die("Inconsistency at i=%d", i);
    }
  i=0;
  HASH_FOR_ALL(test1, n)
    {
      i += 1 + n->key;
    }
  HASH_END_FOR;
  ASSERT(i == 262144);
  puts("OK");
}

/* TEST 2: external strings */

struct node2 {
  char *key;
  int data;
};

#define HASH_NODE struct node2
#define HASH_PREFIX(x) test2_##x
#define HASH_KEY_STRING key
#define HASH_NOCASE
#define HASH_AUTO_POOL 4096

#define HASH_WANT_FIND
#define HASH_WANT_NEW

#include "ucw/hashtable.h"

static void test2(void)
{
  int i;

  test2_init();
  for (i=0; i<1024; i+=2)
    {
      char x[32];
      sprintf(x, "abc%d", i);
      test2_new(xstrdup(x));
    }
  for (i=0; i<1024; i++)
    {
      char x[32];
      struct node2 *n;
      sprintf(x, "ABC%d", i);
      n = test2_find(x);
      if (!n != (i&1))
	die("Inconsistency at i=%d", i);
    }
  puts("OK");
}

/* TEST 3: internal strings + pools */

static struct mempool *pool3;

struct node3 {
  int data;
  char key[1];
};

#define HASH_NODE struct node3
#define HASH_PREFIX(x) test3_##x
#define HASH_KEY_ENDSTRING key

#define HASH_WANT_FIND
#define HASH_WANT_NEW

#define HASH_USE_POOL pool3

#include "ucw/hashtable.h"

static void test3(void)
{
  int i;

  pool3 = mp_new(16384);
  test3_init();
  for (i=0; i<1048576; i+=2)
    {
      char x[32];
      sprintf(x, "abc%d", i);
      test3_new(x);
    }
  for (i=0; i<1048576; i++)
    {
      char x[32];
      struct node3 *n;
      sprintf(x, "abc%d", i);
      n = test3_find(x);
      if (!n != (i&1))
	die("Inconsistency at i=%d", i);
    }
  puts("OK");
}

/* TEST 4: complex keys */

#include "ucw/hashfunc.h"

struct node4 {
  int port;
  int data;
  char host[1];
};

#define HASH_NODE struct node4
#define HASH_PREFIX(x) test4_##x
#define HASH_KEY_COMPLEX(x) x host, x port
#define HASH_KEY_DECL char *host, int port

#define HASH_WANT_CLEANUP
#define HASH_WANT_FIND
#define HASH_WANT_NEW
#define HASH_WANT_LOOKUP
#define HASH_WANT_DELETE
#define HASH_WANT_REMOVE

#define HASH_GIVE_HASHFN
static uns test4_hash(char *host, int port)
{
  return hash_string_nocase(host) ^ hash_u32(port);
}

#define HASH_GIVE_EQ
static inline int test4_eq(char *host1, int port1, char *host2, int port2)
{
  return !strcasecmp(host1,host2) && port1 == port2;
}

#define HASH_GIVE_EXTRA_SIZE
static inline uns test4_extra_size(char *host, int port UNUSED)
{
  return strlen(host);
}

#define HASH_GIVE_INIT_KEY
static inline void test4_init_key(struct node4 *n, char *host, int port)
{
  strcpy(n->host, host);
  n->port = port;
}

#include "ucw/hashtable.h"

static void test4(void)
{
  int i;
  char x[32];
  struct node4 *n;

  test4_init();
  for (i=0; i<1024; i++)
    if ((i % 3) == 0)
      {
	sprintf(x, "abc%d", i);
	n = test4_new(x, i%10);
	n->data = i;
      }
  for (i=0; i<1024; i++)
    {
      sprintf(x, "abc%d", i);
      n = test4_lookup(x, i%10);
      n->data = i;
    }
  for (i=0; i<1024; i++)
    if (i % 2)
      {
	sprintf(x, "aBc%d", i);
	if ((i % 7) < 3)
	  {
	    n = test4_find(x, i%10);
	    ASSERT(n);
	    test4_remove(n);
	  }
	else
	  test4_delete(x, i%10);
      }
  for (i=0; i<1024; i++)
    {
      sprintf(x, "ABC%d", i);
      n = test4_find(x, i%10);
      if (!n != (i&1) || (n && n->data != i))
	die("Inconsistency at i=%d", i);
    }
  test4_cleanup();
  puts("OK");
}

/* TEST 5: integers again, but this time dynamically */

struct node5 {
  int key;
  int data;
};

#define HASH_NODE struct node5
#define HASH_PREFIX(x) test5_##x
#define HASH_KEY_ATOMIC key
#define HASH_ATOMIC_TYPE int
#define HASH_TABLE_DYNAMIC

struct test5_table;

#define HASH_GIVE_INIT_DATA
static inline void test5_init_data(struct test5_table *table UNUSED, struct node5 *n)
{
  n->data = n->key + 123;
}

#define HASH_WANT_FIND
#define HASH_WANT_NEW
#define HASH_WANT_DELETE

#include "ucw/hashtable.h"

static void test5(void)
{
  int i;
  struct test5_table tab;

  test5_init(&tab);
  for (i=0; i<1024; i++)
    {
      struct node5 *n = test5_new(&tab, i);
      ASSERT(n->data == i+123);
    }
  for (i=1; i<1024; i+=2)
    test5_delete(&tab, i);
  for (i=0; i<1024; i++)
    {
      struct node5 *n = test5_find(&tab, i);
      if (!n != (i&1) || (n && n->data != i+123))
	die("Inconsistency at i=%d", i);
    }
  i=0;
  HASH_FOR_ALL_DYNAMIC(test5, &tab, n)
    i += 1 + n->key;
  HASH_END_FOR;
  ASSERT(i == 262144);
  puts("OK");
}

int
main(int argc, char **argv)
{
  uns m = ~0U;
  if (argc > 1)
    {
      m = 0;
      for (int i=1; i<argc; i++)
	m |= 1 << atol(argv[i]);
    }
  if (m & (1 << 1))
    test1();
  if (m & (1 << 2))
    test2();
  if (m & (1 << 3))
    test3();
  if (m & (1 << 4))
    test4();
  if (m & (1 << 5))
    test5();
  return 0;
}
