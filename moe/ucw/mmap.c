/*
 *	UCW Library -- Mapping of Files
 *
 *	(c) 1999--2002 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

void *
mmap_file(const char *name, unsigned *len, int writeable)
{
  int fd = open(name, writeable ? O_RDWR : O_RDONLY);
  struct stat st;
  void *x;

  if (fd < 0)
    die("open(%s): %m", name);
  if (fstat(fd, &st) < 0)
    die("fstat(%s): %m", name);
  if (len)
    *len = st.st_size;
  if (st.st_size)
    {
      x = mmap(NULL, st.st_size, writeable ? (PROT_READ | PROT_WRITE) : PROT_READ, MAP_SHARED, fd, 0);
      if (x == MAP_FAILED)
	die("mmap(%s): %m", name);
    }
  else	/* For empty file, we can return any non-zero address */
    x = "";
  close(fd);
  return x;
}

void
munmap_file(void *start, unsigned len)
{
  munmap(start, len);
}
