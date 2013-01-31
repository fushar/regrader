/*
 *	UCW Library -- Main Loop
 *
 *	(c) 2004--2005 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _UCW_MAINLOOP_H
#define _UCW_MAINLOOP_H

#include "ucw/clists.h"

/***
 * [[conventions]]
 * Conventions
 * -----------
 *
 * The descriptions of structures contain some fields marked with `[*]`.
 * These are the only ones that are intended to be manipulated by the user.
 * The remaining fields serve for internal use only and you must initialize them
 * to zeroes.
 ***/

/***
 * [[time]]
 * Time manipulation
 * -----------------
 *
 * This part allows you to get the current time and request
 * to have your function called when the time comes.
 ***/

extern timestamp_t main_now;			/** Current time in milliseconds since the UNIX epoch. See @main_get_time(). **/
extern ucw_time_t main_now_seconds;		/** Current time in seconds since the epoch. **/
extern timestamp_t main_idle_time;		/** Total time in milliseconds spent in the poll() call. **/
extern clist main_timer_list, main_file_list, main_hook_list, main_process_list;

/**
 * This is a description of a timer.
 * You fill in a handler function, any user-defined data you wish to pass
 * to the handler, and then you invoke @timer_add().
 *
 * The handler() function must either call @timer_del() to delete the timer,
 * or call @timer_add() with a different expiration time.
 **/
struct main_timer {
  cnode n;
  timestamp_t expires;
  void (*handler)(struct main_timer *tm);	/* [*] Function to be called when the timer expires. */
  void *data;					/* [*] Data for use by the handler */
};

/**
 * Adds a new timer into the mainloop to be watched and called
 * when it expires. It can also be used to modify an already running
 * timer. It is permitted (and usual) to call this function from the
 * timer's handler itself if you want the timer to trigger again.
 *
 * The @expire parameter is absolute, just add <<var_main_now,`main_now`>> if you need a relative timer.
 **/
void timer_add(struct main_timer *tm, timestamp_t expires);
/**
 * Removes a timer from the active ones. It is permitted (and usual) to call
 * this function from the timer's handler itself if you want to deactivate
 * the timer.
 **/
void timer_del(struct main_timer *tm);

/**
 * Forces refresh of <<var_main_now,`main_now`>>. You do not usually
 * need to call this, since it is called every time the loop polls for
 * changes. It is here if you need extra precision or some of the
 * hooks takes a long time.
 **/
void main_get_time(void);

/***
 * [[file]]
 * Activity on file descriptors
 * ----------------------------
 *
 * You can let the mainloop watch over a set of file descriptors
 * for a changes.
 *
 * It supports two ways of use. With the first one, you provide
 * low-level handlers for reading and writing (`read_handler` and
 * `write_handler`). They will be called every time the file descriptor
 * is ready to be read from or written to.
 *
 * Return non-zero if you want to get the handler called again right now (you
 * handled a block of data and expect more). If you return `0`, the hook will
 * be called again in the next iteration, if it is still ready to be read/written.
 *
 * This way is suitable for listening sockets, interactive connections, where
 * you need to parse everything that comes right away and similar cases.
 *
 * The second way is to ask mainloop to read or write a buffer of data. You
 * provide a `read_done` or `write_done` handler respectively and call @file_read()
 * or @file_write(). This is handy for data connections where you need to transfer
 * data between two endpoints or for binary connections where the size of message
 * is known in advance.
 *
 * It is possible to combine both methods, but it may be tricky to do it right.
 *
 * Both ways use `error_handler` to notify you about errors.
 ***/

/**
 * If you want mainloop to watch a file descriptor, fill at last `fd` into this
 * structure. To get any useful information from the mainloop, provide some handlers
 * too.
 *
 * After that, insert it into the mainloop by calling @file_add().
 **/
