/*
 *	UCW Library -- Linked Lists of Simple Items
 *
 *	(c) 2006 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/mempool.h"
#include "ucw/conf.h"
#include "ucw/simple-lists.h"

simp_node *
simp_append(struct mempool *mp, clist *l)
{
  simp_node *n = mp_alloc_fast(mp, sizeof(*n));
  clist_add_tail(l, &n->n);
  return n;
}

simp2_node *
simp2_append(struct mempool *mp, clist *l)
{
  simp2_node *n = mp_alloc_fast(mp, sizeof(*n));
  clist_add_tail(l, &n->n);
  return n;
}

/* Configuration sections for common lists */

struct cf_section cf_string_list_config = {
  CF_TYPE(simp_node),
  CF_ITEMS {
    CF_STRING("String", PTR_TO(simp_node, s)),
    CF_END
  }
};

struct cf_section cf_2string_list_config = {
  CF_TYPE(simp2_node),
  CF_ITEMS {
    CF_STRING("Src", PTR_TO(simp2_node, s1)),
    CF_STRING("Dest", PTR_TO(simp2_node, s2)),
    CF_END
  }
};
