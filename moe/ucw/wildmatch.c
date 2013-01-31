/*
 *	UCW Library -- Fast Pattern Matcher for Short Wildcard Patterns (only `?' and `*' supported)
 *
 *	Traditional NFA -> DFA method with on-the-fly DFA construction.
 *
 *	(c) 1999 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/mempool.h"
#include "ucw/wildmatch.h"

#include <stdio.h>
#include <string.h>

#define MAX_STATES 32		/* Must be <= 32, state 0 is reserved, state 1 is initial */
#define MAX_CACHED 256		/* Maximum number of cached DFA states */
#define HASH_SIZE 512		/* Number of entries in DFA hash table (at least MAX_CACHED+MAX_STATES) */
#define HASH_SKIP 137

struct nfa_state {
  char ch;			/* 0 for non-matching state */
  byte final;			/* Accepting state */
  u32 match_states;		/* States to go to when input character == ch */
  u32 default_states;		/* States to go to whatever the input is */
};

struct dfa_state {
  uintptr_t edge[256];		/* Outgoing DFA edges. Bit 0 is set for incomplete edges which
				 * contain just state set and clear for complete ones which point
				 * to other states. NULL means `no match'.
				 */
  u32 nfa_set;			/* A set of NFA states this DFA state represents */
  int final;			/* This is an accepting state */
  struct dfa_state *next;	/* Next in the chain of free states */
};

struct wildpatt {
  struct nfa_state nfa[MAX_STATES];
  struct dfa_state *hash[HASH_SIZE];
  struct dfa_state *dfa_start;
  uns nfa_states;
  uns dfa_cache_counter;
  struct mempool *pool;
  struct dfa_state *free_states;
};

static inline unsigned
wp_hash(u32 set)
{
  set ^= set >> 16;
  set ^= set >> 8;
  return set % HASH_SIZE;
}

static struct dfa_state *
wp_new_state(struct wildpatt *w, u32 set)
{
  unsigned h = wp_hash(set);
  struct dfa_state *d;
  unsigned bit;
  u32 def_set;

  while (d = w->hash[h])
    {
      if (d->nfa_set == set)
	return d;
      h = (h + HASH_SKIP) % HASH_SIZE;
    }
  if (d = w->free_states)
    w->free_states = d->next;
  else
    d = mp_alloc(w->pool, sizeof(*d));
  w->hash[h] = d;
  bzero(d, sizeof(*d));
  d->nfa_set = set;
  def_set = 0;
  for(bit=1; bit <= w->nfa_states; bit++)
    if (set & (1 << bit))
      {
	struct nfa_state *n = &w->nfa[bit];
	if (n->ch)
	  d->edge[(unsigned char)n->ch] |= n->match_states | 1;
	d->final |= n->final;
	def_set |= n->default_states;
      }
  if (def_set)
    {
      unsigned i;
      def_set |= 1;
      for(i=0; i<256; i++)
	d->edge[i] |= def_set;
    }
  w->dfa_cache_counter++;
  return d;
}

struct wildpatt *
wp_compile(const char *p, struct mempool *pool)
{
  struct wildpatt *w;
  uns i;

  if (strlen(p) >= MAX_STATES)		/* Too long */
    return NULL;
  w = mp_alloc_zero(pool, sizeof(*w));
  w->pool = pool;
  for(i=1; *p; p++)
    {
      struct nfa_state *n = w->nfa + i;
      if (*p == '?')
	n->default_states |= 1 << (++i);/* Default edge to a new state */
      else if (*p == '*')
	n->default_states |= 1 << i;	/* Default edge to the same state */
      else
	{
	  n->ch = *p;			/* Edge to new state labelled with 'c' */
	  n->match_states = 1 << (++i);
	}
    }
  w->nfa[i].final = 1;
  w->nfa_states = i;
  w->dfa_start = wp_new_state(w, 1 << 1);
  return w;
}

static void
wp_prune_cache(struct wildpatt *w)
{
  /*
   *	I was unable to trigger cache overflow on my large set of
   *	test cases, so I decided to handle it in an extremely dumb
   *	way.   --mj
   */
  int i;
  for(i=0; i<HASH_SIZE; i++)
    if (w->hash[i] && w->hash[i]->nfa_set != (1 << 1))
      {
	struct dfa_state *d = w->hash[i];
	w->hash[i] = NULL;
	d->next = w->free_states;
	w->free_states = d;
      }
  w->dfa_cache_counter = 1;	/* Only the initial state remains */
}

int
wp_match(struct wildpatt *w, const char *s)
{
  struct dfa_state *d;

  if (w->dfa_cache_counter >= MAX_CACHED)
    wp_prune_cache(w);
  d = w->dfa_start;
  while (*s)
    {
      uintptr_t next = d->edge[(unsigned char)*s];
      if (next & 1)
	{
	  /* Need to lookup/create the destination state */
	  struct dfa_state *new = wp_new_state(w, next & ~1);
	  d->edge[(unsigned char)*s] = (uintptr_t) new;
	  d = new;
	}
      else if (!next)
	return 0;
      else
	d = (struct dfa_state *) next;
      s++;
    }
  return d->final;
}

int
wp_min_size(const char *p)
{
  int s = 0;

  while (*p)
    if (*p++ != '*')
      s++;
  return s;
}

#ifdef TEST

void
wp_dump(struct wildpatt *w)
{
  int i;

  puts("NFA:");
  for(i=1; i<=w->nfa_states; i++)
    {
      struct nfa_state *n = w->nfa + i;
      printf("%2d: %d %02x %08x %08x\n", i, n->final, n->ch, n->match_states, n->default_states);
    }
  puts("DFA:");
  for(i=0; i<HASH_SIZE; i++)
    if (w->hash[i])
      printf("%3d: %08x\n", i, w->hash[i]->nfa_set);
  printf("%d DFA states cached.\n", w->dfa_cache_counter);
}

int main(int argc, char **argv)
{
  struct wildpatt *w;
  char buf[1024];

  if (argc != 2) return 1;
  w = wp_compile(argv[1], mp_new(65536));
  if (!w)
    {
      puts("Compile error");
      return 1;
    }
  wp_dump(w);
  while (fgets(buf, sizeof(buf)-1, stdin))
    {
      char *c = strchr(buf, '\n');
      if (!c) break;
      *c = 0;
#if 0
      printf("%d\n", wp_match(w, buf));
#else
      if (wp_match(w, buf))
	puts(buf);
#endif
    }
  wp_dump(w);
  return 0;
}

#endif
