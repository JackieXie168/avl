#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sysexits.h>

#include "avl.h"

typedef struct toestand {
	avl_node_t node;
	unsigned int totaal;
	unsigned int kannibalen;
	bool bootje;
	avl_tree_t overgangen;
	avl_node_t *terug;
	unsigned int afstand;
} toestand_t;

static unsigned int totaal = 4;
static unsigned int kannibalen = 2;

static const toestand_t toestand_0 = {{0}};

static toestand_t *toestand_new(unsigned int i, unsigned int j, bool b) {
	toestand_t *t;

	t = malloc(sizeof *t);
	if(!t) {
		perror("malloc()");
		exit(EX_OSERR);
	}
	*t = toestand_0;

	t->totaal = i;
	t->kannibalen = i;
	t->bootje = b;

	avl_node_init(&t->node, t);
	avl_tree_init(&t->overgangen, (avl_compare_t)avl_pointer_cmp, NULL);
	t->afstand = ~0U;

	return t;
}

static int toestand_cmp(const toestand_t *a, const toestand_t *b) {
	return avl_unsigned_int_cmp(a->totaal, b->totaal);
}

static avl_tree_t toestanden = AVL_TREE_INIT((avl_compare_t)toestand_cmp, NULL);

static void genereer_toestanden(void) {
	int tl, kl, kr, ml, mr;
	toestand_t *t;

	for(tl = 0; tl <= totaal; tl++) {
		for(kl = 0; kl <= kannibalen && kl <= tl; kl++) {
			ml = tl - kl;
			kr = kannibalen - kl;
			mr = totaal - tl - kr;
			
			if(kl > ml || kr > mr)
				continue;

			t = toestand_new(tl, kl, false);
			avl_insert_right(&toestanden, &t->node);
			t = toestand_new(tl, kl, true);
			avl_insert_right(&toestanden, &t->node);
		}
	}
}

int main(void) {
	genereer_toestanden();
	return 0;
}
