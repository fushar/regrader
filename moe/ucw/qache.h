/*
 *      Simple and Quick Shared Memory Cache
 *
 *	(c) 2005 Martin Mares <mj@ucw.cz>
 */

#ifndef _UCW_QACHE_H
#define _UCW_QACHE_H

struct qache_params {
  char *file_name;
  uns block_size;			/* Cache block size (a power of two) */
  uns cache_size;			/* Size of the whole cache */
  uns max_entries;			/* Maximum number of cached entries */
  int force_reset;			/* Force creation of a new cache even if the old one seems usable, -1 if reset should never be done */
  uns format_id;			/* Data format ID (old cache not used if formats differ) */
};

typedef byte qache_key_t[16];

struct qache;

/* Create and destroy a cache */
struct qache *qache_open(struct qache_params *p);
void qache_close(struct qache *q, uns retain_data);

/* Insert new item to the cache with a given key and data. If pos_hint is non-zero, it serves
 * as a hint about the position of the entry (if it's known that an entry with the particular key
 * was located there a moment ago). Returns position of the new entry.
 */
uns qache_insert(struct qache *q, qache_key_t *key, uns pos_hint, void *data, uns size);

/* Look up data in the cache, given a key and a position hint (as above). If datap is non-NULL, data
 * from the cache entry are copied either to *datap (if *datap is NULL, new memory is allocated by
 * calling xmalloc and *datap is set to point to that memory). The *sizep contains the maximum number
 * of bytes to be copied (~0U if unlimited) and it is replaced by the number of bytes available (so it
 * can be greater than the original value requested). The start indicates starting offset inside the
 * entry's data.
 */
uns qache_lookup(struct qache *q, qache_key_t *key, uns pos_hint, byte **datap, uns *sizep, uns start);

/* Inspect data in the cache (but don't modify LRU nor anything else), given a position.
 * If key is non-NULL, it's filled with the cache key. The rest works as in qache_lookup.
 * Returns 0 if the entry is empty, ~0 for position out of range, entry number otherwise.
 */
uns qache_probe(struct qache *q, qache_key_t *key, uns pos, byte **datap, uns *sizep, uns start);

/* Delete data from the cache, given a key and a position hint. */
uns qache_delete(struct qache *q, qache_key_t *key, uns pos_hint);

/* Debugging dump (beware, doesn't lock the cache!) */
void qache_debug(struct qache *q);

/* Check consistency of the cache structure */
void qache_audit(struct qache *q);

#endif
