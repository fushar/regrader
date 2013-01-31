/*
 *	UCW Library -- Logging
 *
 *	(c) 1997--2009 Martin Mares <mj@ucw.cz>
 *	(c) 2008 Tomas Gavenciak <gavento@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _UCW_LOG_H_
#define _UCW_LOG_H_

#include "ucw/clists.h"

/*** === Messages and streams ***/

/**
 * Inside the logging system, a log message is always represented by this structure.
 **/
struct log_msg {
  char *m;				// The formatted message itself, ending with \n\0
  int m_len;				// Length without the \0
  struct tm *tm;			// Current time
  struct timeval *tv;
  uns flags;				// Category and other flags as passed to msg()
  char *raw_msg;			// Unformatted parts
  char *stime;
  char *sutime;
};

/**
 * Each stream is represented by an instance of this structure.
 **/
struct log_stream {
  char *name;				// Optional name, allocated by the user (or constructor)
  int regnum;				// Stream number, already encoded by LS_SET_STRNUM(); -1 if closed
  uns levels;				// Bitmask of accepted severity levels (default: all)
  uns types;				// Bitmask of accepted message types (default: all)
  uns msgfmt;				// Formatting flags (LSFMT_xxx)
  uns use_count;			// Number of references to the stream
  uns stream_flags;			// Various other flags (LSFLAG_xxx)
  int (*filter)(struct log_stream* ls, struct log_msg *m);	// Filter function, return non-zero to discard the message
  clist substreams;			// Pass the message to these streams (simple_list of pointers)
  int (*handler)(struct log_stream *ls, struct log_msg *m);	// Called to commit the message, return 0 for success, errno on error
  void (*close)(struct log_stream* ls);	// Called upon log_close_stream()
  void *user_data;			// Not used by the logging system
  // Private data of the handler follow
};

/**
 * Formatting flags specifying the format of the message passed to the handler.
 **/
enum ls_fmt {
  LSFMT_LEVEL =		1,		// severity level (one letter) */
  LSFMT_TIME =		2,		// date and time (YYYY-mm-dd HH:MM:SS) */
  LSFMT_USEC = 		4,		// also micro-seconds */
  LSFMT_TITLE =		8,		// program title (log_title) */
  LSFMT_PID =		16,		// program PID (log_pid) */
  LSFMT_LOGNAME =	32,		// name of the log_stream */
  LSFMT_TYPE =		64,		// message type
};

#define LSFMT_DEFAULT (LSFMT_LEVEL | LSFMT_TIME | LSFMT_TITLE | LSFMT_PID)	/** Default format **/

/**
 * General stream flags.
 **/
enum ls_flag {
  LSFLAG_ERR_IS_FATAL =	1,		// When a logging error occurs, die() immediately
  LSFLAG_ERR_REPORTED =	2,		// A logging error has been already reported on this stream
};

/***
 * === Message flags
 *
 * The @flags parameter of msg() is divided to several groups of bits (from the LSB):
 * message severity level (`L_xxx`), destination stream, message type
 * and control bits (e.g., `L_SIGHANDLER`).
 ***/

enum ls_flagbits {			// Bit widths of groups
  LS_LEVEL_BITS =	8,
  LS_STRNUM_BITS =	16,
  LS_TYPE_BITS =	5,
  LS_CTRL_BITS =	3,
};

enum ls_flagpos {			// Bit positions of groups
  LS_LEVEL_POS =	0,
  LS_STRNUM_POS =	LS_LEVEL_POS + LS_LEVEL_BITS,
  LS_TYPE_POS =		LS_STRNUM_POS + LS_STRNUM_BITS,
  LS_CTRL_POS =		LS_TYPE_POS + LS_TYPE_BITS,
};

enum ls_flagmasks {			// Bit masks of groups
  LS_LEVEL_MASK =	((1 << LS_LEVEL_BITS) - 1) << LS_LEVEL_POS,
  LS_STRNUM_MASK =	((1 << LS_STRNUM_BITS) - 1) << LS_STRNUM_POS,
  LS_TYPE_MASK =	((1 << LS_TYPE_BITS) - 1) << LS_TYPE_POS,
  LS_CTRL_MASK =	((1 << LS_CTRL_BITS) - 1) << LS_CTRL_POS,
};

// "Get" macros (break flags to parts)
#define LS_GET_LEVEL(flags)	(((flags) & LS_LEVEL_MASK) >> LS_LEVEL_POS)	/** Extract severity level **/
#define LS_GET_STRNUM(flags)	(((flags) & LS_STRNUM_MASK) >> LS_STRNUM_POS)	/** Extract stream number **/
#define LS_GET_TYPE(flags)	(((flags) & LS_TYPE_MASK) >> LS_TYPE_POS)	/** Extract message type **/
#define LS_GET_CTRL(flags)	(((flags) & LS_CTRL_MASK) >> LS_CTRL_POS)	/** Extract control bits **/

// "Set" macros (parts to flags)
#define LS_SET_LEVEL(level)	((level) << LS_LEVEL_POS)			/** Convert severity level to flags **/
#define LS_SET_STRNUM(strnum)	((strnum) << LS_STRNUM_POS)			/** Convert stream number to flags **/
#define LS_SET_TYPE(type)	((type) << LS_TYPE_POS)				/** Convert message type to flags **/
#define LS_SET_CTRL(ctrl)	((ctrl) << LS_CTRL_POS)				/** Convert control bits to flags **/

