/*****************************************************************************

    setdiff.c - Example program for the AVL-tree library.

    Copyright (C) 1998  Michael H. Buselli <cosine@cosine.org>
    Copyright (C) 2000-2002  Wessel Dankers <wsl@nl.linux.org>

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

static int stricmp(const char *a, const char *b) {
	return *a - *b;
}

int main(int argc, char **argv) {
	avl_tree_t t, u;
	avl_node_t *c;

	if(argc != 3) {
		fprintf(stderr, "Requires exactly 2 arguments.\n");
		exit(2);
	}

	avl_tree_init(&t, (avl_compare_t)stricmp, NULL);
	avl_tree_init(&u, (avl_compare_t)stricmp, NULL);

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
