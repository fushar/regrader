/*
 *      Simple and Quick Shared Memory Cache
 *
 *	(c) 2005 Martin Mares <mj@ucw.cz>
 */

#undef LOCAL_DEBUG

#include "ucw/lib.h"
#include "ucw/bitops.h"
#include "ucw/fastbuf.h"
#include "ucw/ff-binary.h"
#include "ucw/qache.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

/*
 *  The cache lives in a mmapped file of the following format:
 *	qache_header
 *	qache_entry[max_entries]	table of entries and their keys
 *	u32 qache_hash[hash_size]	hash table pointing to keys
 *	u32 block_next[num_blocks]	next block pointers
 *	padding				to a multiple of block size
 *	blocks[]			data blocks
 */

struct qache_header {
  u32 magic;				/* QCACHE_MAGIC */
  u32 block_size;			/* Parameters as in qache_params */
  u32 block_shift;			/* block_size = 1 << block_shift */
  u32 num_blocks;
  u32 format_id;
  u32 entry_table_start;		/* Array of qache_entry's */
  u32 max_entries;
  u32 hash_table_start;			/* Hash table containing all keys */
  u32 hash_size;
  u32 next_table_start;			/* Array of next pointers */
  u32 first_data_block;
};

#define QACHE_MAGIC 0xb79f6d12

struct qache_entry {
  u32 lru_prev, lru_next;		/* Entry #0: head of the cyclic LRU list */
  u32 data_len;				/* Entry #0: number of free blocks, Free entries: ~0U */
  u32 first_data_block;			/* Entry #0: first free block */
  qache_key_t key;
  u32 hash_next;			/* Entry #0: first free entry, Free entries: next free */
};

struct qache {
  struct qache_header *hdr;
  struct qache_entry *entry_table;
  u32 *hash_table;
  u32 *next_table;
  int fd;
  byte *mmap_data;
  uns file_size;
  char *file_name;
  uns locked;
};

#define first_free_entry entry_table[0].hash_next
#define first_free_block entry_table[0].first_data_block
#define num_free_blocks entry_table[0].data_len

static inline char *
format_key(qache_key_t *key)
{
  static char keybuf[2*sizeof(qache_key_t)+1];
  for (uns i=0; i<sizeof(qache_key_t); i++)
    sprintf(keybuf+2*i, "%02x", (*key)[i]);
  return keybuf;
}

static void
qache_msync(struct qache *q UNUSED, uns start UNUSED, uns len UNUSED)
{
#ifndef CONFIG_LINUX
  /* We don't need msyncing on Linux, since the mappings are guaranteed to be coherent */
  len += (start % CPU_PAGE_SIZE);
  start -= start % CPU_PAGE_SIZE;
  len = ALIGN_TO(len, CPU_PAGE_SIZE);
  if (msync(q->mmap_data + start, len, MS_ASYNC | MS_INVALIDATE) < 0)
    msg(L_ERROR, "Cache %s: msync failed: %m", q->file_name);
#endif
}

static void
qache_msync_block(struct qache *q, uns blk)
{
  DBG("\tSyncing block %d", blk);
  qache_msync(q, blk << q->hdr->block_shift, q->hdr->block_size);
}

static void
qache_lock(struct qache *q)
{
  /* We cannot use flock() since it happily permits locking a shared fd (e.g., after fork()) multiple times */
  ASSERT(!q->locked);
  struct flock fl = { .l_type = F_WRLCK, .l_whence = SEEK_SET, .l_start = 0, .l_len = sizeof(struct qache_header) };
  if (fcntl(q->fd, F_SETLKW, &fl) < 0)
    die("fcntl lock on %s: %m", q->file_name);
  q->locked = 1;
  DBG("Locked cache %s", q->file_name);
}

static void
qache_unlock(struct qache *q, uns dirty)
{
  ASSERT(q->locked);
  if (dirty)				/* Sync header, entry table and hash table */
    qache_msync(q, 0, q->hdr->first_data_block << q->hdr->block_shift);
  struct flock fl = { .l_type = F_UNLCK, .l_whence = SEEK_SET, .l_start = 0, .l_len = sizeof(struct qache_header) };
  if (fcntl(q->fd, F_SETLKW, &fl) < 0)
    die("fcntl unlock on %s: %m", q->file_name);
  q->locked = 0;
  DBG("Unlocked cache %s (dirty=%d)", q->file_name, dirty);
}

