/*
 *	UCW Library -- Thread Pools and Work Queues
 *
 *	(c) 2006 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _UCW_WORKQUEUE_H
#define _UCW_WORKQUEUE_H

/*
 *  A thread pool is a set of threads receiving work requests from a common queue,
 *  each work request contains a pointer to a function inside the thread.
 *
 *  A work queue is an interface for submitting work requests. It's bound to a single
 *  thread pool, it remembers running requests and gathers replies. A single work queue
 *  should not be used by multiple threads simultaneously.
 *
 *  Requests can have priorities. Requests with the highest priority are served first.
 *  Requests of priority 0 are guaranteed to be served on first-come-first-served
 *  basis, requests of higher priorities are unordered.
 *
 *  When a thread pool is initialized, new_thread() is called for every thread first,
 *  allocating struct worker_thread (and user-defined thread context following it) for
 *  each thread. Then the threads are fired and each of them executes the init_thread()
 *  callback. These callbacks are serialized and worker_pool_init() function waits
 *  until all of them finish.
 */

#include "ucw/semaphore.h"
#include "ucw/clists.h"

#include <pthread.h>

struct worker_thread {				// One of threads serving requests
  cnode n;
  pthread_t thread;
  struct worker_pool *pool;
  int id;					// Inside the pool
  /* user-defined data can follow */
};

struct raw_queue {				// Generic queue with locking
  pthread_mutex_t queue_mutex;
  clist pri0_queue;				// Ordinary queue for requests with priority=0
  struct work **pri_heap;			// A heap for request with priority>0
  uns heap_cnt, heap_max;
  sem_t *queue_sem;				// Number of requests queued
};

struct worker_pool {
  struct raw_queue requests;
  uns num_threads;
  uns stack_size;				// 0 for default
  struct worker_thread *(*new_thread)(void);	// default: xmalloc the struct
  void (*free_thread)(struct worker_thread *t);	// default: xfree
  void (*init_thread)(struct worker_thread *t);	// default: empty
  void (*cleanup_thread)(struct worker_thread *t); // default: empty
  clist worker_threads;
  sem_t *init_cleanup_sem;
};

struct work_queue {
  struct worker_pool *pool;
  uns nr_running;				// Number of requests in service
  struct raw_queue finished;			// Finished requests queue up here
};

struct work {					// A single request
  cnode n;
  uns priority;
  struct work_queue *reply_to;			// Where to queue the request when it's finished
  void (*go)(struct worker_thread *t, struct work *w);		// Called inside the worker thread
};

void worker_pool_init(struct worker_pool *p);
void worker_pool_cleanup(struct worker_pool *p);

void raw_queue_init(struct raw_queue *q);
void raw_queue_cleanup(struct raw_queue *q);
void raw_queue_put(struct raw_queue *q, struct work *w);
struct work *raw_queue_get(struct raw_queue *q);
struct work *raw_queue_try_get(struct raw_queue *q);

void work_queue_init(struct worker_pool *p, struct work_queue *q);
void work_queue_cleanup(struct work_queue *q);
void work_submit(struct work_queue *q, struct work *w);
struct work *work_wait(struct work_queue *q);
struct work *work_try_wait(struct work_queue *q);

#endif	/* !_UCW_WORKQUEUE_H */
