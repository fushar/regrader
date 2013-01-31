/*
 *	UCW Library -- A Simple Millisecond Timer
 *
 *	(c) 2007 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

timestamp_t
get_timestamp(void)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (timestamp_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void
init_timer(timestamp_t *timer)
{
  *timer = get_timestamp();
}

uns
get_timer(timestamp_t *timer)
{
  timestamp_t t = *timer;
  *timer = get_timestamp();
  return MIN(*timer-t, ~0U);
}

uns
switch_timer(timestamp_t *oldt, timestamp_t *newt)
{
  *newt = get_timestamp();
  return MIN(*newt-*oldt, ~0U);
}
