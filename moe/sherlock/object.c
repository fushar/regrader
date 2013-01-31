/*
 *	Sherlock Library -- Object Functions
 *
 *	(c) 1997--2006 Martin Mares <mj@ucw.cz>
 *	(c) 2004 Robert Spalek <robert@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "sherlock/sherlock.h"
#include "ucw/mempool.h"
#include "ucw/fastbuf.h"
#include "sherlock/object.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void
obj_dump(struct odes *d)
{
  for(struct oattr *a=d->attrs; a; a=a->next)
    for(struct oattr *b=a; b; b=b->same)
      if (a->attr >= OBJ_ATTR_SON)
	{
	  printf("(%c\n", a->attr - OBJ_ATTR_SON);
	  obj_dump(b->son);
	  printf(")\n");
	}
      else
	printf("%c%s\n", (a==b ? a->attr : ' '), b->val);
}

void
obj_dump_indented(struct odes *d, uns indent)
{
  for(struct oattr *a=d->attrs; a; a=a->next)
    for(struct oattr *b=a; b; b=b->same)
      {
	for (uns i=0; i<indent; i++)
	  putchar('\t');
	if (a->attr >= OBJ_ATTR_SON)
	  {
	    printf("(%c\n", a->attr - OBJ_ATTR_SON);
	    obj_dump_indented(b->son, indent+1);
	    for (uns i=0; i<=indent; i++)
	      putchar('\t');
	    printf(")\n");
	  }
	else
	  printf("%c%s\n", (a==b ? a->attr : ' '), b->val);
      }
}

static struct oattr *
oa_new(struct odes *o, uns x, byte *v)
{
  uns l = strlen(v)+1;
  struct oattr *a = mp_alloc(o->pool, sizeof(struct oattr) + l);

  a->next = a->same = NULL;
  a->attr = x;
  a->val = (byte*) (a+1);
  memcpy(a->val, v, l);
  return a;
}

static struct oattr *
oa_new_ref(struct odes *o, uns x, byte *v)
{
  struct oattr *a = mp_alloc(o->pool, sizeof(struct oattr));

  a->next = a->same = NULL;
  a->attr = x;
  a->val = v;
  return a;
}

static struct oattr *
oa_new_son(struct odes *o, uns x, struct odes *son)
{
  struct oattr *a = mp_alloc(o->pool, sizeof(struct oattr));

  a->next = a->same = NULL;
  a->attr = x;
  a->son = son;
  son->parent = o;
  return a;
}

struct odes *
obj_new(struct mempool *pool)
{
  struct odes *o = mp_alloc(pool, sizeof(struct odes));
  o->pool = pool;
  o->attrs = NULL;
  o->cached_attr = NULL;
  o->parent = NULL;
  return o;
}

struct oattr *
obj_find_attr(struct odes *o, uns x)
{
  struct oattr *a;
  for(a=o->attrs; a && a->attr != x; a=a->next)
    ;
  return a;
}

struct oattr *
obj_find_attr_last(struct odes *o, uns x)
{
  struct oattr *a = obj_find_attr(o, x);

  if (a)
    {
      while (a->same)
	a = a->same;
    }
  return a;
}

uns
obj_del_attr(struct odes *o, struct oattr *a)
{
  struct oattr *x, **p;
  uns aa = a->attr;

  o->cached_attr = NULL;
  p = &o->attrs;
  while (x = *p)
    {
      if (x->attr == aa)
	{
	  if (x == a)
	    {
	      if (x->same)
		{
		  x->same->next = x->next;
		  *p = x->same;
		}
	      else
		*p = x->next;
	      return 1;
	    }
	  p = &x->same;
	  while (x = *p)
	    {
	      if (x == a)
		{
		  *p = x->same;
		  return 1;
		}
	      p = &x->same;
	    }
	  return 0;
	}
      p = &x->next;
    }
  return 0;
}

byte *
obj_find_aval(struct odes *o, uns x)
{
  struct oattr *a = obj_find_attr(o, x);
  return a ? a->val : NULL;
}

uns
obj_find_anum(struct odes *o, uns x, uns def)
{
  struct oattr *a = obj_find_attr(o, x);
  return a ? (uns)atol(a->val) : def;
}

u32
obj_find_x32(struct odes *o, uns x, u32 def)
{
  struct oattr *a = obj_find_attr(o, x);
  return a ? (u32)strtoul(a->val, NULL, 16) : def;
}

u64
obj_find_x64(struct odes *o, uns x, u64 def)
{
  struct oattr *a = obj_find_attr(o, x);
  return a ? (u64)strtoull(a->val, NULL, 16) : def;
}

struct oattr *
obj_set_attr(struct odes *o, uns x, byte *v)
{
  struct oattr *a, **z;

  z = &o->attrs;
  while (a = *z)
    {
      if (a->attr == x)
	{
	  *z = a->next;
	  goto set;
	}
      z = &a->next;
    }

 set:
  if (v)
    {
      a = oa_new(o, x, v);
      a->next = o->attrs;
      o->attrs = a;
    }
  else
    a = NULL;
  o->cached_attr = a;
  return a;
}

struct oattr *
obj_set_attr_num(struct odes *o, uns a, uns v)
{
  byte x[32];

  sprintf(x, "%d", v);
  return obj_set_attr(o, a, x);
}

static inline struct oattr *
obj_link_attr(struct odes *o, struct oattr *b)
{
  struct oattr *a, **z;

  if (!(a = o->cached_attr) || a->attr != b->attr)
    {
      z = &o->attrs;
      while ((a = *z) && a->attr != b->attr)
	z = &a->next;
      if (!a)
	{
	  *z = b;
	  /* b->next is NULL */
	  goto done;
	}
    }
  while (a->same)
    a = a->same;
  a->same = b;
 done:
  o->cached_attr = b;
  return b;
}

