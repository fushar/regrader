/*
 *	UCW Library -- Configuration files: journaling
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
#include "ucw/mempool.h"

#include <string.h>

static struct old_pools {
  struct old_pools *prev;
  struct mempool *pool;
} *pools;			// link-list of older cf_pool's

uns cf_need_journal = 1;	// some programs do not need journal
static struct cf_journal_item {
  struct cf_journal_item *prev;
  byte *ptr;
  uns len;
  byte copy[0];
} *journal;

void
cf_journal_block(void *ptr, uns len)
{
  if (!cf_need_journal)
    return;
  struct cf_journal_item *ji = cf_malloc(sizeof(struct cf_journal_item) + len);
  ji->prev = journal;
  ji->ptr = ptr;
  ji->len = len;
  memcpy(ji->copy, ptr, len);
  journal = ji;
}

void
cf_journal_swap(void)
  // swaps the contents of the memory and the journal, and reverses the list
{
  struct cf_journal_item *curr, *prev, *next;
  for (next=NULL, curr=journal; curr; next=curr, curr=prev)
  {
    prev = curr->prev;
    curr->prev = next;
    for (uns i=0; i<curr->len; i++)
    {
      byte x = curr->copy[i];
      curr->copy[i] = curr->ptr[i];
      curr->ptr[i] = x;
    }
  }
  journal = next;
}

struct cf_journal_item *
cf_journal_new_transaction(uns new_pool)
{
  if (new_pool)
    cf_pool = mp_new(1<<10);
  struct cf_journal_item *oldj = journal;
  journal = NULL;
  return oldj;
}

void
cf_journal_commit_transaction(uns new_pool, struct cf_journal_item *oldj)
{
  if (new_pool)
  {
    struct old_pools *p = cf_malloc(sizeof(struct old_pools));
    p->prev = pools;
    p->pool = cf_pool;
    pools = p;
  }
  if (oldj)
  {
    struct cf_journal_item **j = &journal;
    while (*j)
      j = &(*j)->prev;
    *j = oldj;
  }
}

void
cf_journal_rollback_transaction(uns new_pool, struct cf_journal_item *oldj)
{
  if (!cf_need_journal)
    die("Cannot rollback the configuration, because the journal is disabled.");
  cf_journal_swap();
  journal = oldj;
  if (new_pool)
  {
    mp_delete(cf_pool);
    cf_pool = pools ? pools->pool : NULL;
  }
}

void
cf_journal_delete(void)
{
  for (struct old_pools *p=pools; p; p=pools)
  {
    pools = p->prev;
    mp_delete(p->pool);
  }
}

/* TODO: more space efficient journal */
