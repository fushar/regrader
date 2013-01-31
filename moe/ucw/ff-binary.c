/*
 *	UCW Library -- Fast Buffered I/O: Binary Numbers
 *
 *	(c) 1997--2006 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/fastbuf.h"
#include "ucw/ff-binary.h"

#define GEN(type, name, size, endian)				\
type bget##name##_##endian##_slow(struct fastbuf *f)		\
{								\
  byte buf[size/8];						\
  if (bread(f, buf, sizeof(buf)) != sizeof(buf))		\
    return ~(type)0;						\
  return get_u##size##_##endian(buf);				\
}								\
void bput##name##_##endian##_##slow(struct fastbuf *f, type x)	\
{								\
  byte buf[size/8];						\
  put_u##size##_##endian(buf, x);				\
  bwrite_slow(f, buf, sizeof(buf));				\
}

#define FF_ALL(type, name, size) GEN(type,name,size,be) GEN(type,name,size,le)

FF_ALL(int, w, 16)
FF_ALL(uns, l, 32)
FF_ALL(u64, q, 64)
FF_ALL(u64, 5, 40)
