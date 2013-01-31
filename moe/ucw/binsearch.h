/*
 *	UCW Library -- Generic Binary Search
 *
 *	(c) 2005 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

/***
 * [[defs]]
 * Definitions
 * -----------
 ***/

/**
 * Find the first element not lower than @x in the sorted array @ary of @N elements (non-decreasing order).
 * Returns the index of the found element or @N if no exists. Uses `ary_lt_x(ary,i,x)` to compare the @i'th element with @x.
 * The time complexity is `O(log(N))`.
 **/
#define BIN_SEARCH_FIRST_GE_CMP(ary,N,x,ary_lt_x)  ({		\
  uns l = 0, r = (N);						\
  while (l < r)							\
    {								\
      uns m = (l+r)/2;						\
      if (ary_lt_x(ary,m,x))					\
	l = m+1;						\
      else							\
	r = m;							\
    }								\
  l;								\
})

/**
 * The default comparision macro for @BIN_SEARCH_FIRST_GE_CMP().
 **/
#define ARY_LT_NUM(ary,i,x) (ary)[i] < (x)

/**
 * Same as @BIN_SEARCH_FIRST_GE_CMP(), but uses the default `<` operator for comparisions.
 **/
#define BIN_SEARCH_FIRST_GE(ary,N,x) BIN_SEARCH_FIRST_GE_CMP(ary,N,x,ARY_LT_NUM)

/**
 * Search the sorted array @ary of @N elements (non-decreasing) for the first occurence of @x.
 * Returns the index or -1 if no such element exists. Uses the `<` operator for comparisions.
 **/
#define BIN_SEARCH_EQ(ary,N,x) ({ int i = BIN_SEARCH_FIRST_GE(ary,N,x); if (i >= (N) || (ary)[i] != (x)) i=-1; i; })
