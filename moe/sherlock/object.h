/*
 *	Sherlock Library -- Objects and operations on them
 *
 *	(c) 1997--2006 Martin Mares <mj@ucw.cz>
 *	(c) 2004--2005, Robert Spalek <robert@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

/*
 *  This is the main data structure used by Sherlock for many different
 *  purposes, most notably storage of documents in various stages of processing
 *
 *  Each object consists of a sequence of attributes whose names are single
 *  characters and values are either strings or subobjects. The order of attributes
 *  is not maintained (except for a couple of very special cases), while the order
 *  of multiple values of a single attribute is.
 *
 *  Objects exist either in the form of struct odes (an in-memory representation
 *  with very easy manipulation) or as a bucket (a linear stream of bytes in one of
 *  several possible formats, some of which are compressed, used for sending objects
 *  between processes and storing them in files [see sherlock/bucket.h for bucket files]).
 *
 *  See doc/objects for a more detailed description on how objects are used to
 *  represent documents.
 */

#ifndef _SHERLOCK_OBJECT_H
#define _SHERLOCK_OBJECT_H

struct fastbuf;
struct mempool;

/* object.c: In-memory representation of objects */

struct odes {				/* Object description */
  struct oattr *attrs;
  struct mempool *pool;
  struct oattr *cached_attr;
  struct odes *parent;
};

struct oattr {				/* Object attribute */
  struct oattr *next, *same;
  uns attr;				/* +OBJ_ATTR_SON if it's a sub-object */
  union {
    byte *val;
    struct odes *son;
  };
};

#define OBJ_ATTR_SON 256

void obj_dump(struct odes *);
void obj_dump_indented(struct odes *, uns);
struct odes *obj_new(struct mempool *);
struct oattr *obj_find_attr(struct odes *, uns);
struct oattr *obj_find_attr_last(struct odes *, uns);
uns obj_del_attr(struct odes *, struct oattr *);
byte *obj_find_aval(struct odes *, uns);
uns obj_find_anum(struct odes *, uns, uns);
u32 obj_find_x32(struct odes *, uns, u32);
u64 obj_find_x64(struct odes *, uns, u64);
struct oattr *obj_set_attr(struct odes *, uns, byte *);
struct oattr *obj_set_attr_num(struct odes *, uns, uns);
struct oattr *obj_add_attr(struct odes *, uns, byte *);
struct oattr *obj_add_attr_ref(struct odes *o, uns x, byte *v);	// no strdup()
struct oattr *obj_add_attr_num(struct odes *o, uns, uns);
struct oattr *obj_add_attr_son(struct odes *, uns, struct odes *);
struct oattr *obj_prepend_attr(struct odes *, uns, byte *);
struct oattr *obj_insert_attr(struct odes *o, struct oattr *first, struct oattr *after, byte *v);
void obj_move_attr_to_head(struct odes *o, uns);
void obj_move_attr_to_tail(struct odes *o, uns);
struct odes *obj_find_son(struct odes *, uns);
struct odes *obj_add_son(struct odes *, uns);
struct oattr *obj_add_son_ref(struct odes *o, uns x, struct odes *son);
void obj_add_attr_clone(struct odes *o, struct oattr *a);
struct odes *obj_clone(struct mempool *pool, struct odes *src);

/* Supported bucket formats */

enum bucket_type {
  BUCKET_TYPE_COMPAT = 0x7fffffff,	/* and less -- buckets created by older versions of Sherlock */
  BUCKET_TYPE_PLAIN = 0x80000000,	/* plain textual buckets */
  BUCKET_TYPE_V30 = 0x80000001,		/* v3.0 uncompressed buckets */
  BUCKET_TYPE_V33 = 0x80000002,		/* v3.3 uncompressed buckets */
  BUCKET_TYPE_V33_LIZARD = 0x80000003	/* v3.3 buckets compressed by lizard */
};

/* buck2obj.c: Reading of objects from buckets */

struct parsed_attr {
  int attr;
  byte *val;
  uns len;
};
struct buck2obj_buf;

