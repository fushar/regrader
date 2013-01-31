/*
 *	UCW Library -- Fast Access to Unaligned Data
 *
 *	(c) 1997--2007 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _UCW_UNALIGNED_H
#define _UCW_UNALIGNED_H

/* Big endian format */

#if defined(CPU_ALLOW_UNALIGNED) && defined(CPU_BIG_ENDIAN)
static inline uns get_u16_be(const void *p) { return *(u16 *)p; } /** Read 16-bit integer value from an unaligned sequence of 2 bytes (big-endian version). **/
static inline u32 get_u32_be(const void *p) { return *(u32 *)p; } /** Read 32-bit integer value from an unaligned sequence of 4 bytes (big-endian version). **/
static inline u64 get_u64_be(const void *p) { return *(u64 *)p; } /** Read 64-bit integer value from an unaligned sequence of 8 bytes (big-endian version). **/
static inline void put_u16_be(void *p, uns x) { *(u16 *)p = x; } /** Write 16-bit integer value to an unaligned sequence of 2 bytes (big-endian version). **/
static inline void put_u32_be(void *p, u32 x) { *(u32 *)p = x; } /** Write 32-bit integer value to an unaligned sequence of 4 bytes (big-endian version). **/
static inline void put_u64_be(void *p, u64 x) { *(u64 *)p = x; } /** Write 64-bit integer value to an unaligned sequence of 8 bytes (big-endian version). **/
#else
static inline uns get_u16_be(const void *p)
{
  const byte *c = p;
  return (c[0] << 8) | c[1];
}
static inline u32 get_u32_be(const void *p)
{
  const byte *c = p;
  return (c[0] << 24) | (c[1] << 16) | (c[2] << 8) | c[3];
}
static inline u64 get_u64_be(const void *p)
{
  return ((u64) get_u32_be(p) << 32) | get_u32_be((const byte *)p+4);
}
static inline void put_u16_be(void *p, uns x)
{
  byte *c = p;
  c[0] = x >> 8;
  c[1] = x;
}
static inline void put_u32_be(void *p, u32 x)
{
  byte *c = p;
  c[0] = x >> 24;
  c[1] = x >> 16;
  c[2] = x >> 8;
  c[3] = x;
}
static inline void put_u64_be(void *p, u64 x)
{
  put_u32_be(p, x >> 32);
  put_u32_be((byte *)p+4, x);
}
#endif

static inline u64 get_u40_be(const void *p) /** Read 40-bit integer value from an unaligned sequence of 5 bytes (big-endian version). **/
{
  const byte *c = p;
  return ((u64)c[0] << 32) | get_u32_be(c+1);
}

static inline void put_u40_be(void *p, u64 x)
{
  byte *c = p;
  c[0] = x >> 32;
  put_u32_be(c+1, x);
}

/* Little-endian format */

#if defined(CPU_ALLOW_UNALIGNED) && !defined(CPU_BIG_ENDIAN)
static inline uns get_u16_le(const void *p) { return *(u16 *)p; } /** Read 16-bit integer value from an unaligned sequence of 2 bytes (little-endian version). **/
static inline u32 get_u32_le(const void *p) { return *(u32 *)p; } /** Read 32-bit integer value from an unaligned sequence of 4 bytes (little-endian version). **/
static inline u64 get_u64_le(const void *p) { return *(u64 *)p; } /** Read 64-bit integer value from an unaligned sequence of 8 bytes (little-endian version). **/
static inline void put_u16_le(void *p, uns x) { *(u16 *)p = x; } /** Write 16-bit integer value to an unaligned sequence of 2 bytes (little-endian version). **/
static inline void put_u32_le(void *p, u32 x) { *(u32 *)p = x; } /** Write 32-bit integer value to an unaligned sequence of 4 bytes (little-endian version). **/
static inline void put_u64_le(void *p, u64 x) { *(u64 *)p = x; } /** Write 64-bit integer value to an unaligned sequence of 8 bytes (little-endian version). **/
#else
static inline uns get_u16_le(const void *p)
{
  const byte *c = p;
  return c[0] | (c[1] << 8);
}
static inline u32 get_u32_le(const void *p)
{
  const byte *c = p;
  return c[0] | (c[1] << 8) | (c[2] << 16) | (c[3] << 24);
}
static inline u64 get_u64_le(const void *p)
{
  return get_u32_le(p) | ((u64) get_u32_le((const byte *)p+4) << 32);
}
static inline void put_u16_le(void *p, uns x)
{
  byte *c = p;
  c[0] = x;
  c[1] = x >> 8;
}
static inline void put_u32_le(void *p, u32 x)
{
  byte *c = p;
  c[0] = x;
  c[1] = x >> 8;
  c[2] = x >> 16;
  c[3] = x >> 24;
}
static inline void put_u64_le(void *p, u64 x)
{
  put_u32_le(p, x);
  put_u32_le((byte *)p+4, x >> 32);
}
#endif

