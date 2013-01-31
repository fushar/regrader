/*
 *	Sherlock Library -- Generating Buckets from Objects
 *
 *	(c) 2004, Robert Spalek <robert@ucw.cz>
 *	(c) 2005, Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "sherlock/sherlock.h"
#include "ucw/fastbuf.h"
#include "ucw/ff-unicode.h"
#include "sherlock/object.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static uns use_v33;
static int hdr_sep;

void
put_attr_set_type(uns type)
{
  switch (type)
    {
    case BUCKET_TYPE_PLAIN:
      use_v33 = 0;
      hdr_sep = -1;
      break;
    case BUCKET_TYPE_V30:
      use_v33 = 0;
      hdr_sep = '\n';
      break;
    case BUCKET_TYPE_V33:
    case BUCKET_TYPE_V33_LIZARD:
      use_v33 = 1;
      hdr_sep = 0;
      break;
    default:
      die("Don't know how to generate buckets of type %08x", type);
    }
}

uns
size_attr(uns len)
{
  if (use_v33)
    {
      len++;
      return len + utf8_space(len);
    }
  else
    return len + 2;
}

uns
size_object(struct odes *d)
{
  uns sz = 0;
  for (struct oattr *a=d->attrs; a; a=a->next)
    for (struct oattr *b=a; b; b=b->same)
      if (a->attr >= OBJ_ATTR_SON)
	sz += 3 + size_object(b->son) + 2;
      else
	sz += size_attr(strlen(b->val));
  return sz;
}

inline byte *
put_attr(byte *ptr, uns type, byte *val, uns len)
{
  if (use_v33)
  {
    ptr = utf8_32_put(ptr, len+1);
    memcpy(ptr, val, len);
    ptr += len;
    *ptr++ = type;
  }
  else
  {
    *ptr++ = type;
    memcpy(ptr, val, len);
    ptr += len;
    *ptr++ = '\n';
  }
  return ptr;
}

byte *
put_attr_str(byte *ptr, uns type, byte *val)
{
  return put_attr(ptr, type, val, strlen(val));
}

inline byte *
put_attr_vformat(byte *ptr, uns type, byte *mask, va_list va)
{
  uns len;
  if (use_v33)
  {
    len = vsprintf(ptr+1, mask, va);
    if (len >= 127)
    {
      byte tmp[6], *tmp_end = tmp;
      tmp_end = utf8_32_put(tmp_end, len+1);
      uns l = tmp_end - tmp;
      memmove(ptr+l, ptr+1, len);
      memcpy(ptr, tmp, l);
      ptr += l + len;
    }
    else
    {
      *ptr = len+1;
      ptr += len+1;
    }
    *ptr++ = type;
  }
  else
  {
    *ptr++ = type;
    len = vsprintf(ptr, mask, va);
    ptr += len;
    *ptr++ = '\n';
  }
  return ptr;
}

byte *
put_attr_format(byte *ptr, uns type, char *mask, ...)
{
  va_list va;
  va_start(va, mask);
  byte *ret = put_attr_vformat(ptr, type, mask, va);
  va_end(va);
  return ret;
}

byte *
put_attr_num(byte *ptr, uns type, uns val)
{
  if (use_v33)
  {
    uns len = sprintf(ptr+1, "%d", val) + 1;
    *ptr = len;
    ptr += len;
    *ptr++ = type;
  }
  else
    ptr += sprintf(ptr, "%c%d\n", type, val);
  return ptr;
}

byte *
put_attr_separator(byte *ptr)
{
  if (hdr_sep >= 0)
    *ptr++ = hdr_sep;
  return ptr;
}

byte *
put_attr_push(byte *ptr, uns type)
{
  byte name = type;
  return put_attr(ptr, '(', &name, 1);
}

byte *
put_attr_pop(byte *ptr)
{
  return put_attr(ptr, ')', NULL, 0);
}

byte *
put_object(byte *t, struct odes *d)
{
  for (struct oattr *a=d->attrs; a; a=a->next)
    for (struct oattr *b=a; b; b=b->same)
      if (a->attr >= OBJ_ATTR_SON)
	{
	  t = put_attr_push(t, a->attr - OBJ_ATTR_SON);
	  t = put_object(t, b->son);
	  t = put_attr_pop(t);
	}
      else
	t = put_attr_str(t, a->attr, b->val);
  return t;
}

inline void
bput_attr_large(struct fastbuf *b, uns type, byte *val, uns len)
{
  if (use_v33)
  {
    bput_utf8_32(b, len+1);
    bwrite(b, val, len);
    bputc(b, type);
  }
  else
  {
    bputc(b, type);
    bwrite(b, val, len);
    bputc(b, '\n');
  }
}

inline void
bput_attr(struct fastbuf *b, uns type, byte *val, uns len)
{
  bput_attr_large(b, type, val, len);
}

void
bput_attr_str(struct fastbuf *b, uns type, byte *val)
{
  bput_attr(b, type, val, strlen(val));
}

void
bput_attr_vformat(struct fastbuf *b, uns type, byte *mask, va_list va)
{
  int len;
  if (use_v33)
  {
    va_list va2;
    va_copy(va2, va);
    len = vsnprintf(NULL, 0, mask, va2);
    va_end(va2);
    if (len < 0)
      die("vsnprintf() does not support size=0");
    bput_utf8_32(b, len+1);
    vbprintf(b, mask, va);
    bputc(b, type);
  }
  else
  {
    bputc(b, type);
    len = vbprintf(b, mask, va);
    bputc(b, '\n');
  }
}

void
bput_attr_format(struct fastbuf *b, uns type, char *mask, ...)
{
  va_list va;
  va_start(va, mask);
  bput_attr_vformat(b, type, mask, va);
  va_end(va);
}

void
bput_attr_num(struct fastbuf *b, uns type, uns val)
{
  if (use_v33)
  {
    byte tmp[12];
    uns len = sprintf(tmp, "%d", val);
    bputc(b, len+1);
    bwrite(b, tmp, len);
    bputc(b, type);
  }
  else
    bprintf(b, "%c%d\n", type, val);
}

void
bput_attr_separator(struct fastbuf *b)
{
  if (hdr_sep >= 0)
    bputc(b, hdr_sep);
}

void
bput_attr_push(struct fastbuf *b, uns type)
{
  byte name = type;
  bput_attr(b, '(', &name, 1);
}

void
bput_attr_pop(struct fastbuf *b)
{
  bput_attr(b, ')', NULL, 0);
}

static inline void
do_bput_oattr(struct fastbuf *f, struct oattr *a)
{
  for(struct oattr *b=a; b; b=b->same)
    if (a->attr >= OBJ_ATTR_SON)
      {
	bput_attr_push(f, a->attr - OBJ_ATTR_SON);
	bput_object(f, b->son);
	bput_attr_pop(f);
      }
    else
      {
#ifdef DEBUG_ASSERTS
	byte *z;
	for (z = b->val; *z; z++)
	  if (*z < ' ' && *z != '\t')
	    {
	      log(L_ERROR, "obj_write: Found non-ASCII character %02x in %c%s", *z, a->attr, b->val);
	      ASSERT(0);
	    }
#endif
	bput_attr_str(f, a->attr, b->val);
      }
}

void
bput_oattr(struct fastbuf *f, struct oattr *a)
{
  if (a)
    do_bput_oattr(f, a);
}

void
bput_object(struct fastbuf *f, struct odes *d)
{
  for(struct oattr *a=d->attrs; a; a=a->next)
    do_bput_oattr(f, a);
}

static inline void
do_bput_oattr_nocheck(struct fastbuf *f, struct oattr *a)
{
  for(struct oattr *b=a; b; b=b->same)
    if (a->attr >= OBJ_ATTR_SON)
      {
	bput_attr_push(f, a->attr - OBJ_ATTR_SON);
	bput_object_nocheck(f, b->son);
	bput_attr_pop(f);
      }
    else
      bput_attr_large(f, a->attr, b->val, strlen(b->val));
}

void
bput_oattr_nocheck(struct fastbuf *f, struct oattr *a)
{
  if (a)
    do_bput_oattr_nocheck(f, a);
}

void
bput_object_nocheck(struct fastbuf *f, struct odes *d)
{
  for(struct oattr *a=d->attrs; a; a=a->next)
    do_bput_oattr_nocheck(f, a);
}

void
obj_write(struct fastbuf *b, struct odes *o, uns bucket_type)
{
  put_attr_set_type(bucket_type);
  bput_object(b, o);
}

void
obj_write_nocheck(struct fastbuf *b, struct odes *o, uns bucket_type)
{
  put_attr_set_type(bucket_type);
  bput_object_nocheck(b, o);
}
