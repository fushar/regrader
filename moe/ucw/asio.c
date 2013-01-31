/*
 *	UCW Library -- Asynchronous I/O
 *
 *	(c) 2006 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#undef LOCAL_DEBUG

#include "ucw/lib.h"
#include "ucw/asio.h"
#include "ucw/threads.h"

#include <string.h>
#include <unistd.h>
#include <errno.h>

static uns asio_num_users;
static struct worker_pool asio_wpool;

static void
asio_init_unlocked(void)
{
  if (asio_num_users++)
    return;

  DBG("ASIO: INIT");
  asio_wpool.num_threads = 1;
  worker_pool_init(&asio_wpool);
}

static void
asio_cleanup_unlocked(void)
{
  if (--asio_num_users)
    return;

  DBG("ASIO: CLEANUP");
  worker_pool_cleanup(&asio_wpool);
}

void
asio_init_queue(struct asio_queue *q)
{
  ucwlib_lock();
  asio_init_unlocked();
  ucwlib_unlock();

  DBG("ASIO: New queue %p", q);
  ASSERT(q->buffer_size);
  q->allocated_requests = 0;
  q->running_requests = 0;
  q->running_writebacks = 0;
  q->use_count = 0;
  clist_init(&q->idle_list);
  clist_init(&q->done_list);
  work_queue_init(&asio_wpool, &q->queue);
}

void
asio_cleanup_queue(struct asio_queue *q)
{
  DBG("ASIO: Removing queue %p", q);
  ASSERT(!q->running_requests);
  ASSERT(!q->running_writebacks);
  ASSERT(!q->allocated_requests);
  ASSERT(clist_empty(&q->done_list));

  struct asio_request *r;
  while (r = clist_remove_head(&q->idle_list))
    {
      big_free(r->buffer, q->buffer_size);
      xfree(r);
    }

  work_queue_cleanup(&q->queue);

  ucwlib_lock();
  asio_cleanup_unlocked();
  ucwlib_unlock();
}

struct asio_request *
asio_get(struct asio_queue *q)
{
  q->allocated_requests++;
  struct asio_request *r = clist_head(&q->idle_list);
  if (!r)
    {
      r = xmalloc_zero(sizeof(*r));
      r->queue = q;
      r->buffer = big_alloc(q->buffer_size);
      DBG("ASIO: Got %p (new)", r);
    }
  else
    {
      clist_remove(&r->work.n);
      DBG("ASIO: Got %p", r);
    }
  r->op = ASIO_FREE;
  r->fd = -1;
  r->len = 0;
  r->status = -1;
  r->returned_errno = -1;
  r->submitted = 0;
  return r;
}

static int
asio_raw_wait(struct asio_queue *q)
{
  struct asio_request *r = (struct asio_request *) work_wait(&q->queue);
  if (!r)
    return 0;
  r->submitted = 0;
  q->running_requests--;
  if (r->op == ASIO_WRITE_BACK)
    {
      DBG("ASIO: Finished writeback %p", r);
      if (r->status < 0)
	die("Asynchronous write to fd %d failed: %s", r->fd, strerror(r->returned_errno));
      if (r->status != (int)r->len)
	die("Asynchronous write to fd %d wrote only %d bytes out of %d", r->fd, r->status, r->len);
      q->running_writebacks--;
      asio_put(r);
    }
  else
    clist_add_tail(&q->done_list, &r->work.n);
  return 1;
}

static void
asio_handler(struct worker_thread *t UNUSED, struct work *w)
{
  struct asio_request *r = (struct asio_request *) w;

  DBG("ASIO: Servicing %p (%s on fd=%d, len=%d)", r,
      (char*[]) { "?", "READ", "WRITE", "WRITEBACK" }[r->op], r->fd, r->len);
  errno = 0;
  switch (r->op)
    {
    case ASIO_READ:
      r->status = read(r->fd, r->buffer, r->len);
      break;
    case ASIO_WRITE:
    case ASIO_WRITE_BACK:
      r->status = write(r->fd, r->buffer, r->len);
      break;
    default:
      die("ASIO: Got unknown request type %d", r->op);
    }
  r->returned_errno = errno;
  DBG("ASIO: Finished %p (status=%d, errno=%d)", r, r->status, r->returned_errno);
}

void
asio_submit(struct asio_request *r)
{
  struct asio_queue *q = r->queue;
  DBG("ASIO: Submitting %p on queue %p", r, q);
  ASSERT(r->op != ASIO_FREE);
  ASSERT(!r->submitted);
  if (r->op == ASIO_WRITE_BACK)
    {
      while (q->running_writebacks >= q->max_writebacks)
	{
	  DBG("ASIO: Waiting for free writebacks");
	  if (!asio_raw_wait(q))
	    ASSERT(0);
	}
      q->running_writebacks++;
    }
  q->running_requests++;
  r->submitted = 1;
  r->work.go = asio_handler;
  r->work.priority = 0;
  work_submit(&q->queue, &r->work);
}

struct asio_request *
asio_wait(struct asio_queue *q)
{
  struct asio_request *r;
  while (!(r = clist_head(&q->done_list)))
    {
      DBG("ASIO: Waiting on queue %p", q);
      if (!asio_raw_wait(q))
	return NULL;
    }
  clist_remove(&r->work.n);
  DBG("ASIO: Done %p", r);
  return r;
}

void
asio_put(struct asio_request *r)
{
  struct asio_queue *q = r->queue;
  DBG("ASIO: Put %p", r);
  ASSERT(!r->submitted);
  ASSERT(q->allocated_requests);
  clist_add_tail(&q->idle_list, &r->work.n);
  q->allocated_requests--;
}

void
asio_sync(struct asio_queue *q)
{
  DBG("ASIO: Syncing queue %p", q);
  while (q->running_requests)
    if (!asio_raw_wait(q))
      ASSERT(0);
}

#ifdef TEST

int main(void)
{
  struct asio_queue q;
  struct asio_request *r;

  q.buffer_size = 4096;
  q.max_writebacks = 2;
  asio_init_queue(&q);

#if 0

  for (;;)
    {
      r = asio_get(&q);
      r->op = ASIO_READ;
      r->fd = 0;
      r->len = q.buffer_size;
      asio_submit(r);
      r = asio_wait(&q);
      ASSERT(r);
      if (r->status <= 0)
	{
	  asio_put(r);
	  break;
	}
      r->op = ASIO_WRITE_BACK;
      r->fd = 1;
      r->len = r->status;
      asio_submit(r);
    }
  asio_sync(&q);

#else

  r = asio_get(&q);
  r->op = ASIO_READ;
  r->fd = 0;
  r->len = 1;
  asio_submit(r);
  r = asio_wait(&q);
  ASSERT(r);
  asio_put(r);

  for (uns i=0; i<10; i++)
    {
      r = asio_get(&q);
      r->op = ASIO_WRITE_BACK;
      r->fd = 1;
      r->len = 1;
      r->buffer[0] = 'A' + i;
      asio_submit(r);
    }
  asio_sync(&q);

  r = asio_get(&q);
  r->op = ASIO_WRITE;
  r->fd = 1;
  r->len = 1;
  r->buffer[0] = '\n';
  asio_submit(r);
  r = asio_wait(&q);
  ASSERT(r);
  asio_put(r);

#endif

  asio_cleanup_queue(&q);
  return 0;
}

#endif