enum entry_audit_flags {
  ET_FREE_LIST = 1,
  ET_LRU = 2,
  ET_HASH = 4
};

static char *
audit_entries(struct qache *q, byte *entrymap)
{
  uns i, j;

  DBG("Auditing entries");

  /* Check the free list */
  i = q->first_free_entry;
  while (i)
    {
      if (i >= q->hdr->max_entries || (entrymap[i] & ET_FREE_LIST) || q->entry_table[i].data_len != ~0U)
	return "inconsistent free entry list";
      entrymap[i] |= ET_FREE_LIST;
      i = q->entry_table[i].hash_next;
    }

  /* Check the hash table */
  for (i=0; i<q->hdr->hash_size; i++)
    {
      j = q->hash_table[i];
      while (j)
	{
	  if (j >= q->hdr->max_entries || (entrymap[j] & (ET_HASH | ET_FREE_LIST)))
	    return "inconsistent hash chains";
	  entrymap[j] |= ET_HASH;
	  j = q->entry_table[j].hash_next;
	}
    }

  /* Check the LRU */
  i = 0;
  do
    {
      j = q->entry_table[i].lru_next;
      if ((entrymap[i] & (ET_LRU | ET_FREE_LIST)) || j >= q->hdr->max_entries || q->entry_table[j].lru_prev != i)
	return "inconsistent LRU list";
      entrymap[i] |= ET_LRU;
      i = j;
    }
  while (i);

  /* Check if all non-free items are in all lists */
  for (i=1; i<q->hdr->max_entries; i++)
    {
      if (entrymap[i] != ((q->entry_table[i].data_len == ~0U) ? ET_FREE_LIST : (ET_LRU | ET_HASH)))
	return "inconsistent lists";
    }
  return NULL;
}

enum block_audit_flags {
  BT_FREE_LIST = 1,
  BT_ALLOC = 2
};

static char *
audit_blocks(struct qache *q, byte *entrymap, byte *blockmap)
{
  uns i, j;

  DBG("Auditing blocks");

  /* Check the free list */
  for (i=q->first_free_block; i; i=q->next_table[i])
    {
      if (i < q->hdr->first_data_block || i >= q->hdr->num_blocks || (blockmap[i] & BT_FREE_LIST))
	return "inconsistent free block list";
      blockmap[i] |= BT_FREE_LIST;
    }

  /* Check allocation lists of entries */
  for (i=1; i<q->hdr->max_entries; i++)
    if (!(entrymap[i] & ET_FREE_LIST))
      {
	uns blocks = 0;
	for (j=q->entry_table[i].first_data_block; j; j=q->next_table[j])
	  {
	    if (blockmap[j])
	      return "inconsistent entry block list";
	    blockmap[j] |= BT_ALLOC;
	    blocks++;
	  }
	if (((q->entry_table[i].data_len + q->hdr->block_size - 1) >> q->hdr->block_shift) != blocks)
	  return "inconsistent entry data length";
      }

  /* Check if all blocks belong somewhere */
  for (i=q->hdr->first_data_block; i < q->hdr->num_blocks; i++)
    if (!blockmap[i])
      {
	DBG("Block %d unreferenced", i);
	return "unreferenced blocks found";
      }

  return NULL;
}

static char *
do_audit(struct qache *q)
{
  byte *entry_map = xmalloc_zero(q->hdr->max_entries);
  byte *block_map = xmalloc_zero(q->hdr->num_blocks);
  byte *err = audit_entries(q, entry_map);
  if (!err)
    err = audit_blocks(q, entry_map, block_map);
  xfree(block_map);
  xfree(entry_map);
  return err;
}

static void
qache_setup_pointers(struct qache *q)
{
  q->hdr = (struct qache_header *) q->mmap_data;
  q->entry_table = (struct qache_entry *) (q->mmap_data + q->hdr->entry_table_start);
  q->hash_table = (u32 *) (q->mmap_data + q->hdr->hash_table_start);
  q->next_table = (u32 *) (q->mmap_data + q->hdr->next_table_start);
}

