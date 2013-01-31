/*
 *	The UCW Library -- Prime numbers
 *
 *	(c) 2008 Michal Vaner <vorner@ucw.cz>
 *
 *	Code taken from ucw/lib.h by:
 *
 *	(c) 1997--2008 Martin Mares <mj@ucw.cz>
 *	(c) 2005 Tomas Valla <tom@ucw.cz>
 *	(c) 2006 Robert Spalek <robert@ucw.cz>
 *	(c) 2007 Pavel Charvat <pchar@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _UCW_PRIME_H
#define _UCW_PRIME_H

#include "ucw/lib.h"

/* prime.c */

/**
 * Return a non-zero value iff @x is a prime number.
 * The time complexity is `O(sqrt(x))`.
 **/
int isprime(uns x);

/**
 * Return some prime greater than @x. The function does not checks overflows, but it should
 * be safe at least for @x lower than `1U << 31`.
 * If the Cramer's conjecture is true, it should have complexity `O(sqrt(x) * log(x)^2)`.
 **/
uns nextprime(uns x);

/* primetable.c */

/**
 * Quickly lookup a precomputed table to return a prime number greater than @x.
 * Returns zero if there is no such prime (we guarantee the existance of at
 * least one prime greater than `1U << 31` in the table).
 **/
uns next_table_prime(uns x);

/**
 * Quickly lookup a precomputed table to return a prime number smaller than @x.
 * Returns zero if @x is smaller than `7`.
 **/
uns prev_table_prime(uns x);

#endif // _UCW_PRIME_H