struct main_file {
  cnode n;
  int fd;					/* [*] File descriptor */
  int (*read_handler)(struct main_file *fi);	/* [*] To be called when ready for reading/writing; must call file_chg() afterwards */
  int (*write_handler)(struct main_file *fi);
  void (*error_handler)(struct main_file *fi, int cause);	/* [*] Handler to call on errors */
  void *data;					/* [*] Data for use by the handlers */
  byte *rbuf;					/* Read/write pointers for use by file_read/write */
  uns rpos, rlen;
  byte *wbuf;
  uns wpos, wlen;
  void (*read_done)(struct main_file *fi);	/* [*] Called when file_read is finished; rpos < rlen if EOF */
  void (*write_done)(struct main_file *fi);	/* [*] Called when file_write is finished */
  struct main_timer timer;
  struct pollfd *pollfd;
};

/**
 * Specifies when or why an error happened. This is passed to the error handler.
 * `errno` is still set to the original source of error. The only exception
 * is `MFERR_TIMEOUT`, in which case `errno` is not set and the only possible
 * cause of it is timeout on the file descriptor (see @file_set_timeout).
 **/
enum main_file_err_cause {
  MFERR_READ,
  MFERR_WRITE,
  MFERR_TIMEOUT
};

/**
 * Inserts a <<struct_main_file,`main_file`>> structure into the mainloop to be
 * watched for activity. You can call this at any time, even inside a handler
 * (of course for a different file descriptor than the one of the handler).
 **/
void file_add(struct main_file *fi);
/**
 * Tells the mainloop the file has changed its state. Call it whenever you
 * change any of the handlers.
 *
 * Can be called only on active files (only the ones added by @file_add()).
 **/
void file_chg(struct main_file *fi);
/**
 * Removes a file from the watched set. You have to call this on closed files
 * too, since the mainloop does not handle close in any way.
 *
 * Can be called from a handler.
 **/
void file_del(struct main_file *fi);
/**
 * Asks the mainloop to read @len bytes of data from @fi into @buf.
 * It cancels any previous unfinished read requested this way and overwrites
 * `read_handler`.
 *
 * When the read is done, read_done() handler is called. If an EOF occurred,
 * `rpos < rlen` (eg. not all data were read).
 *
 * Can be called from a handler.
 *
 * You can use a call with zero @len to cancel current read, but all read data
 * will be thrown away.
 **/
void file_read(struct main_file *fi, void *buf, uns len);
/**
 * Requests that the mainloop writes @len bytes of data from @buf to @fi.
 * Cancels any previous unfinished write and overwrites `write_handler`.
 *
 * When it is written, write_done() handler is called.
 *
 * Can be called from a handler.
 *
 * If you call it with zero @len, it will cancel the previous write, but note
 * some data may already be written.
 **/
void file_write(struct main_file *fi, void *buf, uns len);
/**
 * Sets a timer for a file @fi. If the timer is not overwritten or disabled
 * until @expires, the file timeouts and error_handler() is called with
 * <<enum_main_file_err_cause,`MFERR_TIMEOUT`>>.
 *
 * The mainloop does not disable or reset it, when something happens, it just
 * bundles a timer with the file. If you want to watch for inactivity, it is
 * your task to reset it whenever your handler is called.
 *
 * The @expires parameter is absolute (add <<var_main_now,`main_now`>> if you
 * need relative). The call and overwrites previously set timeout. Value of `0`
 * disables the timeout (the <<enum_main_file_err_cause,`MFERR_TIMEOUT`>> will
 * not trigger).
 *
 * The use-cases for this are mainly sockets or pipes, when:
 *
 * - You want to drop inactive connections (no data come or go for a given time, not
 *   incomplete messages).
 * - You want to enforce answer in a given time (for example authentication).
 * - You give maximum time for a whole connection.
 **/
void file_set_timeout(struct main_file *fi, timestamp_t expires);
/**
 * Closes all file descriptors known to mainloop. Often used between fork()
 * and exec().
 **/
void file_close_all(void);

