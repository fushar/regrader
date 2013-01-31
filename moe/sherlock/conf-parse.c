/*
 *	Sherlock Library -- Configuration Parsing Helpers
 *
 *	(c) 2006 Martin Mares <mj@ucw.cz>
 *	(c) 2006 Robert Spalek <robert@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "sherlock/sherlock.h"
#include "sherlock/object.h"
#include "ucw/chartype.h"
#include "ucw/fastbuf.h"
#include "ucw/ff-unicode.h"
#include "ucw/unicode.h"
#include "sherlock/conf.h"

/*** Attribute names ***/

static byte *
attr_sub_parser(byte *c, uns *ptr)
{
  if (c[0] && !c[1])
    *ptr = c[0];
  else if (c[0] == '(' && c[1] && c[1] != ')' && c[2] == ')' && !c[3])
    *ptr = OBJ_ATTR_SON + c[1];
  else
    return "Invalid attribute name";
  return NULL;
}

static byte *
attr_parser(byte *c, uns *ptr)
{
  byte *err;
  if (err = attr_sub_parser(c, ptr))
    return err;
  if (*ptr >= OBJ_ATTR_SON)
    return "Names of sub-objects are not allowed here";
  return NULL;
}

static void
attr_sub_dumper(struct fastbuf *b, uns *ptr)
{
  if (!*ptr)
    bprintf(b, "<none> ");
  else if (*ptr < OBJ_ATTR_SON)
    bprintf(b, "%c ", *ptr);
  else
    bprintf(b, "(%c) ", *ptr - OBJ_ATTR_SON);
}

struct cf_user_type cf_type_attr = {
  .size = sizeof(uns),
  .name = "attr",
  .parser = (cf_parser1 *) attr_parser,
  .dumper = (cf_dumper1 *) attr_sub_dumper
};

struct cf_user_type cf_type_attr_sub = {
  .size = sizeof(uns),
  .name = "attr_sub",
  .parser = (cf_parser1 *) attr_sub_parser,
  .dumper = (cf_dumper1 *) attr_sub_dumper
};

/*** Unicode characters ***/

static uns
uni_parser(byte *c, u16 *up)
{
  uns u;
  byte *cc = (byte*)utf8_get(c, &u);
  if (*cc || u == UNI_REPLACEMENT)
    {
      for (uns i=0; i<4; i++)
	if (!Cxdigit(c[i]))
	  return 1;
	else
	  u = (u << 4) | Cxvalue(c[i]);
      if (c[4])
	return 1;
    }
  *up = u;
  return 0;
}

static byte *
unichar_parser(byte *c, uns *up)
{
  u16 u;
  if (uni_parser(c, &u))
    return "Expecting one UTF-8 character or its code";
  *up = u;
  return 0;
}

static void
unichar_dumper(struct fastbuf *b, uns *up)
{
  bput_utf8(b, *up);
  bputc(b, ' ');
}

struct cf_user_type cf_type_unichar = {
  .size = sizeof(uns),
  .name = "unichar",
  .parser = (cf_parser1 *) unichar_parser,
  .dumper = (cf_dumper1 *) unichar_dumper
};

/*** Unicode ranges ***/

static byte *
unirange_parser(byte *s, struct unirange *ur)
{
  byte *c;
  if ((c = strchr(s, '-')) && c > s)
    {
      *c++ = 0;
      if (uni_parser(s, &ur->min) || uni_parser(c, &ur->max))
	goto err;
    }
  else
    {
      if (uni_parser(s, &ur->min))
	goto err;
      ur->max = ur->min;
    }
  if (ur->min > ur->max)
    return "Invalid code range (min>max)";
  return NULL;

 err:
  return "Incorrect syntax of a code range";
}

static void
unirange_dumper(struct fastbuf *b, struct unirange *ur)
{
  bprintf(b, (ur->min == ur->max ? "%04x " : "%04x-%04x "), ur->min, ur->max);
}

struct cf_user_type cf_type_unirange = {
  .size = sizeof(struct unirange),
  .name = "unirange",
  .parser = (cf_parser1 *) unirange_parser,
  .dumper = (cf_dumper1 *) unirange_dumper
};

/*** Unsigned integer ranges ***/

static byte *
unsrange_parser(byte *s, struct unsrange *r)
{
  byte *c, *msg;
  if ((c = strchr(s, '-')) && c > s)
    {
      *c++ = 0;
      if (*c == '-')
        return "Incorrect syntax of an unsigned range";
      if ((msg = cf_parse_int(s, &r->min)) || (msg = cf_parse_int(c, &r->max)))
	return msg;
    }
  else
    {
      if (msg = cf_parse_int(s, &r->min))
	return msg;
      r->max = r->min;
    }
  if (r->min > r->max)
    return "Invalid unsigned range (min>max)";
  return NULL;
}

static void
unsrange_dumper(struct fastbuf *b, struct unsrange *r)
{
  bprintf(b, (r->min == r->max ? "%u " : "%u-%u "), r->min, r->max);
}

struct cf_user_type cf_type_unsrange = {
  .size = sizeof(struct unsrange),
  .name = "gerr_range",
  .parser = (cf_parser1 *) unsrange_parser,
  .dumper = (cf_dumper1 *) unsrange_dumper
};

/* Configuration sections for (word|meta|string)-types */

static byte *
parse_u8(byte *s, uns *w)
{
  CF_JOURNAL_VAR(*w);
  byte *msg = cf_parse_int(s, (int *)w);
  if (msg)
    return msg;
  if (*w > 255)
    return "Weights are limited to 0..255";
  return NULL;
}

static void
dump_u8(struct fastbuf *fb, uns *ptr)
{
  bprintf(fb, "%d ", *ptr);
}

static struct cf_user_type weight_type = {
  .size = sizeof(uns),
  .name = "weight",
  .parser = (cf_parser1*) parse_u8,
  .dumper = (cf_dumper1*) dump_u8
};

void
cf_generate_word_type_config(struct cf_section *sec, byte **names, uns multiple, uns just_u8)
{
  uns number = 0;
  while (names[number])
    number++;
  struct cf_item *items = sec->cfg = xmalloc((number + 1) * sizeof(struct cf_item));
  for (uns i = 0; i < number; i++) {
    if (just_u8)
      items[i] = (struct cf_item) CF_USER_ARY(names[i], ((uns*) NULL) + i*multiple, &weight_type, multiple);
    else
      items[i] = (struct cf_item) CF_UNS_ARY(names[i], ((uns*) NULL) + i*multiple, multiple);
  }
  items[number] = (struct cf_item) CF_END;
}
