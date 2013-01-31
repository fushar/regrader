/*
 *	UCW Library -- Bit Array Signatures -- A Dubious Detector of Duplicates
 *
 *	(c) 2002 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _UCW_BITSIG_H
#define _UCW_BITSIG_H

struct bitsig;

struct bitsig *bitsig_init(uns perrlog, uns maxn);
void bitsig_free(struct bitsig *b);
int bitsig_member(struct bitsig *b, byte *item);
int bitsig_insert(struct bitsig *b, byte *item);

#endif