static int
qache_open_existing(struct qache *q, struct qache_params *par)
{
  if ((q->fd = open(q->file_name, O_RDWR, 0)) < 0)
    return 0;

  struct stat st;
  char *err = "stat failed";
  if (fstat(q->fd, &st) < 0)
    goto close_and_fail;

  err = "invalid file size";
  if (st.st_size < (int)sizeof(struct qache_header) || (st.st_size % par->block_size))
    goto close_and_fail;
  q->file_size = st.st_size;

  err = "requested size change";
  if (q->file_size != par->cache_size)
    goto close_and_fail;

  err = "cannot mmap";
  if ((q->mmap_data = mmap(NULL, q->file_size, PROT_READ | PROT_WRITE, MAP_SHARED, q->fd, 0)) == MAP_FAILED)
    goto close_and_fail;
  struct qache_header *h = (struct qache_header *) q->mmap_data;

  qache_setup_pointers(q);
  qache_lock(q);

  err = "incompatible format";
  if (h->magic != QACHE_MAGIC ||
      h->block_size != par->block_size ||
      h->max_entries != par->max_entries ||
      h->format_id != par->format_id)
    goto unlock_and_fail;

  err = "incomplete file";
  if (h->num_blocks*h->block_size != q->file_size)
    goto unlock_and_fail;

  if (err = do_audit(q))
    goto unlock_and_fail;

  qache_unlock(q, 0);
  msg(L_INFO, "Cache %s: using existing data", q->file_name);
  return 1;

 unlock_and_fail:
  qache_unlock(q, 0);
  munmap(q->mmap_data, q->file_size);
 close_and_fail:
  msg(L_INFO, "Cache %s: ignoring old contents (%s)", q->file_name, err);
  close(q->fd);
  return 0;
}

static void
qache_create(struct qache *q, struct qache_params *par)
{
  q->fd = open(q->file_name, O_RDWR | O_CREAT | O_TRUNC, 0666);
  if (q->fd < 0)
    die("Cache %s: unable to create (%m)", q->file_name);
  struct fastbuf *fb = bfdopen_shared(q->fd, 16384);

  struct qache_header h;
  bzero(&h, sizeof(h));
  h.magic = QACHE_MAGIC;
  h.block_size = par->block_size;
  h.block_shift = bit_fls(h.block_size);
  h.num_blocks = par->cache_size >> h.block_shift;
  h.format_id = par->format_id;
  h.entry_table_start = sizeof(h);
  h.max_entries = par->max_entries;
  h.hash_table_start = h.entry_table_start + h.max_entries * sizeof(struct qache_entry);
  h.hash_size = 1;
  while (h.hash_size < h.max_entries)
    h.hash_size *= 2;
  h.next_table_start = h.hash_table_start + h.hash_size * 4;
  h.first_data_block = (h.next_table_start + 4*h.num_blocks + h.block_size - 1) >> h.block_shift;
  if (h.first_data_block >= h.num_blocks)
    die("Cache %s: Requested size is too small even to hold the maintenance structures", q->file_name);
  bwrite(fb, &h, sizeof(h));

  /* Entry #0: heads of all lists */
  ASSERT(btell(fb) == (ucw_off_t)h.entry_table_start);
  struct qache_entry ent;
  bzero(&ent, sizeof(ent));
  ent.first_data_block = h.first_data_block;
  ent.data_len = h.num_blocks - h.first_data_block;
  ent.hash_next = 1;
  bwrite(fb, &ent, sizeof(ent));

  /* Other entries */
  bzero(&ent, sizeof(ent));
  ent.data_len = ~0U;
  for (uns i=1; i<h.max_entries; i++)
    {
      ent.hash_next = (i == h.max_entries-1 ? 0 : i+1);
      bwrite(fb, &ent, sizeof(ent));
    }

  /* The hash table */
  ASSERT(btell(fb) == (ucw_off_t)h.hash_table_start);
  for (uns i=0; i<h.hash_size; i++)
    bputl(fb, 0);

  /* The next pointers */
  ASSERT(btell(fb) == (ucw_off_t)h.next_table_start);
  for (uns i=0; i<h.num_blocks; i++)
    bputl(fb, (i < h.first_data_block || i == h.num_blocks-1) ? 0 : i+1);

  /* Padding */
  ASSERT(btell(fb) <= (ucw_off_t)(h.first_data_block << h.block_shift));
  while (btell(fb) < (ucw_off_t)(h.first_data_block << h.block_shift))
    bputc(fb, 0);

  /* Data blocks */
  for (uns i=h.first_data_block; i<h.num_blocks; i++)
    for (uns j=0; j<h.block_size; j+=4)
      bputl(fb, 0);

  ASSERT(btell(fb) == (ucw_off_t)par->cache_size);
  bclose(fb);
  msg(L_INFO, "Cache %s: created (%d bytes, %d slots, %d buckets)", q->file_name, par->cache_size, h.max_entries, h.hash_size);

  if ((q->mmap_data = mmap(NULL, par->cache_size, PROT_READ | PROT_WRITE, MAP_SHARED, q->fd, 0)) == MAP_FAILED)
    die("Cache %s: mmap failed (%m)", par->file_name);
  q->file_size = par->cache_size;
  qache_setup_pointers(q);
}

