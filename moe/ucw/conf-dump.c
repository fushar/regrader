/*
 *	UCW Library -- Configuration files: dumping
 *
 *	(c) 2001--2006 Robert Spalek <robert@ucw.cz>
 *	(c) 2003--2006 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/conf.h"
#include "ucw/getopt.h"
#include "ucw/conf-internal.h"
#include "ucw/clists.h"
#include "ucw/fastbuf.h"

static void
spaces(struct fastbuf *fb, uns nr)
{
  for (uns i=0; i<nr; i++)
    bputs(fb, "  ");
}

static void
dump_basic(struct fastbuf *fb, void *ptr, enum cf_type type, union cf_union *u)
{
  switch (type) {
    case CT_INT:	bprintf(fb, "%d ", *(uns*)ptr); break;
    case CT_U64:	bprintf(fb, "%llu ", (long long) *(u64*)ptr); break;
    case CT_DOUBLE:	bprintf(fb, "%lg ", *(double*)ptr); break;
    case CT_IP:		bprintf(fb, "%08x ", *(uns*)ptr); break;
    case CT_STRING:
      if (*(char**)ptr)
	bprintf(fb, "'%s' ", *(char**)ptr);
      else
	bprintf(fb, "NULL ");
      break;
    case CT_LOOKUP:	bprintf(fb, "%s ", *(int*)ptr >= 0 ? u->lookup[ *(int*)ptr ] : "???"); break;
    case CT_USER:
      if (u->utype->dumper)
	u->utype->dumper(fb, ptr);
      else
	bprintf(fb, "??? ");
      break;
  }
}

static void dump_section(struct fastbuf *fb, struct cf_section *sec, int level, void *ptr);

static char *class_names[] = { "end", "static", "dynamic", "parser", "section", "list", "bitmap" };

static void
dump_item(struct fastbuf *fb, struct cf_item *item, int level, void *ptr)
{
  ptr += (uintptr_t) item->ptr;
  enum cf_type type = item->type;
  uns size = cf_type_size(item->type, item->u.utype);
  int i;
  spaces(fb, level);
  bprintf(fb, "%s: C%s #", item->name, class_names[item->cls]);
  if (item->number == CF_ANY_NUM)
    bputs(fb, "any ");
  else
    bprintf(fb, "%d ", item->number);
  if (item->cls == CC_STATIC || item->cls == CC_DYNAMIC || item->cls == CC_BITMAP) {
    bprintf(fb, "T%s ", cf_type_names[type]);
    if (item->type == CT_USER)
      bprintf(fb, "U%s S%d ", item->u.utype->name, size);
  }
  if (item->cls == CC_STATIC) {
    for (i=0; i<item->number; i++)
      dump_basic(fb, ptr + i * size, type, &item->u);
  } else if (item->cls == CC_DYNAMIC) {
    ptr = * (void**) ptr;
    if (ptr) {
      int real_nr = DARY_LEN(ptr);
      bprintf(fb, "N%d ", real_nr);
      for (i=0; i<real_nr; i++)
	dump_basic(fb, ptr + i * size, type, &item->u);
    } else
      bprintf(fb, "NULL ");
  } else if (item->cls == CC_BITMAP) {
    u32 mask = * (u32*) ptr;
    for (i=0; i<32; i++) {
      if (item->type == CT_LOOKUP && !item->u.lookup[i])
	break;
      if (mask & (1<<i)) {
	if (item->type == CT_INT)
	  bprintf(fb, "%d ", i);
	else if (item->type == CT_LOOKUP)
	  bprintf(fb, "%s ", item->u.lookup[i]);
      }
    }
  }
  bputc(fb, '\n');
  if (item->cls == CC_SECTION)
    dump_section(fb, item->u.sec, level+1, ptr);
  else if (item->cls == CC_LIST) {
    uns idx = 0;
    CLIST_FOR_EACH(cnode *, n, * (clist*) ptr) {
      spaces(fb, level+1);
      bprintf(fb, "item %d\n", ++idx);
      dump_section(fb, item->u.sec, level+2, n);
    }
  }
}

static void
dump_section(struct fastbuf *fb, struct cf_section *sec, int level, void *ptr)
{
  spaces(fb, level);
  bprintf(fb, "S%d F%x:\n", sec->size, sec->flags);
  for (struct cf_item *item=sec->cfg; item->cls; item++)
    dump_item(fb, item, level, ptr);
}

void
cf_dump_sections(struct fastbuf *fb)
{
  dump_section(fb, &cf_sections, 0, NULL);
}

