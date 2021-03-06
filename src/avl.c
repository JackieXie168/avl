/*****************************************************************************

	avl.c - Source code for libavl

	Copyright (c) 1998  Michael H. Buselli <cosine@cosine.org>
	Copyright (c) 2000-2009  Wessel Dankers <wsl@fruit.je>

	This file is part of libavl.

	libavl is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	libavl is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU General Public License
	and a copy of the GNU Lesser General Public License along with
	libavl.  If not, see <http://www.gnu.org/licenses/>.

	Augmented AVL-tree. Original by Michael H. Buselli <cosine@cosine.org>.

	Modified by Wessel Dankers <wsl@fruit.je> to add a bunch of bloat
	to the sourcecode, change the interface and replace a few bugs.
	Mail him if you find any new bugs.

*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define AVL_INLINE
#define AVL_NO_INLINE

#include "avl.h"

static void avl_rebalance(avl_tree_t *, avl_node_t *);

#ifdef AVL_COUNT
#define NODE_COUNT(n)  ((n) ? (n)->count : 0)
#define L_COUNT(n)     (NODE_COUNT((n)->left))
#define R_COUNT(n)     (NODE_COUNT((n)->right))
#define CALC_COUNT(n)  (L_COUNT(n) + R_COUNT(n) + 1)
#endif

#ifdef AVL_DEPTH
#define NODE_DEPTH(n)  ((n) ? (n)->depth : 0)
#define L_DEPTH(n)     (NODE_DEPTH((n)->left))
#define R_DEPTH(n)     (NODE_DEPTH((n)->right))
#define CALC_DEPTH(n)  ((unsigned char)((L_DEPTH(n) > R_DEPTH(n) ? L_DEPTH(n) : R_DEPTH(n)) + 1))
#endif

const avl_node_t avl_node_0 = {0};
const avl_tree_t avl_tree_0 = {0};
const avl_allocator_t avl_allocator_0 = {0};

#ifdef AVL_CAST_QUAL_KLUDGES
static inline avl_node_t *avl_const_node(const avl_node_t *node) {
	union {
		const avl_node_t *c;
		avl_node_t *v;
	} u;
	u.c = node;
	return u.v;
}

static inline void *avl_const_item(const void *item) {
	union {
		const void *c;
		void *v;
	} u;
	u.c = item;
	return u.v;
}
#else
#define avl_const_node(x) ((avl_node_t *)(x))
#define avl_const_item(x) ((void *)(x))
#endif

#ifdef __GNUC__
#define unused __attribute__((unused))
#else
#define unused
#endif

static int avl_check_balance(avl_node_t *avlnode) {
#ifdef AVL_DEPTH
	int d;
	d = R_DEPTH(avlnode) - L_DEPTH(avlnode);
	return d < -1 ? -1 : d > 1;
#else
/*	int d;
 *	d = ffs(R_COUNT(avlnode)) - ffs(L_COUNT(avlnode));
 *	d = d < -1 ? -1 : d > 1;
 */
#ifdef AVL_COUNT
	int pl, r;

	pl = ffs(L_COUNT(avlnode));
	r = R_COUNT(avlnode);

	if(r >> pl + 1)
		return 1;
	if(pl < 2 || r >> pl - 2)
		return 0;
	return -1;
#else
#error No balancing possible.
#endif
#endif
}

#ifdef AVL_COUNT
unsigned long avl_count(const avl_tree_t *avltree) {
	if(!avltree)
		return 0;
	return NODE_COUNT(avltree->top);
}

avl_node_t *avl_at(const avl_tree_t *avltree, unsigned long index) {
	avl_node_t *avlnode;
	unsigned long c;

	if(!avltree)
		return NULL;

	avlnode = avltree->top;

	while(avlnode) {
		c = L_COUNT(avlnode);

		if(index < c) {
			avlnode = avlnode->left;
		} else if(index > c) {
			avlnode = avlnode->right;
			index -= c+1;
		} else {
			return avlnode;
		}
	}
	return NULL;
}

unsigned long avl_index(const avl_node_t *avlnode) {
	avl_node_t *next;
	unsigned long c;

	if(!avlnode)
		return 0;

	c = L_COUNT(avlnode);

	while((next = avlnode->parent)) {
		if(avlnode == next->right)
			c += L_COUNT(next) + 1;
		avlnode = next;
	}

	return c;
}
#endif

