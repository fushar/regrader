/*
 *	UCW Library -- Binomial Heaps
 *
 *	(c) 2003 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

/*
 *  This is a generic implementation of Binomial Heaps. Each time you include
 *  this file with parameters set in the corresponding preprocessor macros
 *  as described below, it generates functions for manipulating the particular
 *  version of the binomial heap.
 */

/***
 * [[generator]]
 * Interface to the generator
 * --------------------------
 *
 * To use the binomial heaps, you need to specify:
 *
 * - `BH_PREFIX(x)`  -- macro to add a name prefix (used on all global names
 *			defined by the generator). All further names mentioned
 *			here except for macro names will be implicitly prefixed.
 *
 * Then you continue by including `ucw/binheap-node.h` which defines <<struct_bh_node,struct bh_node>>
 * and <<struct_bh_heap,struct bh_heap>> (both without prefix). The heap elements are always allocated by
 * you and they must include `struct bh_node` which serves as a handle used for all
 * the heap functions and it contains all information needed for heap-keeping.
 * The heap itself is also allocated by you and it's represented by `struct bh_heap`.
 *
 * When you have the declaration of heap nodes, you continue with defining:
 *
 * - `less(p,q)`     -- returns `1` if the key corresponding to `bh_node *p`
 *			is less than the one corresponding to `*q`.
 *
 * Then specify what operations you request:
 *
 * - `init(heap\*)` -- initialize the heap (always defined).
 * - `insert(heap\*, node\*)` -- insert the node to the heap (`BH_WANT_INSERT`).
 * - `node\* findmin(heap\*)` -- find node with minimum key (`BH_WANT_FINDMIN`).
 * - `node\* deletemin(heap\*)` -- findmin and delete the node (`BH_WANT_DELETEMIN`).
 *
 * Then include `ucw/binheap.h` and voila, you have a binomial heap
 * suiting all your needs (at least those which you've revealed :) ).
 *
 * You also get a iterator macro at no extra charge:
 *
 *   BH_FOR_ALL(bh_prefix, heap*, variable)
 *     {
 *       // node* variable gets declared automatically
 *       do_something_with_node(variable);
 *       // use BH_BREAK and BH_CONTINUE instead of break and continue
 *       // you must not alter contents of the binomial heap here
 *     }
 *   BH_END_FOR;
 *
 * After including this file, all parameter macros are automatically undef'd.
 ***/

#define BH_NODE struct bh_node
#define BH_HEAP struct bh_heap

static void
BH_PREFIX(merge)(BH_NODE *a, BH_NODE *b)
{
  BH_NODE **pp = &a->first_son;
  BH_NODE *q = b->first_son;
  BH_NODE *p, *r, *s;

  while ((p = *pp) && q)
    {
      /* p,q are the next nodes of a,b; pp points to where p is linked */
      if (p->order < q->order)		/* p is smaller => skip it */
	pp = &p->next_sibling;
      else if (p->order > q->order)	/* q is smaller => insert it before p */
	{
	  r = q;
	  q = q->next_sibling;
	  r->next_sibling = p;
	  *pp = r;
	  pp = &r->next_sibling;
	}
      else				/* p and q are of the same order => need to merge them */
	{
	  if (BH_PREFIX(less)(p, q))	/* we'll hang r below s */
	    {
	      r = q;
	      s = p;
	    }
	  else
	    {
	      r = p;
	      s = q;
	    }
	  *pp = p->next_sibling;	/* unlink p,q from their lists */
	  q = q->next_sibling;

	  if (s->last_son)		/* merge r to s, increasing order */
	    s->last_son->next_sibling = r;
	  else
	    s->first_son = r;
	  s->last_son = r;
	  s->order++;
	  r->next_sibling = NULL;

	  if (!q || q->order > s->order) /* put the result into the b's list if possible */
	    {
	      s->next_sibling = q;
	      q = s;
	    }
	  else				/* otherwise put the result to the a's list */
	    {
	      p = s->next_sibling = *pp;
	      *pp = s;
	      if (p && p->order == s->order) /* 3-collision */
		pp = &s->next_sibling;
	    }
	}
    }
  if (!p)
    *pp = q;
}

#ifdef BH_WANT_INSERT
static void
BH_PREFIX(insert)(BH_HEAP *heap, BH_NODE *a)
{
  BH_NODE sh;

  sh.first_son = a;
  a->first_son = a->last_son = a->next_sibling = NULL;
  BH_PREFIX(merge)(&heap->root, &sh);
}
#endif

#ifdef BH_WANT_FINDMIN
static BH_NODE *
BH_PREFIX(findmin)(BH_HEAP *heap)
{
  BH_NODE *p, *best;

  best = NULL;
  for (p=heap->root.first_son; p; p=p->next_sibling)
    if (!best || BH_PREFIX(less)(p, best))
      best = p;
  return best;
}
#endif

#ifdef BH_WANT_DELETEMIN
static BH_NODE *
BH_PREFIX(deletemin)(BH_HEAP *heap)
{
  BH_NODE *p, **pp, **bestp;

  bestp = NULL;
  for (pp=&heap->root.first_son; p=*pp; pp=&p->next_sibling)
    if (!bestp || BH_PREFIX(less)(p, *bestp))
      bestp = pp;
  if (!bestp)
    return NULL;

  p = *bestp;
  *bestp = p->next_sibling;
  BH_PREFIX(merge)(&heap->root, p);
  return p;
}
#endif

static inline void
BH_PREFIX(init)(BH_HEAP *heap)
{
  bzero(heap, sizeof(*heap));
}

#ifndef BH_FOR_ALL

#define BH_FOR_ALL(bh_px, bh_heap, bh_var)	\
do {						\
  struct bh_node *bh_stack[32];			\
  uns bh_sp = 0;				\
  if (bh_stack[0] = (bh_heap)->root.first_son)  \
    bh_sp++;					\
  while (bh_sp) {				\
    struct bh_node *bh_var = bh_stack[--bh_sp];	\
    if (bh_var->next_sibling)			\
      bh_stack[bh_sp++] = bh_var->next_sibling; \
    if (bh_var->first_son)			\
      bh_stack[bh_sp++] = bh_var->first_son;
#define BH_END_FOR				\
  }						\
} while (0)

#define BH_BREAK { bh_sp=0; break; }
#define BH_CONTINUE continue

#endif

#undef BH_PREFIX
#undef BH_NODE
#undef BH_HEAP
#undef BH_WANT_INSERT
#undef BH_WANT_FINDMIN
#undef BH_WANT_DELETEMIN
