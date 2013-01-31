/*
 *	UCW Library -- Single-Linked Lists
 *
 *	(c) 2005 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/slists.h"

static inline snode *
slist_raw_prev(slist *l, snode *n)
{
  snode *m = &l->head;
  while (m)
    {
      if (n == m->next)
	return m;
      m = m->next;
    }
  ASSERT(0);
}

void *
slist_prev(slist *l, snode *n)
{
  snode *p = slist_raw_prev(l, n);
  return (p == &l->head) ? NULL : p;
}

void
slist_insert_before(slist *l, snode *what, snode *before)
{
  what->next = before;
  slist_raw_prev(l, before)->next = what;
}

void
slist_remove(slist *l, snode *n)
{
  if (n)
    {
      snode *p = slist_raw_prev(l, n);
      slist_remove_after(l, p);
    }
}

#ifdef TEST

#include <stdio.h>
#include <alloca.h>

int main(void)
{
  slist l;

  struct x {
    snode n;
    int val;
  };

  slist_init(&l);
  for (int i=1; i<=10; i++)
    {
      struct x *x = alloca(sizeof(*x));
      x->val = i;
      if (i % 2)
	slist_add_head(&l, &x->n);
      else
	slist_add_tail(&l, &x->n);
    }

  struct x *x, *prev;
  SLIST_WALK_DELSAFE(x, l, prev)
    if (x->val == 5)
      slist_remove_after(&l, &prev->n);
    else if (x->val == 6)
      slist_remove(&l, &x->n);
  SLIST_FOR_EACH(struct x *, x, l)
    printf("%d/", x->val);
  putchar('\n');
}

#endif