static const avl_node_t *avl_search_leftmost_equal(const avl_tree_t *tree, const avl_node_t *node, const void *item) {
	avl_cmp_t cmp = tree->cmp;
	void *userdata = tree->userdata;
	const avl_node_t *r = node;

	for(;;) {
		for(;;) {
			node = node->left;
			if(!node)
				return r;
			if(cmp(item, node->item, userdata))
				break;
			r = node;
		}
		for(;;) {
			node = node->right;
			if(!node)
				return r;
			if(!cmp(item, node->item, userdata))
				break;
		}
		r = node;
	}
}

static const avl_node_t *avl_search_rightmost_equal(const avl_tree_t *tree, const avl_node_t *node, const void *item) {
	avl_cmp_t cmp = tree->cmp;
	void *userdata = tree->userdata;
	const avl_node_t *r = node;

	for(;;) {
		for(;;) {
			node = node->right;
			if(!node)
				return r;
			if(cmp(item, node->item, userdata))
				break;
			r = node;
		}
		for(;;) {
			node = node->left;
			if(!node)
				return r;
			if(!cmp(item, node->item, userdata))
				break;
		}
		r = node;
	}
}

/* Searches for an item, returning either some exact
 * match, or (if no exact match could be found) the first (leftmost)
 * of the nodes that have an item larger than the search item.
 * If exact is not NULL, *exact will be set to:
 *    0  if the returned node is inequal or NULL
 *    1  if the returned node is equal
 * Returns NULL if no equal or larger element could be found.
 * O(lg n) */
static avl_node_t *avl_search_leftish(const avl_tree_t *tree, const void *item, int *exact) {
	avl_node_t *node;
	avl_cmp_t cmp;
	void *userdata;
	int c;

	if(!exact)
		exact = &c;

	if(!tree)
		return *exact = 0, (avl_node_t *)NULL;

	node = tree->top;
	if(!node)
		return *exact = 0, (avl_node_t *)NULL;

	cmp = tree->cmp;
	userdata = tree->userdata;

	for(;;) {
		c = cmp(item, node->item, userdata);

		if(c < 0) {
			if(node->left)
				node = node->left;
			else
				return *exact = 0, node;
		} else if(c > 0) {
			if(node->right)
				node = node->right;
			else
				return *exact = 0, node->next;
		} else {
			return *exact = 1, node;
		}
	}
}

/* Searches for an item, returning either some exact
 * match, or (if no exact match could be found) the last (rightmost)
 * of the nodes that have an item smaller than the search item.
 * If exact is not NULL, *exact will be set to:
 *    0  if the returned node is inequal or NULL
 *    1  if the returned node is equal
 * Returns NULL if no equal or smaller element could be found.
 * O(lg n) */
static avl_node_t *avl_search_rightish(const avl_tree_t *tree, const void *item, int *exact) {
	avl_node_t *node;
	avl_cmp_t cmp;
	void *userdata;
	int c;

	if(!exact)
		exact = &c;

	if(!tree)
		return *exact = 0, (avl_node_t *)NULL;

	node = tree->top;
	if(!node)
		return *exact = 0, (avl_node_t *)NULL;

	cmp = tree->cmp;
	userdata = tree->userdata;

	for(;;) {
		c = cmp(item, node->item, userdata);

		if(c < 0) {
			if(node->left)
				node = node->left;
			else
				return *exact = 0, node->prev;
		} else if(c > 0) {
			if(node->right)
				node = node->right;
			else
				return *exact = 0, node;
		} else {
			return *exact = 1, node;
		}
	}
}

avl_node_t *avl_search_left(const avl_tree_t *tree, const void *item, int *exact) {
	avl_node_t *node;
	int c;

	if(!exact)
		exact = &c;

	if(!tree)
		return *exact = 0, (avl_node_t *)NULL;

	node = avl_search_leftish(tree, item, exact);
	if(*exact)
		return avl_const_node(avl_search_leftmost_equal(tree, node, item));

	return avl_const_node(node);
}

