/*
 *	A filter removing C++-style comments
 *
 *	(c) 2007 Martin Mares <mj@ucw.cz>
 */

#include "judge.h"

int main(void)
{
  struct stream *i = sopen_fd("stdin", 0);
  struct stream *o = sopen_fd("stdout", 1);

  int c, d, nl = 1;
  c = sgetc(i);
  for (;;)
    {
      if (c == '/')
	{
	  if ((d = sgetc(i)) == '/')
	    {
	      // skipping a comment...
	      while ((c = sgetc(i)) >= 0 && c != '\n')
		;
	      if (nl)
		{
		  c = sgetc(i);
		  continue;
		}
	    }
	  else
	    {
	      sputc(o, c);
	      c = d;
	    }
	}
      if (c < 0)
	break;
      sputc(o, c);
      nl = (c == '\n');
      c = sgetc(i);
    }

  sclose(o);
  sclose(i);
  return 0;
}
