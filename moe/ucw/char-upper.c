/*
 *	UCW Library -- Uppercase Map
 *
 *	(c) 1997--2004 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/chartype.h"

const byte _c_upper[256] = {
#define CHAR(code,upper,lower,cat) upper,
#include "ucw/char-map.h"
#undef CHAR
};