avl_node_t *avl_search_right(const avl_tree_t *tree, const void *item, int *exact) {
	const avl_node_t *node;
	int c;

	if(!exact)
		exact = &c;

	node = avl_search_rightish(tree, item, exact);
	if(*exact)
		return avl_const_node(avl_search_rightmost_equal(tree, node, item));

	return avl_const_node(node);
}

/* Searches for a node with the key closest (or equal) to the given item.
 * If avlnode is not NULL, *avlnode will be set to the node found or NULL
 * if the tree is empty. Return values:
 *   -1  if the returned node is smaller
 *    0  if the returned node is equal or if the tree is empty
 *    1  if the returned node is greater
 * O(lg n) */
int avl_search_closest_FIXME(const avl_tree_t *tree, const void *item, avl_node_t **avlnode) {
	avl_node_t *node;
	int e;
	if(!tree || !tree->top) {
		if(avlnode)
			*avlnode = NULL;
		return 0;
	}
	node = avl_search_leftish(tree, item, &e);
	if(node) {
		e = !e;
	} else {
		node = tree->tail;
		if(node)
			e = -1;
	}
	if(avlnode)
		*avlnode = node;
	return e;
}

avl_node_t *avl_search(const avl_tree_t *avltree, const void *item) {
	int c;
	avl_node_t *n;
	n = avl_search_rightish(avltree, item, &c);
	return c ? n : NULL;
}

avl_tree_t *avl_tree_init(avl_tree_t *avltree, avl_cmp_t cmp, avl_free_t free) {
	if(avltree) {
		avltree->head = NULL;
		avltree->tail = NULL;
		avltree->top = NULL;
		avltree->cmp = cmp;
		avltree->free = free;
	}
	return avltree;
}

avl_tree_t *avl_tree_malloc(avl_cmp_t cmp, avl_free_t free) {
	return avl_tree_init(malloc(sizeof(avl_tree_t)), cmp, free);
}

avl_tree_t *avl_tree_clear(avl_tree_t *avltree) {
	if(avltree)
		avltree->top = avltree->head = avltree->tail = NULL;
	return avltree;
}

static void avl_node_free(avl_tree_t *avltree, avl_node_t *node) {
	avl_allocator_t *allocator;
	avl_deallocate_t deallocate;

	allocator = avltree->allocator;
	if(allocator) {
		deallocate = allocator->deallocate;
		if(deallocate)
			deallocate(allocator, node);
	} else {
		free(node);
	}
}

avl_tree_t *avl_tree_purge(avl_tree_t *avltree) {
	avl_node_t *node, *next;
	avl_free_t func;
	avl_allocator_t *allocator;
	avl_deallocate_t deallocate;
	void *userdata;

	if(!avltree)
		return NULL;

	userdata = avltree->userdata;

	func = avltree->free;
	allocator = avltree->allocator;
	deallocate = allocator
		? allocator->deallocate
		: (avl_deallocate_t)NULL;

	for(node = avltree->head; node; node = next) {
		next = node->next;
		if(func)
			func(node->item, userdata);
		if(allocator) {
			if(deallocate)
				deallocate(allocator, node);
		} else {
			free(node);
		}
	}

	return avl_tree_clear(avltree);
}

void avl_tree_free(avl_tree_t *avltree) {
	if(!avltree)
		return;
	(void)avl_tree_purge(avltree);
	free(avltree);
}

static void avl_node_clear(avl_node_t *newnode) {
	newnode->left = newnode->right = NULL;
#	ifdef AVL_COUNT
	newnode->count = 1;
#	endif
#	ifdef AVL_DEPTH
	newnode->depth = 1;
#	endif
}

avl_node_t *avl_node_init(avl_node_t *newnode, const void *item) {
	if(newnode)
		newnode->item = avl_const_item(item);
	return newnode;
}

avl_node_t *avl_alloc(avl_tree_t *avltree, const void *item) {
	avl_node_t *newnode;
	avl_allocator_t *allocator = avltree
		? avltree->allocator
		: (avl_allocator_t *)NULL;
	avl_allocate_t allocate;
	if(allocator) {
		allocate = allocator->allocate;
		if(allocator) {
			newnode = allocate(allocator);
		} else {
			errno = ENOSYS;
			newnode = NULL;
		}
	} else {
		newnode = malloc(sizeof *newnode);
	}
	return avl_node_init(newnode, item);
}

