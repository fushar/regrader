/*
 *	UCW Library -- Shell-Like Pattern Matching (currently only '?' and '*')
 *
 *	(c) 1997 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"

#define Convert(x) (x)
#define MATCH_FUNC_NAME str_match_pattern

#include "ucw/str-match.h"