struct qache *
qache_open(struct qache_params *par)
{
  struct qache *q = xmalloc_zero(sizeof(*q));
  q->file_name = xstrdup(par->file_name);

  ASSERT(par->block_size >= 8 && !(par->block_size & (par->block_size-1)));
  par->cache_size = ALIGN_TO(par->cache_size, par->block_size);

  if (par->force_reset <= 0 && qache_open_existing(q, par))
    ;
  else if (par->force_reset < 0)
    die("Cache %s: read-only access requested, but no data available", q->file_name);
  else
    qache_create(q, par);
  return q;
}

void
qache_close(struct qache *q, uns retain_data)
{
  munmap(q->mmap_data, q->file_size);
  close(q->fd);
  if (!retain_data && unlink(q->file_name) < 0)
    msg(L_ERROR, "Cache %s: unlink failed (%m)", q->file_name);
  xfree(q->file_name);
  xfree(q);
}

static uns
qache_hash(struct qache *q, qache_key_t *key)
{
  uns h = ((*key)[0] << 24) | ((*key)[1] << 16) | ((*key)[2] << 8) | (*key)[3];
  return h % q->hdr->hash_size;
}

static uns
qache_hash_find(struct qache *q, qache_key_t *key, uns pos_hint)
{
  ASSERT(q->locked);

  if (pos_hint && pos_hint < q->hdr->max_entries && q->entry_table[pos_hint].data_len != ~0U && !memcmp(q->entry_table[pos_hint].key, key, sizeof(*key)))
    return pos_hint;

  uns h = qache_hash(q, key);
  for (uns e = q->hash_table[h]; e; e=q->entry_table[e].hash_next)
    if (!memcmp(q->entry_table[e].key, key, sizeof(*key)))
      return e;
  return 0;
}

static void
qache_hash_insert(struct qache *q, uns e)
{
  uns h = qache_hash(q, &q->entry_table[e].key);
  q->entry_table[e].hash_next = q->hash_table[h];
  q->hash_table[h] = e;
}

static void
qache_hash_remove(struct qache *q, uns e)
{
  struct qache_entry *entry = &q->entry_table[e];
  uns f, *hh;
  for (hh=&q->hash_table[qache_hash(q, &entry->key)]; f=*hh; hh=&(q->entry_table[f].hash_next))
    if (!memcmp(q->entry_table[f].key, entry->key, sizeof(qache_key_t)))
      {
	*hh = entry->hash_next;
	return;
      }
  ASSERT(0);
}

static uns
qache_alloc_entry(struct qache *q)
{
  uns e = q->first_free_entry;
  ASSERT(q->locked && e);
  struct qache_entry *entry = &q->entry_table[e];
  ASSERT(entry->data_len == ~0U);
  q->first_free_entry = entry->hash_next;
  entry->data_len = 0;
  return e;
}

static void
qache_free_entry(struct qache *q, uns e)
{
  struct qache_entry *entry = &q->entry_table[e];
  ASSERT(q->locked && entry->data_len != ~0U);
  entry->data_len = ~0U;
  entry->hash_next = q->first_free_entry;
  q->first_free_entry = e;
}

static inline void *
get_block_start(struct qache *q, uns block)
{
  ASSERT(block && block < q->hdr->num_blocks);
  return q->mmap_data + (block << q->hdr->block_shift);
}

static uns
qache_alloc_block(struct qache *q)
{
  ASSERT(q->locked && q->num_free_blocks);
  uns blk = q->first_free_block;
  q->first_free_block = q->next_table[blk];
  q->num_free_blocks--;
  DBG("\tAllocated block %d", blk);
  return blk;
}

static void
qache_free_block(struct qache *q, uns blk)
{
  ASSERT(q->locked);
  q->next_table[blk] = q->first_free_block;
  q->first_free_block = blk;
  q->num_free_blocks++;
  DBG("\tFreed block %d", blk);
}

