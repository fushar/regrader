/*
 *	UCW Library -- Configuration files: parsing input streams
 *
 *	(c) 2001--2006 Robert Spalek <robert@ucw.cz>
 *	(c) 2003--2009 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/conf.h"
#include "ucw/getopt.h"
#include "ucw/conf-internal.h"
#include "ucw/clists.h"
#include "ucw/mempool.h"
#include "ucw/fastbuf.h"
#include "ucw/chartype.h"
#include "ucw/string.h"
#include "ucw/stkstring.h"

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

/* Text file parser */

static const char *name_parse_fb;
static struct fastbuf *parse_fb;
static uns line_num;

#define MAX_LINE	4096
static char line_buf[MAX_LINE];
static char *line = line_buf;

#include "ucw/bbuf.h"
static bb_t copy_buf;
static uns copied;

#define GBUF_TYPE	uns
#define GBUF_PREFIX(x)	split_##x
#include "ucw/gbuf.h"
static split_t word_buf;
static uns words;
static uns ends_by_brace;		// the line is ended by "{"

static int
get_line(char **msg)
{
  int err = bgets_nodie(parse_fb, line_buf, MAX_LINE);
  line_num++;
  if (err <= 0) {
    *msg = err < 0 ? "Line too long" : NULL;
    return 0;
  }
  line = line_buf;
  while (Cblank(*line))
    line++;
  return 1;
}

static void
append(char *start, char *end)
{
  uns len = end - start;
  bb_grow(&copy_buf, copied + len + 1);
  memcpy(copy_buf.ptr + copied, start, len);
  copied += len + 1;
  copy_buf.ptr[copied-1] = 0;
}

static char *
get_word(uns is_command_name)
{
  char *msg;
  if (*line == '\'') {
    line++;
    while (1) {
      char *start = line;
      while (*line && *line != '\'')
	line++;
      append(start, line);
      if (*line)
	break;
      copy_buf.ptr[copied-1] = '\n';
      if (!get_line(&msg))
	return msg ? : "Unterminated apostrophe word at the end";
    }
    line++;

  } else if (*line == '"') {
    line++;
    uns start_copy = copied;
    while (1) {
      char *start = line;
      uns escape = 0;
      while (*line) {
	if (*line == '"' && !escape)
	  break;
	else if (*line == '\\')
	  escape ^= 1;
	else
	  escape = 0;
	line++;
      }
      append(start, line);
      if (*line)
	break;
      if (!escape)
	copy_buf.ptr[copied-1] = '\n';
      else // merge two lines
	copied -= 2;
      if (!get_line(&msg))
	return msg ? : "Unterminated quoted word at the end";
    }
    line++;

    char *tmp = stk_str_unesc(copy_buf.ptr + start_copy);
    uns l = strlen(tmp);
    bb_grow(&copy_buf, start_copy + l + 1);
    strcpy(copy_buf.ptr + start_copy, tmp);
    copied = start_copy + l + 1;

  } else {
    // promised that *line is non-null and non-blank
    char *start = line;
    while (*line && !Cblank(*line)
	&& *line != '{' && *line != '}' && *line != ';'
	&& (*line != '=' || !is_command_name))
      line++;
    if (*line == '=') {				// nice for setting from a command-line
      if (line == start)
	return "Assignment without a variable";
      *line = ' ';
    }
    if (line == start)				// already the first char is control
      line++;
    append(start, line);
  }
  while (Cblank(*line))
    line++;
  return NULL;
}

static char *
get_token(uns is_command_name, char **err)
{
  *err = NULL;
  while (1) {
    if (!*line || *line == '#') {
      if (!is_command_name || !get_line(err))
	return NULL;
    } else if (*line == ';') {
      *err = get_word(0);
      if (!is_command_name || *err)
	return NULL;
    } else if (*line == '\\' && !line[1]) {
      if (!get_line(err)) {
	if (!*err)
	  *err = "Last line ends by a backslash";
	return NULL;
      }
      if (!*line || *line == '#')
	msg(L_WARN, "The line %s:%d following a backslash is empty", name_parse_fb ? : "", line_num);
    } else {
      split_grow(&word_buf, words+1);
      uns start = copied;
      word_buf.ptr[words++] = copied;
      *err = get_word(is_command_name);
      return *err ? NULL : copy_buf.ptr + start;
    }
  }
}

