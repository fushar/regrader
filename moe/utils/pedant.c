/*
 *	A Pedantic Check of Text Input/Output File Syntax
 *
 *	(c) 2005--2007 Martin Mares <mj@ucw.cz>
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <getopt.h>

static int max = -1;
static int no_stats;
static int line = 1;
static int warn_total, warn_shown;

static void warn(char *msg, ...)
{
#define NUM_TYPES 16
  static char *seen[NUM_TYPES];
  static int cnt[NUM_TYPES];
  int type = 0;

  va_list args;
  va_start(args, msg);

  warn_total++;
  if (max >= 0)
    {
      for (type=0; type < NUM_TYPES && seen[type] && seen[type] != msg; type++)
	;
      if (type >= NUM_TYPES)
	goto done;
      seen[type] = msg;
      if (cnt[type]++ >= max)
	goto done;
    }
  warn_shown++;
  printf("Line %d: ", line);
  vprintf(msg, args);
  putchar('\n');
done:
  va_end(args);
}

static void check(void)
{
  int pos = 0;
  int maxlen = 0;
  int lastlen = -1;
  int space = 0;
  int c;
  while ((c = getchar()) >= 0)
    {
      if (c == '\n')
	{
	  if (space)
	    warn("Trailing spaces");
	  if (line == 1 && !pos)
	    warn("Leading empty line");
	  if (maxlen < pos)
	    maxlen = pos;
	  if (!lastlen && !pos)
	    warn("Consecutive empty lines");
	  lastlen = pos;
	  line++;
	  pos = space = 0;
	}
      else
	{
	  if (c == ' ')
	    {
	      if (!pos)
		warn("Leading spaces");
	      if (space == 1)
		warn("Consecutive spaces");
	      space++;
	    }
	  else
	    {
	      space = 0;
	      if (c < ' ' || c >= 0x7f)
		warn("Invalid character 0x%02x", c);
	    }
	  pos++;
	}
    }
  if (pos)
    warn("Incomplete line at the end of file");
  else if (!lastlen)
    {
      line--;
      warn("Trailing empty line");
      line++;
    }
  if (warn_shown < warn_total)
    printf("(and %d more warnings)\n", warn_total - warn_shown);
  if (!no_stats)
    printf("Found %d lines, the longest has %d chars\n", line-1, maxlen);
}

static void usage(void)
{
  fprintf(stderr, "Usage: pedant [-m <max>] [-s]\n");
  exit(1);
}

int main(int argc, char **argv)
{
  int opt;
  while ((opt = getopt(argc, argv, "m:s")) >= 0)
    switch (opt)
      {
      case 'm':
	max = atoi(optarg);
	break;
      case 's':
	no_stats++;
	break;
      default:
	usage();
      }
  if (optind < argc)
    usage();

  check();
  return 0;
}
