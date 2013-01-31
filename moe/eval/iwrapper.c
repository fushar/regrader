/*
 *	A Wrapper for Interactive Tests
 *
 *	(c) 2001 Martin Mares <mj@ucw.cz>
 */

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/time.h>
#include <sys/ptrace.h>
#include <sys/signal.h>
#include <sys/sysinfo.h>
#include <sys/syscall.h>
#include <sys/resource.h>

#define NONRET __attribute__((noreturn))
#define UNUSED __attribute__((unused))

static void NONRET __attribute__((format(printf,1,2)))
die(char *msg, ...)
{
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  fputc('\n', stderr);
  exit(1);
}

static void
copy(int fd)
{
  char buf[4096];
  int c;

  while (c = read(fd, buf, sizeof(buf)))
    write(2, buf, c);
}

int
main(int argc, char **argv)
{
  int sep, nbox, nchk;
  char **abox, **achk;
  pid_t pbox, pchk;
  int inpipe[2], outpipe[2], boxepipe[2], chkepipe[2];
  int exited = 0, sbox, schk;

  for (sep=1; sep < argc; sep++)
    if (!strcmp(argv[sep], "@@"))
      break;
  if (sep >= argc - 1)
    die("Usage: iwrapper <testee> @@ <tester>");
  nbox = sep - 1;
  abox = alloca((nbox+1) * sizeof(char *));
  memcpy(abox, argv+1, nbox*sizeof(char *));
  abox[nbox] = NULL;
  nchk = argc - sep - 1;
  achk = alloca((nchk+1) * sizeof(char *));
  memcpy(achk, argv+sep+1, nchk*sizeof(char *));
  achk[nchk] = NULL;

  if (pipe(inpipe) < 0 ||
      pipe(outpipe) < 0 ||
      pipe(boxepipe) < 0 ||
      pipe(chkepipe) < 0)
    die("pipe: %m");

  pbox = fork();
  if (pbox < 0)
    die("fork: %m");
  if (!pbox)
    {
      close(inpipe[1]);
      close(0);
      dup(inpipe[0]);
      close(inpipe[0]);
      close(outpipe[0]);
      close(1);
      dup(outpipe[1]);
      close(outpipe[1]);
      close(boxepipe[0]);
      close(2);
      dup(boxepipe[1]);
      close(boxepipe[1]);
      close(chkepipe[0]);
      close(chkepipe[1]);
      execv(abox[0], abox);
      die("exec: %m");
    }

  pchk = fork();
  if (pchk < 0)
    die("fork: %m");
  if (!pchk)
    {
      close(inpipe[0]);
      close(1);
      dup(inpipe[1]);
      close(inpipe[1]);
      close(outpipe[1]);
      close(0);
      dup(outpipe[0]);
      close(outpipe[0]);
      close(chkepipe[0]);
      close(2);
      dup(chkepipe[1]);
      close(chkepipe[1]);
      close(boxepipe[0]);
      close(boxepipe[1]);
      execv(achk[0], achk);
      die("exec: %m");
    }

  close(inpipe[0]);
  close(inpipe[1]);
  close(outpipe[0]);
  close(outpipe[1]);
  close(chkepipe[1]);
  close(boxepipe[1]);

  sbox = schk = 0;
  while (exited != 3)
    {
      int st;
      pid_t p = wait(&st);
      if (p < 0)
	die("wait: %m");
      if (p == pbox)
	{
	  exited |= 1;
	  sbox = st;
	}
      else if (p == pchk)
	{
	  exited |= 2;
	  schk = st;
	}
      else
	die("Unknown process %d died", p);
    }

  if (!WIFEXITED(sbox))
    die("Sandbox fault, status=%x", sbox);
  if (!WIFEXITED(schk) || WEXITSTATUS(schk) >= 100)
    die("Checker fault, status=%x", schk);
  if (WEXITSTATUS(sbox))
    {
      copy(boxepipe[0]);
      copy(chkepipe[0]);
      return 1;
    }
  else if (WEXITSTATUS(schk))
    {
      copy(chkepipe[0]);
      copy(boxepipe[0]);
      return 1;
    }
  copy(boxepipe[0]);
  copy(chkepipe[0]);
  return 0;
}
