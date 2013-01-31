/*
 *	Sherlock Library -- Sets of object attributes
 *
 *	(c) 2006 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _SHERLOCK_ATTRSET_H
#define _SHERLOCK_ATTRSET_H

#include "ucw/bitarray.h"
#include "sherlock/object.h"

COMPILE_ASSERT(son_value, OBJ_ATTR_SON == 256);
#define ATTR_SET_SIZE 512

struct attr_set {
  BIT_ARRAY(a, ATTR_SET_SIZE);
};

static inline uns
attr_set_match(struct attr_set *set, struct oattr *attr)
{
  return bit_array_isset(set->a, attr->attr);
}

/* Configuration helpers */

extern struct cf_section attr_set_cf, attr_set_cf_sub;

struct clist;
void attr_set_commit(struct attr_set *set, struct clist *list);

#endif
