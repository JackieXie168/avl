/*****************************************************************************

    avl.c - Source code for the AVL-tree library.

    Copyright (C) 1998  Michael H. Buselli <cosine@cosine.org>
    Copyright (C) 2000-2005  Wessel Dankers <wsl@nl.linux.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

    Augmented AVL-tree. Original by Michael H. Buselli <cosine@cosine.org>.

    Modified by Wessel Dankers <wsl@nl.linux.org> to add a bunch of bloat to
    the sourcecode, change the interface and squash a few bugs.
    Mail him if you find new bugs.

*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

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
#define CALC_DEPTH(n)  ((L_DEPTH(n)>R_DEPTH(n)?L_DEPTH(n):R_DEPTH(n)) + 1)
#endif

avl_node_t avl_node_0 = {0};
avl_tree_t avl_tree_0 = {0};

static int avl_check_balance(avl_node_t *avlnode) {
#ifdef AVL_DEPTH
	int d;
	d = R_DEPTH(avlnode) - L_DEPTH(avlnode);
	return d<-1?-1:d>1?1:0;
#else
/*	int d;
 *	d = ffs(R_COUNT(avlnode)) - ffs(L_COUNT(avlnode));
 *	d = d<-1?-1:d>1?1:0;
 */
#ifdef AVL_COUNT
	int pl, r;

	pl = ffs(L_COUNT(avlnode));
	r = R_COUNT(avlnode);

	if(r>>pl+1)
		return 1;
	if(pl<2 || r>>pl-2)
		return 0;
	return -1;
#else
#error No balancing possible.
#endif
#endif
}

#ifdef AVL_COUNT
unsigned int avl_count(const avl_tree_t *avltree) {
	return NODE_COUNT(avltree->top);
}

avl_node_t *avl_at(const avl_tree_t *avltree, unsigned int index) {
	avl_node_t *avlnode;
	unsigned int c;

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

unsigned int avl_index(const avl_node_t *avlnode) {
	avl_node_t *next;
	unsigned int c;

	c = L_COUNT(avlnode);

	while((next = avlnode->parent)) {
		if(avlnode == next->right)
			c += L_COUNT(next) + 1;
		avlnode = next;
	}

	return c;
}
#endif

static avl_node_t *avl_search_leftmost_equal(avl_node_t *node, const void *item, avl_compare_t cmp) {
	avl_node_t *r = node;

	for(;;) {
		for(;;) {
			node = node->left;
			if(!node)
				return r;
			if(cmp(item, node->item))
				break;
			r = node;
		}
		for(;;) {
			node = node->right;
			if(!node)
				return r;
			if(!cmp(item, node->item))
				break;
		}
		r = node;
	}
}

static avl_node_t *avl_search_rightmost_equal(avl_node_t *node, const void *item, avl_compare_t cmp) {
	avl_node_t *r = node;

	for(;;) {
		for(;;) {
			node = node->right;
			if(!node)
				return r;
			if(cmp(item, node->item))
				break;
			r = node;
		}
		for(;;) {
			node = node->left;
			if(!node)
				return r;
			if(!cmp(item, node->item))
				break;
		}
		r = node;
	}
}

avl_node_t *avl_search_left(const avl_tree_t *avltree, const void *item, int *exact) {
	avl_node_t *node;
	avl_compare_t cmp;
	int c;

	if(!exact)
		exact = &c;

	node = avltree->top;

	if(!node)
		return *exact = 0, NULL;

	cmp = avltree->cmp;

	for(;;) {
		c = cmp(item, node->item);

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
			return *exact = 1, avl_search_leftmost_equal(node, item, cmp);
		}
	}
}

avl_node_t *avl_search_right(const avl_tree_t *avltree, const void *item, int *exact) {
	avl_node_t *node;
	avl_compare_t cmp;
	int c;

	if(!exact)
		exact = &c;

	node = avltree->top;

	if(!node)
		return *exact = 0, NULL;

	cmp = avltree->cmp;

	for(;;) {
		c = cmp(item, node->item);

		if(c < 0) {
			if(node->left)
				node = node->left;
			else
				return *exact = 0, node;
		} else if(c > 0) {
			if(node->right)
				node = node->right;
			else
				return *exact = 0, node;
		} else {
			return *exact = 1, avl_search_rightmost_equal(node, item, cmp);
		}
	}
}


