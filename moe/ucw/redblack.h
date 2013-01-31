/*
 *	UCW Library -- Red-black trees
 *
 *	(c) 2002--2005, Robert Spalek <robert@ucw.cz>
 *
 *	Skeleton based on hash-tables by:
 *
 *	(c) 2002, Martin Mares <mj@ucw.cz>
 *
 */

/*
 * Data structure description:
 *
 * A red-black tree is a binary search tree, where records are stored
 * in nodes (may be also leaves).  Every node has a colour.  The
 * following restrictions hold:
 *
 * - a parent of a red node is black
 * - every path from the root to a node with less than 2 children
 *   contains the same number of black nodes
 *
 * A usual interpretation is, that leaves are intervals between records
 * and contain no data.  Every leaf is black.  This is equivalent, but
 * saves the space.
 */

/*
 *  This is not a normal header file, it's a generator of red-black trees.
 *  Each time you include it with parameters set in the corresponding
 *  preprocessor macros, it generates a tree structure with the parameters
 *  given.
 *
 *  You need to specify:
 *
 *  TREE_NODE		data type where a node dwells (usually a struct).
 *  TREE_PREFIX(x)	macro to add a name prefix (used on all global names
 *			defined by the tree generator).
 *
 *  Then decide on type of keys:
 *
 *  TREE_KEY_ATOMIC=f	use node->f as a key of an atomic type (i.e.,
 *			a type which can be compared using '>', `==', and '<')
 *			& TREE_ATOMIC_TYPE (defaults to int).
 *  | TREE_KEY_STRING=f	use node->f as a string key, allocated
 *			separately from the rest of the node.
 *  | TREE_KEY_ENDSTRING=f use node->f as a string key, allocated
 *			automatically at the end of the node struct
 *			(to be declared as "char f[1]" at the end).
 *  | TREE_KEY_COMPLEX	use a multi-component key; as the name suggests,
 *			the passing of parameters is a bit complex then.
 *			The TREE_KEY_COMPLEX(x) macro should expand to
 *			`x k1, x k2, ... x kn' and you should also define:
 *    & TREE_KEY_DECL	declaration of function parameters in which key
 *			should be passed to all tree operations.
 *			That is, `type1 k1, type2 k2, ... typen kn'.
 *			With complex keys, TREE_GIVE_CMP is mandatory.
 *
 *  Then specify what operations you request (all names are automatically
 *  prefixed by calling TREE_PREFIX):
 *
 *  <always defined>	init() -- initialize the tree.
 *  TREE_WANT_CLEANUP	cleanup() -- deallocate the tree.
 *  TREE_WANT_FIND	node *find(key) -- find first node with the specified
 *			key, return NULL if no such node exists.
 *  TREE_WANT_FIND_NEXT	node *find_next(node *start) -- find next node with the
 *			specified key, return NULL if no such node exists.
 *			Implies TREE_DUPLICATES.
 *  TREE_WANT_SEARCH	node *search(key) -- find the node with the specified
 *			or, if it does not exist, the nearest one.
 *  TREE_WANT_SEARCH_DOWN node *search_down(key) -- find either the node with
 *                      specified value, or if it does not exist, the node
 *                      with nearest smaller value.
 *  TREE_WANT_BOUNDARY	node *boundary(uns direction) -- finds smallest
 *  			(direction==0) or largest (direction==1) node.
 *  TREE_WANT_ADJACENT	node *adjacent(node *, uns direction) -- finds next
 *			(direction==1) or previous (direction==0) node.
 *  TREE_WANT_NEW	node *new(key) -- create new node with given key.
 *			If it already exists, it is created as the last one.
 *  TREE_WANT_LOOKUP	node *lookup(key) -- find node with given key,
 *			if it doesn't exist, create it. Defining
 *			TREE_GIVE_INIT_DATA is strongly recommended.
 *  TREE_WANT_DELETE	int delete(key) -- delete and deallocate node
 *			with a given key. Returns success.
 *  TREE_WANT_REMOVE	remove(node *) -- delete and deallocate given node.
 *
 *  TREE_WANT_DUMP	dump() -- dumps the whole tree to stdout
 *
 *  You can also supply several functions:
 *
 *  TREE_GIVE_CMP	int cmp(key1, key2) -- return -1, 0, and 1 according to
 *			the relation of keys.  By default, we use <, ==, > for
 *			atomic types and either strcmp or strcasecmp for
 *			strings.
 *  TREE_GIVE_EXTRA_SIZE int extra_size(key) -- returns how many bytes after the
 *			node should be allocated for dynamic data. Default=0
 *			or length of the string with TREE_KEY_ENDSTRING.
 *  TREE_GIVE_INIT_KEY	void init_key(node *,key) -- initialize key in a newly
 *			created node. Defaults: assignment for atomic keys
 *			and static strings, strcpy for end-allocated strings.
 *  TREE_GIVE_INIT_DATA	void init_data(node *) -- initialize data fields in a
 *			newly created node. Very useful for lookup operations.
 *  TREE_GIVE_ALLOC	void *alloc(unsigned int size) -- allocate space for
 *			a node. Default is either normal or pooled allocation
 *			depending on whether we want deletions.
 *			void free(void *) -- the converse.
 *
 *  ... and a couple of extra parameters:
 *
 *  TREE_NOCASE		string comparisons should be case-insensitive.
 *  TREE_ATOMIC_TYPE=t	Atomic values are of type `t' instead of int.
 *  TREE_USE_POOL=pool	Allocate all nodes from given mempool.
 *			Collides with delete/remove functions.
 *  TREE_GLOBAL		Functions are exported (i.e., not static).
 *  TREE_CONSERVE_SPACE	Use as little space as possible at the price of a
 *			little slowdown.
 *  TREE_DUPLICATES	Records with duplicate keys are allowed.
 *  TREE_MAX_DEPTH	Maximal depth of a tree (for stack allocation).
 *
 *  If you set TREE_WANT_ITERATOR, you also get a iterator macro at no
 *  extra charge:
 *
 *  TREE_FOR_ALL(tree_prefix, tree_pointer, variable)
 *    {
 *      // node *variable gets declared automatically
 *      do_something_with_node(variable);
 *      // use TREE_BREAK and TREE_CONTINUE instead of break and continue
 *	// you must not alter contents of the tree here
 *    }
 *  TREE_END_FOR;
 *
 *  Then include "ucw/redblack.h" and voila, you have a tree suiting all your
 *  needs (at least those which you've revealed :) ).
 *
 *  After including this file, all parameter macros are automatically
 *  undef'd.
 */