/* For backwards compatibility. */
avl_node_t *avl_node_malloc_FIXME(const void *item) {
	return avl_alloc(NULL, item);
}

/* Insert a node in an empty tree. If avlnode is NULL, the tree will be
 * cleared and ready for re-use.
 * If the tree is not empty, the old nodes are left dangling.
 * O(1) */
static avl_node_t *avl_insert_top(avl_tree_t *avltree, avl_node_t *newnode) {
	avl_node_clear(newnode);
	newnode->prev = newnode->next = newnode->parent = NULL;
	avltree->head = avltree->tail = avltree->top = newnode;
	return newnode;
}

avl_node_t *avl_insert_top_FIXME(avl_tree_t *avltree, avl_node_t *newnode) {
	return avl_insert_top(avltree, newnode);
}

avl_node_t *avl_insert_before(avl_tree_t *avltree, avl_node_t *node, avl_node_t *newnode) {
	if(!avltree || !newnode)
		return NULL;

	if(!node)
		return avltree->tail
			? avl_insert_after(avltree, avltree->tail, newnode)
			: avl_insert_top(avltree, newnode);

	if(node->left)
		return avl_insert_after(avltree, node->prev, newnode);

	avl_node_clear(newnode);

	newnode->next = node;
	newnode->parent = node;

	newnode->prev = node->prev;
	if(node->prev)
		node->prev->next = newnode;
	else
		avltree->head = newnode;
	node->prev = newnode;

	node->left = newnode;
	avl_rebalance(avltree, node);
	return newnode;
}

avl_node_t *avl_insert_after(avl_tree_t *avltree, avl_node_t *node, avl_node_t *newnode) {
	if(!avltree || !newnode)
		return NULL;

	if(!node)
		return avltree->head
			? avl_insert_before(avltree, avltree->head, newnode)
			: avl_insert_top(avltree, newnode);

	if(node->right)
		return avl_insert_before(avltree, node->next, newnode);

	avl_node_clear(newnode);

	newnode->prev = node;
	newnode->parent = node;

	newnode->next = node->next;
	if(node->next)
		node->next->prev = newnode;
	else
		avltree->tail = newnode;
	node->next = newnode;

	node->right = newnode;
	avl_rebalance(avltree, node);
	return newnode;
}

avl_node_t *avl_insert(avl_tree_t *avltree, avl_node_t *newnode) {
	avl_node_t *node;
	int c;

	node = avl_search_rightish(avltree, newnode->item, &c);
	return c ? NULL : avl_insert_after(avltree, node, newnode);
}

avl_node_t *avl_insert_left(avl_tree_t *avltree, avl_node_t *newnode) {
	return avl_insert_before(avltree, avl_search_left(avltree, newnode->item, NULL), newnode);
}

avl_node_t *avl_insert_right(avl_tree_t *avltree, avl_node_t *newnode) {
	return avl_insert_after(avltree, avl_search_right(avltree, newnode->item, NULL), newnode);
}

avl_node_t *avl_insert_somewhere(avl_tree_t *avltree, avl_node_t *newnode) {
	return avl_insert_after(avltree, avl_search_rightish(avltree, newnode->item, NULL), newnode);
}

avl_node_t *avl_item_insert(avl_tree_t *avltree, const void *item) {
	avl_node_t *newnode;

	if(!avltree)
		return errno = EFAULT, (avl_node_t *)NULL;

	newnode = avl_alloc(avltree, item);
	if(newnode) {
		if(avl_insert(avltree, newnode))
			return newnode;
		avl_node_free(avltree, newnode);
		errno = EEXIST;
	}
	return NULL;
}

avl_node_t *avl_item_insert_somewhere(avl_tree_t *avltree, const void *item) {
	avl_node_t *newnode;

	if(!avltree)
		return errno = EFAULT, (avl_node_t *)NULL;

	newnode = avl_alloc(avltree, item);
	if(newnode)
		return avl_insert_somewhere(avltree, newnode);
	return NULL;
}

avl_node_t *avl_item_insert_before(avl_tree_t *avltree, avl_node_t *node, const void *item) {
	avl_node_t *newnode;

	if(!avltree)
		return errno = EFAULT, (avl_node_t *)NULL;

	newnode = avl_alloc(avltree, item);
	if(newnode)
		return avl_insert_before(avltree, node, newnode);
	return NULL;
}

