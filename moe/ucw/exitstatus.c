/*
 *	UCW Library -- Formatting of Process Exit Status
 *
 *	(c) 2004 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"

#include <stdio.h>
#include <sys/wait.h>
#include <errno.h>

int
format_exit_status(char *msg, int stat)
{
  if (stat < 0)
    sprintf(msg, "failed to fork (err=%d)", errno);
  else if (WIFEXITED(stat) && WEXITSTATUS(stat) < 256)
    {
      if (WEXITSTATUS(stat))
	sprintf(msg, "died with exit code %d", WEXITSTATUS(stat));
      else
	{
	  msg[0] = 0;
	  return 0;
	}
    }
  else if (WIFSIGNALED(stat))
    sprintf(msg, "died on signal %d", WTERMSIG(stat));
  else
    sprintf(msg, "died with status %x", stat);
  return 1;
}
