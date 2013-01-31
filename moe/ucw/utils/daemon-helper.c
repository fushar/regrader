/*
 *	A Simple Wrapper for Starting and Stopping of Daemons
 *
 *	(c) 2003 Martin Mares <mj@ucw.cz>
 *
 *	It would seem that we are reinventing the wheel and the
 *	start-stop-daemon command present in most Linux distributions
 *	is just what we need, but the usual "does the process already
 *	exist?" strategies fail in presence of multiple running daemons.
 *
 *	Return codes:
 *	101	already running
 *	102	not running
 */

#include "ucw/lib.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <alloca.h>

enum action {
  ACTION_NONE,
  ACTION_START,
  ACTION_STOP,
  ACTION_FORCE_STOP,
  ACTION_CHECK,
  ACTION_RELOAD
};

static int action;

static struct option options[] = {
  { "pid-file",		required_argument,	NULL, 'p' },
  { "status-file",	required_argument,	NULL, 's' },
  { "start",		no_argument,		&action, ACTION_START },
  { "stop",		no_argument,		&action, ACTION_STOP },
  { "force-stop",	no_argument,		&action, ACTION_FORCE_STOP },
  { "check",		no_argument,		&action, ACTION_CHECK },
  { "reload",		no_argument,		&action, ACTION_RELOAD },
  { NULL,		no_argument,		NULL, 0 }
};

static void NONRET
usage(void)
{
  fputs("\n\
Usage: daemon-helper --start <options> -- <daemon> <args>\n\
   or: daemon-helper --stop <options>\n\
   or: daemon-helper --force-stop <options>\n\
   or: daemon-helper --reload <options>\n\
   or: daemon-helper --check <options>\n\
\n\
Options:\n\
--pid-file <name>	Name of PID file for this daemon (mandatory)\n\
--status-file <name>	Status file used by the daemon (deleted just before starting)\n\
", stderr);
  exit(1);
}

int
main(int argc, char **argv)
{
  int c, fd;
  char *pidfile = NULL;
  char *statfile = NULL;
  struct flock fl;
  char buf[64];

  while ((c = getopt_long(argc, argv, "", options, NULL)) >= 0)
    switch (c)
      {
      case 0:
	break;
      case 'p':
	pidfile = optarg;
	break;
      case 's':
	statfile = optarg;
	break;
      default:
	usage();
      }
  if (!pidfile)
    usage();

  bzero(&fl, sizeof(fl));
  fl.l_type = F_WRLCK;
  fl.l_whence = SEEK_SET;

  switch (action)
    {
    case ACTION_START:
      if (optind >= argc)
	usage();
      fd = open(pidfile, O_RDWR | O_CREAT, 0666);
      if (fd < 0)
	die("Unable to create %s: %m", pidfile);
      if ((c = fcntl(fd, F_SETLK, &fl)) < 0)
	{
	  if (errno == EAGAIN || errno == EACCES)
	    return 101;
	  else
	    die("fcntl lock on %s failed: %m", pidfile);
	}
      c = sprintf(buf, "%d\n", getpid());
      if (write(fd, buf, c) != c)
	die("write on %s failed: %m", pidfile);
      if (ftruncate(fd, c) < 0)
	die("truncate on %s failed: %m", pidfile);
      if (statfile && unlink(statfile) < 0 && errno != ENOENT)
	die("unlink(%s) failed: %m", statfile);
      setsid();
      /* Disconnect from stdin and stdout, leave stderr to the daemon. */
      close(0);
      open("/dev/null", O_RDWR, 0);
      dup2(0, 1);
      argv += optind;
      argc -= optind;
      char **a = alloca(sizeof(char *) * (argc+1));
      memcpy(a, argv, sizeof(char *) * argc);
      a[argc] = NULL;
      execv(a[0], a);
      die("Cannot execute %s: %m", a[0]);
    case ACTION_STOP:
    case ACTION_FORCE_STOP:
    case ACTION_CHECK:
    case ACTION_RELOAD:
      if (optind < argc)
	usage();
      fd = open(pidfile, O_RDWR);
      if (fd < 0)
	{
	  if (errno == ENOENT)
	    return 102;
	  else
	    die("Unable to open %s: %m", pidfile);
	}
      if ((c = fcntl(fd, F_SETLK, &fl)) >= 0)
	{
	nopid:
	  unlink(pidfile);
	  return 102;
	}
      if (errno != EAGAIN && errno != EACCES)
	die("fcntl lock on %s failed: %m", pidfile);
      if ((c = read(fd, buf, sizeof(buf))) < 0)
	die("read on %s failed: %m", pidfile);
      if (!c)
	goto nopid;
      if (c >= (int) sizeof(buf) || sscanf(buf, "%d", &c) != 1)
	die("PID file syntax error");
      int sig = 0;
      if (action == ACTION_CHECK || action == ACTION_RELOAD)
	{
	  if (action == ACTION_RELOAD)
	    sig = SIGHUP;
	  if (kill(c, sig) < 0 && errno == ESRCH)
	    goto nopid;
	  return 0;
	}
      sig = (action == ACTION_STOP) ? SIGTERM : SIGQUIT;
      if (kill(c, sig) < 0)
	{
	  if (errno == ESRCH)
	    goto nopid;
	  die("Cannot kill process %d: %m", c);
	}
      if ((c = fcntl(fd, F_SETLKW, &fl)) < 0)
	die("Cannot lock %s: %m", pidfile);
      if (statfile)
	unlink(statfile);
      if (unlink(pidfile) < 0)
	die("Cannot unlink %s: %m", pidfile);
      return 0;
    default:
      usage();
    }
}
