/*
 *	UCW Library -- Asynchronous I/O
 *
 *	(c) 2006 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _UCW_ASIO_H
#define _UCW_ASIO_H

#include "ucw/workqueue.h"
#include "ucw/clists.h"

/*
 *  This module takes care of scheduling and executing asynchronous I/O requests
 *  on files opened with O_DIRECT. It is primarily used by the fb-direct fastbuf
 *  back-end, but you can use it explicitly, too.
 *
 *  You can define several I/O queues, each for use by a single thread. Requests
 *  on a single queue are always processed in order of their submits, requests
 *  from different queues may be interleaved (although the current implementation
 *  does not do so). Normal read and write requests are returned to their queue
 *  when they are completed. Write-back requests are automatically freed when
 *  done, but the number of such requests in fly is limited in order to avoid
 *  consuming all memory, so a submit of a write-back request can block.
 */

struct asio_queue {
  uns buffer_size;			// How large buffers do we use [user-settable]
  uns max_writebacks;			// Maximum number of writeback requests active [user-settable]
  uns allocated_requests;
  uns running_requests;			// Total number of running requests
  uns running_writebacks;		// How many of them are writebacks
  clist idle_list;			// Recycled requests waiting for get
  clist done_list;			// Finished requests
  struct work_queue queue;
  uns use_count;			// For use by the caller
};

enum asio_op {
  ASIO_FREE,
  ASIO_READ,
  ASIO_WRITE,
  ASIO_WRITE_BACK,			// Background write with no success notification
};

struct asio_request {
  struct work work;			// asio_requests are internally just work nodes
  struct asio_queue *queue;
  byte *buffer;
  int fd;
  enum asio_op op;
  uns len;
  int status;
  int returned_errno;
  int submitted;
  void *user_data;			// For use by the caller
};

void asio_init_queue(struct asio_queue *q);			// Initialize a new queue
void asio_cleanup_queue(struct asio_queue *q);
struct asio_request *asio_get(struct asio_queue *q);		// Get an empty request
void asio_submit(struct asio_request *r);			// Submit the request (can block if too many writebacks)
struct asio_request *asio_wait(struct asio_queue *q);		// Wait for the first finished request, NULL if no more
void asio_put(struct asio_request *r);				// Return a finished request for recycling
void asio_sync(struct asio_queue *q);				// Wait until all requests are finished

#endif	/* !_UCW_ASIO_H */
