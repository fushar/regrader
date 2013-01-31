/*
 *	The UCW Library -- Threading Helpers
 *
 *	(c) 2006 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _UCW_THREAD_H
#define _UCW_THREAD_H

/* This structure holds per-thread data */

struct ucwlib_context {
  int thread_id;			// Thread ID (either kernel tid or a counter)
  int temp_counter;			// Counter for fb-temp.c
  struct asio_queue *io_queue;		// Async I/O queue for fb-direct.c
  ucw_sighandler_t *signal_handlers;	// Signal handlers for sighandler.c
};

struct ucwlib_context *ucwlib_thread_context(void);

/* Global lock used for initialization, cleanup and other not so frequently accessed global state */

void ucwlib_lock(void);
void ucwlib_unlock(void);

#ifdef CONFIG_UCW_THREADS

extern uns ucwlib_thread_stack_size;

#endif

#endif
