/*****************************************************************************

	canmiss.c - Example program for libavl

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

	$Id$
	$URL$

*****************************************************************************/

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
	avl_tree_t overtochten;
	struct toestand *volgende;
	unsigned int afstand;
} toestand_t;

typedef struct toestand_info {
	unsigned int tt, tl, tr;
	unsigned int kt, kl, kr;
	unsigned int mt, ml, mr;
} toestand_info_t;

typedef struct overtocht_info {
	unsigned int to, ko, mo;
} overtocht_info_t;

static unsigned int totaal = 6;
static unsigned int kannibalen = 3;
static unsigned int boot = 2;

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
	avl_tree_init(&t->overtochten, (avl_compare_t)avl_pointer_cmp, NULL);

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

static bool overtocht_info(overtocht_info_t *oi, const toestand_t *van, const toestand_t *naar) {
	toestand_info_t ni, vi;
	unsigned int to, ko, mo;

	toestand_info(&vi, van->totaal, van->kannibalen);
	toestand_info(&ni, naar->totaal, naar->kannibalen);

	if(!van->bootje == !naar->bootje)
		return false; /* constraint! */

	if(van->bootje) {
		if(vi.tr < ni.tr)
			return false;
		if(vi.kr < ni.kr)
			return false;
		if(vi.mr < ni.mr)
			return false;
		to = vi.tr - ni.tr;
		ko = vi.kr - ni.kr;
		mo = vi.mr - ni.mr;
	} else {
		if(vi.tl < ni.tl)
			return false;
		if(vi.kl < ni.kl)
			return false;
		if(vi.ml < ni.ml)
			return false;
		to = vi.tl - ni.tl;
		ko = vi.kl - ni.kl;
		mo = vi.ml - ni.ml;
	}

	if(to == 0 || to > boot)
		return false; /* constraint! */

	if(oi) {
		oi->to = to;
		oi->ko = ko;
		oi->mo = mo;
	}

	return true;
}

static int toestand_cmp(const toestand_t *a, const toestand_t *b) {
	return avl_unsigned_int_cmp(b->totaal, a->totaal);
}

static avl_tree_t toestanden = AVL_TREE_INIT((avl_compare_t)toestand_cmp, NULL);

static void toon_toestand(const toestand_t *t) {
	unsigned int i;
	toestand_info_t info;
	toestand_info(&info, t->totaal, t->kannibalen);

	printf("     ");
	for(i = 0; i < info.kt; i++)
		putchar(i < info.kl ? 'K' : '_');
	for(i = 0; i < info.mt; i++)
		putchar(i < info.ml ? 'M' : '_');
	if(t->bootje)
		printf(" ,,,,,,,,, \\__/ ");
	else
		printf(" \\__/ ,,,,,,,,, ");
	for(i = 0; i < info.kt; i++)
		putchar(i < info.kr ? 'K' : '_');
	for(i = 0; i < info.mt; i++)
		putchar(i < info.mr ? 'M' : '_');
	putchar('\n');
}

static void toon_overtocht(const toestand_t *van, const toestand_t *naar) {
	unsigned int i;
	char *dir;
	overtocht_info_t oi;
	overtocht_info(&oi, van, naar);

	dir = van->bootje ? "←" : "→";

	printf("             %s \\", dir);

	for(i = 0; i < oi.ko; i++)
		putchar('K');
	for(; i < oi.to; i++)
		putchar('M');
	for(; i < boot; i++)
		putchar('_');

	printf("/ %s\n", dir);
}

static void toon_oplossing(const toestand_t *t) {
	toestand_t *v;
	for(; t; t = v) {
		toon_toestand(t);
		v = t->volgende;
		if(v) {
			putchar('\n');
			toon_overtocht(t, v);
			putchar('\n');
		}
	}
}

static void toon_oplossingen(void) {
	avl_node_t *node;
	toestand_t *t;
	for(node = toestanden.head; node; node = node->next) {
		t = node->item;
		if(t->totaal != totaal)
			break;
		if(t->afstand != ONEINDIG)
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

			for(doel = t->overtochten.head; doel; doel = doel->next) {
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

static void genereer_overtochten(void) {
	avl_node_t *node, *doel;
	toestand_t *t, *dt;

	for(node = toestanden.head; node; node = node->next) {
		t = node->item;
		for(doel = toestanden.head; doel; doel = doel->next) {
			dt = doel->item;
			if(overtocht_info(NULL, t, dt))
				avl_item_insert(&t->overtochten, dt);
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
	genereer_overtochten();
	bereken_dijkstra();
	toon_oplossingen();
	return 0;
}
