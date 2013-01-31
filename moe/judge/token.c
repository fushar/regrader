/*
 *	Tokenizer for judges
 *
 *	(c) 2007 Martin Mares <mj@ucw.cz>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include "judge.h"

#define DEFAULT_MAX_TOKEN (32 << 20)

void tok_init(struct tokenizer *t, struct stream *s)
{
  memset(t, 0, sizeof(*t));
  t->stream = s;
  t->bufsize = 1;
  t->token = xmalloc(t->bufsize);
  t->maxsize = DEFAULT_MAX_TOKEN;
  t->line = 1;
}

void tok_cleanup(struct tokenizer *t)
{
  free(t->token);
}

void tok_err(struct tokenizer *t, char *msg, ...)
{
  va_list args;
  va_start(args, msg);
  fprintf(stderr, "Error at %s line %d:\n", t->stream->name, t->line);
  vfprintf(stderr, msg, args);
  fputc('\n', stderr);
  va_end(args);
  exit(1);
}

static inline int is_white(int c)
{
  return (c == ' ' || c == '\t' || c == '\r' || c == '\n');
}

char *get_token(struct tokenizer *t)
{
  unsigned int len = 0;
  int c;

  // Skip whitespace
  do
    {
      c = sgetc(t->stream);
      if (c < 0)
	return NULL;
      if (c == '\n')
	{
	  t->line++;
	  if (t->flags & TF_REPORT_LINES)
	    {
	      t->toksize = 0;
	      t->token[0] = 0;
	      return t->token;
	    }
	}
    }
  while (is_white(c));

  // This is the token itself
  do
    {
      t->token[len++] = c;
      if (len >= t->bufsize)
	{
	  if (len > t->maxsize)
	    tok_err(t, "Token too long");
	  t->bufsize *= 2;
	  if (t->bufsize > t->maxsize)
	    t->bufsize = t->maxsize+1;
	  t->token = xrealloc(t->token, t->bufsize);
	}
      c = sgetc(t->stream);
    }
  while (c >= 0 && !is_white(c));
  if (c >= 0)
    sungetc(t->stream);

  t->token[len] = 0;
  t->toksize = len;
  return t->token;
}

/*
 *  Parsing functions. They return 1 if successfully parsed, 0 otherwise.
 */

#define PARSE(f, ...)						\
	char *end;						\
	errno = 0;						\
	if (!t->toksize)					\
	  return 0;						\
	*x = f(t->token, &end, ##__VA_ARGS__);			\
	return !(errno || (unsigned char *) end != t->token + t->toksize)

int to_long(struct tokenizer *t, long int *x)
{
  PARSE(strtol, 10);
}

int to_ulong(struct tokenizer *t, unsigned long int *x)
{
  if (t->token[0] == '-')		// strtoul accepts negative numbers, but we don't
    return 0;
  PARSE(strtoul, 10);
}

int to_double(struct tokenizer *t, double *x)
{
  PARSE(strtod);
}

int to_long_double(struct tokenizer *t, long double *x)
{
  PARSE(strtold);
}

int to_int(struct tokenizer *t, int *x)
{
  long int y;
  if (!to_long(t, &y) || y > LONG_MAX || y < LONG_MIN)
    return 0;
  *x = y;
  return 1;
}

int to_uint(struct tokenizer *t, unsigned int *x)
{
  unsigned long int y;
  if (!to_ulong(t, &y) || y > ULONG_MAX)
    return 0;
  *x = y;
  return 1;
}

#define GET(fn, type)									\
	type get_##fn(struct tokenizer *t)						\
	{										\
	  type x;									\
	  if (!get_token(t))								\
	    tok_err(t, "Unexpected end of file");					\
	  if (!to_##fn(t, &x))								\
	    tok_err(t, "Expected " #fn);						\
	  return x;									\
	}

GET(int, int)
GET(uint, unsigned int)
GET(long, long int)
GET(ulong, unsigned long int)
GET(double, double)
GET(long_double, long double)

void get_nl(struct tokenizer *t)
{
  char *tok = get_token(t);
  if (tok && *tok)
    tok_err(t, "Expected end of line");
}
