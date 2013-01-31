/*
 *	UCW Library -- Logging to Syslog
 *
 *	(c) 2009 Martin Mares <mj@ucw.cz>
 *	(c) 2008 Tomas Gavenciak <gavento@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/log.h"

#include <string.h>
#include <syslog.h>

struct syslog_stream {
  struct log_stream ls;
  int facility;
};

static int syslog_open_count;

static void
syslog_close(struct log_stream *ls UNUSED)
{
  if (!--syslog_open_count)
    closelog();
}

/* Convert syslog facility to its identifier. */
static int
syslog_facility(const char *name)
{
  // Unfortunately, there is no standard way how to get at the list of facility names
  static const struct {
    const char *name;
    int id;
  } facilities[] = {
    { "auth",		LOG_AUTH },
    { "authpriv",	LOG_AUTHPRIV },
    { "cron",		LOG_CRON },
    { "daemon",		LOG_DAEMON },
    { "ftp",		LOG_FTP },
    { "kern",		LOG_KERN },
    { "lpr",		LOG_LPR },
    { "mail",		LOG_MAIL },
    { "news",		LOG_NEWS },
    { "syslog",		LOG_SYSLOG },
    { "user",		LOG_USER },
    { "uucp",		LOG_UUCP },
    { "local0",		LOG_LOCAL0 },
    { "local1",		LOG_LOCAL1 },
    { "local2",		LOG_LOCAL2 },
    { "local3",		LOG_LOCAL3 },
    { "local4",		LOG_LOCAL4 },
    { "local5",		LOG_LOCAL5 },
    { "local6",		LOG_LOCAL6 },
    { "local7",		LOG_LOCAL7 },
  };

  for (uns i=0; i < ARRAY_SIZE(facilities); i++)
    if (!strcmp(facilities[i].name, name))
      return facilities[i].id;
  return -1;
}

/* Convert severity level to syslog constants */
static int
syslog_level(int level)
{
  static const int levels[] = {
    [L_DEBUG] =		LOG_DEBUG,
    [L_INFO] =		LOG_INFO,
    [L_INFO_R] =	LOG_INFO,
    [L_WARN] =		LOG_WARNING,
    [L_WARN_R] =	LOG_WARNING,
    [L_ERROR] =		LOG_ERR,
    [L_ERROR_R] =	LOG_ERR,
    [L_FATAL] =		LOG_CRIT,
  };
  return ((level < (int)ARRAY_SIZE(levels)) ? levels[level] : LOG_NOTICE);
}

/* simple syslog write handler */
static int
syslog_handler(struct log_stream *ls, struct log_msg *m)
{
  struct syslog_stream *ss = (struct syslog_stream *) ls;
  int prio;
  ASSERT(ls);
  ASSERT(m);

  prio = syslog_level(LS_GET_LEVEL(m->flags)) | ss->facility;
  syslog(prio, "%s", m->m);
  return 0;
}

struct log_stream *
log_new_syslog(const char *facility, int options)
{
  int fac = syslog_facility(facility);
  if (fac < 0)
    die("No such syslog facility: %s", facility);

  struct log_stream *ls = log_new_stream(sizeof(struct syslog_stream));
  struct syslog_stream *ss = (struct syslog_stream *) ls;
  ls->name = "syslog";
  ls->msgfmt = 0;
  ls->handler = syslog_handler;
  ls->close = syslog_close;
  ss->facility = fac;

  if (!syslog_open_count++)
    openlog(log_title, options, LOG_INFO);
  return ls;
}

int
log_syslog_facility_exists(const char *facility)
{
  return (syslog_facility(facility) >= 0);
}
