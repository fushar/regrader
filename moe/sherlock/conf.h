/*
 *	Sherlock Library -- Configuration Parsing Helpers
 *
 *	(c) 2006 Martin Mares <mj@ucw.cz>
 *	(c) 2006 Robert Spalek <robert@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _SHERLOCK_CONF_H
#define _SHERLOCK_CONF_H

#include "ucw/conf.h"

/* All of the following objects are defined in conf-parse.c
 *
 * Object names */

extern struct cf_user_type cf_type_attr, cf_type_attr_sub;

/* Unicode character and ranges */

struct unirange {
  u16 min, max;
};

extern struct cf_user_type cf_type_unirange;
extern struct cf_user_type cf_type_unichar;

/* Unsigned integer ranges */

struct unsrange {
  uns min, max;
};

extern struct cf_user_type cf_type_unsrange;

/* Sections for (word|meta|string)-types */
void cf_generate_word_type_config(struct cf_section *sec, byte **names, uns multiple, uns just_u8);

#endif