static char *
split_command(void)
{
  words = copied = ends_by_brace = 0;
  char *msg, *start_word;
  if (!(start_word = get_token(1, &msg)))
    return msg;
  if (*start_word == '{')			// only one opening brace
    return "Unexpected opening brace";
  while (*line != '}')				// stays for the next time
  {
    if (!(start_word = get_token(0, &msg)))
      return msg;
    if (*start_word == '{') {
      words--;					// discard the brace
      ends_by_brace = 1;
      break;
    }
  }
  return NULL;
}

/* Parsing multiple files */

static char *
parse_fastbuf(const char *name_fb, struct fastbuf *fb, uns depth)
{
  char *err;
  name_parse_fb = name_fb;
  parse_fb = fb;
  line_num = 0;
  line = line_buf;
  *line = 0;
  while (1)
  {
    err = split_command();
    if (err)
      goto error;
    if (!words)
      return NULL;
    char *name = copy_buf.ptr + word_buf.ptr[0];
    char *pars[words-1];
    for (uns i=1; i<words; i++)
      pars[i-1] = copy_buf.ptr + word_buf.ptr[i];
    if (!strcasecmp(name, "include"))
    {
      if (words != 2)
	err = "Expecting one filename";
      else if (depth > 8)
	err = "Too many nested files";
      else if (*line && *line != '#')		// because the contents of line_buf is not re-entrant and will be cleared
	err = "The include command must be the last one on a line";
      if (err)
	goto error;
      struct fastbuf *new_fb = bopen_try(pars[0], O_RDONLY, 1<<14);
      if (!new_fb) {
	err = cf_printf("Cannot open file %s: %m", pars[0]);
	goto error;
      }
      uns ll = line_num;
      err = parse_fastbuf(stk_strdup(pars[0]), new_fb, depth+1);
      line_num = ll;
      bclose(new_fb);
      if (err)
	goto error;
      parse_fb = fb;
      continue;
    }
    enum cf_operation op;
    char *c = strchr(name, ':');
    if (!c)
      op = strcmp(name, "}") ? OP_SET : OP_CLOSE;
    else {
      *c++ = 0;
      switch (Clocase(*c)) {
	case 's': op = OP_SET; break;
	case 'c': op = Clocase(c[1]) == 'l' ? OP_CLEAR: OP_COPY; break;
	case 'a': switch (Clocase(c[1])) {
		    case 'p': op = OP_APPEND; break;
		    case 'f': op = OP_AFTER; break;
		    default: op = OP_ALL;
		  }; break;
	case 'p': op = OP_PREPEND; break;
	case 'r': op = (c[1] && Clocase(c[2]) == 'm') ? OP_REMOVE : OP_RESET; break;
	case 'e': op = OP_EDIT; break;
	case 'b': op = OP_BEFORE; break;
	default: op = OP_SET; break;
      }
      if (strcasecmp(c, cf_op_names[op])) {
	err = cf_printf("Unknown operation %s", c);
	goto error;
      }
    }
    if (ends_by_brace)
      op |= OP_OPEN;
    err = cf_interpret_line(name, op, words-1, pars);
    if (err)
      goto error;
  }
error:
  if (name_fb)
    msg(L_ERROR, "File %s, line %d: %s", name_fb, line_num, err);
  else if (line_num == 1)
    msg(L_ERROR, "Manual setting of configuration: %s", err);
  else
    msg(L_ERROR, "Manual setting of configuration, line %d: %s", line_num, err);
  return "included from here";
}

#ifndef DEFAULT_CONFIG
#define DEFAULT_CONFIG NULL
#endif
char *cf_def_file = DEFAULT_CONFIG;
static int cf_def_loaded;

#ifndef ENV_VAR_CONFIG
#define ENV_VAR_CONFIG NULL
#endif
char *cf_env_file = ENV_VAR_CONFIG;

static uns postpone_commit;			// only for cf_getopt()
static uns everything_committed;		// after the 1st load, this flag is set on

static int
done_stack(void)
{
  if (cf_check_stack())
    return 1;
  if (cf_commit_all(postpone_commit ? CF_NO_COMMIT : everything_committed ? CF_COMMIT : CF_COMMIT_ALL))
    return 1;
  if (!postpone_commit)
    everything_committed = 1;
  return 0;
}

static int
load_file(const char *file)
{
  cf_init_stack();
  struct fastbuf *fb = bopen_try(file, O_RDONLY, 1<<14);
  if (!fb) {
    msg(L_ERROR, "Cannot open %s: %m", file);
    return 1;
  }
  char *err_msg = parse_fastbuf(file, fb, 0);
  bclose(fb);
  return !!err_msg || done_stack();
}

