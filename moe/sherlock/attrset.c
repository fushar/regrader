/*
 *	Sherlock Library -- Parsing Attribute Sets
 *
 *	(c) 2006 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "sherlock/sherlock.h"
#include "sherlock/object.h"
#include "sherlock/attrset.h"
#include "ucw/clists.h"
#include "sherlock/conf.h"

struct attr_node {
  cnode n;
  uns attr;
};

struct cf_section attr_set_cf = {
  CF_TYPE(struct attr_node),
  CF_ITEMS {
    CF_USER("Attr", PTR_TO(struct attr_node, attr), &cf_type_attr),
    CF_END
  }
};

struct cf_section attr_set_cf_sub = {
  CF_TYPE(struct attr_node),
  CF_ITEMS {
    CF_USER("Attr", PTR_TO(struct attr_node, attr), &cf_type_attr_sub),
    CF_END
  }
};

void
attr_set_commit(struct attr_set *set, clist *l)
{
  CF_JOURNAL_VAR(*set);
  bit_array_zero(set->a, ATTR_SET_SIZE);
  CLIST_FOR_EACH(struct attr_node *, n, *l)
    bit_array_set(set->a, n->attr);
}
