#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "judge.h"

int main(int argc, char **argv)
{
  struct stream *i = sopen_fd("stdin", 0);

  struct tokenizer t;
  tok_init(&t, i);

  int opt, verbose = 0;
  while ((opt = getopt(argc, argv, "lsv")) >= 0)
    switch (opt)
      {
      case 'l':
	t.flags = TF_REPORT_LINES;
	break;
      case 's':
	t.maxsize = 16;
	break;
      case 'v':
	verbose++;
	break;
      default:
	return 42;
      }

  char *tok;
  while (tok = get_token(&t))
    {
      printf("<%s>", tok);
#define T(f, type, fmt) { type x; if (to_##f(&t, &x)) printf(" = " #f " " fmt, x); }
      if (verbose)
	{
	  T(int, int, "%d");
	  T(uint, unsigned int, "%u");
	  T(long, long int, "%ld");
	  T(ulong, unsigned long int, "%lu");
	  T(double, double, "%f");
	  T(long_double, long double, "%Lf");
	}
#undef T
      putchar('\n');
    }
  tok_cleanup(&t);

  sclose(i);
  return 0;
}
