/*
 *	UCW Library -- Mapping of File Parts
 *
 *	(c) 2003--2006 Martin Mares <mj@ucw.cz>
 *	(c) 2003--2009 Robert Spalek <robert@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/lfs.h"
#include "ucw/partmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#ifdef CONFIG_UCW_PARTMAP_IS_MMAP
#define PARTMAP_WINDOW ~(size_t)0
#else
#ifdef TEST
#define PARTMAP_WINDOW 4096
#else
#define PARTMAP_WINDOW 16777216
#endif
#endif

struct partmap *
partmap_open(char *name, int writeable)
{
  struct partmap *p = xmalloc_zero(sizeof(struct partmap));

  p->fd = ucw_open(name, writeable ? O_RDWR : O_RDONLY);
  if (p->fd < 0)
    die("open(%s): %m", name);
  if ((p->file_size = ucw_seek(p->fd, 0, SEEK_END)) < 0)
    die("lseek(%s): %m", name);
  p->writeable = writeable;
#ifdef CONFIG_UCW_PARTMAP_IS_MMAP
  partmap_load(p, 0, p->file_size);
#endif
  return p;
}

ucw_off_t
partmap_size(struct partmap *p)
{
  return p->file_size;
}

void
partmap_close(struct partmap *p)
{
  if (p->start_map)
    munmap(p->start_map, p->end_off - p->start_off);
  close(p->fd);
  xfree(p);
}

void
partmap_load(struct partmap *p, ucw_off_t start, uns size)
{
  if (p->start_map)
    munmap(p->start_map, p->end_off - p->start_off);
  ucw_off_t end = start + size;
  ucw_off_t win_start = start/CPU_PAGE_SIZE * CPU_PAGE_SIZE;
  size_t win_len = PARTMAP_WINDOW;
  if (win_len > (size_t) (p->file_size - win_start))
    win_len = ALIGN_TO(p->file_size - win_start, CPU_PAGE_SIZE);
  if ((ucw_off_t) (win_start+win_len) < end)
    die("partmap_map: Window is too small for mapping %d bytes", size);
  if (win_len)
    {
      p->start_map = ucw_mmap(NULL, win_len, p->writeable ? (PROT_READ | PROT_WRITE) : PROT_READ, MAP_SHARED, p->fd, win_start);
      if (p->start_map == MAP_FAILED)
	die("mmap failed at position %lld: %m", (long long)win_start);
    }
  else
    p->start_map = NULL;
  p->start_off = win_start;
  p->end_off = win_start+win_len;
  madvise(p->start_map, win_len, MADV_SEQUENTIAL);
}

#ifdef TEST
int main(int argc, char **argv)
{
  struct partmap *p = partmap_open(argv[1], 0);
  uns l = partmap_size(p);
  uns i;
  for (i=0; i<l; i++)
    putchar(*(char *)partmap_map(p, i, 1));
  partmap_close(p);
  return 0;
}
#endif
