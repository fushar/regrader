/*
 *	UCW Library -- Rate Limiting based on the Token Bucket Filter
 *
 *	(c) 2009 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/tbf.h"

void
tbf_init(struct token_bucket_filter *f)
{
  if (!f->burst)
    f->burst = MAX(2*f->rate, 1);
  f->last_hit = 0;
  f->bucket = f->burst;
}

int
tbf_limit(struct token_bucket_filter *f, timestamp_t now)
{
  timestamp_t delta_t = now - f->last_hit;
  f->last_hit = now;

  double b = f->bucket + f->rate * delta_t / 1000;
  b = MIN(b, f->burst);
  if (b >= 1)
    {
      uns dropped = f->drop_count;
      f->bucket = b - 1;
      f->drop_count = 0;
      return dropped;
    }
  else
    {
      f->bucket = b;
      f->drop_count++;
      return -f->drop_count;
    }
}

#ifdef TEST

int main(void)
{
  struct token_bucket_filter t = { .rate = 1, .burst = 2 };
  tbf_init(&t);
  for (timestamp_t now = 0; now < 3000; now += 77)
    {
      int res = tbf_limit(&t, now);
      msg(L_DEBUG, "t=%u result=%d bucket=%f", (uns) now, res, t.bucket);
    }
  return 0;
}

#endif
