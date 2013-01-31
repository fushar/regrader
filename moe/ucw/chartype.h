/*
 *	UCW Library -- Character Types
 *
 *	(c) 1997--2004 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _UCW_CHARTYPE_H
#define _UCW_CHARTYPE_H

/***
 * We define our own routines to classify 8-bit characters (based on US-ASCII charset).
 * This way we bypass most possible problems with different compilation environments.
 *
 * All functions and macros accept any numbers and if it is necessary, they simply ignore higher bits.
 * It does not matter whether a parameter is signed or unsigned. Parameters are evaluated exactly once,
 * so they can have side-effects.
 ***/

#define _C_UPPER 1			/* Upper-case letters */
#define _C_LOWER 2			/* Lower-case letters */
#define _C_PRINT 4			/* Printable */
#define _C_DIGIT 8			/* Digits */
#define _C_CTRL 16			/* Control characters */
#define _C_XDIGIT 32			/* Hexadecimal digits */
#define _C_BLANK 64			/* White spaces (spaces, tabs, newlines) */
#define _C_INNER 128			/* `inner punctuation' -- underscore etc. */

#define _C_ALPHA (_C_UPPER | _C_LOWER)
#define _C_ALNUM (_C_ALPHA | _C_DIGIT)
#define _C_WORD (_C_ALNUM | _C_INNER)
#define _C_WSTART (_C_ALPHA | _C_INNER)

extern const byte _c_cat[256], _c_upper[256], _c_lower[256];

#define Category(x) (_c_cat[(byte)(x)])
#define Ccat(x,y) (Category(x) & y)

#define Cupper(x) Ccat(x, _C_UPPER)	/** Checks for an upper-case character (`A-Z`). **/
#define Clower(x) Ccat(x, _C_LOWER)	/** Checks for a lower-case character (`a-z`). **/
#define Calpha(x) Ccat(x, _C_ALPHA)	/** Checks for an alphabetic character (`a-z`, `A-Z`). **/
#define Calnum(x) Ccat(x, _C_ALNUM)	/** Checks for an alpha-numeric character (`a-z`, `A-Z`, `0-9`). */
#define Cprint(x) Ccat(x, _C_PRINT)	/** Checks for printable characters, including 8-bit values (`\t`, `0x20-0x7E`, `0x80-0xFF`). **/
#define Cdigit(x) Ccat(x, _C_DIGIT)	/** Checks for a digit (`0-9`). **/
#define Cxdigit(x) Ccat(x, _C_XDIGIT)	/** Checks for a hexadecimal digit (`0-9`, `a-f`, `A-F`). **/
#define Cword(x) Ccat(x, _C_WORD)	/** Checks for an alpha-numeric character or an inner punctation (`a-z`, `A-Z`, `0-9`, `_`). **/
#define Cblank(x) Ccat(x, _C_BLANK)	/** Checks for a white space (`0x20`, `\t`, `\n`, `\r`, `0x8`, `0xC`). **/
#define Cctrl(x) Ccat(x, _C_CTRL)	/** Checks for control characters (`0x0-0x1F`, `0x7F`). **/
#define Cspace(x) Cblank(x)

#define Cupcase(x) (_c_upper[(byte)(x)]) /** Convert a letter to upper case, leave non-letter characters unchanged. **/
#define Clocase(x) (_c_lower[(byte)(x)]) /** Convert a letter to lower case, leave non-letter characters unchanged. **/

/**
 * Compute the value of a valid hexadecimal character (ie. passed the @Cxdigit() check).
 **/
static inline uns Cxvalue(byte x)
{
  return (x < (uns)'A') ? x - '0' : (x & 0xdf) - 'A' + 10;
}

#endif