#include <stdio.h>
#include <string.h>

#if !defined(TREE_NODE) || !defined(TREE_PREFIX)
#error Some of the mandatory configuration macros are missing.
#endif

#define P(x) TREE_PREFIX(x)

/* Declare buckets and the tree.  */

typedef TREE_NODE P(node);

#if defined(TREE_WANT_FIND_NEXT) || defined(TREE_WANT_ADJACENT) || defined(TREE_WANT_ITERATOR) || defined(TREE_WANT_REMOVE)
#	define TREE_STORE_PARENT
#endif

typedef struct P(bucket) {
	struct P(bucket) *son[2];
#ifdef TREE_STORE_PARENT
	struct P(bucket) *parent;
#endif
#if !defined(TREE_CONSERVE_SPACE) && (defined(TREE_GIVE_EXTRA_SIZE) || defined(TREE_KEY_ENDSTRING))
	uns red_flag:1;
#endif
	P(node) n;
#if !defined(TREE_CONSERVE_SPACE) && !defined(TREE_GIVE_EXTRA_SIZE) && !defined(TREE_KEY_ENDSTRING)
	uns red_flag:1;
#endif
} P(bucket);

struct P(tree) {
	uns count;
	uns height;			/* of black nodes */
	P(bucket) *root;
};

typedef struct P(stack_entry) {
	P(bucket) *buck;
	uns son;
} P(stack_entry);

#define T struct P(tree)

/* Preset parameters */

#if defined(TREE_KEY_ATOMIC)

#define TREE_KEY(x) x TREE_KEY_ATOMIC

#ifndef TREE_ATOMIC_TYPE
#	define TREE_ATOMIC_TYPE int
#endif
#define TREE_KEY_DECL TREE_ATOMIC_TYPE TREE_KEY()

#ifndef TREE_GIVE_CMP
#	define TREE_GIVE_CMP
	static inline int P(cmp) (TREE_ATOMIC_TYPE x, TREE_ATOMIC_TYPE y)
	{
		if (x < y)
			return -1;
		else if (x > y)
			return 1;
		else
			return 0;
	}
#endif

#ifndef TREE_GIVE_INIT_KEY
#	define TREE_GIVE_INIT_KEY
	static inline void P(init_key) (P(node) *n, TREE_ATOMIC_TYPE k)
	{ TREE_KEY(n->) = k; }
#endif

#elif defined(TREE_KEY_STRING) || defined(TREE_KEY_ENDSTRING)

