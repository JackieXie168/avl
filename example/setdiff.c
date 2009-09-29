/*****************************************************************************

	setdiff.c - Example program for libavl

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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "avl.h"

static void readinto(avl_tree_t *t, char *fname) {
	FILE *f;
	char *s, *e;
	int len;

	f = fopen(fname, "r");
	if(!f) {
		perror(fname);
		exit(1);
	}

	for(;;) {
		s = malloc(65536);
		if(!s) {
			perror("malloc()");
			exit(2);
		}
		if(!fgets(s, 65536, f)) {
			free(s);
			break;
		}
		len = strlen(s);
		if(s[len-1] == '\n')
			s[len-1] = '\0';
		else
			len++;
		e = realloc(s, len);
		if(e)
			s = e;
		avl_item_insert_somewhere(t, s);
	}
	fclose(f);
}

int main(int argc, char **argv) {
	avl_tree_t u = AVL_TREE_INITIALIZER((avl_compare_t)strcmp, NULL);
	avl_tree_t t = AVL_TREE_INITIALIZER((avl_compare_t)strcmp, NULL);
	avl_node_t *c;

	if(argc != 3) {
		fprintf(stderr, "Requires exactly 2 arguments.\n");
		exit(2);
	}

	readinto(&t, argv[1]);
	readinto(&u, argv[2]);

	for(c = t.head; c; c = c->next)
		if(!avl_search(&u, c->item))
			printf("-%s\n", (char *)c->item);

	for(c = u.head; c; c = c->next)
		if(!avl_search(&t, c->item))
			printf("+%s\n", (char *)c->item);

	return 0;
}
