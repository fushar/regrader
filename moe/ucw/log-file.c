/*
 *	UCW Library -- Logging to Files
 *
 *	(c) 1997--2009 Martin Mares <mj@ucw.cz>
 *	(c) 2008 Tomas Gavenciak <gavento@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/log.h"
#include "ucw/log-internal.h"
#include "ucw/lfs.h"
#include "ucw/threads.h"
#include "ucw/simple-lists.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

struct file_stream {
  struct log_stream ls;		// ls.name is the current name of the log file
  int fd;
  uns flags;			// FF_xxx
  char *orig_name;		// Original name with strftime escapes
};

#define MAX_EXPAND 64		// Maximum size of expansion of strftime escapes

static int log_switch_nest;

static void
do_log_reopen(struct file_stream *fs, const char *name)
{
  int fd = ucw_open(name, O_WRONLY | O_CREAT | O_APPEND, 0666);
  if (fd < 0)
    die("Unable to open log file %s: %m", name);
  if (fs->fd >= 0)
    close(fs->fd);
  fs->fd = fd;
  if (fs->flags & FF_FD2_FOLLOWS)
    dup2(fd, 2);
  if (fs->ls.name)
    {
      xfree(fs->ls.name);
      fs->ls.name = NULL;	// We have to keep the stream consistent -- die() below can invoke logging
    }
  fs->ls.name = xstrdup(name);
}

static int
do_log_switch(struct file_stream *fs, struct tm *tm)
{
  if (!(fs->flags & FF_FORMAT_NAME))
    {
      if (fs->fd >= 0)
	return 1;
      else
	{
	  do_log_reopen(fs, fs->orig_name);
	  return 1;
	}
    }

  int buflen = strlen(fs->orig_name) + MAX_EXPAND;
  char name[buflen];
  int switched = 0;

  ucwlib_lock();
  if (!log_switch_nest)		// Avoid infinite loops if we die when switching logs
    {
      log_switch_nest++;
      int l = strftime(name, buflen, fs->orig_name, tm);
      if (l < 0 || l >= buflen)
	die("Error formatting log file name: %m");
      if (!fs->ls.name || strcmp(name, fs->ls.name))
	{
	  do_log_reopen(fs, name);
	  switched = 1;
	}
      log_switch_nest--;
    }
  ucwlib_unlock();
  return switched;
}

static void
file_close(struct log_stream *ls)
{
  struct file_stream *fs = (struct file_stream *) ls;
  if ((fs->flags & FF_CLOSE_FD) && fs->fd >= 0)
    close(fs->fd);
  xfree(fs->ls.name);
  xfree(fs->orig_name);
}

static int
file_handler(struct log_stream *ls, struct log_msg *m)
{
  struct file_stream *fs = (struct file_stream *) ls;
  if ((fs->flags & FF_FORMAT_NAME) && m->tm)
    do_log_switch(fs, m->tm);

  int r = write(fs->fd, m->m, m->m_len);
  return ((r < 0) ? errno : 0);
}

struct log_stream *
log_new_fd(int fd, uns flags)
{
  struct log_stream *ls = log_new_stream(sizeof(struct file_stream));
  struct file_stream *fs = (struct file_stream *) ls;
  fs->fd = fd;
  fs->flags = flags;
  ls->msgfmt = LSFMT_DEFAULT;
  ls->handler = file_handler;
  ls->close = file_close;
  ls->name = xmalloc(16);
  snprintf(ls->name, 16, "fd%d", fd);
  return ls;
}

struct log_stream *
log_new_file(const char *path, uns flags)
{
  struct log_stream *ls = log_new_stream(sizeof(struct file_stream));
  struct file_stream *fs = (struct file_stream *) ls;
  fs->fd = -1;
  fs->orig_name = xstrdup(path);
  if (strchr(path, '%'))
    fs->flags = FF_FORMAT_NAME;
  fs->flags |= FF_CLOSE_FD | flags;
  ls->msgfmt = LSFMT_DEFAULT;
  ls->handler = file_handler;
  ls->close = file_close;

  time_t now = time(NULL);
  struct tm *tm = localtime(&now);
  ASSERT(tm);
  do_log_switch(fs, tm);		// die()'s on errors
  return ls;
}

int
log_switch(void)
{
  time_t now = time(NULL);
  struct tm *tm = localtime(&now);
  ASSERT(tm);

  int switched = 0;
  for (int i=0; i < log_streams_after; i++)
    if (log_streams.ptr[i]->handler == file_handler)
      switched |= do_log_switch((struct file_stream *) log_streams.ptr[i], tm);
  return switched;
}

void
log_switch_disable(void)
{
  log_switch_nest++;
}

void
log_switch_enable(void)
{
  ASSERT(log_switch_nest);
  log_switch_nest--;
}

void
log_file(const char *name)
{
  if (!name)
    return;

  struct log_stream *ls = log_new_file(name, FF_FD2_FOLLOWS);
  struct log_stream *def = log_stream_by_flags(0);
  log_rm_substream(def, NULL);
  log_add_substream(def, ls);
  log_close_stream(ls);
}

#ifdef TEST

int main(int argc, char **argv)
{
  log_init(argv[0]);
  log_file("/proc/self/fd/1");
  // struct log_stream *ls = log_new_fd(1, 0);
  // struct log_stream *ls = log_new_file("/tmp/quork-%Y%m%d-%H%M%S", 0);
  for (int i=1; i<argc; i++)
    msg(L_INFO, argv[i]);
  log_close_all();
  return 0;
}

#endif