#ifdef TREE_KEY_STRING
#	define TREE_KEY(x) x TREE_KEY_STRING
#	ifndef TREE_GIVE_INIT_KEY
#		define TREE_GIVE_INIT_KEY
		static inline void P(init_key) (P(node) *n, char *k)
		{ TREE_KEY(n->) = k; }
#	endif
#else
#	define TREE_KEY(x) x TREE_KEY_ENDSTRING
#	define TREE_GIVE_EXTRA_SIZE
	static inline int P(extra_size) (char *k)
	{ return strlen(k); }
#	ifndef TREE_GIVE_INIT_KEY
#		define TREE_GIVE_INIT_KEY
		static inline void P(init_key) (P(node) *n, char *k)
		{ strcpy(TREE_KEY(n->), k); }
#	endif
#endif
#define TREE_KEY_DECL char *TREE_KEY()

#ifndef TREE_GIVE_CMP
#	define TREE_GIVE_CMP
	static inline int P(cmp) (char *x, char *y)
	{
#		ifdef TREE_NOCASE
			return strcasecmp(x,y);
#		else
			return strcmp(x,y);
#		endif
   	}
#endif

#elif defined(TREE_KEY_COMPLEX)

#define TREE_KEY(x) TREE_KEY_COMPLEX(x)

#else
#error You forgot to set the tree key type.
#endif

#ifndef TREE_CONSERVE_SPACE
	static inline uns P(red_flag) (P(bucket) *node)
	{ return node->red_flag; }
	static inline void P(set_red_flag) (P(bucket) *node, uns flag)
	{ node->red_flag = flag; }
	static inline P(bucket) * P(tree_son) (P(bucket) *node, uns id)
	{ return node->son[id]; }
	static inline void P(set_tree_son) (P(bucket) *node, uns id, P(bucket) *son)
	{ node->son[id] = son; }
#else
	/* Pointers are aligned, hence we can use lower bits.  */
	static inline uns P(red_flag) (P(bucket) *node)
	{ return ((uintptr_t) node->son[0]) & 1L; }
	static inline void P(set_red_flag) (P(bucket) *node, uns flag)
	{ node->son[0] = (void*) ( (((uintptr_t) node->son[0]) & ~1L) | (flag & 1L) ); }
	static inline P(bucket) * P(tree_son) (P(bucket) *node, uns id)
	{ return (void *) (((uintptr_t) node->son[id]) & ~1L); }
	static inline void P(set_tree_son) (P(bucket) *node, uns id, P(bucket) *son)
	{ node->son[id] = (void *) ((uintptr_t) son | (((uintptr_t) node->son[id]) & 1L) ); }
#endif

/* Defaults for missing parameters.  */

#ifndef TREE_GIVE_CMP
#error Unable to determine how to compare two keys.
#endif

#ifdef TREE_GIVE_EXTRA_SIZE
/* This trickery is needed to avoid `unused parameter' warnings */
#	define TREE_EXTRA_SIZE P(extra_size)
#else
/*
 *  Beware, C macros are expanded iteratively, not recursively,
 *  hence we get only a _single_ argument, although the expansion
 *  of TREE_KEY contains commas.
 */
#	define TREE_EXTRA_SIZE(x) 0
#endif

#ifndef TREE_GIVE_INIT_KEY
#	error Unable to determine how to initialize keys.
#endif

#ifndef TREE_GIVE_INIT_DATA
static inline void P(init_data) (P(node) *n UNUSED)
{
}
#endif

#include <stdlib.h>

#ifndef TREE_GIVE_ALLOC
#	ifdef TREE_USE_POOL
		static inline void * P(alloc) (unsigned int size)
		{ return mp_alloc_fast(TREE_USE_POOL, size); }
#		define TREE_SAFE_FREE(x)
#	else
		static inline void * P(alloc) (unsigned int size)
		{ return xmalloc(size); }

		static inline void P(free) (void *x)
		{ xfree(x); }
#	endif
#endif

#ifndef	TREE_SAFE_FREE
#	define TREE_SAFE_FREE(x)	P(free) (x)
#endif

#ifdef TREE_GLOBAL
#	define STATIC
#else
#	define STATIC static
#endif

#ifndef TREE_MAX_DEPTH
#	define TREE_MAX_DEPTH 64
#endif

#if defined(TREE_WANT_FIND_NEXT) && !defined(TREE_DUPLICATES)
#	define TREE_DUPLICATES
#endif

#ifdef TREE_WANT_LOOKUP
#ifndef TREE_WANT_FIND
#	define TREE_WANT_FIND
#endif
#ifndef TREE_WANT_NEW
#	define TREE_WANT_NEW
#endif
#endif

/* Now the operations */

STATIC void P(init) (T *t)
{
	t->count = t->height = 0;
	t->root = NULL;
}

