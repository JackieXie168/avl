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
	avl_tree_t kanten;
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

	avl_tree_init(&t->kanten, (avl_compare_t)avl_pointer_cmp, NULL);
	t->afstand = ~0U;

	return t;
}

static int toestand_cmp(const toestand_t *a, const toestand_t *b) {
	return avl_unsigned_int_cmp(a->totaal, b->totaal);
}

static avl_tree_t toestanden = AVL_TREE_INIT((avl_compare_t)toestand_cmp, NULL);

static void genereer_toestanden(void) {
	int i, j;
	toestand_t *t;

	for(i = 0; i <= totaal; i++) {
		for(j = 0; j <= kannibalen && j <= i; j++) {
			if(j > i - j) /* teveel kannibalen links */
				continue;
			if(kannibalen - j > totaal - kannibalen - i + j) /* idem rechts */
				continue;
			t = toestand_new(i, j, false);
			avl_insert_right(&toestanden, &t->node);
			t = toestand_new(i, j, true);
			avl_insert_right(&toestanden, &t->node);
		}
	}
}

int main(void) {
	genereer_toestanden();
	return 0;
}
