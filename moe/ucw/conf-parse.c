/*
 *	UCW Library -- Configuration files: parsers for basic types
 *
 *	(c) 2001--2006 Robert Spalek <robert@ucw.cz>
 *	(c) 2003--2006 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/conf.h"
#include "ucw/chartype.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

struct unit {
  uns name;			// one-letter name of the unit
  uns num, den;			// fraction
};

static const struct unit units[] = {
  { 'd', 86400, 1 },
  { 'h', 3600, 1 },
  { 'k', 1000, 1 },
  { 'm', 1000000, 1 },
  { 'g', 1000000000, 1 },
  { 'K', 1024, 1 },
  { 'M', 1048576, 1 },
  { 'G', 1073741824, 1 },
  { '%', 1, 100 },
  { 0, 0, 0 }
};

static const struct unit *
lookup_unit(const char *value, const char *end, char **msg)
{
  if (end && *end) {
    if (end == value || end[1] || *end >= '0' && *end <= '9')
      *msg = "Invalid number";
    else {
      for (const struct unit *u=units; u->name; u++)
	if ((char)u->name == *end)
	  return u;
      *msg = "Invalid unit";
    }
  }
  return NULL;
}

static char cf_rngerr[] = "Number out of range";

char *
cf_parse_int(const char *str, int *ptr)
{
  char *msg = NULL;
  if (!*str)
    msg = "Missing number";
  else {
    const struct unit *u;
    char *end;
    errno = 0;
    uns x = strtoul(str, &end, 0);
    if (errno == ERANGE)
      msg = cf_rngerr;
    else if (u = lookup_unit(str, end, &msg)) {
      u64 y = (u64)x * u->num;
      if (y % u->den)
	msg = "Number is not an integer";
      else {
	y /= u->den;
	if (y > 0xffffffff)
	  msg = cf_rngerr;
	*ptr = y;
      }
    } else
      *ptr = x;
  }
  return msg;
}

char *
cf_parse_u64(const char *str, u64 *ptr)
{
  char *msg = NULL;
  if (!*str)
    msg = "Missing number";
  else {
    const struct unit *u;
    char *end;
    errno = 0;
    u64 x = strtoull(str, &end, 0);
    if (errno == ERANGE)
      msg = cf_rngerr;
    else if (u = lookup_unit(str, end, &msg)) {
      if (x > ~(u64)0 / u->num)
	msg = "Number out of range";
      else {
	x *= u->num;
	if (x % u->den)
	  msg = "Number is not an integer";
	else
	  *ptr = x / u->den;
      }
    } else
      *ptr = x;
  }
  return msg;
}

char *
cf_parse_double(const char *str, double *ptr)
{
  char *msg = NULL;
  if (!*str)
    msg = "Missing number";
  else {
    const struct unit *u;
    double x;
    uns read_chars;
    if (sscanf(str, "%lf%n", &x, &read_chars) != 1)
      msg = "Invalid number";
    else if (u = lookup_unit(str, str + read_chars, &msg))
      *ptr = x * u->num / u->den;
    else
      *ptr = x;
  }
  return msg;
}

char *
cf_parse_ip(const char *p, u32 *varp)
{
  if (!*p)
    return "Missing IP address";
  uns x = 0;
  char *p2;
  if (*p == '0' && (p[1] | 32) == 'x' && Cxdigit(p[2])) {
    errno = 0;
    x = strtoul(p, &p2, 16);
    if (errno == ERANGE || x > 0xffffffff)
      goto error;
    p = p2;
  }
  else
    for (uns i = 0; i < 4; i++) {
      if (i) {
	if (*p++ != '.')
	  goto error;
      }
      if (!Cdigit(*p))
	goto error;
      errno = 0;
      uns y = strtoul(p, &p2, 10);
      if (errno == ERANGE || p2 == (char*) p || y > 255)
	goto error;
      p = p2;
      x = (x << 8) + y;
    }
  *varp = x;
  return *p ? "Trailing characters" : NULL;
error:
  return "Invalid IP address";
}

