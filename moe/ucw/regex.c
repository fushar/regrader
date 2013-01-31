/*
 *	UCW Library -- Interface to Regular Expression Libraries
 *
 *	(c) 1997--2004 Martin Mares <mj@ucw.cz>
 *	(c) 2001 Robert Spalek <robert@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/chartype.h"
#include "ucw/hashfunc.h"
#include "ucw/regex.h"

#include <stdio.h>
#include <string.h>

#ifdef CONFIG_POSIX_REGEX

/* POSIX regular expression library */

#include <regex.h>

struct regex {
  regex_t rx;
  regmatch_t matches[10];
};

regex *
rx_compile(const char *p, int icase)
{
  regex *r = xmalloc_zero(sizeof(regex));

  int err = regcomp(&r->rx, p, REG_EXTENDED | (icase ? REG_ICASE : 0));
  if (err)
    {
      char msg[256];
      regerror(err, &r->rx, msg, sizeof(msg)-1);
      /* regfree(&r->rx) not needed */
      die("Error parsing regular expression `%s': %s", p, msg);
    }
  return r;
}

void
rx_free(regex *r)
{
  regfree(&r->rx);
  xfree(r);
}

int
rx_match(regex *r, const char *s)
{
  int err = regexec(&r->rx, s, 10, r->matches, 0);
  if (!err)
    {
      /* regexec doesn't support anchored expressions, so we have to check ourselves that the full string is matched */
      return !(r->matches[0].rm_so || s[r->matches[0].rm_eo]);
    }
  else if (err == REG_NOMATCH)
    return 0;
  else if (err == REG_ESPACE)
    die("Regex matching ran out of memory");
  else
    die("Regex matching failed with unknown error %d", err);
}

int
rx_subst(regex *r, const char *by, const char *src, char *dest, uns destlen)
{
  char *end = dest + destlen - 1;

  if (!rx_match(r, src))
    return 0;

  while (*by)
    {
      if (*by == '\\')
	{
	  by++;
	  if (*by >= '0' && *by <= '9')	/* \0 gets replaced by entire pattern */
	    {
	      uns j = *by++ - '0';
	      if (j <= r->rx.re_nsub && r->matches[j].rm_so >= 0)
		{
		  const char *s = src + r->matches[j].rm_so;
		  uns i = r->matches[j].rm_eo - r->matches[j].rm_so;
		  if (dest + i >= end)
		    return -1;
		  memcpy(dest, s, i);
		  dest += i;
		  continue;
		}
	    }
	}
      if (dest < end)
	*dest++ = *by++;
      else
	return -1;
    }
  *dest = 0;
  return 1;
}

#elif defined(CONFIG_PCRE)

/* PCRE library */

#include <pcre.h>

struct regex {
  pcre *rx;
  pcre_extra *extra;
  uns match_array_size;
  uns real_matches;
  int matches[0];			/* (max_matches+1) pairs (pos,len) plus some workspace */
};

regex *
rx_compile(const char *p, int icase)
{
  const char *err;
  int errpos, match_array_size, eno;

  pcre *rx = pcre_compile(p, PCRE_ANCHORED | PCRE_EXTRA | (icase ? PCRE_CASELESS : 0), &err, &errpos, NULL);
  if (!rx)
    die("Error parsing regular expression `%s': %s at position %d", p, err, errpos);
  eno = pcre_fullinfo(rx, NULL, PCRE_INFO_CAPTURECOUNT, &match_array_size);
  if (eno)
    die("Internal error: pcre_fullinfo() failed with error %d", eno);
  match_array_size = 3*(match_array_size+1);
  regex *r = xmalloc_zero(sizeof(regex) + match_array_size * sizeof(int));
  r->rx = rx;
  r->match_array_size = match_array_size;
  r->extra = pcre_study(r->rx, 0, &err);
  if (err)
    die("Error studying regular expression `%s': %s", p, err);
  return r;
}

void
rx_free(regex *r)
{
  xfree(r->rx);
  xfree(r->extra);
  xfree(r);
}

int
rx_match(regex *r, const char *s)
{
  int len = str_len(s);
  int err = pcre_exec(r->rx, r->extra, s, len, 0, 0, r->matches, r->match_array_size);
  if (err >= 0)
    {
      r->real_matches = err;
      /* need to check that the full string matches */
      return !(r->matches[0] || s[r->matches[1]]);
    }
  else if (err == PCRE_ERROR_NOMATCH)
    return 0;
  else if (err == PCRE_ERROR_NOMEMORY)
    die("Regex matching ran out of memory");
  else
    die("Regex matching failed with unknown error %d", err);
}