#ifdef TREE_WANT_CLEANUP
static void P(cleanup_subtree) (T *t, P(bucket) *node)
{
	if (!node)
		return;
	P(cleanup_subtree) (t, P(tree_son) (node, 0));
	P(cleanup_subtree) (t, P(tree_son) (node, 1));
	P(free) (node);
	t->count--;
}

STATIC void P(cleanup) (T *t)
{
	P(cleanup_subtree) (t, t->root);
	ASSERT(!t->count);
	t->height = 0;
}
#endif

static uns P(fill_stack) (P(stack_entry) *stack, uns max_depth, P(bucket) *node, TREE_KEY_DECL, uns son_id UNUSED)
{
	uns i;
	stack[0].buck = node;
	for (i=0; stack[i].buck; i++)
	{
		int cmp;
		cmp = P(cmp) (TREE_KEY(), TREE_KEY(stack[i].buck->n.));
		if (cmp == 0)
			break;
		else if (cmp < 0)
			stack[i].son = 0;
		else
			stack[i].son = 1;
		ASSERT(i+1 < max_depth);
		stack[i+1].buck = P(tree_son) (stack[i].buck, stack[i].son);
	}
#ifdef TREE_DUPLICATES
	if (stack[i].buck)
	{
		uns idx;
		/* Find first/last of equal keys according to son_id.  */
		idx = P(fill_stack) (stack+i+1, max_depth-i-1,
			P(tree_son) (stack[i].buck, son_id), TREE_KEY(), son_id);
		if (stack[i+1+idx].buck)
		{
			stack[i].son = son_id;
			i = i+1+idx;
		}
	}
#endif
	stack[i].son = 10;
	return i;
}

#ifdef TREE_WANT_FIND
STATIC P(node) * P(find) (T *t, TREE_KEY_DECL)
{
	P(stack_entry) stack[TREE_MAX_DEPTH];
	uns depth;
	depth = P(fill_stack) (stack, TREE_MAX_DEPTH, t->root, TREE_KEY(), 0);
	return stack[depth].buck ? &stack[depth].buck->n : NULL;
}
#endif

#ifdef TREE_WANT_SEARCH_DOWN
STATIC P(node) * P(search_down) (T *t, TREE_KEY_DECL)
{
	P(node) *last_right=NULL;
	P(bucket) *node=t->root;
	while(node)
	{
		int cmp;
		cmp = P(cmp) (TREE_KEY(), TREE_KEY(node->n.));
		if (cmp == 0)
			return &node->n;
		else if (cmp < 0)
			node=P(tree_son) (node, 0);
		else
		{
			last_right=&node->n;
			node=P(tree_son) (node, 1);
		}
	}
	return last_right;
}
#endif

#ifdef TREE_WANT_BOUNDARY
STATIC P(node) * P(boundary) (T *t, uns direction)
{
	P(bucket) *n = t->root, *ns;
	if (!n)
		return NULL;
	else
	{
		uns son = !!direction;
		while ((ns = P(tree_son) (n, son)))
			n = ns;
		return &n->n;
	}
}
#endif

#ifdef TREE_STORE_PARENT
STATIC P(node) * P(adjacent) (P(node) *start, uns direction)
{
	P(bucket) *node = SKIP_BACK(P(bucket), n, start);
	P(bucket) *next = P(tree_son) (node, direction);
	if (next)
	{
		while (1)
		{
			node = P(tree_son) (next, 1 - direction);
			if (!node)
				break;
			next = node;
		}
	}
	else
	{
		next = node->parent;
		while (next && node == P(tree_son) (next, direction))
		{
			node = next;
			next = node->parent;
		}
		if (!next)
			return NULL;
		ASSERT(node == P(tree_son) (next, 1 - direction));
	}
	return &next->n;
}
#endif

#if defined(TREE_DUPLICATES) || defined(TREE_WANT_DELETE) || defined(TREE_WANT_REMOVE)
static int P(find_next_node) (P(stack_entry) *stack, uns max_depth, uns direction)
{
	uns depth = 0;
	if (stack[0].buck)
	{
		ASSERT(depth+1 < max_depth);
		stack[depth].son = direction;
		stack[depth+1].buck = P(tree_son) (stack[depth].buck, direction);
		depth++;
		while (stack[depth].buck)
		{
			ASSERT(depth+1 < max_depth);
			stack[depth].son = 1 - direction;
			stack[depth+1].buck = P(tree_son) (stack[depth].buck, 1 - direction);
			depth++;
		}
	}
	return depth;
}
#endif