static inline u64 get_u40_le(const void *p) /** Read 40-bit integer value from an unaligned sequence of 5 bytes (little-endian version). **/
{
  const byte *c = p;
  return get_u32_le(c) | ((u64) c[4] << 32);
}

static inline void put_u40_le(void *p, u64 x)
{
  byte *c = p;
  put_u32_le(c, x);
  c[4] = x >> 32;
}

/* The native format */

#ifdef CPU_BIG_ENDIAN

static inline uns get_u16(const void *p) { return get_u16_be(p); } /** Read 16-bit integer value from an unaligned sequence of 2 bytes (native byte-order). **/
static inline u32 get_u32(const void *p) { return get_u32_be(p); } /** Read 32-bit integer value from an unaligned sequence of 4 bytes (native byte-order). **/
static inline u64 get_u64(const void *p) { return get_u64_be(p); } /** Read 64-bit integer value from an unaligned sequence of 8 bytes (native byte-order). **/
static inline u64 get_u40(const void *p) { return get_u40_be(p); } /** Read 40-bit integer value from an unaligned sequence of 5 bytes (native byte-order). **/
static inline void put_u16(void *p, uns x) { return put_u16_be(p, x); } /** Write 16-bit integer value to an unaligned sequence of 2 bytes (native byte-order). **/
static inline void put_u32(void *p, u32 x) { return put_u32_be(p, x); } /** Write 32-bit integer value to an unaligned sequence of 4 bytes (native byte-order). **/
static inline void put_u64(void *p, u64 x) { return put_u64_be(p, x); } /** Write 64-bit integer value to an unaligned sequence of 8 bytes (native byte-order). **/
static inline void put_u40(void *p, u64 x) { return put_u40_be(p, x); } /** Write 40-bit integer value to an unaligned sequence of 5 bytes (native byte-order). **/

#else

static inline uns get_u16(const void *p) { return get_u16_le(p); }
static inline u32 get_u32(const void *p) { return get_u32_le(p); }
static inline u64 get_u64(const void *p) { return get_u64_le(p); }
static inline u64 get_u40(const void *p) { return get_u40_le(p); }
static inline void put_u16(void *p, uns x) { return put_u16_le(p, x); }
static inline void put_u32(void *p, u32 x) { return put_u32_le(p, x); }
static inline void put_u64(void *p, u64 x) { return put_u64_le(p, x); }
static inline void put_u40(void *p, u64 x) { return put_u40_le(p, x); }

#endif

/* Just for completeness */

static inline uns get_u8(const void *p) { return *(const byte *)p; } /** Read 8-bit integer value. **/
static inline void put_u8(void *p, uns x) { *(byte *)p = x; } /** Write 8-bit integer value. **/

/* Backward compatibility macros */

#define GET_U8(p) get_u8(p)
#define GET_U16(p) get_u16(p)
#define GET_U32(p) get_u32(p)
#define GET_U64(p) get_u64(p)
#define GET_U40(p) get_u40(p)

#define PUT_U8(p,x) put_u8(p,x);
#define PUT_U16(p,x) put_u16(p,x)
#define PUT_U32(p,x) put_u32(p,x)
#define PUT_U64(p,x) put_u64(p,x)
#define PUT_U40(p,x) put_u40(p,x)

#endif
