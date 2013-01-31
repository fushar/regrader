/*
 *	UCW Library -- IP address access lists
 *
 *	(c) 1997--2007 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _UCW_IPACCESS_H
#define _UCW_IPACCESS_H

#include "ucw/clists.h"

extern struct cf_section ipaccess_cf;
int ipaccess_check(clist *l, u32 ip);

/* Low-level handling of addresses and masks */

struct ip_addrmask {
  u32 addr;
  u32 mask;
};

extern struct cf_user_type ip_addrmask_type;
int ip_addrmask_match(struct ip_addrmask *am, u32 ip);

#endif