#ifdef TREE_WANT_FIND_NEXT
STATIC P(node) * P(find_next) (P(node) *start)
{
	P(node) *next = P(adjacent) (start, 1);
	if (next && P(cmp) (TREE_KEY(start->), TREE_KEY(next->)) == 0)
		return next;
	else
		return NULL;

}
#endif

#ifdef TREE_WANT_SEARCH
STATIC P(node) * P(search) (T *t, TREE_KEY_DECL)
{
	P(stack_entry) stack[TREE_MAX_DEPTH];
	uns depth;
	depth = P(fill_stack) (stack, TREE_MAX_DEPTH, t->root, TREE_KEY(), 0);
	if (!stack[depth].buck)
	{
		if (depth > 0)
			depth--;
		else
			return NULL;
	}
	return &stack[depth].buck->n;
}
#endif

#if 0
#define TREE_TRACE(txt...) do { printf(txt); fflush(stdout); } while (0)
#else
#define TREE_TRACE(txt...)
#endif

static inline P(bucket) * P(rotation) (P(bucket) *node, uns son_id)
{
	/* Destroys red_flag's in node, son.  Returns new root.  */
	P(bucket) *son = P(tree_son) (node, son_id);
	TREE_TRACE("Rotation (node %d, son %d), direction %d\n", node->n.key, son->n.key, son_id);
	node->son[son_id] = P(tree_son) (son, 1-son_id);
	son->son[1-son_id] = node;
#ifdef TREE_STORE_PARENT
	if (node->son[son_id])
		node->son[son_id]->parent = node;
	son->parent = node->parent;
	node->parent = son;
#endif
	return son;
}

static void P(rotate_after_insert) (T *t, P(stack_entry) *stack, uns depth)
{
	P(bucket) *node;
	P(bucket) *parent, *grand, *uncle;
	int s1, s2;
try_it_again:
	node = stack[depth].buck;
	ASSERT(P(red_flag) (node));
	/* At this moment, node became red.  The paths sum have
	 * been preserved, but we have to check the parental
	 * condition.  */
	if (depth == 0)
	{
		ASSERT(t->root == node);
		return;
	}
	parent = stack[depth-1].buck;
	if (!P(red_flag) (parent))
		return;
	if (depth == 1)
	{
		ASSERT(t->root == parent);
		P(set_red_flag) (parent, 0);
		t->height++;
		return;
	}
	grand = stack[depth-2].buck;
	ASSERT(!P(red_flag) (grand));
	/* The parent is also red, the grandparent exists and it
	 * is black.  */
	s1 = stack[depth-1].son;
	s2 = stack[depth-2].son;
	uncle = P(tree_son) (grand, 1-s2);
	if (uncle && P(red_flag) (uncle))
	{
		/* Red parent and uncle, black grandparent.
		 * Exchange and try another iteration. */
		P(set_red_flag) (parent, 0);
		P(set_red_flag) (uncle, 0);
		P(set_red_flag) (grand, 1);
		depth -= 2;
		TREE_TRACE("Swapping colours (parent %d, uncle %d, grand %d), passing thru\n", parent->n.key, uncle->n.key, grand->n.key);
		goto try_it_again;
	}
	/* Black uncle and grandparent, we need to rotate.  Test
	 * the direction.  */
	if (s1 == s2)
	{
		node = P(rotation) (grand, s2);
		P(set_red_flag) (parent, 0);
		P(set_red_flag) (grand, 1);
	}
	else
	{
		grand->son[s2] = P(rotation) (parent, s1);
		node = P(rotation) (grand, s2);
		P(set_red_flag) (grand, 1);
		P(set_red_flag) (parent, 1);
		P(set_red_flag) (node, 0);
	}
	if (depth >= 3)
		P(set_tree_son) (stack[depth-3].buck, stack[depth-3].son, node);
	else
		t->root = node;
}

