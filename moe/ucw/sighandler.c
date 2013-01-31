/*
 *	UCW Library -- Catching of signals and calling callback functions
 *
 *	(c) 2004, Robert Spalek <robert@ucw.cz>
 *	(c) 2006 Martin Mares <mj@ucw.cz>
 */

#include "ucw/lib.h"
#include "ucw/threads.h"

#include <stdlib.h>
#include <string.h>
#include <signal.h>

static int sig_handler_nest[NSIG];
static struct sigaction sig_handler_old[NSIG];

static void
signal_handler_internal(int sig)
{
  struct ucwlib_context *ctx = ucwlib_thread_context();
  if (!ctx->signal_handlers || !ctx->signal_handlers[sig] || ctx->signal_handlers[sig](sig))
    abort();
}

void
handle_signal(int signum)
{
  ucwlib_lock();
  if (!sig_handler_nest[signum]++)
    {
      struct sigaction act;
      bzero(&act, sizeof(act));
      act.sa_handler = signal_handler_internal;
      act.sa_flags = SA_NODEFER;
      if (sigaction(signum, &act, &sig_handler_old[signum]) < 0)
	die("sigaction: %m");
    }
  ucwlib_unlock();
}

void
unhandle_signal(int signum)
{
  ucwlib_lock();
  ASSERT(sig_handler_nest[signum]);
  if (!--sig_handler_nest[signum])
    {
      if (sigaction(signum, &sig_handler_old[signum], NULL) < 0)
	die("sigaction: %m");
    }
  ucwlib_unlock();
}

ucw_sighandler_t
set_signal_handler(int signum, ucw_sighandler_t newh)
{
  struct ucwlib_context *ctx = ucwlib_thread_context();
  if (!ctx->signal_handlers)
    ctx->signal_handlers = xmalloc_zero(NSIG * sizeof(ucw_sighandler_t));
  ucw_sighandler_t old = ctx->signal_handlers[signum];
  ctx->signal_handlers[signum] = newh;
  return old;
}
