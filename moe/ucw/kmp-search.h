/*
 *      Knuth-Morris-Pratt's Substring Search for N given strings
 *
 *      (c) 1999--2005, Robert Spalek <robert@ucw.cz>
 *      (c) 2006, Pavel Charvat <pchar@ucw.cz>
 *
 *      (In fact, the algorithm is usually referred to as Aho-McCorasick,
 *      but that's just an extension of KMP to multiple strings.)
 */

/*
 *  This is not a normal header file, it's a generator of KMP algorithm.
 *  Each time you include it with parameters set in the corresponding
 *  preprocessor macros, it generates KMP structures and functions
 *  with the parameters given. See ucw/kmp.h before reading this description.
 *
 *  This file defines:
 *
 *  	struct search		structure with both the internal and the user-defined variables
 *  				used during the search and accessible from all macros
 *
 * 	void search(kmp,search,src) executes the search; search structure is allocated by the caller (possible input/output)
 *
 *  	void run(kmp,src)	the same, but automatically allocates search structre from the stack
 *
 *
 *  Parameters to the generator (these marked with [*] are mandatory):
 *
 *  [*]	KMPS_PREFIX(x)		macro to add a name prefix (used on all global names
 *				defined by the KMP search generator)
 *  [*]	KMPS_KMP_PREFIX(x)	prefix used for ucw/kmp.h
 *
 *  KMPS_SOURCE			user-defined text source (together with KMPS_GET_CHAR);
 *  				if unset, the one from ucw/kmp.h is taken
 *  KMPS_GET_CHAR(kmp,src,search) analogy to KMP_GET_CHAR, but it must store the next character to search->c
 *
 *  KMPS_ADD_CONTROLS		add control characters (see KMP_CONTROL_CHAR in kmp.h) at both ends of the input string
 *  KMPS_MERGE_CONTROLS 	merge adjacent control characters to a single one
 *
 *  KMPS_VARS			user-defined variables in struct search (in .u substructure to avoid collisions)
 *
 *  KMPS_INIT(kmp,src,search)	statement executed at the beginning of search()
 *  KMPS_EXIT(kmp,src,search)	... at the end
 *  KMPS_STEP(kmp,src,search)	... after each step (read of next character + current state update)
 *  				of the algorithm, but before KMPS_FOUND[_CHAIN]
 *  KMPS_FOUND_CHAIN(kmp,src,search) ... for each state representing locally longest match
 *				(stored in search->out - NOT necessary search->s!);
 *  				all matches form a NULL-terminated link list (search->out, search->out->next, ...)
 *  				in order of decreasing length
 *  KMPS_FOUND(kmp,src,search)  ... called for every match (in search->out)
 *  KMPS_WANT_BEST		algorithm computes globally longest match, which is available
 *  				in search->best in KMPS_EXIT; if there is no match, it points to the null state
 */

#define P(x) KMPS_PREFIX(x)
#define KP(x) KMPS_KMP_PREFIX(x)

#ifdef KMPS_SOURCE
typedef KMPS_SOURCE P(search_source_t);
#else
typedef KP(source_t) P(search_source_t);
#endif

#ifndef KMPS_GET_CHAR
#define KMPS_GET_CHAR(kmp,src,s) (KP(get_char)(kmp, &src, &s->c))
#endif

struct P(search) {
  struct KP(state) *s;		/* current state */
  struct KP(state) *out;	/* output state */
# ifdef KMPS_WANT_BEST
  struct KP(state) *best;	/* longest match */
# endif
  KP(char_t) c;			/* last character */
# ifdef KMPS_ADD_CONTROLS
  uns eof;
# endif
# ifdef KMPS_VARS
  struct {
    KMPS_VARS
  } u;				/* user-defined */
# endif
};

static void
P(search) (struct KP(struct) *kmp, struct P(search) *s, P(search_source_t) src)
{
  s->s = &kmp->null;
# ifdef KMPS_WANT_BEST
  s->best = &kmp->null;
# endif
# ifdef KMPS_ADD_CONTROLS
  s->c = KP(control)();
  s->eof = 0;
# else
  s->c = 0;
# endif
# ifdef KMPS_INIT
  { KMPS_INIT(kmp, src, s); }
# endif
# ifndef KMPS_ADD_CONTROLS
  goto start_read;
# endif
  for (;;)
  {
    for (struct KP(state) *t = s->s; t && !(s->s = KP(hash_find)(&kmp->hash, t, s->c)); t = t->back);
    s->s = s->s ? : &kmp->null;

#   ifdef KMPS_STEP
    { KMPS_STEP(kmp, src, s); }
#   endif

#   if defined(KMPS_FOUND) || defined(KMPS_FOUND_CHAIN) || defined(KMPS_WANT_BEST)
    s->out = s->s->len ? s->s : s->s->next;
    if (s->out)
      {
#	ifdef KMPS_WANT_BEST
	if (s->out->len > s->best->len)
	  s->best = s->out;
#	endif
#       ifdef KMPS_FOUND_CHAIN
	{ KMPS_FOUND_CHAIN(kmp, src, s); }
#       endif
#       ifdef KMPS_FOUND
	do
          { KMPS_FOUND(kmp, src, s); }
	while (s->out = s->out->next);
#       endif
      }
#   endif

#   ifdef KMPS_ADD_CONTROLS
    if (s->eof)
      break;
#   endif

#   ifndef KMPS_ADD_CONTROLS
start_read: ;
#   endif
#   ifdef KMPS_MERGE_CONTROLS
    KP(char_t) last_c = s->c;
#   endif

    do
      {
	if (!KMPS_GET_CHAR(kmp, src, s))
	  {
#           ifdef KMPS_ADD_CONTROLS
	    if (!KP(is_control)(kmp, s->c))
	      {
                s->c = KP(control)();
                s->eof = 1;
		break;
	      }
#           endif
	    goto exit;
	  }
      }
    while (0
#     ifdef KMPS_MERGE_CONTROLS
      || (KP(is_control)(kmp, last_c) && KP(is_control)(kmp, s->c))
#     endif
      );
  }
exit: ;
# ifdef KMPS_EXIT
  { KMPS_EXIT(kmp, src, s); }
# endif
}

static inline void
P(run) (struct KP(struct) *kmp, P(search_source_t) src)
{
  struct P(search) search;
  P(search)(kmp, &search, src);
}

#undef P
#undef KMPS_PREFIX
#undef KMPS_KMP_PREFIX
#undef KMPS_SOURCE
#undef KMPS_GET_CHAR
#undef KMPS_ADD_CONTROLS
#undef KMPS_MERGE_CONTROLS
#undef KMPS_VARS
#undef KMPS_INIT
#undef KMPS_EXIT
#undef KMPS_FOUND
#undef KMPS_FOUND_CHAIN
#undef KMPS_WANT_BEST
#undef KMPS_STEP