#ifdef TREE_WANT_NEW
STATIC P(node) * P(new) (T *t, TREE_KEY_DECL)
{
	P(stack_entry) stack[TREE_MAX_DEPTH];
	P(bucket) *added;
	uns depth;
	depth = P(fill_stack) (stack, TREE_MAX_DEPTH, t->root, TREE_KEY(), 1);
#ifdef TREE_DUPLICATES
	/* It is the last found value, hence everything in the right subtree is
	 * strongly _bigger_.  */
	depth += P(find_next_node) (stack+depth, TREE_MAX_DEPTH-depth, 1);
#endif
	ASSERT(!stack[depth].buck);
	/* We are in a leaf, hence we can easily append a new leaf to it.  */
	added = P(alloc) (sizeof(struct P(bucket)) + TREE_EXTRA_SIZE(TREE_KEY()) );
	added->son[0] = added->son[1] = NULL;
	stack[depth].buck = added;
	if (depth > 0)
	{
#ifdef TREE_STORE_PARENT
		added->parent = stack[depth-1].buck;
#endif
		P(set_tree_son) (stack[depth-1].buck, stack[depth-1].son, added);
	}
	else
	{
#ifdef TREE_STORE_PARENT
		added->parent = NULL;
#endif
		t->root = added;
	}
	P(set_red_flag) (added, 1);	/* Set it red to not disturb the path sum.  */
	P(init_key) (&added->n, TREE_KEY());
	P(init_data) (&added->n);
	t->count++;
	/* Let us reorganize the red_flag's and the structure of the tree.  */
	P(rotate_after_insert) (t, stack, depth);
	return &added->n;
}
#endif

#ifdef TREE_WANT_LOOKUP
STATIC P(node) * P(lookup) (T *t, TREE_KEY_DECL)
{
	P(node) *node;
	node = P(find) (t, TREE_KEY());
	if (node)
		return node;
	return P(new) (t, TREE_KEY());
}
#endif

#if defined(TREE_WANT_REMOVE) || defined(TREE_WANT_DELETE)
static void P(rotate_after_delete) (T *t, P(stack_entry) *stack, int depth)
{
	uns iteration = 0;
	P(bucket) *parent, *sibling, *instead;
	uns parent_red, del_son, sibl_red;
missing_black:
	if (depth < 0)
	{
		t->height--;
		return;
	}
	parent = stack[depth].buck;
	parent_red = P(red_flag) (parent);
	del_son = stack[depth].son;
	/* For the 1st iteration: we have deleted parent->son[del_son], which
	 * was a black node with no son.  Hence there is one mising black
	 * vertex in that path, which we are going to fix now.
	 *
	 * For other iterations: in that path, there is also missing a black
	 * node.  */
	if (!iteration)
		ASSERT(!P(tree_son) (parent, del_son));
	sibling = P(tree_son) (parent, 1-del_son);
	ASSERT(sibling);
	sibl_red = P(red_flag) (sibling);
	instead = NULL;
	if (!sibl_red)
	{
		P(bucket) *son[2];
		uns red[2];
		son[0] = P(tree_son) (sibling, 0);
		son[1] = P(tree_son) (sibling, 1);
		red[0] = son[0] ? P(red_flag) (son[0]) : 0;
		red[1] = son[1] ? P(red_flag) (son[1]) : 0;
		if (!red[0] && !red[1])
		{
			P(set_red_flag) (sibling, 1);
			P(set_red_flag) (parent, 0);
			if (parent_red)
				return;
			else
			{
				depth--;
				iteration++;
				TREE_TRACE("Swapping colours (parent %d, sibling %d), passing thru\n", parent->n.key, sibling->n.key);
				goto missing_black;
			}
		} else if (!red[del_son])
		{
			instead = P(rotation) (parent, 1-del_son);
			P(set_red_flag) (instead, parent_red);
			P(set_red_flag) (parent, 0);
			P(set_red_flag) (son[1-del_son], 0);
		} else /* red[del_son] */
		{
			parent->son[1-del_son] = P(rotation) (sibling, del_son);
			instead = P(rotation) (parent, 1-del_son);
			P(set_red_flag) (instead, parent_red);
			P(set_red_flag) (parent, 0);
			P(set_red_flag) (sibling, 0);
		}
	} else /* sibl_red */
	{
		P(bucket) *grand[2], *son;
		uns red[2];
		ASSERT(!parent_red);
		son = P(tree_son) (sibling, del_son);
		ASSERT(son && !P(red_flag) (son));
		grand[0] = P(tree_son) (son, 0);
		grand[1] = P(tree_son) (son, 1);
		red[0] = grand[0] ? P(red_flag) (grand[0]) : 0;
		red[1] = grand[1] ? P(red_flag) (grand[1]) : 0;
		if (!red[0] && !red[1])
		{
			instead = P(rotation) (parent, 1-del_son);
			P(set_red_flag) (instead, 0);
			P(set_red_flag) (parent, 0);
			P(set_red_flag) (son, 1);
		}
		else if (!red[del_son])
		{
			parent->son[1-del_son] = P(rotation) (sibling, del_son);
			instead = P(rotation) (parent, 1-del_son);
			P(set_red_flag) (instead, 0);
			P(set_red_flag) (parent, 0);
			P(set_red_flag) (sibling, 1);
			P(set_red_flag) (grand[1-del_son], 0);
		} else /* red[del_son] */
		{
			sibling->son[del_son] = P(rotation) (son, del_son);
			parent->son[1-del_son] = P(rotation) (sibling, del_son);
			instead = P(rotation) (parent, 1-del_son);
			P(set_red_flag) (instead, 0);
			P(set_red_flag) (parent, 0);
			P(set_red_flag) (sibling, 1);
			P(set_red_flag) (son, 0);
		}
	}
	/* We have performed all desired rotations and need to store the new
	 * pointer to the subtree.  */
	ASSERT(instead);
	if (depth > 0)
		P(set_tree_son) (stack[depth-1].buck, stack[depth-1].son, instead);
	else
		t->root = instead;
}