struct oattr *
obj_add_attr(struct odes *o, uns x, byte *v)
{
  return obj_link_attr(o, oa_new(o, x, v));
}

struct oattr *
obj_add_attr_ref(struct odes *o, uns x, byte *v)
{
  return obj_link_attr(o, oa_new_ref(o, x, v));
}

struct oattr *
obj_add_attr_num(struct odes *o, uns a, uns v)
{
  byte x[32];

  sprintf(x, "%d", v);
  return obj_add_attr(o, a, x);
}

struct oattr *
obj_add_attr_son(struct odes *o, uns x, struct odes *son)
{
  return obj_link_attr(o, oa_new_son(o, x, son));
}

struct oattr *
obj_prepend_attr(struct odes *o, uns x, byte *v)
{
  struct oattr *a, *b, **z;

  b = oa_new(o, x, v);
  z = &o->attrs;
  while (a = *z)
    {
      if (a->attr == x)
	{
	  b->same = a;
	  b->next = a->next;
	  a->next = NULL;
	  *z = b;
	  return b;
	}
      z = &a->next;
    }
  b->next = o->attrs;
  o->attrs = b;
  return b;
}

struct oattr *
obj_insert_attr(struct odes *o, struct oattr *first, struct oattr *after, byte *v)
{
  struct oattr *b = oa_new(o, first->attr, v);
  b->same = after->same;
  after->same = b;
  return b;
}

void
obj_move_attr_to_head(struct odes *o, uns x)
{
  struct oattr *a, **z;

  z = &o->attrs;
  while (a = *z)
    {
      if (a->attr == x)
	{
	  *z = a->next;
	  a->next = o->attrs;
	  o->attrs = a;
	  break;
	}
      z = &a->next;
    }
}

void
obj_move_attr_to_tail(struct odes *o, uns x)
{
  struct oattr *a, **z;

  z = &o->attrs;
  while (a = *z)
    {
      if (a->attr == x)
	{
	  *z = a->next;
	  while (*z)
	    z = &(*z)->next;
	  *z = a;
	  a->next = NULL;
	  break;
	}
      z = &a->next;
    }
}

struct odes *
obj_find_son(struct odes *o, uns x)
{
  ASSERT(x >= OBJ_ATTR_SON);
  struct oattr *a = obj_find_attr(o, x);
  return a ? a->son : NULL;
}

struct oattr *
obj_add_son_ref(struct odes *o, uns x, struct odes *son)
{
  struct oattr *oa = oa_new_son(o, x, son);
  obj_link_attr(o, oa);
  return oa;
}

struct odes *
obj_add_son(struct odes *o, uns x)
{
  struct odes *son = obj_new(o->pool);
  obj_add_son_ref(o, x, son);
  return son;
}

static void obj_clone_attr_list(struct odes *dest, struct odes *src);

static struct oattr *
obj_clone_attr(struct odes *dest, struct oattr *a)
{
  struct oattr *res = NULL, **rr = &res;
  if (a->attr < OBJ_ATTR_SON)
    {
      for (; a; a=a->same)
	{
	  *rr = oa_new(dest, a->attr, a->val);
	  rr = &(*rr)->same;
	}
    }
  else
    {
      for (; a; a=a->same)
	{
	  struct odes *dson = obj_new(dest->pool);
	  *rr = oa_new_son(dest, a->attr, dson);
	  rr = &(*rr)->same;
	  obj_clone_attr_list(dson, a->son);
	}
    }
  return res;
}

static void
obj_clone_attr_list(struct odes *dest, struct odes *src)
{
  struct oattr **p = &dest->attrs;
  for (struct oattr *a = src->attrs; a; a=a->next)
    {
      *p = obj_clone_attr(dest, a);
      p = &(*p)->next;
    }
}

void
obj_add_attr_clone(struct odes *o, struct oattr *a)
{
  obj_link_attr(o, obj_clone_attr(o, a));
}

struct odes *
obj_clone(struct mempool *pool, struct odes *src)
{
  struct odes *dest = obj_new(pool);
  obj_clone_attr_list(dest, src);
  return dest;
}