static int
load_string(const char *string)
{
  cf_init_stack();
  struct fastbuf fb;
  fbbuf_init_read(&fb, (byte *)string, strlen(string), 0);
  char *msg = parse_fastbuf(NULL, &fb, 0);
  return !!msg || done_stack();
}

/* Safe loading and reloading */

struct conf_entry {	/* We remember a list of actions to apply upon reload */
  cnode n;
  enum {
    CE_FILE = 1,
    CE_STRING = 2,
  } type;
  char *arg;
};

static clist conf_entries;

static void
cf_remember_entry(uns type, const char *arg)
{
  if (!cf_need_journal)
    return;
  if (!postpone_commit)
    return;
  struct conf_entry *ce = cf_malloc(sizeof(*ce));
  ce->type = type;
  ce->arg = cf_strdup(arg);
  clist_add_tail(&conf_entries, &ce->n);
}

int
cf_reload(const char *file)
{
  cf_journal_swap();
  struct cf_journal_item *oldj = cf_journal_new_transaction(1);
  uns ec = everything_committed;
  everything_committed = 0;

  if (!conf_entries.head.next)
    clist_init(&conf_entries);
  clist old_entries;
  clist_move(&old_entries, &conf_entries);
  postpone_commit = 1;

  int err = 0;
  if (file)
    err = load_file(file);
  else
    CLIST_FOR_EACH(struct conf_entry *, ce, old_entries) {
      if (ce->type == CE_FILE)
	err |= load_file(ce->arg);
      else
	err |= load_string(ce->arg);
      if (err)
	break;
      cf_remember_entry(ce->type, ce->arg);
    }

  postpone_commit = 0;
  if (!err)
    err |= done_stack();

  if (!err) {
    cf_journal_delete();
    cf_journal_commit_transaction(1, NULL);
  } else {
    everything_committed = ec;
    cf_journal_rollback_transaction(1, oldj);
    cf_journal_swap();
    clist_move(&conf_entries, &old_entries);
  }
  return err;
}

int
cf_load(const char *file)
{
  struct cf_journal_item *oldj = cf_journal_new_transaction(1);
  int err = load_file(file);
  if (!err) {
    cf_journal_commit_transaction(1, oldj);
    cf_remember_entry(CE_FILE, file);
    cf_def_loaded = 1;
  } else
    cf_journal_rollback_transaction(1, oldj);
  return err;
}

int
cf_set(const char *string)
{
  struct cf_journal_item *oldj = cf_journal_new_transaction(0);
  int err = load_string(string);
  if (!err) {
    cf_journal_commit_transaction(0, oldj);
    cf_remember_entry(CE_STRING, string);
  } else
    cf_journal_rollback_transaction(0, oldj);
  return err;
}

/* Command-line parser */

static void
load_default(void)
{
  if (cf_def_loaded++)
    return;
  if (cf_def_file)
    {
      char *env;
      if (cf_env_file && (env = getenv(cf_env_file)))
        {
	  if (cf_load(env))
	    die("Cannot load config file %s", env);
	}
      else if (cf_load(cf_def_file))
        die("Cannot load default config %s", cf_def_file);
    }
  else
    {
      // We need to create an empty pool and initialize all configuration items
      struct cf_journal_item *oldj = cf_journal_new_transaction(1);
      cf_init_stack();
      done_stack();
      cf_journal_commit_transaction(1, oldj);
    }
}

static void
final_commit(void)
{
  if (postpone_commit) {
    postpone_commit = 0;
    if (done_stack())
      die("Cannot commit after the initialization");
  }
}

int
cf_getopt(int argc, char * const argv[], const char *short_opts, const struct option *long_opts, int *long_index)
{
  clist_init(&conf_entries);
  postpone_commit = 1;

  static int other_options = 0;
  while (1) {
    int res = getopt_long (argc, argv, short_opts, long_opts, long_index);
    if (res == 'S' || res == 'C' || res == 0x64436667)
    {
      if (other_options)
	die("The -S and -C options must precede all other arguments");
      if (res == 'S') {
	load_default();
	if (cf_set(optarg))
	  die("Cannot set %s", optarg);
      } else if (res == 'C') {
	if (cf_load(optarg))
	  die("Cannot load config file %s", optarg);
      }
#ifdef CONFIG_DEBUG
      else {   /* --dumpconfig */
	load_default();
	final_commit();
	struct fastbuf *b = bfdopen(1, 4096);
	cf_dump_sections(b);
	bclose(b);
	exit(0);
      }
#endif
    } else {
      /* unhandled option or end of options */
      if (res != ':' && res != '?') {
	load_default();
	final_commit();
      }
      other_options++;
      return res;
    }
  }
}
