/*
 *	UCW Library -- Bit Array Signatures -- A Dubious Detector of Duplicates
 *
 *	(c) 2002 Martin Mares <mj@ucw.cz>
 *
 *	Greatly inspired by: Faloutsos, C. and Christodoulakis, S.: Signature files
 *	(An access method for documents and its analytical performance evaluation),
 *	ACM Trans. Office Inf. Syst., 2(4):267--288, Oct. 1984.
 *
 *	This data structure provides a very compact representation
 *	of a set of strings with insertion and membership search,
 *	but with a certain low probability it cheats by incidentally
 *	reporting a non-member as a member. Generally the larger you
 *	create the structure, the lower this probability is.
 *
 *	How does it work: the structure is just an array of M bits
 *	and each possible element is hashed to a set of (at most) L
 *	bit positions. For each element of the represented set, we
 *	set its L bits to ones and we report as present all elements
 *	whose all L bits ar set.
 *
 *	Analysis: Let's assume N items have already been stored	and let A
 *	denote L/M (density of the hash function). The probability that
 *	a fixed bit of the array is set by any of the N items is
 *    	1 - (1-1/M)^(NL) = 1 - ((1-1/M)^M)^NA = approx. 1 - e^-NA.
 *	This is minimized by setting A=(ln 2)/N (try taking derivative).
 *	Given a non-present item, the probability that all of the bits
 *	corresponding to this item are set by the other items (that is,
 *	the structure gives a false answer) is (1-e^-NA)^L = 2^-L.
 *	Hence, if we want to give false answers with probability less
 *	than epsilon, we take L := -log_2 epsilon, M := 1.45*N*L.
 *
 *	Example: For a set of 10^7 items with P[error] < 10^-6, we set
 *	L := 20 and M :=  290*10^6 bits = cca 34.5 MB (29 bits per item).
 *
 *	We leave L and an upper bound for N as parameters set during
 *	creation of the structure. Currently, the structure is limited
 *	to 4 Gb = 512 MB.
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/bitsig.h"
#include "ucw/md5.h"

#include <string.h>

struct bitsig {
  uns l, m, n, maxn, max_m_mult;
  u32 hash[4];
  uns hindex;
  byte array[0];
};

struct bitsig *
bitsig_init(uns perrlog, uns maxn)
{
  struct bitsig *b;
  u64 m;
  uns mbytes;

  m = ((u64) maxn * perrlog * 145 + 99) / 100;
  if (m >= (u64) 1 << 32)
    die("bitsig_init: bitsig array too large (maximum is 4 Gb)");
  mbytes = (m + 7) >> 3U;
  b = xmalloc(sizeof(struct bitsig) + mbytes);
  b->l = perrlog;
  b->m = m;
  b->n = 0;
  b->maxn = maxn;
  b->max_m_mult = (0xffffffff / m) * m;
  bzero(b->array, mbytes);
  msg(L_DEBUG, "Initialized bitsig array with l=%d, m=%u (%u KB), expecting %d items", b->l, b->m, (mbytes+1023)/1024, maxn);
  return b;
}

void
bitsig_free(struct bitsig *b)
{
  xfree(b);
}

static void
bitsig_hash_init(struct bitsig *b, byte *item)
{
  md5_hash_buffer((byte *) b->hash, item, strlen(item));
  b->hindex = 0;
}

static inline uns
bitsig_hash_bit(struct bitsig *b)
{
  u32 h;
  do
    {
      h = b->hash[b->hindex];
      b->hash[b->hindex] *= 3006477127U;
      b->hindex = (b->hindex+1) % 4;
    }
  while (h >= b->max_m_mult);
  return h % b->m;
}

int
bitsig_member(struct bitsig *b, byte *item)
{
  uns i, bit;

  bitsig_hash_init(b, item);
  for (i=0; i<b->l; i++)
    {
      bit = bitsig_hash_bit(b);
      if (!(b->array[bit >> 3] & (1 << (bit & 7))))
	return 0;
    }
  return 1;
}

int
bitsig_insert(struct bitsig *b, byte *item)
{
  uns i, bit, was;

  bitsig_hash_init(b, item);
  was = 1;
  for (i=0; i<b->l; i++)
    {
      bit = bitsig_hash_bit(b);
      if (!(b->array[bit >> 3] & (1 << (bit & 7))))
	{
	  was = 0;
	  b->array[bit >> 3] |= (1 << (bit & 7));
	}
    }
  if (!was && b->n++ == b->maxn+1)
    msg(L_ERROR, "bitsig: Too many items inserted, error rate will be higher than estimated!");
  return was;
}

#ifdef TEST

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  struct bitsig *b = bitsig_init(atol(argv[1]), atol(argv[2]));
  byte buf[1024];

  while (fgets(buf, 1024, stdin))
    printf("%d\n", bitsig_insert(b, buf));

  return 0;
}

#endif