avl_node_t *avl_search_rightish(const avl_tree_t *avltree, const void *item, int *exact) {
	avl_node_t *node;
	avl_compare_t cmp;
	int c;

	if(!exact)
		exact = &c;

	node = avltree->top;

	if(!node)
		return *exact = 0, NULL;

	cmp = avltree->cmp;

	for(;;) {
		c = cmp(item, node->item);

		if(c < 0) {
			if(node->left)
				node = node->left;
			else
				return *exact = 0, node;
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


avl_node_t *avl_search(const avl_tree_t *avltree, const void *item) {
	int c;
	avl_node_t *n;
	n = avl_search_rightish(avltree, item, &c);
	return c ? n : NULL;
}

avl_tree_t *avl_tree_init(avl_tree_t *rc, avl_compare_t cmp, avl_freeitem_t freeitem) {
	if(rc) {
		rc->head = NULL;
		rc->tail = NULL;
		rc->top = NULL;
		rc->cmp = cmp;
		rc->freeitem = freeitem;
	}
	return rc;
}

avl_tree_t *avl_tree_alloc(avl_compare_t cmp, avl_freeitem_t freeitem) {
	return avl_tree_init(malloc(sizeof(avl_tree_t)), cmp, freeitem);
}

void avl_tree_clear(avl_tree_t *avltree) {
	avltree->top = avltree->head = avltree->tail = NULL;
}

void avl_tree_purge(avl_tree_t *avltree) {
	avl_node_t *node, *next;
	avl_freeitem_t freeitem;

	freeitem = avltree->freeitem;

	for(node = avltree->head; node; node = next) {
		next = node->next;
		if(freeitem)
			freeitem(node->item);
		free(node);
	}

	avl_tree_clear(avltree);
}

void avl_tree_free(avl_tree_t *avltree) {
	avl_tree_purge(avltree);
	free(avltree);
}

static void avl_node_clear(avl_node_t *newnode) {
	newnode->left = newnode->right = NULL;
	#ifdef AVL_COUNT
	newnode->count = 1;
	#endif
	#ifdef AVL_DEPTH
	newnode->depth = 1;
	#endif
}

avl_node_t *avl_node_init(avl_node_t *newnode, void *item) {
	if(newnode)
		newnode->item = item;
	return newnode;
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

avl_node_t *avl_insert_before(avl_tree_t *avltree, avl_node_t *node, avl_node_t *newnode) {
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

avl_node_t *avl_item_insert(avl_tree_t *avltree, void *item) {
	avl_node_t *newnode;

	newnode = avl_node_init(malloc(sizeof(avl_node_t)), item);
	if(newnode) {
		if(avl_insert(avltree, newnode))
			return newnode;
		free(newnode);
		errno = EEXIST;
	}
	return NULL;
}

avl_node_t *avl_item_insert_somewhere(avl_tree_t *avltree, void *item) {
	avl_node_t *newnode;

	newnode = avl_node_init(malloc(sizeof(avl_node_t)), item);
	if(newnode)
		return avl_insert_somewhere(avltree, newnode);
	return NULL;
}

avl_node_t *avl_item_insert_before(avl_tree_t *avltree, avl_node_t *node, void *item) {
	avl_node_t *newnode;

	newnode = avl_node_init(malloc(sizeof(avl_node_t)), item);
	if(newnode)
		return avl_insert_before(avltree, node, newnode);
	return NULL;
}

avl_node_t *avl_item_insert_after(avl_tree_t *avltree, avl_node_t *node, void *item) {
	avl_node_t *newnode;

	newnode = avl_node_init(malloc(sizeof(avl_node_t)), item);
	if(newnode)
		return avl_insert_after(avltree, node, newnode);
	return NULL;
}

avl_node_t *avl_item_insert_left(avl_tree_t *avltree, void *item) {
	avl_node_t *newnode;

	newnode = avl_node_init(malloc(sizeof(avl_node_t)), item);
	if(newnode)
		return avl_insert_left(avltree, newnode);
	return NULL;
}

avl_node_t *avl_item_insert_right(avl_tree_t *avltree, void *item) {
	avl_node_t *newnode;

	newnode = avl_node_init(malloc(sizeof(avl_node_t)), item);
	if(newnode)
		return avl_insert_right(avltree, newnode);
	return NULL;
}

void avl_unlink(avl_tree_t *avltree, avl_node_t *avlnode) {
	avl_node_t *parent;
	avl_node_t **superparent;
	avl_node_t *subst, *left, *right;
	avl_node_t *balnode;

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
}

void *avl_delete(avl_tree_t *avltree, avl_node_t *avlnode) {
	void *item = NULL;
	if(avlnode) {
		item = avlnode->item;
		avl_unlink(avltree, avlnode);
		if(avltree->freeitem)
			avltree->freeitem(item);
		free(avlnode);
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
			#ifdef AVL_DEPTH
			if(L_DEPTH(child) >= R_DEPTH(child)) {
			#else
			#ifdef AVL_COUNT
			if(L_COUNT(child) >= R_COUNT(child)) {
			#else
			#error No balancing possible.
			#endif
			#endif
				avlnode->left = child->right;
				if(avlnode->left)
					avlnode->left->parent = avlnode;
				child->right = avlnode;
				avlnode->parent = child;
				*superparent = child;
				child->parent = parent;
				#ifdef AVL_COUNT
				avlnode->count = CALC_COUNT(avlnode);
				child->count = CALC_COUNT(child);
				#endif
				#ifdef AVL_DEPTH
				avlnode->depth = CALC_DEPTH(avlnode);
				child->depth = CALC_DEPTH(child);
				#endif
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
				#ifdef AVL_COUNT
				avlnode->count = CALC_COUNT(avlnode);
				child->count = CALC_COUNT(child);
				gchild->count = CALC_COUNT(gchild);
				#endif
				#ifdef AVL_DEPTH
				avlnode->depth = CALC_DEPTH(avlnode);
				child->depth = CALC_DEPTH(child);
				gchild->depth = CALC_DEPTH(gchild);
				#endif
			}
		break;
		case 1:
			child = avlnode->right;
			#ifdef AVL_DEPTH
			if(R_DEPTH(child) >= L_DEPTH(child)) {
			#else
			#ifdef AVL_COUNT
			if(R_COUNT(child) >= L_COUNT(child)) {
			#else
			#error No balancing possible.
			#endif
			#endif
				avlnode->right = child->left;
				if(avlnode->right)
					avlnode->right->parent = avlnode;
				child->left = avlnode;
				avlnode->parent = child;
				*superparent = child;
				child->parent = parent;
				#ifdef AVL_COUNT
				avlnode->count = CALC_COUNT(avlnode);
				child->count = CALC_COUNT(child);
				#endif
				#ifdef AVL_DEPTH
				avlnode->depth = CALC_DEPTH(avlnode);
				child->depth = CALC_DEPTH(child);
				#endif
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
				#ifdef AVL_COUNT
				avlnode->count = CALC_COUNT(avlnode);
				child->count = CALC_COUNT(child);
				gchild->count = CALC_COUNT(gchild);
				#endif
				#ifdef AVL_DEPTH
				avlnode->depth = CALC_DEPTH(avlnode);
				child->depth = CALC_DEPTH(child);
				gchild->depth = CALC_DEPTH(gchild);
				#endif
			}
		break;
		default:
			#ifdef AVL_COUNT
			avlnode->count = CALC_COUNT(avlnode);
			#endif
			#ifdef AVL_DEPTH
			avlnode->depth = CALC_DEPTH(avlnode);
			#endif
		}
		avlnode = parent;
	}
}

#define AVL_CMP_DEFINE(t) int avl_##t##_cmp(const t a, const t b) { return AVL_CMP(a,b); }
#define AVL_CMP_DEFINE_T(t) int avl_##t##_t_cmp(const t##_t a, const t##_t b) { return AVL_CMP(a,b); }
#define AVL_CMP_DEFINE_NAMED(n,t) int avl_##n##_cmp(const t a, const t b) { return AVL_CMP(a,b); }

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

AVL_CMP_DEFINE_NAMED(pointer, void *)

#ifdef __GNUC__
__extension__
AVL_CMP_DEFINE_NAMED(long_long, long long)
__extension__
AVL_CMP_DEFINE_NAMED(unsigned_long_long, unsigned long long)
__extension__
AVL_CMP_DEFINE_NAMED(long_double, long double)
#endif

#ifdef HAVE_C99
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

#ifdef HAVE_POSIX
AVL_CMP_DEFINE_T(time)
AVL_CMP_DEFINE_T(size)
AVL_CMP_DEFINE_T(ssize)
AVL_CMP_DEFINE_T(socklen)

int avl_cmp_timeval(const struct timeval *a, const struct timeval *b) {
	int r;
	r = AVL_CMP(a->tv_sec, b->tv_sec);
	if(!r)
		r = AVL_CMP(a->tv_usec, b->tv_usec);
	return r;
}
#endif
