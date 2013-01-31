/*
 *	UCW Library Utilities -- A Simple Logger for use in shell scripts
 *
 *	(c) 2001--2009 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/log.h"

#include <stdio.h>
#include <string.h>

int
main(int argc, char **argv)
{
  byte buf[1024], *c;

  log_init("logger");
  if (argc < 3 || argc > 4 || strlen(argv[2]) != 1)
    die("Usage: logger [<logname>:]<progname> <level> [<text>]");
  if (c = strchr(argv[1], ':'))
    {
      *c++ = 0;
      log_init(c);
      log_file(argv[1]);
    }
  else
    log_init(argv[1]);

  uns level = 0;
  while (level < L_MAX && LS_LEVEL_LETTER(level) != argv[2][0])
    level++;
  if (level >= L_MAX)
    die("Unknown logging level `%s'", argv[2]);

  if (argc > 3)
    msg(level, argv[3]);
  else
    while (fgets(buf, sizeof(buf), stdin))
      {
	c = strchr(buf, '\n');
	if (c)
	  *c = 0;
	msg(level, buf);
      }
  return 0;
}
