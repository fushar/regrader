/*
 *	UCW Library -- Poor Man's Profiler
 *
 *	(c) 2001 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

/*
 *  Usage:
 *		#define PROFILE_xxx
 *		#include "ucw/profile.h"
 *		prof_t cnt;
 *		prof_init(&cnt);
 *		...
 *		prof_start(&cnt);
 *		...
 *		prof_stop(&cnt);
 *		printf("%s\n", PROF_STR(cnt));
 */

#ifndef _UCW_PROFILE_H
#define _UCW_PROFILE_H

/* PROFILE_TOD: gettimeofday() profiler */

struct prof_tod {
  u32 start_sec, start_usec;
  s32 sec, usec;
};

void prof_tod_init(struct prof_tod *);
void prof_tod_switch(struct prof_tod *, struct prof_tod *);
int prof_tod_format(char *, struct prof_tod *);

/* PROFILE_TSC: i386 TSC profiler */

#ifdef CPU_I386

struct prof_tsc {
  u64 start_tsc;
  u64 ticks;
};

void prof_tsc_init(struct prof_tsc *);
int prof_tsc_format(char *, struct prof_tsc *);

#endif

/* PROFILE_KTSC: Linux kernel TSC profiler */

#ifdef CONFIG_LINUX

struct prof_ktsc {
  u64 start_user, start_sys;
  u64 ticks_user, ticks_sys;
};

void prof_ktsc_init(struct prof_ktsc *);
void prof_ktsc_switch(struct prof_ktsc *, struct prof_ktsc *);
int prof_ktsc_format(char *, struct prof_ktsc *);

#endif

/* Select the right profiler */

#if defined(PROFILE_TOD)

#define PROFILER
#define PROF_STR_SIZE 21
typedef struct prof_tod prof_t;
#define prof_init prof_tod_init
#define prof_switch prof_tod_switch
#define prof_format prof_tod_format

#elif defined(PROFILE_TSC)

#define PROFILER
#define PROFILER_INLINE
#define PROF_STR_SIZE 24

typedef struct prof_tsc prof_t;
#define prof_init prof_tsc_init
#define prof_format prof_tsc_format

#define rdtscll(val) __asm__ __volatile__("rdtsc" : "=A" (val))

static inline void prof_start(prof_t *c)
{
  rdtscll(c->start_tsc);
}

static inline void prof_stop(prof_t *c)
{
  u64 tsc;
  rdtscll(tsc);
  tsc -= c->start_tsc;
  c->ticks += tsc;
}

static inline void prof_switch(prof_t *o, prof_t *n)
{
  u64 tsc;
  rdtscll(tsc);
  n->start_tsc = tsc;
  tsc -= o->start_tsc;
  o->ticks += tsc;
}

#elif defined(PROFILE_KTSC)

#define PROFILER
#define PROF_STR_SIZE 50
typedef struct prof_ktsc prof_t;
#define prof_init prof_ktsc_init
#define prof_switch prof_ktsc_switch
#define prof_format prof_ktsc_format

#endif

#ifdef PROFILER

/* Stuff common for all profilers */
#ifndef PROFILER_INLINE
static inline void prof_start(prof_t *c) { prof_switch(NULL, c); }
static inline void prof_stop(prof_t *c) { prof_switch(c, NULL); }
#endif
#define PROF_STR(C) ({ static char _x[PROF_STR_SIZE]; prof_format(_x, &(C)); _x; })

#else

/* Dummy profiler with no output */
typedef struct { } prof_t;
static inline void prof_init(prof_t *c UNUSED) { }
static inline void prof_start(prof_t *c UNUSED) { }
static inline void prof_stop(prof_t *c UNUSED) { }
static inline void prof_switch(prof_t *c UNUSED, prof_t *d UNUSED) { }
static inline void prof_format(char *b, prof_t *c UNUSED) { b[0]='?'; b[1]=0; }
#define PROF_STR_SIZE 2
#define PROF_STR(C) "?"

#endif

#endif