static void P(remove_by_stack) (T *t, P(stack_entry) *stack, uns depth)
{
	P(bucket) *node = stack[depth].buck;
	P(bucket) *son;
	uns i;
	for (i=0; i<depth; i++)
		ASSERT(P(tree_son) (stack[i].buck, stack[i].son) == stack[i+1].buck);
	if (P(tree_son) (node, 0) && P(tree_son) (node, 1))
	{
		P(bucket) *xchg;
		uns flag_node, flag_xchg;
		uns d = P(find_next_node) (stack+depth, TREE_MAX_DEPTH-depth, 1);

		ASSERT(d >= 2);
		d--;
		xchg = stack[depth+d].buck;
		flag_node = P(red_flag) (node);
		flag_xchg = P(red_flag) (xchg);
		ASSERT(!P(tree_son) (xchg, 0));
		son = P(tree_son) (xchg, 1);
		stack[depth].buck = xchg;	/* Magic iff d == 1.  */
		stack[depth+d].buck = node;
		xchg->son[0] = P(tree_son) (node, 0);
		xchg->son[1] = P(tree_son) (node, 1);
		if (depth > 0)
			P(set_tree_son) (stack[depth-1].buck, stack[depth-1].son, xchg);
		else
			t->root = xchg;
		node->son[0] = NULL;
		node->son[1] = son;
		P(set_tree_son) (stack[depth+d-1].buck, stack[depth+d-1].son, node);
#ifdef TREE_STORE_PARENT
		xchg->parent = depth > 0 ? stack[depth-1].buck : NULL;
		xchg->son[0]->parent = xchg;
		xchg->son[1]->parent = xchg;
		node->parent = stack[depth+d-1].buck;
		if (son)
			son->parent = node;
#endif
		P(set_red_flag) (xchg, flag_node);
		P(set_red_flag) (node, flag_xchg);
		depth += d;
	}
	else if (P(tree_son) (node, 0))
		son = P(tree_son) (node, 0);
	else
		son = P(tree_son) (node, 1);
	/* At this moment, stack[depth].buck == node and it has at most one son
	 * and it is stored in the variable son.  */
	t->count--;
	if (depth > 0)
	{
		P(set_tree_son) (stack[depth-1].buck, stack[depth-1].son, son);
#ifdef TREE_STORE_PARENT
		if (son)
			son->parent = stack[depth-1].buck;
#endif
	}
	else
	{
		t->root = son;
#ifdef TREE_STORE_PARENT
		if (son)
			son->parent = NULL;
#endif
	}
	if (P(red_flag) (node))
	{
		ASSERT(!son);
		return;
	}
	TREE_SAFE_FREE(node);
	/* We have deleted a black node.  */
	if (son)
	{
		ASSERT(P(red_flag) (son));
		P(set_red_flag) (son, 0);
		return;
	}
	P(rotate_after_delete) (t, stack, (int) depth - 1);
}
#endif

#ifdef TREE_WANT_REMOVE
STATIC void P(remove) (T *t, P(node) *Node)
{
	P(stack_entry) stack[TREE_MAX_DEPTH];
	P(bucket) *node = SKIP_BACK(P(bucket), n, Node);
	uns depth = 0, i;
	stack[0].buck = node;
	stack[0].son = 10;
	while (node->parent)
	{
		depth++;
		ASSERT(depth < TREE_MAX_DEPTH);
		stack[depth].buck = node->parent;
		stack[depth].son = P(tree_son) (node->parent, 0) == node ? 0 : 1;
		node = node->parent;
	}
	for (i=0; i<(depth+1)/2; i++)
	{
		P(stack_entry) tmp = stack[i];
		stack[i] = stack[depth-i];
		stack[depth-i] = tmp;
	}
	P(remove_by_stack) (t, stack, depth);
}
#endif

