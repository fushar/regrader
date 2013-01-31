/*
 *	UCW Library -- Linked Lists of Simple Items
 *
 *	(c) 2006 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _UCW_SIMPLE_LISTS_H
#define _UCW_SIMPLE_LISTS_H

#include "ucw/clists.h"

/***
 * To simplify very common usage of circular linked links, whose nodes can hold only one or two trivial values,
 * we define some generic node types, called the simple nodes.
 *
 * To avoid some type casts, values in simple nodes are defined as unions of most frequent types.
 ***/

/**
 * Simple node with one value.
 **/
typedef struct simp_node {
  cnode n;
  union {
    char *s;
    void *p;
    int i;
    uns u;
  };
} simp_node;

/**
 * Simple node with two values.
 **/
typedef struct simp2_node {
  cnode n;
  union {
    char *s1;
    void *p1;
    int i1;
    uns u1;
  };
  union {
    char *s2;
    void *p2;
    int i2;
    uns u2;
  };
} simp2_node;

struct mempool;

/**
 * Allocate a new one-value node on memory pool @mp and insert it to @l. The value is undefined and should be changed afterwards.
 **/
simp_node *simp_append(struct mempool *mp, clist *l);

/**
 * Allocate a new two-value node on memory pool @mp and insert it to @l. The values are undefined and should be changed afterwards.
 **/
simp2_node *simp2_append(struct mempool *mp, clist *l);

/* Configuration sections */

/**
 * Default definition of the configuration section with one-value string node. Identifier of the value is `String`.
 **/
extern struct cf_section cf_string_list_config;

/**
 * Default definition of the configuration section with two-value string node. Identifiers of the values are `Src` and `Dest`.
 **/
extern struct cf_section cf_2string_list_config;

#endif
