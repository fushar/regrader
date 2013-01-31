/*
 *	UCW Library -- Binomial Heaps: Declarations
 *
 *	(c) 2003 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _UCW_BINHEAP_NODE_H
#define _UCW_BINHEAP_NODE_H

/***
 * [[common]]
 * Common definitions
 * ------------------
 ***/

/**
 * Common header of binomial heap nodes.
 **/
struct bh_node {
  struct bh_node *first_son;
  struct bh_node *last_son;
  struct bh_node *next_sibling;
  byte order;
};

/**
 * A binomial heap.
 **/
struct bh_heap {
  struct bh_node root;
};

#endif
