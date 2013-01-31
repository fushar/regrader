/*
 *	Sherlock Library -- Nested Object Reading Functions
 *
 *	(c) 2005 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _SHERLOCK_OBJREAD_H
#define _SHERLOCK_OBJREAD_H

#include "sherlock/object.h"

struct obj_read_state {
  struct odes *root, *obj;
  void (*error_callback)(struct obj_read_state *st, char *msg);
  void *user;
  int errors;
};

void default_obj_read_error(struct obj_read_state *st, char *msg);

static inline void
obj_read_start(struct obj_read_state *st, struct odes *obj)
{
  st->root = st->obj = obj;
  st->errors = 0;
  st->error_callback = default_obj_read_error;
}

static inline void
obj_read_push(struct obj_read_state *st, byte *ptr)
{
  if (unlikely(!ptr[0] || ptr[1]))
    {
      if (!st->errors++)
	st->error_callback(st, "Malformed object: bad `(' attribute");
    }
  else
    st->obj = obj_add_son(st->obj, ptr[0] + OBJ_ATTR_SON);
}

static inline void
obj_read_pop(struct obj_read_state *st, byte *ptr)
{
  if (unlikely(ptr[0]))
    {
      if (!st->errors++)
	st->error_callback(st, "Malformed object: bad ')' attribute");
    }
  else if (unlikely(!st->obj->parent))
    {
      if (!st->errors++)
	st->error_callback(st, "Malformed object: improper nesting of `( ... )' blocks");
    }
  else
    st->obj = st->obj->parent;
}

static inline void
obj_read_attr(struct obj_read_state *st, uns type, byte *val)
{
  if (type == '(')
    obj_read_push(st, val);
  else if (type == ')')
    obj_read_pop(st, val);
  else
    obj_add_attr(st->obj, type, val);
}

static inline void
obj_read_attr_ref(struct obj_read_state *st, uns type, byte *val)
{
  if (type == '(')
    obj_read_push(st, val);
  else if (type == ')')
    obj_read_pop(st, val);
  else
    obj_add_attr_ref(st->obj, type, val);
}

static inline int
obj_read_end(struct obj_read_state *st)
{
  if (unlikely(st->obj != st->root))
    {
      if (!st->errors++)
	st->error_callback(st, "Malformed object: truncated `( ... )' block");
    }
  return st->errors;
}

#endif
