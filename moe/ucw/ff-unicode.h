/*
 *	UCW Library: Reading and writing of UTF-8 and UTF-16 on Fastbuf Streams
 *
 *	(c) 2001--2004 Martin Mares <mj@ucw.cz>
 *	(c) 2004 Robert Spalek <robert@ucw.cz>
 *	(c) 2007--2008 Pavel Charvat <pchar@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _UCW_FF_UNICODE_H
#define _UCW_FF_UNICODE_H

#include "ucw/fastbuf.h"
#include "ucw/unicode.h"

/* ** UTF-8 ** */

int bget_utf8_slow(struct fastbuf *b, uns repl);
int bget_utf8_32_slow(struct fastbuf *b, uns repl);
void bput_utf8_slow(struct fastbuf *b, uns u);
void bput_utf8_32_slow(struct fastbuf *b, uns u);

static inline int
bget_utf8_repl(struct fastbuf *b, uns repl)
{
  uns u;
  if (bavailr(b) >= 3)
    {
      b->bptr = utf8_get_repl(b->bptr, &u, repl);
      return u;
    }
  else
    return bget_utf8_slow(b, repl);
}

static inline int
bget_utf8_32_repl(struct fastbuf *b, uns repl)
{
  uns u;
  if (bavailr(b) >= 6)
    {
      b->bptr = utf8_32_get_repl(b->bptr, &u, repl);
      return u;
    }
  else
    return bget_utf8_32_slow(b, repl);
}

static inline int bget_utf8(struct fastbuf *b) /** Read a single utf8 character from range [0, 0xffff]. **/
{
  return bget_utf8_repl(b, UNI_REPLACEMENT);
}

static inline int bget_utf8_32(struct fastbuf *b) /** Read a single utf8 character (from the whole unicode range). **/
{
  return bget_utf8_32_repl(b, UNI_REPLACEMENT);
}

static inline void bput_utf8(struct fastbuf *b, uns u) /** Write a single utf8 character from range [0, 0xffff]. **/
{
  if (bavailw(b) >= 3)
    b->bptr = utf8_put(b->bptr, u);
  else
    bput_utf8_slow(b, u);
}

static inline void bput_utf8_32(struct fastbuf *b, uns u) /** Write a single utf8 character (from the whole unicode range). **/
{
  if (bavailw(b) >= 6)
    b->bptr = utf8_32_put(b->bptr, u);
  else
    bput_utf8_32_slow(b, u);
}

/* ** UTF-16 ** */

int bget_utf16_be_slow(struct fastbuf *b, uns repl);
int bget_utf16_le_slow(struct fastbuf *b, uns repl);
void bput_utf16_be_slow(struct fastbuf *b, uns u);
void bput_utf16_le_slow(struct fastbuf *b, uns u);

static inline int
bget_utf16_be_repl(struct fastbuf *b, uns repl)
{
  uns u;
  if (bavailr(b) >= 4)
    {
      b->bptr = utf16_be_get_repl(b->bptr, &u, repl);
      return u;
    }
  else
    return bget_utf16_be_slow(b, repl);
}

static inline int
bget_utf16_le_repl(struct fastbuf *b, uns repl)
{
  uns u;
  if (bavailr(b) >= 4)
    {
      b->bptr = utf16_le_get_repl(b->bptr, &u, repl);
      return u;
    }
  else
    return bget_utf16_le_slow(b, repl);
}

/**
 * Read an utf16 character from fastbuf.
 * Big endian version.
 **/
static inline int bget_utf16_be(struct fastbuf *b)
{
  return bget_utf16_be_repl(b, UNI_REPLACEMENT);
}

/**
 * Read an utf16 character from fastbuf.
 * Little endian version.
 **/
static inline int bget_utf16_le(struct fastbuf *b)
{
  return bget_utf16_le_repl(b, UNI_REPLACEMENT);
}

/**
 * Write an utf16 character to fastbuf.
 * Big endian version.
 **/
static inline void bput_utf16_be(struct fastbuf *b, uns u)
{
  if (bavailw(b) >= 4)
    b->bptr = utf16_be_put(b->bptr, u);
  else
    bput_utf16_be_slow(b, u);
}

/**
 * Write an utf16 character to fastbuf.
 * Little endian version.
 **/
static inline void bput_utf16_le(struct fastbuf *b, uns u)
{
  if (bavailw(b) >= 4)
    b->bptr = utf16_le_put(b->bptr, u);
  else
    bput_utf16_le_slow(b, u);
}

#endif
