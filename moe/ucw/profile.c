/*
 *	UCW Library -- Poor Man's Profiler
 *
 *	(c) 2001 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/profile.h"

#include <stdio.h>

/* PROFILE_TOD */

#include <sys/time.h>

void
prof_tod_init(struct prof_tod *c)
{
  c->sec = c->usec = 0;
}

void
prof_tod_switch(struct prof_tod *o, struct prof_tod *n)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  if (n)
    {
      n->start_sec = tv.tv_sec;
      n->start_usec = tv.tv_usec;
    }
  if (o)
    {
      o->sec += tv.tv_sec - o->start_sec;
      o->usec += tv.tv_usec - o->start_usec;
      if (o->usec < 0)
	{
	  o->usec += 1000000;
	  o->sec--;
	}
      else while (o->usec >= 1000000)
	{
	  o->usec -= 1000000;
	  o->sec++;
	}
    }
}

int
prof_tod_format(char *buf, struct prof_tod *c)
{
  return sprintf(buf, "%d.%06d", c->sec, c->usec);
}

/* PROFILE_TSC */

#ifdef CPU_I386

void
prof_tsc_init(struct prof_tsc *c)
{
  c->ticks = 0;
}

int
prof_tsc_format(char *buf, struct prof_tsc *c)
{
  return sprintf(buf, "%lld", c->ticks);
}

#endif

/* PROFILE_KTSC */

#ifdef CONFIG_LINUX

#include <fcntl.h>
#include <unistd.h>
static int self_prof_fd = -1;

void
prof_ktsc_init(struct prof_ktsc *c)
{
  if (self_prof_fd < 0)
    {
      self_prof_fd = open("/proc/self/profile", O_RDONLY, 0);
      if (self_prof_fd < 0)
	die("Unable to open /proc/self/profile: %m");
    }
  c->ticks_user = 0;
  c->ticks_sys = 0;
}

void
prof_ktsc_switch(struct prof_ktsc *o, struct prof_ktsc *n)
{
  unsigned long long u, s;
  byte buf[256];

  int l = pread(self_prof_fd, buf, sizeof(buf)-1, 0);
  ASSERT(l > 0 && l < (int)sizeof(buf)-1);
  buf[l] = 0;
  l = sscanf(buf, "%lld%lld", &u, &s);
  ASSERT(l == 2);

  if (n)
    {
      n->start_user = u;
      n->start_sys = s;
    }
  if (o)
    {
      u -= o->start_user;
      o->ticks_user += u;
      s -= o->start_sys;
      o->ticks_sys += s;
    }
}

int
prof_ktsc_format(char *buf, struct prof_ktsc *c)
{
  return sprintf(buf, "%lld+%lld", (long long) c->ticks_user, (long long) c->ticks_sys);
}

#endif
