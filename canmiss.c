#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sysexits.h>

#include "avl.h"

#define ONEINDIG (~0U - 100U)

typedef struct toestand {
	avl_node_t node;
	unsigned int totaal;
	unsigned int kannibalen;
	bool bootje;
	avl_tree_t overgangen;
	struct toestand *volgende;
	unsigned int afstand;
} toestand_t;

typedef struct toestand_info {
	unsigned int tt, tl, tr;
	unsigned int kt, kl, kr;
	unsigned int mt, ml, mr;
} toestand_info_t;

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
	t->kannibalen = j;
	t->bootje = b;

	avl_node_init(&t->node, t);
	avl_tree_init(&t->overgangen, (avl_compare_t)avl_pointer_cmp, NULL);

	return t;
}

static void toestand_info(toestand_info_t *i, unsigned int tl, unsigned int kl) {
	unsigned int mt, ml;

	i->tt = totaal;
	i->tl = tl;
	i->tr = totaal - tl;

	i->kt = kannibalen;
	i->kl = kl;
	i->kr = kannibalen - kl;

	i->mt = mt = totaal - kannibalen;
	i->ml = ml = tl - kl;
	i->mr = mt - ml;
}

static int toestand_cmp(const toestand_t *a, const toestand_t *b) {
	return avl_unsigned_int_cmp(b->totaal, a->totaal);
}

static avl_tree_t toestanden = AVL_TREE_INIT((avl_compare_t)toestand_cmp, NULL);

static void toon_toestand(const toestand_t *t) {
	unsigned int i;
	toestand_info_t info;
	toestand_info(&info, t->totaal, t->kannibalen);

	for(i = 0; i < info.kt; i++)
		putchar(i < info.kl ? 'K' : '_');
	for(i = 0; i < info.mt; i++)
		putchar(i < info.ml ? 'M' : '_');
	putchar(t->bootje ? '<' : '>');
	for(i = 0; i < info.kt; i++)
		putchar(i < info.kr ? 'K' : '_');
	for(i = 0; i < info.mt; i++)
		putchar(i < info.mr ? 'M' : '_');
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

		if(t->totaal)
			t->afstand = ONEINDIG;
		else
			t->afstand = 0;
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
	unsigned int opvarenden;
	toestand_info_t ti, dti;

	for(node = toestanden.head; node; node = node->next) {
		t = node->item;
		toestand_info(&ti, t->totaal, t->kannibalen);
		for(doel = toestanden.head; doel; doel = doel->next) {
			dt = doel->item;
			toestand_info(&dti, dt->totaal, dt->kannibalen);

			if(!t->bootje == !dt->bootje)
				continue; /* constraint! */

			if(t->bootje) {
				if(ti.tr < dti.tr)
					continue;
				if(ti.kr < dti.kr)
					continue;
				if(ti.mr < dti.mr)
					continue;
				opvarenden = ti.tr - dti.tr;
			} else {
				if(ti.tl < dti.tl)
					continue;
				if(ti.kl < dti.kl)
					continue;
				if(ti.ml < dti.ml)
					continue;
				opvarenden = ti.tl - dti.tl;
			}

			if(opvarenden == 0 || opvarenden > 2)
				continue; /* constraint! */

			toon_toestand(t); putchar(32);
			toon_toestand(dt); putchar(10);

			avl_item_insert(&t->overgangen, dt);
		}
	}
}

static void genereer_toestanden(void) {
	unsigned int tl, kl;
	toestand_t *t;
	toestand_info_t info;

	for(tl = 0; tl <= totaal; tl++) {
		for(kl = 0; kl <= kannibalen; kl++) {
			toestand_info(&info, tl, kl);
			if(info.kl > info.tl || info.kr > info.tr)
				continue; /* logically impossible */
			if(info.ml > info.tl || info.mr > info.tr)
				continue; /* logically impossible */
			if(info.ml && info.kl > info.ml)
				continue; /* constraint! */
			if(info.mr && info.kr > info.mr)
				continue; /* constraint! */
			t = toestand_new(tl, kl, false);
			avl_insert_somewhere(&toestanden, &t->node);
			t = toestand_new(tl, kl, true);
			avl_insert_somewhere(&toestanden, &t->node);
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
