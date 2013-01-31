/*
 *	The UCW Library -- POSIX semaphores wrapper
 *
 *	(c) 2006 Robert Spalek <robert@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _UCW_SEMAPHORE_H
#define _UCW_SEMAPHORE_H

#include <semaphore.h>

#ifdef CONFIG_DARWIN

#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include "ucw/fastbuf.h" // For the temp_file_name

/* In Darwin, sem_init() is unfortunately not implemented and the guide
 * recommends emulating it using sem_open().  */

static inline sem_t *
sem_alloc(void)
{
  char buf[TEMP_FILE_NAME_LEN];
  int mode, retry = 10;
  sem_t *sem;
  do
    {
      temp_file_name(buf, &mode);
      sem = sem_open(buf, mode | O_CREAT, 0777, 0);
    }
  while (sem == (sem_t*) SEM_FAILED && errno == EEXIST && retry --);
  ASSERT(sem != (sem_t*) SEM_FAILED);
  return sem;
}

static inline void
sem_free(sem_t *sem)
{
  sem_close(sem);
}

#else

static inline sem_t *
sem_alloc(void)
{
  sem_t *sem = xmalloc(sizeof(sem_t));
  int res = sem_init(sem, 0, 0);
  ASSERT(!res);
  return sem;
}

static inline void
sem_free(sem_t *sem)
{
  sem_destroy(sem);
  xfree(sem);
}

#endif

#endif