static void
qache_lru_insert(struct qache *q, uns e)
{
  struct qache_entry *head = &q->entry_table[0];
  struct qache_entry *entry = &q->entry_table[e];
  ASSERT(q->locked && !entry->lru_prev && !entry->lru_next);
  uns succe = head->lru_next;
  struct qache_entry *succ = &q->entry_table[succe];
  head->lru_next = e;
  entry->lru_prev = 0;
  entry->lru_next = succe;
  succ->lru_prev = e;
}

static void
qache_lru_remove(struct qache *q, uns e)
{
  ASSERT(q->locked);
  struct qache_entry *entry = &q->entry_table[e];
  q->entry_table[entry->lru_prev].lru_next = entry->lru_next;
  q->entry_table[entry->lru_next].lru_prev = entry->lru_prev;
  entry->lru_prev = entry->lru_next = 0;
}

static uns
qache_lru_get(struct qache *q)
{
  return q->entry_table[0].lru_prev;
}

static void
qache_ll_delete(struct qache *q, uns e)
{
  struct qache_entry *entry = &q->entry_table[e];
  uns blk = entry->first_data_block;
  while (entry->data_len)
    {
      uns next = q->next_table[blk];
      qache_free_block(q, blk);
      blk = next;
      if (entry->data_len >= q->hdr->block_size)
	entry->data_len -= q->hdr->block_size;
      else
	entry->data_len = 0;
    }
  qache_lru_remove(q, e);
  qache_hash_remove(q, e);
  qache_free_entry(q, e);
}

uns
qache_insert(struct qache *q, qache_key_t *key, uns pos_hint, void *data, uns size)
{
  qache_lock(q);

  uns e = qache_hash_find(q, key, pos_hint);
  if (e)
    {
      qache_ll_delete(q ,e);
      DBG("Insert <%s>: deleting old entry %d", format_key(key), e);
    }

  uns blocks = (size + q->hdr->block_size - 1) >> q->hdr->block_shift;
  if (blocks > q->hdr->num_blocks - q->hdr->first_data_block)
    {
      qache_unlock(q, 0);
      return 0;
    }
  while (q->num_free_blocks < blocks || !q->first_free_entry)
    {
      e = qache_lru_get(q);
      DBG("Insert <%s>: evicting entry %d to make room for %d blocks", format_key(key), e, blocks);
      ASSERT(e);
      qache_ll_delete(q, e);
    }
  e = qache_alloc_entry(q);
  struct qache_entry *entry = &q->entry_table[e];
  entry->data_len = size;
  memcpy(entry->key, key, sizeof(*key));
  DBG("Insert <%s>: created entry %d with %d data blocks", format_key(key), e, blocks);

  entry->first_data_block = 0;
  while (size)
    {
      uns chunk = (size & (q->hdr->block_size-1)) ? : q->hdr->block_size;
      uns blk = qache_alloc_block(q);
      q->next_table[blk] = entry->first_data_block;
      memcpy(get_block_start(q, blk), data+size-chunk, chunk);
      qache_msync_block(q, blk);
      entry->first_data_block = blk;
      size -= chunk;
    }

  qache_lru_insert(q, e);
  qache_hash_insert(q, e);
  qache_unlock(q, 1);
  return e;
}

static void
copy_out(struct qache *q, struct qache_entry *entry, byte **datap, uns *sizep, uns start)
{
  if (sizep)
    {
      uns size = *sizep;
      uns avail = (start > entry->data_len) ? 0 : entry->data_len - start;
      uns xfer = MIN(size, avail);
      *sizep = avail;
      if (datap)
	{
	  if (!*datap)
	    *datap = xmalloc(xfer);
	  uns blk = entry->first_data_block;
	  while (start >= q->hdr->block_size)
	    {
	      blk = q->next_table[blk];
	      start -= q->hdr->block_size;
	    }
	  byte *data = *datap;
	  while (xfer)
	    {
	      uns len = MIN(xfer, q->hdr->block_size - start);
	      memcpy(data, get_block_start(q, blk), len);
	      blk = q->next_table[blk];
	      data += len;
	      xfer -= len;
	      start = 0;
	    }
	}
    }
  else
    ASSERT(!datap);
}

uns
qache_lookup(struct qache *q, qache_key_t *key, uns pos_hint, byte **datap, uns *sizep, uns start)
{
  qache_lock(q);
  uns e = qache_hash_find(q, key, pos_hint);
  if (e)
    {
      struct qache_entry *entry = &q->entry_table[e];
      DBG("Lookup <%s>: found entry %d", format_key(key), e);
      qache_lru_remove(q, e);
      qache_lru_insert(q, e);
      copy_out(q, entry, datap, sizep, start);
      qache_unlock(q, 1);     /* Yes, modified -- we update the LRU */
    }
  else
    {
      DBG("Lookup <%s>: not found", format_key(key));
      qache_unlock(q, 0);
    }
  return e;
}

