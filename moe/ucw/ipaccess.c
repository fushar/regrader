/*
 *	UCW Library -- IP address access lists
 *
 *	(c) 1997--2007 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/clists.h"
#include "ucw/conf.h"
#include "ucw/getopt.h"
#include "ucw/fastbuf.h"
#include "ucw/ipaccess.h"

#include <string.h>

struct ipaccess_entry {
  cnode n;
  int allow;
  struct ip_addrmask addr;
};

static char *
addrmask_parser(char *c, void *ptr)
{
  /*
   * This is tricky: addrmasks will be compared by memcmp(), so we must ensure
   * that even the padding between structure members is zeroed out.
   */
  struct ip_addrmask *am = ptr;
  bzero(am, sizeof(*am));

  char *p = strchr(c, '/');
  if (p)
    *p++ = 0;
  char *err = cf_parse_ip(c, &am->addr);
  if (err)
    return err;
  if (p)
    {
      uns len;
      if (!cf_parse_int(p, &len) && len <= 32)
	am->mask = ~(len == 32 ? 0 : ~0U >> len);
      else if (cf_parse_ip(p, &am->mask))
	return "Invalid prefix length or netmask";
    }
  else
    am->mask = ~0U;
  return NULL;
}

static void
addrmask_dumper(struct fastbuf *fb, void *ptr)
{
  struct ip_addrmask *am = ptr;
  bprintf(fb, "%08x/%08x ", am->addr, am->mask);
}

struct cf_user_type ip_addrmask_type = {
  .size = sizeof(struct ip_addrmask),
  .name = "ip_addrmask",
  .parser = addrmask_parser,
  .dumper = addrmask_dumper
};

struct cf_section ipaccess_cf = {
  CF_TYPE(struct ipaccess_entry),
  CF_ITEMS {
    CF_LOOKUP("Mode", PTR_TO(struct ipaccess_entry, allow), ((const char* const []) { "deny", "allow", NULL })),
    CF_USER("IP", PTR_TO(struct ipaccess_entry, addr), &ip_addrmask_type),
    CF_END
  }
};

int ip_addrmask_match(struct ip_addrmask *am, u32 ip)
{
  return !((ip ^ am->addr) & am->mask);
}

int
ipaccess_check(clist *l, u32 ip)
{
  CLIST_FOR_EACH(struct ipaccess_entry *, a, *l)
    if (ip_addrmask_match(&a->addr, ip))
      return a->allow;
  return 0;
}

#ifdef TEST

#include <stdio.h>

static clist t;

static struct cf_section test_cf = {
  CF_ITEMS {
    CF_LIST("A", &t, &ipaccess_cf),
    CF_END
  }
};

int main(int argc, char **argv)
{
  cf_declare_section("T", &test_cf, 0);
  if (cf_getopt(argc, argv, CF_SHORT_OPTS, CF_NO_LONG_OPTS, NULL) != -1)
    die("Invalid arguments");

  byte buf[256];
  while (fgets(buf, sizeof(buf), stdin))
    {
      char *c = strchr(buf, '\n');
      if (c)
	*c = 0;
      u32 ip;
      if (cf_parse_ip(buf, &ip))
	puts("Invalid IP address");
      else if (ipaccess_check(&t, ip))
	puts("Allowed");
      else
	puts("Denied");
    }
  return 0;
}

#endif
