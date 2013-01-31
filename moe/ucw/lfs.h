/*
 *	UCW Library -- Large File Support
 *
 *	(c) 1999--2002 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _UCW_LFS_H
#define _UCW_LFS_H

#include <fcntl.h>
#include <unistd.h>

#ifdef CONFIG_LFS

#define ucw_open open64
#define ucw_seek lseek64
#define ucw_pread pread64
#define ucw_pwrite pwrite64
#define ucw_ftruncate ftruncate64
#define ucw_mmap(a,l,p,f,d,o) mmap64(a,l,p,f,d,o)
#define ucw_pread pread64
#define ucw_pwrite pwrite64
#define ucw_stat stat64
#define ucw_fstat fstat64
typedef struct stat64 ucw_stat_t;

#else	/* !CONFIG_LFS */

#define ucw_open open
#define ucw_seek(f,o,w) lseek(f,o,w)
#define ucw_ftruncate(f,o) ftruncate(f,o)
#define ucw_mmap(a,l,p,f,d,o) mmap(a,l,p,f,d,o)
#define ucw_pread pread
#define ucw_pwrite pwrite
#define ucw_stat stat
#define ucw_fstat fstat
typedef struct stat ucw_stat_t;

#endif	/* !CONFIG_LFS */

#if defined(_POSIX_SYNCHRONIZED_IO) && (_POSIX_SYNCHRONIZED_IO > 0)
#define ucw_fdatasync fdatasync
#else
#define ucw_fdatasync fsync
#endif

#define HAVE_PREAD

static inline ucw_off_t
ucw_file_size(const char *name)
{
  int fd = ucw_open(name, O_RDONLY);
  if (fd < 0)
    die("Cannot open %s: %m", name);
  ucw_off_t len = ucw_seek(fd, 0, SEEK_END);
  close(fd);
  return len;
}

#endif	/* !_UCW_LFS_H */