uns
qache_probe(struct qache *q, qache_key_t *key, uns pos, byte **datap, uns *sizep, uns start)
{
  if (!pos || pos >= q->hdr->max_entries)
    {
      DBG("Probe %d: Out of range", pos);
      return ~0U;
    }

  qache_lock(q);
  uns ret = 0;
  struct qache_entry *entry = &q->entry_table[pos];
  if (entry->data_len != ~0U)
    {
      DBG("Probe %d: Found key <%s>", format_key(entry->key));
      if (key)
	memcpy(key, entry->key, sizeof(qache_key_t));
      copy_out(q, entry, datap, sizep, start);
      ret = pos;
    }
  else
    DBG("Probe %d: Empty", pos);
  qache_unlock(q, 0);
  return ret;
}

uns
qache_delete(struct qache *q, qache_key_t *key, uns pos_hint)
{
  qache_lock(q);
  uns e = qache_hash_find(q, key, pos_hint);
  if (e)
    {
      DBG("Delete <%s: deleting entry %d", format_key(key), e);
      qache_ll_delete(q, e);
    }
  else
    DBG("Delete <%s>: No match", format_key(key));
  qache_unlock(q, 1);
  return e;
}

void
qache_debug(struct qache *q)
{
  msg(L_DEBUG, "Cache %s: block_size=%d (%d data), num_blocks=%d (%d first data), %d slots, %d hash buckets",
      q->file_name, q->hdr->block_size, q->hdr->block_size, q->hdr->num_blocks, q->hdr->first_data_block,
      q->hdr->max_entries, q->hdr->hash_size);

  msg(L_DEBUG, "Table of cache entries:");
  msg(L_DEBUG, "\tEntry\tLruPrev\tLruNext\tDataLen\tDataBlk\tHashNxt\tKey");
  for (uns e=0; e<q->hdr->max_entries; e++)
    {
      struct qache_entry *ent = &q->entry_table[e];
      msg(L_DEBUG, "\t%d\t%d\t%d\t%d\t%d\t%d\t%s", e, ent->lru_prev, ent->lru_next, ent->data_len,
	  ent->first_data_block, ent->hash_next, format_key(&ent->key));
    }

  msg(L_DEBUG, "Hash table:");
  for (uns h=0; h<q->hdr->hash_size; h++)
    msg(L_DEBUG, "\t%04x\t%d", h, q->hash_table[h]);

  msg(L_DEBUG, "Next pointers:");
  for (uns blk=q->hdr->first_data_block; blk<q->hdr->num_blocks; blk++)
    msg(L_DEBUG, "\t%d\t%d", blk, q->next_table[blk]);
}

void
qache_audit(struct qache *q)
{
  char *err;
  qache_lock(q);
  if (err = do_audit(q))
    die("Cache %s: %s", q->file_name, err);
  qache_unlock(q, 0);
}

#ifdef TEST

int main(int argc UNUSED, char **argv UNUSED)
{
  struct qache_params par = {
    .file_name = "tmp/test",
    .block_size = 256,
    .cache_size = 65536,
    .max_entries = 123,
    .force_reset = 0,
    .format_id = 0xfeedcafe
  };
  struct qache *q = qache_open(&par);

  qache_key_t key = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef };
#define N 100
  uns i, j;
  byte data[11*N];
  for (i=0; i<N; i++)
    {
      key[3] = i / 16; key[15] = i % 16;
      for (j=0; j<11*i; j++)
	data[j] = 0x33 + i*j;
      qache_insert(q, &key, 0, data, 11*i);
    }
  qache_debug(q);
  qache_audit(q);

  uns found = 0;
  for (i=0; i<100; i++)
    {
      key[3] = i / 16; key[15] = i % 16;
      byte *dptr = data;
      uns sz = sizeof(data);
      uns e = qache_lookup(q, &key, 0, &dptr, &sz, 0);
      if (e)
	{
	  ASSERT(sz == 11*i);
	  for (j=0; j<sz; j++)
	    ASSERT(data[j] == (byte)(0x33 + i*j));
	  found++;
	}
    }
  msg(L_INFO, "Found %d of %d entries", found, N);

  qache_close(q, 1);
  return 0;
}

#endif
