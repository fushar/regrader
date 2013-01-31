/*
 *	UCW Library -- Temporary Files
 *
 *	(c) 2002--2008 Martin Mares <mj@ucw.cz>
 *	(c) 2008 Michal Vaner <vorner@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/conf.h"
#include "ucw/threads.h"
#include "ucw/lfs.h"
#include "ucw/fastbuf.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>

static char *temp_prefix = "temp";
static char *temp_dir;
static int public_dir = 1;

static struct cf_section temp_config = {
  CF_ITEMS {
    CF_STRING("Dir", &temp_dir),
    CF_STRING("Prefix", &temp_prefix),
    CF_INT("PublicDir", &public_dir),
    CF_END
  }
};

static void CONSTRUCTOR temp_global_init(void)
{
  cf_declare_section("Tempfiles", &temp_config, 0);
}

void
temp_file_name(char *name_buf, int *open_flags)
{
  char *dir = temp_dir;
  if (!dir && !(dir = getenv("TMPDIR")))
    dir = "/tmp";

  int len;
  if (public_dir)
    {
      struct timeval tv;
      if (gettimeofday(&tv, NULL))
	die("gettimeofday() failed: %m");
      len = snprintf(name_buf, TEMP_FILE_NAME_LEN, "%s/%s%u", dir, temp_prefix, (uns) tv.tv_usec);
      if (open_flags)
	*open_flags = O_EXCL;
    }
  else
    {
      struct ucwlib_context *ctx = ucwlib_thread_context();
      int cnt = ++ctx->temp_counter;
      int pid = getpid();
      if (ctx->thread_id == pid)
	len = snprintf(name_buf, TEMP_FILE_NAME_LEN, "%s/%s%d-%d", dir, temp_prefix, pid, cnt);
      else
	len = snprintf(name_buf, TEMP_FILE_NAME_LEN, "%s/%s%d-%d-%d", dir, temp_prefix, pid, ctx->thread_id, cnt);
      if (open_flags)
	*open_flags = 0;
    }
  ASSERT(len < TEMP_FILE_NAME_LEN);
}

int
open_tmp(char *name_buf, int open_flags, int mode)
{
  int create_flags, fd, retry = 10;
  do
    {
      temp_file_name(name_buf, &create_flags);
      fd = ucw_open(name_buf, open_flags | create_flags, mode);
    }
  while (fd < 0 && errno == EEXIST && retry --);
  if (fd < 0)
    die("Unable to create temp file %s: %m", name_buf);
  return fd;
}

#ifdef TEST

#include "ucw/getopt.h"

int main(int argc, char **argv)
{
  log_init(NULL);
  if (cf_getopt(argc, argv, CF_SHORT_OPTS, CF_NO_LONG_OPTS, NULL) >= 0)
    die("Hey, whaddya want?");

  char buf[TEMP_FILE_NAME_LEN];
  int fd = open_tmp(buf, O_RDWR | O_CREAT | O_TRUNC, 0666);
  close(fd);
  unlink(buf);
  return 0;
}

#endif
