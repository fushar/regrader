/*
 *	Utility functions for judges
 *
 *	(c) 2007 Martin Mares <mj@ucw.cz>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "judge.h"

void die(char *msg, ...)
{
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  fputc('\n', stderr);
  va_end(args);
  exit(2);
}

void *xmalloc(size_t size)
{
  void *p = malloc(size);
  if (!p)
    die("Out of memory (unable to allocate %z bytes)", size);
  return p;
}

void *xrealloc(void *p, size_t size)
{
  p = realloc(p, size);
  if (!p)
    die("Out of memory (unable to allocate %z bytes)", size);
  return p;
}
