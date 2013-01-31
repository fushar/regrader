#include "ucw/lib.h"
#include "ucw/getopt.h"

void
reset_getopt(void)
{
  // Should work on GNU libc
  optind = 0;
}

#ifdef TEST
#include <stdio.h>

static void
parse(int argc, char **argv)
{
  static struct option longopts[] = {
    { "longa", 0, 0, 'a' },
    { "longb", 0, 0, 'b' },
    { "longc", 1, 0, 'c' },
    { "longd", 1, 0, 'd' },
    { 0, 0, 0, 0 }
  };
  int opt;
  while ((opt = getopt_long(argc, argv, "abc:d:", longopts, NULL)) >= 0)
    switch (opt)
      {
	case 'a':
	case 'b':
	  printf("option %c\n", opt);
	  break;
	case 'c':
	case 'd':
	  printf("option %c with value `%s'\n", opt, optarg);
	  break;
	case '?':
	  printf("unknown option\n");
	  break;
	default:
	  printf("getopt returned unexpected char 0x%02x\n", opt);
	  break;
      }
  if (optind != argc)
    printf("%d nonoption arguments\n", argc - optind);
}

int
main(int argc, char **argv)
{
  opterr = 0;
  parse(argc, argv);
  printf("reset\n");
  reset_getopt();
  parse(argc, argv);
  return 0;
}
#endif
