#include "ucw/lib.h"
#include "ucw/getopt.h"
#include "ucw/fastbuf.h"
#include "ucw/ff-binary.h"
#include "ucw/lizard.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

static char *options = CF_SHORT_OPTS "cdtx";
static char *help = "\
Usage: lizard-test <options> input-file [output-file]\n\
\n\
Options:\n"
CF_USAGE
"-c\t\tCompress\n\
-d\t\tDecompress\n\
-t\t\tCompress, decompress, and compare (in memory only, default)\n\
-x\t\tLet the test crash by shrinking the output buffer\n\
";

static void NONRET
usage(void)
{
  fputs(help, stderr);
  exit(1);
}

int
main(int argc, char **argv)
{
  int opt;
  uns action = 't';
  uns crash = 0;
  log_init(argv[0]);
  while ((opt = cf_getopt(argc, argv, options, CF_NO_LONG_OPTS, NULL)) >= 0)
    switch (opt)
    {
      case 'c':
      case 'd':
      case 't':
	action = opt;
	break;
      case 'x':
	crash++;
	break;
      default:
	usage();
    }
  if (action == 't' && argc != optind+1
  || action != 't' && argc != optind+2)
    usage();

  void *mi, *mo;
  int li, lo;
  uns adler = 0;

  struct stat st;
  stat(argv[optind], &st);
  li = st.st_size;
  struct fastbuf *fi = bopen(argv[optind], O_RDONLY, 1<<16);
  if (action != 'd')
  {
    lo = li * LIZARD_MAX_MULTIPLY + LIZARD_MAX_ADD;
    li += LIZARD_NEEDS_CHARS;
  }
  else
  {
    lo = bgetl(fi);
    adler = bgetl(fi);
    li -= 8;
  }
  mi = xmalloc(li);
  mo = xmalloc(lo);
  li = bread(fi, mi, li);
  bclose(fi);

  printf("%d ", li);
  if (action == 'd')
    printf("->expected %d (%08x) ", lo, adler);
  fflush(stdout);
  if (action != 'd')
    lo = lizard_compress(mi, li, mo);
  else
  {
    lo = lizard_decompress(mi, mo);
    if (adler32(mo, lo) != adler)
      printf("wrong Adler32 ");
  }
  printf("-> %d ", lo);
  fflush(stdout);

  if (action != 't')
  {
    struct fastbuf *fo = bopen(argv[optind+1], O_CREAT | O_TRUNC | O_WRONLY, 1<<16);
    if (action == 'c')
    {
      bputl(fo, li);
      bputl(fo, adler32(mi, li));
    }
    bwrite(fo, mo, lo);
    bclose(fo);
  }
  else
  {
    int smaller_li;
    if (li >= (int) CPU_PAGE_SIZE)
      smaller_li = li - CPU_PAGE_SIZE;
    else
      smaller_li = 0;
    struct lizard_buffer *buf = lizard_alloc();
    byte *ptr = lizard_decompress_safe(mo, buf, crash ? smaller_li : li);
    if (!ptr)
      printf("err: %m");
    else if (memcmp(mi, ptr, li))
      printf("WRONG");
    else
      printf("OK");
    lizard_free(buf);
  }
  printf("\n");
}
