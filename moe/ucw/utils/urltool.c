/*
 *	Sherlock Utilities -- URL Handling Tool
 *
 *	(c) 2004 Martin Mares <mj@ucw.cz>
 */

#include "ucw/lib.h"
#include "ucw/getopt.h"
#include "ucw/url.h"
#include "ucw/fastbuf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static byte *base_url;
static struct url base;
static uns opt_split = 0, opt_normalize = 0, opt_forgive = 0;
static struct fastbuf *fout;
static uns err_count;

static void
process_url(byte *url)
{
  byte buf1[MAX_URL_SIZE], buf2[MAX_URL_SIZE], buf3[MAX_URL_SIZE], buf4[MAX_URL_SIZE];
  int e;
  struct url ur;

  if ((e = url_deescape(url, buf1)) || (e = url_split(buf1, &ur, buf2)))
    goto error;
  if ((base_url || opt_normalize) && (e = url_normalize(&ur, &base)))
    goto error;
  if (opt_normalize && (e = url_canonicalize(&ur)))
    goto error;
  if (opt_split)
    {
      if (ur.protocol)
	bprintf(fout, "protocol=%s\n", ur.protocol);
      if (ur.user)
	bprintf(fout, "user=%s\n", ur.user);
      if (ur.pass)
	bprintf(fout, "pass=%s\n", ur.pass);
      if (ur.host)
	bprintf(fout, "host=%s\n", ur.host);
      if (ur.port != ~0U)
	bprintf(fout, "port=%d\n", ur.port);
      if (ur.rest)
	bprintf(fout, "rest=%s\n", ur.rest);
      bputc(fout, '\n');
    }
  else
    {
      if ((e = url_pack(&ur, buf3)) || (e = url_enescape(buf3, buf4)))
	goto error;
      bprintf(fout, "%s\n", buf4);
    }
  return;

 error:
  msg(L_ERROR, "%s: %s", url, url_error(e));
  err_count++;
}

static char *shortopts = CF_SHORT_OPTS "b:fns";
static struct option longopts[] =
{
  CF_LONG_OPTS
  { "base",		1, 0, 'b' },
  { "forgive",		0, 0, 'f' },
  { "normalize",	0, 0, 'n' },
  { "split",		0, 0, 's' },
  { NULL,		0, 0, 0 }
};

static char *help = "\
Usage: urltool [<options>] <operations> [<URL's>]\n\
\n\
Options:\n"
CF_USAGE "\
-b, --base <URL>\tInput URL's are relative to this base\n\
-f, --forgive\t\tReturn exit status 0 even if there were errors\n\
\n\
Operations:\n\
-s, --split\t\tSplit a given URL to components\n\
-n, --normalize\t\tNormalize given URL\n\
";

static void NONRET
usage(byte *msg)
{
  if (msg)
    {
      fputs(msg, stderr);
      fputc('\n', stderr);
    }
  fputs(help, stderr);
  exit(1);
}

int
main(int argc, char **argv)
{
  int opt, err;
  byte *base_url = NULL;
  byte basebuf1[MAX_URL_SIZE], basebuf2[MAX_URL_SIZE];

  log_init(argv[0]);
  while ((opt = cf_getopt(argc, argv, shortopts, longopts, NULL)) >= 0)
    switch (opt)
      {
      case 'b':
	base_url = optarg;
	err = url_canon_split(base_url, basebuf1, basebuf2, &base);
	if (err)
	  die("Invalid base URL: %s", url_error(err));
	break;
      case 's':
	opt_split = 1;
	break;
      case 'n':
	opt_normalize = 1;
	break;
      case 'f':
	opt_forgive = 1;
	break;
      default:
	usage("Invalid option");
      }

  fout = bfdopen_shared(1, 4096);
  if (optind >= argc)
    {
      struct fastbuf *fin = bfdopen_shared(0, 4096);
      byte url[MAX_URL_SIZE];
      while (bgets(fin, url, sizeof(url)))
	process_url(url);
      bclose(fin);
    }
  else
    while (optind < argc)
      process_url(argv[optind++]);
  bclose(fout);

  return (err_count && !opt_forgive);
}
