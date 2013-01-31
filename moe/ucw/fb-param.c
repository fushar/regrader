/*
 *	UCW Library -- FastIO on files with run-time parametrization
 *
 *	(c) 2007 Pavel Charvat <pchar@ucw.cz>
 *	(c) 2007 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/conf.h"
#include "ucw/lfs.h"
#include "ucw/fastbuf.h"

#include <fcntl.h>
#include <stdio.h>

struct fb_params fbpar_def = {
  .buffer_size = 65536,
  .read_ahead = 1,
  .write_back = 1,
};

static char *
fbpar_cf_commit(struct fb_params *p UNUSED)
{
  if (p->type == FB_DIRECT)
    {
#ifndef CONFIG_UCW_THREADS
      return "Direct I/O is supported only with CONFIG_UCW_THREADS";
#endif
#ifdef CONFIG_DARWIN
      return "Direct I/O is not supported on darwin";
#endif
#ifndef CONFIG_DIRECT_IO
      return "Direct I/O disabled by configure switch -CONFIG_DIRECT_IO";
#endif
#ifndef CONFIG_UCW_FB_DIRECT
      return "Direct I/O disabled by configure switch -CONFIG_UCW_FB_DIRECT";
#endif
    }
  return NULL;
}

struct cf_section fbpar_cf = {
# define F(x) PTR_TO(struct fb_params, x)
  CF_TYPE(struct fb_params),
  CF_COMMIT(fbpar_cf_commit),
  CF_ITEMS {
    CF_LOOKUP("Type", (int *)F(type), ((const char * const []){"std", "direct", "mmap", NULL})),
    CF_UNS("BufSize", F(buffer_size)),
    CF_UNS("KeepBackBuf", F(keep_back_buf)),
    CF_UNS("ReadAhead", F(read_ahead)),
    CF_UNS("WriteBack", F(write_back)),
    CF_END
  }
# undef F
};

static struct cf_section fbpar_global_cf = {
  CF_ITEMS {
    CF_SECTION("Defaults", &fbpar_def, &fbpar_cf),
    CF_END
  }
};

static void CONSTRUCTOR
fbpar_global_init(void)
{
  cf_declare_section("FBParam", &fbpar_global_cf, 0);
}

static struct fastbuf *
bopen_fd_internal(int fd, struct fb_params *params, uns mode, const char *name)
{
  char buf[32];
  if (!name)
    {
      sprintf(buf, "fd%d", fd);
      name = buf;
    }
  struct fastbuf *fb;
  switch (params->type)
    {
#ifdef CONFIG_UCW_FB_DIRECT
      case FB_DIRECT:
	fb = fbdir_open_fd_internal(fd, name, params->asio,
	    params->buffer_size ? : fbpar_def.buffer_size,
	    params->read_ahead ? : fbpar_def.read_ahead,
	    params->write_back ? : fbpar_def.write_back);
	if (!~mode && !fbdir_cheat && ((int)(mode = fcntl(fd, F_GETFL)) < 0 || fcntl(fd, F_SETFL, mode | O_DIRECT)) < 0)
          msg(L_WARN, "Cannot set O_DIRECT on fd %d: %m", fd);
	return fb;
#endif
      case FB_STD:
	fb = bfdopen_internal(fd, name,
	    params->buffer_size ? : fbpar_def.buffer_size);
	if (params->keep_back_buf)
	  bconfig(fb, BCONFIG_KEEP_BACK_BUF, 1);
	return fb;
      case FB_MMAP:
	if (!~mode && (int)(mode = fcntl(fd, F_GETFL)) < 0)
          die("Cannot get flags of fd %d: %m", fd);
	return bfmmopen_internal(fd, name, mode);
      default:
	ASSERT(0);
    }
}

static struct fastbuf *
bopen_file_internal(const char *name, int mode, struct fb_params *params, int try)
{
  if (!params)
    params = &fbpar_def;
#ifdef CONFIG_UCW_FB_DIRECT
  if (params->type == FB_DIRECT && !fbdir_cheat)
    mode |= O_DIRECT;
#endif
  if (params->type == FB_MMAP && (mode & O_ACCMODE) == O_WRONLY)
    mode = (mode & ~O_ACCMODE) | O_RDWR;
  int fd = ucw_open(name, mode, 0666);
  if (fd < 0)
    if (try)
      return NULL;
    else
      die("Unable to %s file %s: %m", (mode & O_CREAT) ? "create" : "open", name);
  struct fastbuf *fb = bopen_fd_internal(fd, params, mode, name);
  ASSERT(fb);
  if (mode & O_APPEND)
    bseek(fb, 0, SEEK_END);
  return fb;
}

struct fastbuf *
bopen_file(const char *name, int mode, struct fb_params *params)
{
  return bopen_file_internal(name, mode, params, 0);
}

struct fastbuf *
bopen_file_try(const char *name, int mode, struct fb_params *params)
{
  return bopen_file_internal(name, mode, params, 1);
}

struct fastbuf *
bopen_fd_name(int fd, struct fb_params *params, const char *name)
{
  return bopen_fd_internal(fd, params ? : &fbpar_def, ~0U, name);
}

/* Function for use by individual file back-ends */

void
bclose_file_helper(struct fastbuf *f, int fd, int is_temp_file)
{
  switch (is_temp_file)
    {
    case 1:
      if (unlink(f->name) < 0)
	msg(L_ERROR, "unlink(%s): %m", f->name);
    case 0:
      if (close(fd))
	die("close(%s): %m", f->name);
    }
}

/* Compatibility wrappers */

struct fastbuf *
bopen_try(const char *name, uns mode, uns buflen)
{
  return bopen_file_try(name, mode, &(struct fb_params){ .type = FB_STD, .buffer_size = buflen });
}

struct fastbuf *
bopen(const char *name, uns mode, uns buflen)
{
  return bopen_file(name, mode, &(struct fb_params){ .type = FB_STD, .buffer_size = buflen });
}

struct fastbuf *
bfdopen(int fd, uns buflen)
{
  return bopen_fd(fd, &(struct fb_params){ .type = FB_STD, .buffer_size = buflen });
}

struct fastbuf *
bfdopen_shared(int fd, uns buflen)
{
  struct fastbuf *f = bfdopen(fd, buflen);
  bconfig(f, BCONFIG_IS_TEMP_FILE, 2);
  return f;
}
