#include <stdio.h>

#include "judge.h"

int main(void)
{
#if 0
  struct stream *i = sopen_read("/etc/profile");
  struct stream *o = sopen_write("/dev/stdout");
#else
  struct stream *i = sopen_fd("stdin", 0);
  struct stream *o = sopen_fd("stdout", 1);
#endif

  int c;
  while ((c = sgetc(i)) >= 0)
    sputc(o, c);

  sclose(i);
  sclose(o);
  return 0;
}
