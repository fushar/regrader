/*
 *	UCW Library -- Temporary Fastbufs
 *
 *	(c) 2002--2008 Martin Mares <mj@ucw.cz>
 *	(c) 2008 Michal Vaner <vorner@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/fastbuf.h"

#include <stdio.h>
#include <fcntl.h>

struct fastbuf *
bopen_tmp_file(struct fb_params *params)
{
  char name[TEMP_FILE_NAME_LEN];
  int fd = open_tmp(name, O_RDWR | O_CREAT | O_TRUNC, 0600);
  struct fastbuf *fb = bopen_fd_name(fd, params, name);
  bconfig(fb, BCONFIG_IS_TEMP_FILE, 1);
  return fb;
}

struct fastbuf *
bopen_tmp(uns buflen)
{
  return bopen_tmp_file(&(struct fb_params){ .type = FB_STD, .buffer_size = buflen });
}

void bfix_tmp_file(struct fastbuf *fb, const char *name)
{
  int was_temp = bconfig(fb, BCONFIG_IS_TEMP_FILE, 0);
  ASSERT(was_temp == 1);
  if (rename(fb->name, name))
    die("Cannot rename %s to %s: %m", fb->name, name);
  bclose(fb);
}

#ifdef TEST

#include "ucw/getopt.h"

int main(int argc, char **argv)
{
  log_init(NULL);
  if (cf_getopt(argc, argv, CF_SHORT_OPTS, CF_NO_LONG_OPTS, NULL) >= 0)
    die("Hey, whaddya want?");

  struct fastbuf *f = bopen_tmp(65536);
  ASSERT(f && f->name);
  bputsn(f, "Hello, world!");
  bclose(f);
  return 0;
}

#endif
