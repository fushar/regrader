/*
 *	I/O functions for judges
 *
 *	(c) 2007 Martin Mares <mj@ucw.cz>
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "judge.h"

#define BUFSIZE 65536

struct stream *sopen_fd(char *name, int fd)
{
  char *slash = strrchr(name, '/');
  char *basename = (slash ? slash+1 : name);
  struct stream *s = xmalloc(sizeof(*s) + BUFSIZE + strlen(basename) + 1);
  s->fd = fd;
  s->pos = s->stop = s->buf;
  s->end = s->buf + BUFSIZE;
  s->name = s->end;
  strcpy(s->name, basename);
  return s;
}

struct stream *sopen_read(char *name)
{
  int fd = open(name, O_RDONLY);
  if (fd < 0)
    die("Unable to open %s for reading: %m", name);
  return sopen_fd(name, fd);
}

struct stream *sopen_write(char *name)
{
  int fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (fd < 0)
    die("Unable to open %s for writing: %m", name);
  return sopen_fd(name, fd);
}

void sflush(struct stream *s)
{
  if (s->pos > s->stop)
    {
      char *p = s->buf;
      int len = s->pos - s->buf;
      while (len > 0)
	{
	  int c = write(s->fd, p, len);
	  if (c <= 0)
	    die("Error writing %s: %m", s->name);
	  p += c;
	  len -= c;
	}
    }
  s->pos = s->stop = s->buf;
}

void sclose(struct stream *s)
{
  sflush(s);
  close(s->fd);
  free(s);
}

static int srefill(struct stream *s)
{
  int len = read(s->fd, s->buf, BUFSIZE);
  if (len < 0)
    die("Error reading %s: %m", s->name);
  s->pos = s->buf;
  s->stop = s->buf + len;
  return len;
}

int sgetc_slow(struct stream *s)
{
  return (srefill(s) ? *s->pos++ : -1);
}

int speekc_slow(struct stream *s)
{
  return (srefill(s) ? *s->pos : -1);
}
