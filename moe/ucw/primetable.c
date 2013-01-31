/*
 *	UCW Library -- Prime Number Table
 *
 *	(c) 2005 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/prime.h"
#include "ucw/binsearch.h"

/* A table of odd primes, each is about 1.2 times the previous one */
static uns prime_table[] = {
  3,
  7,
  13,
  19,
  29,
  37,
  53,
  67,
  89,
  109,
  137,
  173,
  211,
  263,
  331,
  409,
  499,
  601,
  727,
  877,
  1061,
  1279,
  1543,
  1861,
  2239,
  2689,
  3229,
  3877,
  4657,
  5623,
  6761,
  8123,
  9767,
  11731,
  14083,
  16903,
  20287,
  24359,
  29243,
  35099,
  42131,
  50581,
  60703,
  72859,
  87433,
  104933,
  125927,
  151121,
  181361,
  217643,
  261223,
  313471,
  376171,
  451411,
  541699,
  650059,
  780119,
  936151,
  1123391,
  1348111,
  1617739,
  1941293,
  2329559,
  2795477,
  3354581,
  4025507,
  4830619,
  5796797,
  6956203,
  8347483,
  10017011,
  12020431,
  14424539,
  17309471,
  20771371,
  24925661,
  29910821,
  35892991,
  43071601,
  51685939,
  62023139,
  74427803,
  89313379,
  107176057,
  128611313,
  154333591,
  185200339,
  222240413,
  266688509,
  320026249,
  384031507,
  460837813,
  553005391,
  663606499,
  796327811,
  955593439,
  1146712139,
  1376054569,
  1651265507,
  1981518631,
  2377822387,
  2853386881,
  3424064269,
  4108877153,
  4294967291
};

#define NPRIMES ARRAY_SIZE(prime_table)

uns
next_table_prime(uns x)
{
  if (x >= prime_table[NPRIMES-1])
    return 0;
  else
    return prime_table[BIN_SEARCH_FIRST_GE(prime_table, NPRIMES, x+1)];
}

uns
prev_table_prime(uns x)
{
  int i = BIN_SEARCH_FIRST_GE(prime_table, NPRIMES, x);
  return i ? prime_table[i-1] : 0;
}

#ifdef TEST

#include <stdio.h>

int main(void)
{
#if 0		/* Generate the table */
  uns x = 3, xx;
  do
    {
      printf("  %u,\n", x);
      xx = x;
      x = nextprime(1.2*x);
    }
  while (x > xx);
#else
  for (int i=1; i<=100; i++)
    printf("%d\t%d\t%d\n", i, next_table_prime(i), prev_table_prime(i));
  for (uns i=0xfffffff0; i; i++)
    printf("%u\t%u\t%u\n", i, next_table_prime(i), prev_table_prime(i));
  return 0;
#endif
}

#endif
