/*
 *	UCW Library -- Shell-Like Case-Insensitive Pattern Matching (currently only '?' and '*')
 *
 *	(c) 1997 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/chartype.h"

#define Convert(x) Cupcase(x)
#define MATCH_FUNC_NAME str_match_pattern_nocase

#include "ucw/str-match.h"