#ifdef TREE_WANT_DELETE
STATIC int P(delete) (T *t, TREE_KEY_DECL)
{
	P(stack_entry) stack[TREE_MAX_DEPTH];
	uns depth;
	depth = P(fill_stack) (stack, TREE_MAX_DEPTH, t->root, TREE_KEY(), 1);
	if (stack[depth].buck)
	{
		P(remove_by_stack) (t, stack, depth);
		return 1;
	}
	else
		return 0;
}
#endif

#ifdef TREE_WANT_DUMP
static void P(dump_subtree) (struct fastbuf *fb, T *t, P(bucket) *node, P(bucket) *parent, int cmp_res, int level, uns black)
{
	uns flag;
	int i;
	if (!node)
	{
		ASSERT(black == t->height);
		return;
	}
	flag = P(red_flag) (node);
#ifdef TREE_STORE_PARENT
	ASSERT(node->parent == parent);
#endif
	if (parent)
	{
		ASSERT(!flag || !P(red_flag) (parent));
		cmp_res *= P(cmp) (TREE_KEY(node->n.), TREE_KEY(parent->n.));
#ifdef TREE_DUPLICATES
		ASSERT(cmp_res >= 0);
#else
		ASSERT(cmp_res > 0);
#endif
	}
	P(dump_subtree) (fb, t, P(tree_son) (node, 0), node, -1, level+1, black + (1-flag));
	if (fb)
	{
		char tmp[20];
		for (i=0; i<level; i++)
			bputs(fb, "  ");
		sprintf(tmp, "L%d %c\t", level, flag ? 'R' : 'B');
		bputs(fb, tmp);
		P(dump_key) (fb, &node->n);
		P(dump_data) (fb, &node->n);
		bputs(fb, "\n");
	}
	P(dump_subtree) (fb, t, P(tree_son) (node, 1), node, +1, level+1, black + (1-flag));
}

STATIC void P(dump) (struct fastbuf *fb, T *t)
{
	if (fb)
	{
		char tmp[50];
		sprintf(tmp, "Tree of %d nodes and height %d\n", t->count, t->height);
		bputs(fb, tmp);
	}
	P(dump_subtree) (fb, t, t->root, NULL, 0, 0, 0);
	if (fb)
	{
		bputs(fb, "\n");
		bflush(fb);
	}
}
#endif

/* And the iterator */

#ifdef TREE_WANT_ITERATOR
static P(node) * P(first_node) (T *t, uns direction)
{
	P(bucket) *node = t->root, *prev = NULL;
	while (node)
	{
		prev = node;
		node = P(tree_son) (node, direction);
	}
	return prev ? &prev->n : NULL;
}

#ifndef TREE_FOR_ALL

#define TREE_FOR_ALL(t_px, t_ptr, t_var)						\
do											\
{											\
	GLUE_(t_px,node) *t_var = GLUE_(t_px,first_node)(t_ptr, 0);			\
	for (; t_var; t_var = GLUE_(t_px,adjacent)(t_var, 1))				\
	{
#define TREE_END_FOR } } while(0)
#define TREE_BREAK break
#define TREE_CONTINUE continue

#endif
#endif

/* Finally, undefine all the parameters */

#undef P
#undef T

#undef TREE_NODE
#undef TREE_PREFIX
#undef TREE_KEY_ATOMIC
#undef TREE_KEY_STRING
#undef TREE_KEY_ENDSTRING
#undef TREE_KEY_COMPLEX
#undef TREE_KEY_DECL
#undef TREE_WANT_CLEANUP
#undef TREE_WANT_FIND
#undef TREE_WANT_FIND_NEXT
#undef TREE_WANT_SEARCH
#undef TREE_WANT_SEARCH_DOWN
#undef TREE_WANT_BOUNDARY
#undef TREE_WANT_ADJACENT
#undef TREE_WANT_NEW
#undef TREE_WANT_LOOKUP
#undef TREE_WANT_DELETE
#undef TREE_WANT_REMOVE
#undef TREE_WANT_DUMP
#undef TREE_WANT_ITERATOR
#undef TREE_GIVE_CMP
#undef TREE_GIVE_EXTRA_SIZE
#undef TREE_GIVE_INIT_KEY
#undef TREE_GIVE_INIT_DATA
#undef TREE_GIVE_ALLOC
#undef TREE_NOCASE
#undef TREE_ATOMIC_TYPE
#undef TREE_USE_POOL
#undef TREE_STATIC
#undef TREE_CONSERVE_SPACE
#undef TREE_DUPLICATES
#undef TREE_MAX_DEPTH
#undef TREE_STORE_PARENT
#undef TREE_KEY
#undef TREE_EXTRA_SIZE
#undef TREE_SAFE_FREE
#undef TREE_TRACE
#undef STATIC
