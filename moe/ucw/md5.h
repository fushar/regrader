/*
 *	UCW Library -- MD5 Message Digest
 *
 *	This file is in public domain (see ucw/md5.c).
 */

#ifndef _UCW_MD5_H
#define _UCW_MD5_H

/**
 * Internal MD5 hash state.
 * You should use it just as an opaque handle only.
 */
typedef struct {
	u32 buf[4];
	u32 bits[2];
	byte in[64];
} md5_context;

void md5_init(md5_context *context); /** Initialize the MD5 hashing algorithm in @context. **/
/**
 * Push another @len bytes of data from @buf to the MD5 hash
 * represented by @context. You can call it multiple time on the same
 * @context without reinitializing it and the result will be the same
 * as if you concatenated all the data together and fed them here all at
 * once.
 */
void md5_update(md5_context *context, const byte *buf, uns len);
/**
 * Call this after the last @md5_update(). It will terminate the
 * algorithm and return a pointer to the result.
 *
 * Note that the data it points to are stored inside the @context, so
 * if you use it to compute another hash or it ceases to exist, the
 * pointer becomes invalid.
 *
 * To convert the hash to its usual hexadecimal representation, see
 * <<string:mem_to_hex()>>.
 */
byte *md5_final(md5_context *context);

/**
 * This is the core routine of the MD5 algorithm. It takes 16 longwords of
 * data in @in and transforms the hash in @buf according to them.
 *
 * You probably do not want to call this one directly.
 */
void md5_transform(u32 buf[4], const u32 in[16]);

/**
 * MD5 one-shot convenience method. It takes @length bytes from
 * @buffer, creates the hash from them and returns it in @output.
 *
 * It is equivalent to this code:
 *
 *  md5_context c;
 *  md5_init(&c);
 *  md5_update(&c, buffer, length);
 *  memcpy(outbuf, md5_final(&c), MD5_SIZE);
 */
void md5_hash_buffer(byte *outbuf, const byte *buffer, uns length);

#define MD5_HEX_SIZE 33 /** How many bytes a string buffer for MD5 in hexadecimal format should have. **/
#define MD5_SIZE 16 /** Number of bytes the MD5 hash takes in the binary form. **/

#endif /* !_UCW_MD5_H */
