#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sysexits.h>

#include "avl.h"

#define ONEINDIG 10000

typedef struct toestand {
	avl_node_t node;
	int totaal;
	int kannibalen;
	bool bootje;
	avl_tree_t overgangen;
	struct toestand *volgende;
	int afstand;
} toestand_t;

static int totaal = 4;
static int kannibalen = 2;

static const toestand_t toestand_0 = {{0}};

static toestand_t *toestand_new(int i, int j, bool b) {
	toestand_t *t;

	t = malloc(sizeof *t);
	if(!t) {
		perror("malloc()");
		exit(EX_OSERR);
	}
	*t = toestand_0;

	t->totaal = i;
	t->kannibalen = j;
	t->bootje = b;

	avl_node_init(&t->node, t);
	avl_tree_init(&t->overgangen, (avl_compare_t)avl_pointer_cmp, NULL);

	return t;
}

static int toestand_cmp(const toestand_t *a, const toestand_t *b) {
	return avl_unsigned_int_cmp(b->totaal, a->totaal);
}

static avl_tree_t toestanden = AVL_TREE_INIT((avl_compare_t)toestand_cmp, NULL);

static void toon_toestand(const toestand_t *t) {
	int i, missionarissen;
	int tl, tr, kl, kr, ml, mr;

	missionarissen = totaal - kannibalen;
	tl = t->totaal;
	tr = totaal - tl;
	kl = t->kannibalen;
	ml = tl - kl;
	kr = kannibalen - kl;
	mr = tr - kr;

	for(i = 0; i < kannibalen; i++)
		putchar(i < kl ? 'K' : '_');
	for(i = 0; i < missionarissen; i++)
		putchar(i < ml ? 'M' : '_');
	putchar(t->bootje ? '<' : '>');
	for(i = 0; i < kannibalen; i++)
		putchar(i < kr ? 'K' : '_');
	for(i = 0; i < missionarissen; i++)
		putchar(i < mr ? 'M' : '_');
}

static void toon_oplossing(const toestand_t *t) {
	for(; t; t = t->volgende) {
		printf(" → ");
		toon_toestand(t);
	}
	printf(" →\n");
}

static void toon_oplossingen(void) {
	avl_node_t *node;
	toestand_t *t;
	for(node = toestanden.head; node; node = node->next) {
		t = node->item;
		if(t->totaal != totaal)
			break;
		if(t->afstand != ONEINDIG && !t->bootje)
			toon_oplossing(t);
	}
}

static void bereken_dijkstra(void) {
	toestand_t *t, *dt;
	avl_node_t *node, *doel;
	bool veranderd;

	for(node = toestanden.tail; node; node = node->prev) {
		t = node->item;

		if(!t->totaal && t->bootje)
			t->afstand = 0;
		else
			t->afstand = ONEINDIG;
	}

	do {
		veranderd = false;

		for(node = toestanden.head; node; node = node->next) {
			t = node->item;

			for(doel = t->overgangen.head; doel; doel = doel->next) {
				dt = doel->item;

				if(dt->afstand + 1 < t->afstand) {
					t->afstand = dt->afstand + 1;
					t->volgende = dt;
					veranderd = true;
				}
			}
		}
	} while(veranderd);
}

static void genereer_overgangen(void) {
	avl_node_t *node, *doel;
	toestand_t *t, *dt;
	int opvarenden;

	for(node = toestanden.head; node; node = node->next) {
		t = node->item;
		for(doel = toestanden.head; doel; doel = doel->next) {
			dt = doel->item;

			if(!t->bootje == !dt->bootje)
				continue; /* constraint! */

			if(t->totaal > dt->totaal)
				opvarenden = t->totaal - dt->totaal;
			else
				opvarenden = dt->totaal - t->totaal;

			if(opvarenden == 0 || opvarenden > 2)
				continue;

			avl_item_insert(&t->overgangen, dt);
		}
	}
}

static void genereer_toestanden(void) {
	int tl, tr, kl, kr, ml, mr;
	toestand_t *t;

	for(tr = 0; tr <= totaal; tr++) {
		tl = totaal - tr;
		for(kr = 0; kr <= kannibalen; kr++) {
			kl = kannibalen - kr;
			ml = tl - kl;
			mr = tr - kr;
			if(kl > tl || kr > tr || ml > tl || mr > tr)
				continue; /* logically impossible */
			if(ml && kl > ml)
				continue; /* constraint! */
			if(mr && kr > mr)
				continue; /* constraint! */
			t = toestand_new(tl, kl, false);
			avl_insert_left(&toestanden, &t->node);
			t = toestand_new(tl, kl, true);
			avl_insert_left(&toestanden, &t->node);
		}
	}
}

int main(void) {
	genereer_toestanden();
	genereer_overgangen();
	bereken_dijkstra();
	toon_oplossingen();
	return 0;
}
