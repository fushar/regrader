/*
 *	UCW Library -- Fast Buffered I/O on Binary Values
 *
 *	(c) 1997--2007 Martin Mares <mj@ucw.cz>
 *	(c) 2004 Robert Spalek <robert@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _UCW_FF_BINARY_H
#define _UCW_FF_BINARY_H

#include "ucw/fastbuf.h"
#include "ucw/unaligned.h"

#ifdef CPU_BIG_ENDIAN
#define FF_ENDIAN be
#else
#define FF_ENDIAN le
#endif

/***
 *
 * We define several functions to read or write binary integer values.
 *
 * The name patterns for such routines are:
 *
 * - `TYPE bget \#\# NAME \#\# ENDIAN(struct fastbuf *f);`
 * - `void bput \#\# NAME \#\# ENDIAN(struct fastbuf *f, TYPE value);`
 *
 * where `NAME` together with `TYPE` can be:
 *
 * - `w` for 16-bit unsigned integers stored in sequences of 2 bytes, the `TYPE` is int
 * - `l` for 32-bit unsigned integers stored in sequences of 4 bytes, the `TYPE` is uns
 * - `5` for 40-bit unsigned integers stored in sequences of 5 bytes, the `TYPE` is u64
 * - `q` for 64-bit unsigned integers stored in sequences of 8 bytes, the `TYPE` is u64
 *
 * and supported `ENDIAN` suffixes are:
 *
 * - empty for the default order of bytes (defined by CPU)
 * - `_le` for little-endian
 * - `_be` for big-endian
 *
 * If we fail to read enough bytes because of EOF, the reading function returns `(TYPE)-1`.
 *
 ***/

#define GET_FUNC(type, name, bits, endian)			\
  type bget##name##_##endian##_slow(struct fastbuf *f);		\
  static inline type bget##name##_##endian(struct fastbuf *f)	\
  {								\
    if (bavailr(f) >= bits/8)					\
      {								\
	type w = get_u##bits##_##endian(f->bptr);		\
	f->bptr += bits/8;					\
	return w;						\
      }								\
    else							\
      return bget##name##_##endian##_slow(f);			\
  }

#define PUT_FUNC(type, name, bits, endian)			\
  void bput##name##_##endian##_slow(struct fastbuf *f, type x);	\
  static inline void bput##name##_##endian(struct fastbuf *f, type x)	\
  {								\
    if (bavailw(f) >= bits/8)					\
      {								\
	put_u##bits##_##endian(f->bptr, x);			\
	f->bptr += bits/8;					\
      }								\
    else							\
      return bput##name##_##endian##_slow(f, x);		\
  }

#define FF_ALL_X(type, name, bits, defendian)			\
  GET_FUNC(type, name, bits, be)				\
  GET_FUNC(type, name, bits, le)				\
  PUT_FUNC(type, name, bits, be)				\
  PUT_FUNC(type, name, bits, le)				\
  static inline type bget##name(struct fastbuf *f) { return bget##name##_##defendian(f); }		\
  static inline void bput##name(struct fastbuf *f, type x) { bput##name##_##defendian(f, x); }

#define FF_ALL(type, name, bits, defendian) FF_ALL_X(type, name, bits, defendian)

FF_ALL(int, w, 16, FF_ENDIAN)
FF_ALL(uns, l, 32, FF_ENDIAN)
FF_ALL(u64, q, 64, FF_ENDIAN)
FF_ALL(u64, 5, 40, FF_ENDIAN)

#undef GET_FUNC
#undef PUT_FUNC
#undef FF_ENDIAN
#undef FF_ALL_X
#undef FF_ALL

/* I/O on uintptr_t (only native endianity) */

#ifdef CPU_64BIT_POINTERS
#define bputa(x,p) bputq(x,p)
#define bgeta(x) bgetq(x)
#else
#define bputa(x,p) bputl(x,p)
#define bgeta(x) bgetl(x)
#endif

#endif
