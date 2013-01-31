/*
 *	UCW Library -- Setting of Process Title
 *
 *	(c) 2001--2006 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

static char **spt_argv;
static char *spt_start, *spt_end;

void
setproctitle_init(int argc, char **argv)
{
#ifdef CONFIG_LINUX
  int i, len;
  char **env, **oldenv, *t;

  spt_argv = argv;

  /* Create a backup copy of environment */
  oldenv = __environ;
  len = 0;
  for (i=0; oldenv[i]; i++)
    len += strlen(oldenv[i]) + 1;
  __environ = env = xmalloc(sizeof(char *)*(i+1));
  t = xmalloc(len);
  for (i=0; oldenv[i]; i++)
    {
      env[i] = t;
      len = strlen(oldenv[i]) + 1;
      memcpy(t, oldenv[i], len);
      t += len;
    }
  env[i] = NULL;

  /* Scan for consecutive free space */
  spt_start = spt_end = argv[0];
  for (i=0; i<argc; i++)
    if (!i || spt_end+1 == argv[i])
      spt_end = argv[i] + strlen(argv[i]);
  for (i=0; oldenv[i]; i++)
    if (spt_end+1 == oldenv[i])
      spt_end = oldenv[i] + strlen(oldenv[i]);
#endif
}

void
setproctitle(const char *msg, ...)
{
  va_list args;
  byte buf[256];
  int n;

  va_start(args, msg);
  if (spt_end > spt_start)
    {
      n = vsnprintf(buf, sizeof(buf), msg, args);
      if (n >= (int) sizeof(buf) || n < 0)
	sprintf(buf, "<too-long>");
      n = spt_end - spt_start;
      strncpy(spt_start, buf, n);
      spt_start[n] = 0;
      spt_argv[0] = spt_start;
      spt_argv[1] = NULL;
    }
  va_end(args);
}

char *
getproctitle(void)
{
  return (spt_start < spt_end) ? spt_start : NULL;
}
