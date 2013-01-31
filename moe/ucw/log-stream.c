/*
 *	UCW Library -- Logging: Management of Log Streams
 *
 *	(c) 2008 Tomas Gavenciak <gavento@ucw.cz>
 *	(c) 2009 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/log.h"
#include "ucw/log-internal.h"
#include "ucw/simple-lists.h"

#include <string.h>

/* Initial number of streams to allocate (must be >=2) */
#define LS_INIT_STREAMS 8

/* Flag indicating initialization of the module */
static int log_initialized = 0;

/* The head of the list of freed log_streams indexes in log_streams.ptr (~0U if none free).
 * Freed positions in log_streams.ptr are connected into a linked list in the following way:
 * log_streams.ptr[log_streams_free].levels is the index of next freed position (or ~0U) */
static uns log_streams_free = ~0U;

/* Initialize the logstream module.
 * It is not neccessary to call this explicitely as it is called by
 * the first log_new_stream()  (for backward compatibility and ease of use). */
static void
log_init_module(void)
{
  if (log_initialized)
    return;

  /* Create the growing array */
  lsbuf_init(&log_streams);
  lsbuf_set_size(&log_streams, LS_INIT_STREAMS);

  bzero(log_streams.ptr, sizeof(struct log_stream*) * (log_streams.len));
  log_streams_free = ~0U;

  log_initialized = 1;

  /* init the default stream (0) as forwarder to fd2 */
  struct log_stream *ls = log_new_stream(sizeof(*ls));
  ASSERT(ls == log_streams.ptr[0]);
  ASSERT(ls->regnum == 0);
  ls->name = "default";
  log_add_substream(ls, &log_stream_default);
}

void
log_close_all(void)
{
  if (!log_initialized)
    return;

  // Remove substreams of all streams
  for (int i=0; i < log_streams_after; i++)
    if (log_streams.ptr[i]->regnum >= 0)
      log_rm_substream(log_streams.ptr[i], NULL);

  // Close all streams that remain and free all cached structures
  for (int i=0; i < log_streams_after; i++)
    {
      struct log_stream *ls = log_streams.ptr[i];
      if (ls->regnum >= 0)
	log_close_stream(ls);
      ASSERT(ls->regnum < 0 || !ls->use_count);
      xfree(ls);
    }

  /* Back to the default state */
  lsbuf_done(&log_streams);
  log_streams_after = 0;
  log_streams_free = ~0U;
  log_initialized = 0;
}

void
log_add_substream(struct log_stream *where, struct log_stream *what)
{
  ASSERT(where);
  ASSERT(what);

  simp_node *n = xmalloc(sizeof(simp_node));
  n->p = log_ref_stream(what);
  clist_add_tail(&where->substreams, &n->n);
}

int
log_rm_substream(struct log_stream *where, struct log_stream *what)
{
  void *tmp;
  int cnt = 0;
  ASSERT(where);

  CLIST_FOR_EACH_DELSAFE(simp_node *, i, where->substreams, tmp)
    if (i->p == what || !what)
      {
	clist_remove(&i->n);
	log_close_stream(i->p);
	xfree(i);
	cnt++;
      }
  return cnt;
}

struct log_stream *
log_new_stream(size_t size)
{
  struct log_stream *l;
  int index;

  /* Initialize the data structures if needed */
  log_init_module();

  /* Get a free stream, possibly recycling a closed one */
  if (log_streams_free == ~0U)
    {
      lsbuf_grow(&log_streams, log_streams_after+1);
      index = log_streams_after++;
      l = log_streams.ptr[index] = xmalloc(size);
    }
  else
    {
      index = log_streams_free;
      l = xrealloc(log_streams.ptr[index], size);
      log_streams.ptr[index] = l;
      log_streams_free = l->levels;
    }

  /* Initialize the stream */
  bzero(l, sizeof(*l));
  l->levels = ~0U;
  l->types = ~0U;
  l->regnum = LS_SET_STRNUM(index);
  clist_init(&l->substreams);
  return log_ref_stream(l);
}

int
log_close_stream(struct log_stream *ls)
{
  ASSERT(ls);
  ASSERT(ls->use_count);
  if (--ls->use_count)
    return 0;

  /* Unlink all subtreams */
  log_rm_substream(ls, NULL);

  /* Close the stream and add it to the free-list */
  if (ls->close)
    ls->close(ls);
  ls->levels = log_streams_free;
  log_streams_free = LS_GET_STRNUM(ls->regnum);
  ls->regnum = -1;
  return 1;
}

void
log_set_format(struct log_stream *ls, uns mask, uns data)
{
  ls->msgfmt = (ls->msgfmt & mask) | data;
  CLIST_FOR_EACH(simp_node *, i, ls->substreams)
    log_set_format(i->p, mask, data);
}

/*** Registry of type names ***/

int log_register_type(const char *name)
{
  if (!log_type_names)
    {
      log_type_names = xmalloc_zero(LS_NUM_TYPES * sizeof(char *));
      log_type_names[0] = "default";
    }
  uns id;
  for (id=0; id < LS_NUM_TYPES && log_type_names[id]; id++)
    if (!strcmp(log_type_names[id], name))
      return LS_SET_TYPE(id);
  ASSERT(id < LS_NUM_TYPES);
  log_type_names[id] = xstrdup(name);
  return LS_SET_TYPE(id);
}

/** Find a message type by name and return its ID encoded by `LS_SET_TYPE`. Returns -1 if no such type found. **/
int log_find_type(const char *name)
{
  if (!strcmp(name, "default"))
    return 0;
  if (!log_type_names)
    return -1;

  for (uns id=0; id < LS_NUM_TYPES && log_type_names[id]; id++)
    if (!strcmp(log_type_names[id], name))
      return LS_SET_TYPE(id);
  return -1;
}
