/*
 *	UCW Library -- Interface to Regular Expression Libraries
 *
 *	(c) 1997--2004 Martin Mares <mj@ucw.cz>
 *	(c) 2001 Robert Spalek <robert@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _UCW_REGEX_H
#define _UCW_REGEX_H

typedef struct regex regex;

regex *rx_compile(const char *r, int icase);
void rx_free(regex *r);
int rx_match(regex *r, const char *s);
int rx_subst(regex *r, const char *by, const char *src, char *dest, uns destlen);

#endif