#define LS_NUM_TYPES (1 << LS_TYPE_BITS)

/** Register a new message type and return the corresponding flag set (encoded by `LS_SET_TYPE`). **/
int log_register_type(const char *name);

/** Find a message type by name and return the corresponding flag set. Returns -1 if no such type found. **/
int log_find_type(const char *name);

/** Given a flag set, extract the message type ID and return its name. **/
char *log_type_name(uns flags);

/*** === Operations on streams ***/

/**
 * Allocate a new log stream with no handler and an empty substream list.
 * Since struct log_stream is followed by private data, @size bytes of memory are allocated
 * for the whole structure. See below for functions creating specific stream types.
 **/
struct log_stream *log_new_stream(size_t size);

/**
 * Decrement the use count of a stream. If it becomes zero, close the stream,
 * free its memory, and unlink all its substreams.
 **/
int log_close_stream(struct log_stream *ls);

/**
 * Get a new reference on an existing stream. For convenience, the return value is
 * equal to the argument @ls.
 **/
static inline struct log_stream *log_ref_stream(struct log_stream *ls)
{
  ls->use_count++;
  return ls;
}

/**
 * Link a substream to a stream. The substream gains a reference, preventing
 * it from being freed until it is unlinked.
 **/
void log_add_substream(struct log_stream *where, struct log_stream *what);

/**
 * Unlink all occurrences of a substream @what from stream @where. Each
 * occurrence loses a reference. If @what is NULL, all substreams are unlinked.
 * Returns the number of unlinked substreams.
 **/
int log_rm_substream(struct log_stream *where, struct log_stream *what);

/**
 * Set formatting flags of a given stream and all its substreams. The flags are
 * AND'ed with @mask and OR'ed with @data.
 **/
void log_set_format(struct log_stream *ls, uns mask, uns data);

/**
 * Find a stream by its registration number (in the format of logging flags).
 * Returns NULL if there is no such stream.
 **/
struct log_stream *log_stream_by_flags(uns flags);

/** Return a pointer to the default stream (stream #0). **/
static inline struct log_stream *log_default_stream(void)
{
  return log_stream_by_flags(0);
}

/**
 * Close all open streams, un-initialize the module, free all memory and
 * reset the logging mechanism to use stderr only.
 **/
void log_close_all(void);

/***
 * === Logging to files
 *
 * All log files are open in append mode, which guarantees atomicity of write()
 * even in multi-threaded programs.
 ***/

struct log_stream *log_new_file(const char *path, uns flags);	/** Create a stream bound to a log file. See `FF_xxx` for @flags. **/
struct log_stream *log_new_fd(int fd, uns flags);		/** Create a stream bound to a file descriptor. See `FF_xxx` for @flags. **/

enum log_file_flag {		/** Flags used for file-based logging **/
  FF_FORMAT_NAME = 1,		// Internal: Name contains strftime escapes
  FF_CLOSE_FD = 2,		// Close the fd with the stream (use with log_new_fd())
  FF_FD2_FOLLOWS = 4,		// Maintain stderr as a clone of this stream
};

/**
 * When a time-based name of the log file changes, the logger switches to a new
 * log file automatically. This can be sometimes inconvenient, so you can use
 * this function to disable the automatic switches. The calls to this function
 * can be nested.
 **/
void log_switch_disable(void);
void log_switch_enable(void);		/** Negate the effect of log_switch_disable(). **/
int log_switch(void);			/** Switch log files manually. **/

/***
 * === Logging to syslog
 *
 * This log stream uses the libc interface to the system logging daemon (`syslogd`).
 * This interface has several limitations:
 *
 *   * Syslog are poorer than our scheme, so they are translated with a slight
 *     loss of information (most importantly, the distinction between local and
 *     remote messages is lost). If you are interested in details, search the
 *     source for syslog_level().
 *   * Syslog options (especially logging of PID with each message) must be fixed
 *     during initialization of the logger
 *   * Syslog provides its own formatting, so we turn off all formatting flags
 *     of the LibUCW logger. You can override this manually by setting the @msgfmt
 *     field of the log stream, but the result won't be nice.
 *   * Syslog does not support timestamps with sub-second precision.
 ***/

/**
 * Create a log stream for logging to a selected syslog facility.
 * The @options are passed to openlog(). (Beware, due to limitations of the
 * syslog interface in libc, the @options are shared for all syslog streams
 * and they are applied when the first stream is created.)
 **/
struct log_stream *log_new_syslog(const char *facility, int options);

/**
 * Verify that a facility of the given name exists. Return 1 if it does, 0 otherwise.
 **/
int log_syslog_facility_exists(const char *facility);

/***
 * === Configuring log streams
 *
 * If you use the LibUCW mechanism for parsing config files, you can let your
 * user configure arbitrary log streams in the Logging section of the config file
 * (see examples in the default config file). LibUCW automatically verifies that
 * the configuration is consistent (this is performed in the commit hook of the
 * config section), but it opens the streams only upon request. The following
 * functions can be used to control that.
 ***/

/** Open a log stream configured under the specified name and increase its use count. **/
struct log_stream *log_new_configured(const char *name);

/** Open a log stream configured under the specified name and use it as the default destination. **/
void log_configured(const char *name);

/**
 * Verify that a stream called @name was configured. If it wasn't, return an error
 * message. This is intended to be used in configuration commit hooks.
 **/
char *log_check_configured(const char *name);

#endif