avl_node_t *avl_item_insert_after(avl_tree_t *avltree, avl_node_t *node, const void *item) {
	avl_node_t *newnode;

	if(!avltree)
		return errno = EFAULT, (avl_node_t *)NULL;

	newnode = avl_alloc(avltree, item);
	if(newnode)
		return avl_insert_after(avltree, node, newnode);
	return NULL;
}

avl_node_t *avl_item_insert_left(avl_tree_t *avltree, const void *item) {
	avl_node_t *newnode;

	if(!avltree)
		return errno = EFAULT, (avl_node_t *)NULL;

	newnode = avl_alloc(avltree, item);
	if(newnode)
		return avl_insert_left(avltree, newnode);
	return NULL;
}

avl_node_t *avl_item_insert_right(avl_tree_t *avltree, const void *item) {
	avl_node_t *newnode;

	if(!avltree)
		return errno = EFAULT, (avl_node_t *)NULL;

	newnode = avl_alloc(avltree, item);
	if(newnode)
		return avl_insert_right(avltree, newnode);
	return NULL;
}

avl_node_t *avl_unlink(avl_tree_t *avltree, avl_node_t *avlnode) {
	avl_node_t *parent;
	avl_node_t **superparent;
	avl_node_t *subst, *left, *right;
	avl_node_t *balnode;

	if(!avltree || !avlnode)
		return NULL;

	if(avlnode->prev)
		avlnode->prev->next = avlnode->next;
	else
		avltree->head = avlnode->next;

	if(avlnode->next)
		avlnode->next->prev = avlnode->prev;
	else
		avltree->tail = avlnode->prev;

	parent = avlnode->parent;

	superparent = parent
		? avlnode == parent->left ? &parent->left : &parent->right
		: &avltree->top;

	left = avlnode->left;
	right = avlnode->right;
	if(!left) {
		*superparent = right;
		if(right)
			right->parent = parent;
		balnode = parent;
	} else if(!right) {
		*superparent = left;
		left->parent = parent;
		balnode = parent;
	} else {
		subst = avlnode->prev;
		if(subst == left) {
			balnode = subst;
		} else {
			balnode = subst->parent;
			balnode->right = subst->left;
			if(balnode->right)
				balnode->right->parent = balnode;
			subst->left = left;
			left->parent = subst;
		}
		subst->right = right;
		subst->parent = parent;
		right->parent = subst;
		*superparent = subst;
	}

	avl_rebalance(avltree, balnode);

	return avlnode;
}

void *avl_delete(avl_tree_t *avltree, avl_node_t *avlnode) {
	void *item = NULL;
	if(avlnode) {
		item = avlnode->item;
		(void)avl_unlink(avltree, avlnode);
		if(avltree->free)
			avltree->free(item, avltree->userdata);
		avl_node_free(avltree, avlnode);
	}
	return item;
}

void *avl_item_delete(avl_tree_t *avltree, const void *item) {
	return avl_delete(avltree, avl_search(avltree, item));
}

avl_node_t *avl_fixup(avl_tree_t *avltree, avl_node_t *newnode) {
	avl_node_t *oldnode = NULL, *node;

	if(!avltree || !newnode)
		return NULL;

	node = newnode->prev;
	if(node) {
		oldnode = node->next;
		node->next = newnode;
	} else {
		avltree->head = newnode;
	}

	node = newnode->next;
	if(node) {
		oldnode = node->prev;
		node->prev = newnode;
	} else {
		avltree->tail = newnode;
	}

	node = newnode->parent;
	if(node) {
		if(node->left == oldnode)
			node->left = newnode;
		else
			node->right = newnode;
	} else {
		oldnode = avltree->top;
		avltree->top = newnode;
	}

	return oldnode;
}

/*
 * avl_rebalance:
 * Rebalances the tree if one side becomes too heavy.  This function
 * assumes that both subtrees are AVL-trees with consistant data.  The
 * function has the additional side effect of recalculating the count of
 * the tree at this node.  It should be noted that at the return of this
 * function, if a rebalance takes place, the top of this subtree is no
 * longer going to be the same node.
 */
