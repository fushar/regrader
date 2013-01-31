/*
 *	UCW Library -- Hyper-super-meta-alt-control-shift extra fast
 *	str_len() and hash_*() routines
 *
 *	(c) 2002, Robert Spalek <robert@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _UCW_HASHFUNC_H
#define _UCW_HASHFUNC_H

#include "ucw/lib.h"

/*** === String hashes [[strhash]] ***/

/* The following functions need str to be aligned to sizeof(uns).  */
uns str_len_aligned(const char *str) PURE; /** Get the string length (not a really useful hash function, but there is no better place for it). The string must be aligned to sizeof(uns). For unaligned see @str_len(). **/
uns hash_string_aligned(const char *str) PURE; /** Hash the string. The string must be aligned to sizeof(uns). For unaligned see @hash_string(). **/
uns hash_block_aligned(const byte *buf, uns len) PURE; /** Hash arbitrary data. They must be aligned to sizeof(uns). For unaligned see @hash_block(). **/

#ifdef	CPU_ALLOW_UNALIGNED
#define	str_len(str)		str_len_aligned(str)
#define	hash_string(str)	hash_string_aligned(str)
#define	hash_block(str, len)	hash_block_aligned(str, len)
#else
uns str_len(const char *str) PURE; /** Get the string length. If you know it is aligned to sizeof(uns), you can use faster @str_len_aligned(). **/
uns hash_string(const char *str) PURE; /** Hash the string. If it is aligned to sizeof(uns), you can use faster @hash_string_aligned(). **/
uns hash_block(const byte *buf, uns len) PURE; /** Hash arbitrary data. If they are aligned to sizeof(uns), use faster @hash_block_aligned(). **/
#endif

uns hash_string_nocase(const char *str) PURE; /** Hash the string in a case insensitive way. Works only with ASCII characters. **/

/*** === Integer hashes [[inthash]] ***/

/***
 * We hash integers by multiplying by a reasonably large prime with
 * few ones in its binary form (to give the compiler the possibility
 * of using shifts and adds on architectures where multiplication
 * instructions are slow).
 */
static inline uns CONST hash_u32(uns x) { return 0x01008041*x; } /** Hash a 32 bit unsigned integer. **/
static inline uns CONST hash_u64(u64 x) { return hash_u32((uns)x ^ (uns)(x >> 32)); } /** Hash a 64 bit unsigned integer. **/
static inline uns CONST hash_pointer(void *x) { return ((sizeof(x) <= 4) ? hash_u32((uns)(uintptr_t)x) : hash_u64((u64)(uintptr_t)x)); } /** Hash a pointer. **/

#endif
