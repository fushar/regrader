/*
 *	UCW Library -- Byte-based trie -- Testing utility
 *
 *	(c) 2008 Pavel Charvat <pchar@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#undef LOCAL_DEBUG

#include "ucw/lib.h"
#include "ucw/getopt.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TRIE_PREFIX(x) basic_##x
#define TRIE_NODE_TYPE char
#define TRIE_WANT_CLEANUP
#define TRIE_WANT_ADD
#define TRIE_WANT_DELETE
#define TRIE_WANT_FIND
#define TRIE_WANT_AUDIT
#ifdef LOCAL_DEBUG
#define TRIE_TRACE
#endif
#include "ucw/trie.h"

static void
basic_test(void)
{
  basic_init();
  basic_add("str1");
  basic_add("str2");
  if (!basic_find("str1") || !basic_find("str2") || basic_find("x") || basic_find("str123"))
    ASSERT(0);
  basic_audit();
  basic_delete("str1");
  if (basic_find("str1") || !basic_find("str2"))
    ASSERT(0);
  basic_audit();
  basic_cleanup();
}

#define TRIE_PREFIX(x) dynamic_##x
#define TRIE_NODE_TYPE char
#define TRIE_REV
#define TRIE_DYNAMIC
#define TRIE_WANT_CLEANUP
#define TRIE_WANT_ADD
#define TRIE_WANT_FIND
#define TRIE_WANT_AUDIT
#ifdef LOCAL_DEBUG
#define TRIE_TRACE
#endif
#include "ucw/trie.h"

static void
dynamic_test(void)
{
  struct dynamic_trie trie1, trie2;
  dynamic_init(&trie1);
  dynamic_init(&trie2);
  dynamic_add(&trie1, "str1");
  dynamic_add(&trie2, "str2");
  if (!dynamic_find(&trie1, "str1") || dynamic_find(&trie1, "str2") || !dynamic_find(&trie2, "str2"))
    ASSERT(0);
  dynamic_audit(&trie1);
  dynamic_audit(&trie2);
  dynamic_cleanup(&trie1);
  dynamic_cleanup(&trie2);
}


#define TRIE_PREFIX(x) random_##x
#define TRIE_NODE_TYPE char
#define TRIE_LEN_TYPE u16
#undef TRIE_REV
#define TRIE_WANT_CLEANUP
#define TRIE_WANT_FIND
#define TRIE_WANT_ADD
#define TRIE_WANT_REMOVE
#define TRIE_WANT_AUDIT
#ifdef LOCAL_DEBUG
#define TRIE_TRACE
#endif
#include "ucw/trie.h"

#define MAX_STRINGS 200

static uns count;
static char *str[MAX_STRINGS];

static char *
gen_string(void)
{
  uns l = random_max(11);
  char *s = xmalloc(l + 1);
  for (uns i = 0; i < l; i++)
    s[i] = random_max('z' - 'a') + 'a';
  s[l] = 0;
  return s;
}

static char *
gen_unique_string(void)
{
  char *s;
again:
  s = gen_string();
  for (uns i = 0; i < count; i++)
    if (!strcmp(s, str[i]))
      {
	xfree(s);
        goto again;
      }
  return s;
}

static void
insert(void)
{
  if (count == MAX_STRINGS)
    return;
  char *s = gen_unique_string();
  str[count++] = s;
  DBG("add '%s'", s);
  random_add(s);
  random_audit();
}

static void
delete(void)
{
  if (!count)
    return;
  uns i = random_max(count);
  DBG("remove '%s'", str[i]);
  random_remove(str[i]);
  random_audit();
  xfree(str[i]);
  str[i] = str[--count];
}

static void
find(void)
{
  if (!count || !random_max(4))
    {
      char *s = gen_unique_string();
      DBG("negative find '%s'", s);
      if (random_find(s))
	ASSERT(0);
      xfree(s);
    }
  else
    {
      uns i = random_max(count);
      DBG("positive find '%s'", str[i]);
      if (random_find(str[i]) != str[i])
	ASSERT(0);
    }
}

static void
reset(void)
{
  DBG("reset");
  random_cleanup();
  for (uns i = 0; i < count; i++)
    xfree(str[i]);
  count = 0;
  random_init();
  random_audit();
}

static void
random_test(void)
{
  random_init();
  for (uns i = 0; i < 10000; i++)
    {
      int r = random_max(1000);
      if ((r -= 300) < 0)
	insert();
      else if ((r -= 150) < 0)
	delete();
      else if ((r -= 300) < 0)
	find();
      else if ((r -= 1) < 0)
	reset();
    }
  random_cleanup();
}

int main(int argc, char **argv)
{
  log_init(argv[0]);
  if (cf_getopt(argc, argv, CF_SHORT_OPTS, CF_NO_LONG_OPTS, NULL) >= 0 || optind + 1 != argc)
    die("Invalid usage, see the source code");
  srandom(time(NULL));

  char *test = argv[optind];
  if (!strcmp(test, "basic"))
    basic_test();
  else if (!strcmp(test, "dynamic"))
    dynamic_test();
  else if (!strcmp(test, "random"))
    random_test();
  else
    die("Unknown test case");

  return 0;
}