int
rx_subst(regex *r, const char *by, const char *src, char *dest, uns destlen)
{
  char *end = dest + destlen - 1;

  if (!rx_match(r, src))
    return 0;

  while (*by)
    {
      if (*by == '\\')
	{
	  by++;
	  if (*by >= '0' && *by <= '9')	/* \0 gets replaced by entire pattern */
	    {
	      uns j = *by++ - '0';
	      if (j < r->real_matches && r->matches[2*j] >= 0)
		{
		  char *s = src + r->matches[2*j];
		  uns i = r->matches[2*j+1] - r->matches[2*j];
		  if (dest + i >= end)
		    return -1;
		  memcpy(dest, s, i);
		  dest += i;
		  continue;
		}
	    }
	}
      if (dest < end)
	*dest++ = *by++;
      else
	return -1;
    }
  *dest = 0;
  return 1;
}

#else

/* BSD regular expression library */

#include <regex.h>

#define INITIAL_MEM 1024		/* Initial space allocated for each pattern */
#define CHAR_SET_SIZE 256		/* How many characters in the character set.  */

struct regex {
  struct re_pattern_buffer buf;
  struct re_registers regs;		/* Must not change between re_match() calls */
  int len_cache;
};

regex *
rx_compile(const char *p, int icase)
{
  regex *r = xmalloc_zero(sizeof(regex));
  const char *msg;

  r->buf.buffer = xmalloc(INITIAL_MEM);
  r->buf.allocated = INITIAL_MEM;
  if (icase)
    {
      unsigned i;
      r->buf.translate = xmalloc (CHAR_SET_SIZE);
      /* Map uppercase characters to corresponding lowercase ones.  */
      for (i = 0; i < CHAR_SET_SIZE; i++)
        r->buf.translate[i] = Cupcase(i);
    }
  else
    r->buf.translate = NULL;
  re_set_syntax(RE_SYNTAX_POSIX_EXTENDED);
  msg = re_compile_pattern(p, strlen(p), &r->buf);
  if (!msg)
    return r;
  die("Error parsing pattern `%s': %s", p, msg);
}

void
rx_free(regex *r)
{
  xfree(r->buf.buffer);
  if (r->buf.translate)
    xfree(r->buf.translate);
  xfree(r);
}

int
rx_match(regex *r, const char *s)
{
  int len = strlen(s);

  r->len_cache = len;
  if (re_match(&r->buf, s, len, 0, &r->regs) < 0)
    return 0;
  if (r->regs.start[0] || r->regs.end[0] != len) /* XXX: Why regex doesn't enforce implicit "^...$" ? */
    return 0;
  return 1;
}

int
rx_subst(regex *r, const char *by, const char *src, char *dest, uns destlen)
{
  char *end = dest + destlen - 1;

  if (!rx_match(r, src))
    return 0;

  while (*by)
    {
      if (*by == '\\')
	{
	  by++;
	  if (*by >= '0' && *by <= '9')	/* \0 gets replaced by entire pattern */
	    {
	      uns j = *by++ - '0';
	      if (j < r->regs.num_regs)
		{
		  const char *s = src + r->regs.start[j];
		  uns i = r->regs.end[j] - r->regs.start[j];
		  if (r->regs.start[j] > r->len_cache || r->regs.end[j] > r->len_cache)
		    return -1;
		  if (dest + i >= end)
		    return -1;
		  memcpy(dest, s, i);
		  dest += i;
		  continue;
		}
	    }
	}
      if (dest < end)
	*dest++ = *by++;
      else
	return -1;
    }
  *dest = 0;
  return 1;
}

#endif

#ifdef TEST

int main(int argc, char **argv)
{
  regex *r;
  char buf1[4096], buf2[4096];
  int opt_i = 0;

  if (!strcmp(argv[1], "-i"))
    {
      opt_i = 1;
      argv++;
      argc--;
    }
  r = rx_compile(argv[1], opt_i);
  while (fgets(buf1, sizeof(buf1), stdin))
    {
      char *p = strchr(buf1, '\n');
      if (p)
	*p = 0;
      if (argc == 2)
	{
	  if (rx_match(r, buf1))
	    puts("MATCH");
	  else
	    puts("NO MATCH");
	}
      else
	{
	  int i = rx_subst(r, argv[2], buf1, buf2, sizeof(buf2));
	  if (i < 0)
	    puts("OVERFLOW");
	  else if (!i)
	    puts("NO MATCH");
	  else
	    puts(buf2);
	}
    }
  rx_free(r);
}

#endif