/* note: get_attr routines are not thread-safe */
void get_attr_set_type(uns type);
int get_attr(byte **pos, byte *end, struct parsed_attr *attr);
int bget_attr(struct fastbuf *b, struct parsed_attr *attr);
void copy_parsed_attr(struct mempool *pool, struct parsed_attr *attr);

struct buck2obj_buf *buck2obj_alloc(void);
void buck2obj_free(struct buck2obj_buf *buf);

int buck2obj_parse(struct buck2obj_buf *buf, uns buck_type, uns buck_len, struct fastbuf *body,
		   struct odes *o_hdr, uns *body_start, struct odes *o_body,
		   uns allow_zero_copy);
struct odes *obj_read_bucket(struct buck2obj_buf *buf, struct mempool *pool, uns buck_type, uns buck_len, struct fastbuf *body,
			     uns *body_start, uns allow_zero_copy);
  /* If body_start != NULL, then only the header is parsed and *body_start is
   * set to the position of the body. This function does a plenty of optimizations
   * and if the body fastbuf is overwritable (body->can_overwrite_buffer), it can keep the
   * attribute values stored on their original locations in the fastbuf's buffer.
   * However, no such things are performed when reading the header only.
   */

int obj_read(struct fastbuf *, struct odes *);

/* obj2buck.c: Generating buckets from objects */

void put_attr_set_type(uns type);

uns size_attr(uns len);
uns size_object(struct odes *d);

byte *put_attr(byte *ptr, uns type, byte *val, uns len);
byte *put_attr_str(byte *ptr, uns type, byte *val);
byte *put_attr_vformat(byte *ptr, uns type, byte *mask, va_list va);
byte *put_attr_format(byte *ptr, uns type, char *mask, ...) __attribute__((format(printf,3,4)));
byte *put_attr_num(byte *ptr, uns type, uns val);
byte *put_attr_separator(byte *ptr);
byte *put_attr_push(byte *ptr, uns type);
byte *put_attr_pop(byte *ptr);
byte *put_object(byte *t, struct odes *d);

void bput_attr(struct fastbuf *b, uns type, byte *val, uns len);
void bput_attr_large(struct fastbuf *b, uns type, byte *val, uns len);
void bput_attr_str(struct fastbuf *b, uns type, byte *val);
void bput_attr_vformat(struct fastbuf *b, uns type, byte *mask, va_list va);
void bput_attr_format(struct fastbuf *b, uns type, char *mask, ...) __attribute__((format(printf,3,4)));
void bput_attr_num(struct fastbuf *b, uns type, uns val);
void bput_attr_separator(struct fastbuf *b);
void bput_attr_push(struct fastbuf *b, uns type);
void bput_attr_pop(struct fastbuf *b);
void bput_oattr(struct fastbuf *f, struct oattr *a);
void bput_oattr_nocheck(struct fastbuf *f, struct oattr *a);
void bput_object(struct fastbuf *b, struct odes *o);
void bput_object_nocheck(struct fastbuf *b, struct odes *o);

void obj_write(struct fastbuf *b, struct odes *o, uns bucket_type);
void obj_write_nocheck(struct fastbuf *b, struct odes *o, uns bucket_type);

/* obj-linear.c: Linear representation of objects by in-memory buckets */

byte *obj_linearize(struct odes *d, uns min_compress, uns *plen);
struct odes *obj_delinearize(struct buck2obj_buf *bbuf, struct mempool *mp, byte *buf, uns len, uns destructive);

/* obj-format.c: Adding of formatted values */

struct oattr *obj_add_attr_vformat(struct odes *o, uns x, char *fmt, va_list args);
struct oattr *obj_add_attr_format(struct odes *o, uns x, char *fmt, ...) FORMAT_CHECK(printf,3,4);
struct oattr *obj_set_attr_vformat(struct odes *o, uns x, char *fmt, va_list args);
struct oattr *obj_set_attr_format(struct odes *o, uns x, char *fmt, ...) FORMAT_CHECK(printf,3,4);

#endif
