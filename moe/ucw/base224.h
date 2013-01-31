/*
 *	UCW Library -- Base 224 Encoding & Decoding
 *
 *	(c) 2002 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

/**
 * Encodes @len bytes of data pointed to by @src by base224 encoding.
 * Stores them in @dest and returns the number of bytes the output
 * takes.
 */
uns base224_encode(byte *dest, const byte *src, uns len);
/**
 * Decodes @len bytes of data pointed to by @src from base224 encoding.
 * All invalid characters are ignored. The result is stored into @dest
 * and length of the result is returned.
 */
uns base224_decode(byte *dest, const byte *src, uns len);

/**
 * Use this macro to calculate @base224_encode() output buffer size.
 * It can happen 4 more bytes would be needed, this macro takes care
 * of that.
 */
#define BASE224_ENC_LENGTH(x) (((x)*8+38)/39*5)

/*
 * When called for BASE224_IN_CHUNK-byte chunks, the result will be
 * always BASE224_OUT_CHUNK bytes long. If a longer block is split
 * to such chunks, the result will be identical.
 */
#define BASE224_IN_CHUNK 39 /** Chunk size on the un-encoded side. **/
#define BASE224_OUT_CHUNK 40 /** Chunk size on the encoded side. **/