/***
 * [[hooks]]
 * Loop hooks
 * ----------
 *
 * The hooks are called whenever the mainloop perform an iteration.
 * You can shutdown the mainloop from within them or request an iteration
 * to happen without sleeping (just poll, no waiting for events).
 ***/

/**
 * A hook. It contains the function to call and some user data.
 *
 * The handler() must return one value from
 * <<enum_main_hook_return,`main_hook_return`>>.
 *
 * Fill with the hook and data and pass it to @hook_add().
 **/
struct main_hook {
  cnode n;
  int (*handler)(struct main_hook *ho);		/* [*] Hook function; returns HOOK_xxx */
  void *data;					/* [*] For use by the handler */
};

/**
 * Return value of the hook handler().
 * Specifies what should happen next.
 *
 * - `HOOK_IDLE` -- Let the loop sleep until something happens, call after that.
 * - `HOOK_RETRY` -- Force the loop to perform another iteration without sleeping.
 *   This will cause calling of all the hooks again soon.
 * - `HOOK_DONE` -- The loop will terminate if all hooks return this.
 * - `HOOK_SHUTDOWN` -- Shuts down the loop.
 **/
enum main_hook_return {
  HOOK_IDLE,
  HOOK_RETRY,
  HOOK_DONE = -1,
  HOOK_SHUTDOWN = -2
};

/**
 * Inserts a new hook into the loop.
 * May be called from inside a hook handler too.
 **/
void hook_add(struct main_hook *ho);
/**
 * Removes an existing hook from the loop.
 * May be called from inside a hook handler (to delete itself or other hook).
 **/
void hook_del(struct main_hook *ho);

/***
 * [[process]]
 * Child processes
 * ---------------
 *
 * The main loop can watch child processes and notify you,
 * when some of them terminates.
 ***/

/**
 * Description of a watched process.
 * You fill in the handler() and `data`.
 * The rest is set with @process_fork().
 **/
struct main_process {
  cnode n;
  int pid;					/* Process id (0=not running) */
  int status;					/* Exit status (-1=fork failed) */
  char status_msg[EXIT_STATUS_MSG_SIZE];
  void (*handler)(struct main_process *mp);	/* [*] Called when the process exits; process_del done automatically */
  void *data;					/* [*] For use by the handler */
};

/**
 * Asks the mainloop to watch this process.
 * As it is done automatically in @process_fork(), you need this only
 * if you removed the process previously by @process_del().
 **/
void process_add(struct main_process *mp);
/**
 * Removes the process from the watched set. This is done
 * automatically, when the process terminates, so you need it only
 * when you do not want to watch a running process any more.
 */
void process_del(struct main_process *mp);
/**
 * Forks and fills the @mp with information about the new process.
 *
 * If the fork() succeeds, it:
 *
 * - Returns 0 in the child.
 * - Returns 1 in the parent and calls @process_add() on it.
 *
 * In the case of unsuccessful fork(), it:
 *
 * - Fills in the `status_msg` and sets `status` to -1.
 * - Calls the handler() as if the process terminated.
 * - Returns 1.
 **/
int process_fork(struct main_process *mp);

/***
 * [[control]]
 * Control of the mainloop
 * -----------------------
 *
 * These functions control the mainloop as a whole.
 ***/

extern uns main_shutdown;			/** Setting this to nonzero forces the @main_loop() function to terminate. **/
void main_init(void);				/** Initializes the mainloop structures. Call before any `*_add` function. **/
/**
 * Start the mainloop.
 * It will watch the provided objects and call callbacks.
 * Terminates when someone sets <<var_main_shutdown,`main_shutdown`>>
 * to nonzero, when all <<hook,hooks>> return
 * <<enum_main_hook_return,`HOOK_DONE`>> or at last one <<hook,hook>>
 * returns <<enum_main_hook_return,`HOOK_SHUTDOWN`>>.
 **/
void main_loop(void);
void main_debug(void);				/** Prints a lot of debug information about current status of the mainloop. **/

#endif