static void avl_rebalance(avl_tree_t *avltree, avl_node_t *avlnode) {
	avl_node_t *child;
	avl_node_t *gchild;
	avl_node_t *parent;
	avl_node_t **superparent;

	parent = avlnode;

	while(avlnode) {
		parent = avlnode->parent;

		superparent = parent
			? avlnode == parent->left ? &parent->left : &parent->right
			: &avltree->top;

		switch(avl_check_balance(avlnode)) {
		case -1:
			child = avlnode->left;
#			ifdef AVL_DEPTH
			if(L_DEPTH(child) >= R_DEPTH(child)) {
#			else
#			ifdef AVL_COUNT
			if(L_COUNT(child) >= R_COUNT(child)) {
#			else
#			error No balancing possible.
#			endif
#			endif
				avlnode->left = child->right;
				if(avlnode->left)
					avlnode->left->parent = avlnode;
				child->right = avlnode;
				avlnode->parent = child;
				*superparent = child;
				child->parent = parent;
#				ifdef AVL_COUNT
				avlnode->count = CALC_COUNT(avlnode);
				child->count = CALC_COUNT(child);
#				endif
#				ifdef AVL_DEPTH
				avlnode->depth = CALC_DEPTH(avlnode);
				child->depth = CALC_DEPTH(child);
#				endif
			} else {
				gchild = child->right;
				avlnode->left = gchild->right;
				if(avlnode->left)
					avlnode->left->parent = avlnode;
				child->right = gchild->left;
				if(child->right)
					child->right->parent = child;
				gchild->right = avlnode;
				if(gchild->right)
					gchild->right->parent = gchild;
				gchild->left = child;
				if(gchild->left)
					gchild->left->parent = gchild;
				*superparent = gchild;
				gchild->parent = parent;
#				ifdef AVL_COUNT
				avlnode->count = CALC_COUNT(avlnode);
				child->count = CALC_COUNT(child);
				gchild->count = CALC_COUNT(gchild);
#				endif
#				ifdef AVL_DEPTH
				avlnode->depth = CALC_DEPTH(avlnode);
				child->depth = CALC_DEPTH(child);
				gchild->depth = CALC_DEPTH(gchild);
#				endif
			}
		break;
		case 1:
			child = avlnode->right;
#			ifdef AVL_DEPTH
			if(R_DEPTH(child) >= L_DEPTH(child)) {
#			else
#			ifdef AVL_COUNT
			if(R_COUNT(child) >= L_COUNT(child)) {
#			else
#			error No balancing possible.
#			endif
#			endif
				avlnode->right = child->left;
				if(avlnode->right)
					avlnode->right->parent = avlnode;
				child->left = avlnode;
				avlnode->parent = child;
				*superparent = child;
				child->parent = parent;
#				ifdef AVL_COUNT
				avlnode->count = CALC_COUNT(avlnode);
				child->count = CALC_COUNT(child);
#				endif
#				ifdef AVL_DEPTH
				avlnode->depth = CALC_DEPTH(avlnode);
				child->depth = CALC_DEPTH(child);
#				endif
			} else {
				gchild = child->left;
				avlnode->right = gchild->left;
				if(avlnode->right)
					avlnode->right->parent = avlnode;
				child->left = gchild->right;
				if(child->left)
					child->left->parent = child;
				gchild->left = avlnode;
				if(gchild->left)
					gchild->left->parent = gchild;
				gchild->right = child;
				if(gchild->right)
					gchild->right->parent = gchild;
				*superparent = gchild;
				gchild->parent = parent;
#				ifdef AVL_COUNT
				avlnode->count = CALC_COUNT(avlnode);
				child->count = CALC_COUNT(child);
				gchild->count = CALC_COUNT(gchild);
#				endif
#				ifdef AVL_DEPTH
				avlnode->depth = CALC_DEPTH(avlnode);
				child->depth = CALC_DEPTH(child);
				gchild->depth = CALC_DEPTH(gchild);
#				endif
			}
		break;
		default:
#			ifdef AVL_COUNT
			avlnode->count = CALC_COUNT(avlnode);
#			endif
#			ifdef AVL_DEPTH
			avlnode->depth = CALC_DEPTH(avlnode);
#			endif
		}
		avlnode = parent;
	}
}

#define AVL_CMP_DEFINE_NAMED(n, t) \
	__extension__ \
	__attribute__((pure)) \
	int avl_##n##_cmp(const void *a, const void *b, void *userdata) { return AVL_CMP(*(const t *)a, *(const t *)b); }

#define AVL_CMP_DEFINE_T(t) AVL_CMP_DEFINE_NAMED(t, t##_t)
#define AVL_CMP_DEFINE(t) AVL_CMP_DEFINE_NAMED(t, t)

AVL_CMP_DEFINE(float)
AVL_CMP_DEFINE(double)

AVL_CMP_DEFINE(char)
AVL_CMP_DEFINE(short)
AVL_CMP_DEFINE(int)
AVL_CMP_DEFINE(long)

AVL_CMP_DEFINE_NAMED(unsigned_char, unsigned char)
AVL_CMP_DEFINE_NAMED(unsigned_short, unsigned short)
AVL_CMP_DEFINE_NAMED(unsigned_int, unsigned int)
AVL_CMP_DEFINE_NAMED(unsigned_long, unsigned long)

AVL_CMP_DEFINE_T(size)
AVL_CMP_DEFINE_T(ssize)

AVL_CMP_DEFINE_NAMED(pointer, const void *)

#ifdef __GNUC__
AVL_CMP_DEFINE_NAMED(long_long, long long)
AVL_CMP_DEFINE_NAMED(unsigned_long_long, unsigned long long)
AVL_CMP_DEFINE_NAMED(long_double, long double)
#endif

#if AVL_HAVE_C99
AVL_CMP_DEFINE_T(int8)
AVL_CMP_DEFINE_T(uint8)
AVL_CMP_DEFINE_T(int16)
AVL_CMP_DEFINE_T(uint16)
AVL_CMP_DEFINE_T(int32)
AVL_CMP_DEFINE_T(uint32)
AVL_CMP_DEFINE_T(int64)
AVL_CMP_DEFINE_T(uint64)

AVL_CMP_DEFINE_T(int_fast8)
AVL_CMP_DEFINE_T(uint_fast8)
AVL_CMP_DEFINE_T(int_fast16)
AVL_CMP_DEFINE_T(uint_fast16)
AVL_CMP_DEFINE_T(int_fast32)
AVL_CMP_DEFINE_T(uint_fast32)
AVL_CMP_DEFINE_T(int_fast64)
AVL_CMP_DEFINE_T(uint_fast64)

AVL_CMP_DEFINE_T(int_least8)
AVL_CMP_DEFINE_T(uint_least8)
AVL_CMP_DEFINE_T(int_least16)
AVL_CMP_DEFINE_T(uint_least16)
AVL_CMP_DEFINE_T(int_least32)
AVL_CMP_DEFINE_T(uint_least32)
AVL_CMP_DEFINE_T(int_least64)
AVL_CMP_DEFINE_T(uint_least64)
#endif

#if AVL_HAVE_POSIX
#include <sys/time.h>
#include <sys/socket.h>

AVL_CMP_DEFINE_T(time)
AVL_CMP_DEFINE_T(off)
AVL_CMP_DEFINE_T(socklen)

__attribute__((pure))
int avl_timeval_cmp(const void *a, const void *b, void *userdata) {
	int r = AVL_CMP(((struct timeval *)a)->tv_sec, ((struct timeval *)b)->tv_sec);
	if(r)
		return r;
	return AVL_CMP(((struct timeval *)a)->tv_usec, ((struct timeval *)b)->tv_usec);
}

__attribute__((pure))
int avl_timespec_cmp(const void *a, const void *b, void *userdata) {
	int r = AVL_CMP(((struct timespec *)a)->tv_sec, ((struct timespec *)b)->tv_sec);
	if(r)
		return r;
	return AVL_CMP(((struct timespec *)a)->tv_nsec, ((struct timespec *)b)->tv_nsec);
}

__attribute__((pure))
int avl_strcmp(const void *a, const void *b, void *userdata) {
	return strcmp(a, b);
}

__attribute__((pure))
int avl_strcasecmp(const void *a, const void *b, void *userdata) {
	return strcasecmp(a, b);
}
#endif
