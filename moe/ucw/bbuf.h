/*
 *	UCW Library -- A simple growing buffer for byte-sized items.
 *
 *	(c) 2004 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _UCW_BBUF_H
#define _UCW_BBUF_H

#define	GBUF_TYPE	byte
#define	GBUF_PREFIX(x)	bb_##x
#include "ucw/gbuf.h"

/**
 * printf() into a growing buffer with `va_list` arguments.
 * Generates a `'\0'`-terminated string at the beginning of the buffer
 * and returns pointer to it.
 *
 * See @bb_printf().
 **/
char *bb_vprintf(bb_t *bb, const char *fmt, va_list args);
/**
 * printf() into a growing buffer.
 * Generates a `'\0'`-terminated string at the beginning of the buffer
 * and returns pointer to it.
 *
 * See @bb_vprintf().
 **/
char *bb_printf(bb_t *bb, const char *fmt, ...);
/**
 * Like @bb_vprintf(), but it does not start at the beginning of the
 * buffer, but @ofs bytes further.
 *
 * Returns pointer to the new string (eg. @ofs bytes after the
 * beginning of buffer).
 **/
char *bb_vprintf_at(bb_t *bb, uns ofs, const char *fmt, va_list args);
/**
 * Like @bb_vprintf_at(), but it takes individual arguments.
 **/
char *bb_printf_at(bb_t *bb, uns ofs, const char *fmt, ...);

#endif
