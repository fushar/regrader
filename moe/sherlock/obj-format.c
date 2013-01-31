/*
 *	Sherlock Library -- Adding Formatted Attributes
 *
 *	(c) 2005 Martin Mares <mj@ucw.cz>
 *	(c) 2007 Pavel Charvat <pchar@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "sherlock/sherlock.h"
#include "sherlock/object.h"
#include "ucw/stkstring.h"

#include <stdio.h>

struct oattr *
obj_add_attr_vformat(struct odes *o, uns x, char *fmt, va_list args)
{
  return obj_add_attr(o, x, stk_vprintf(fmt, args));
}

struct oattr *obj_add_attr_format(struct odes *o, uns x, char *fmt, ...)
{
  va_list va;
  va_start(va, fmt);
  struct oattr *a = obj_add_attr_vformat(o, x, fmt, va);
  va_end(va);
  return a;
}

struct oattr *
obj_set_attr_vformat(struct odes *o, uns x, char *fmt, va_list args)
{
  return obj_set_attr(o, x, stk_vprintf(fmt, args));
}

struct oattr *obj_set_attr_format(struct odes *o, uns x, char *fmt, ...)
{
  va_list va;
  va_start(va, fmt);
  struct oattr *a = obj_set_attr_vformat(o, x, fmt, va);
  va_end(va);
  return a;
}
