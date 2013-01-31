/*
 *	A judge comparing two sequences of tokens
 *
 *	(c) 2007 Martin Krulis <bobrik@matfyz.cz>
 *	(c) 2007 Martin Mares <mj@ucw.cz>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>

#include "judge.h"

static int ignore_nl, ignore_trailing_nl, ignore_case;
static int real_mode;
static double rel_eps = 1e-5;
static double abs_eps = 1e-30;

static int tokens_equal(struct tokenizer *t1, struct tokenizer *t2)
{
  if (real_mode)
    {
      double x1, x2;
      if (to_double(t1, &x1) && to_double(t2, &x2))
	{
	  if (x1 == x2)
	    return 1;
	  double eps = fabs(x2 * rel_eps);
	  if (eps < abs_eps)
	    eps = abs_eps;
	  return (fabs(x1-x2) <= eps);
	}
      // If they fail to convert, compare them as strings.
    }
  return !(ignore_case ? strcasecmp : strcmp)(t1->token, t2->token);
}

static int trailing_nl(struct tokenizer *t)
{
  // Ignore empty lines at the end of file
  if (t->token[0] || !ignore_trailing_nl)
    return 0;
  t->flags &= ~TF_REPORT_LINES;
  return !get_token(t);
}

static void usage(void)
{
  fprintf(stderr, "Usage: judge-tok [<options>] <output> <correct>\n\
\n\
Options:\n\
-n\t\tIgnore newlines\n\
-t\t\tIgnore newlines at the end of file\n\
-i\t\tIgnore case\n\
-r\t\tMatch tokens as real numbers and allow small differences:\n\
-e <epsilon>\tSet maximum allowed relative error (default: %g)\n\
-E <epsilon>\tSet maximum allowed absolute error (default: %g)\n\
", rel_eps, abs_eps);
  exit(2);
}

int main(int argc, char **argv)
{
  struct tokenizer t1, t2;
  int opt;

  while ((opt = getopt(argc, argv, "ntire:E:")) >= 0)
    switch (opt)
      {
      case 'n':
	ignore_nl++;
	break;
      case 't':
	ignore_trailing_nl++;
	break;
      case 'i':
	ignore_case++;
	break;
      case 'r':
	real_mode++;
	break;
      case 'e':
	rel_eps = atof(optarg);
	break;
      case 'E':
	abs_eps = atof(optarg);
	break;
      default:
	usage();
      }
  if (optind + 2 != argc)
    usage();

  tok_init(&t1, sopen_read(argv[optind]));
  tok_init(&t2, sopen_read(argv[optind+1]));
  if (!ignore_nl)
    t1.flags = t2.flags = TF_REPORT_LINES;

  for (;;)
    {
      char *a = get_token(&t1), *b = get_token(&t2);
      if (!a)
	{
	  if (b && !trailing_nl(&t2))
	    tok_err(&t1, "Ends too early");
	  break;
	}
      else if (!b)
	{
	  if (a && !trailing_nl(&t1))
	    tok_err(&t2, "Garbage at the end");
	  break;
	}
      else if (!tokens_equal(&t1, &t2))
	tok_err(&t1, "Found <%s>, expected <%s>", a, b);
    }

  return 0;
}
