/*
 *	Sherlock Library -- Main Include File
 *
 *	(c) 1997--2004 Martin Mares <mj@ucw.cz>
 */

/*
 *  This file should be included as the very first include in all
 *  source files, especially before all OS includes since it sets
 *  up libc feature macros.
 */

#ifndef _SHERLOCK_LIB_H
#define _SHERLOCK_LIB_H

#include "ucw/lib.h"

#ifdef CONFIG_MAX_CONTEXTS
#define CONFIG_CONTEXTS
#endif

/* Version string */

#define SHER_VER SHERLOCK_VERSION SHERLOCK_VERSION_SUFFIX

/* Types */

typedef u32 oid_t;			/* Object ID */

#ifdef CONFIG_LARGE_FILES		/* File positions */
#define BYTES_PER_O 5
#define bgeto(f) bget5(f)
#define bputo(f,l) bput5(f,l)
#define GET_O(p) GET_U40(p)
#define PUT_O(p,x) PUT_U40(p,x)
#else
#define BYTES_PER_O 4
#define bgeto(f) bgetl(f)
#define bputo(f,l) bputl(f,l)
#define GET_O(p) GET_U32(p)
#define PUT_O(p,x) PUT_U32(p,x)
#endif

/* Area ID's */

#ifdef CONFIG_AREAS
typedef u32 area_t;
#define AREA_NONE 0
#define AREA_ANY ~0U
#else
typedef struct { } area_t;
#define AREA_NONE (area_t){}
#define AREA_ANY (area_t){}
#endif

/* An alias for the libucw logging function */

#define log msg

#endif
