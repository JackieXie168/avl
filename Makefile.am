lib_LTLIBRARIES = libavl.la
libavl_la_SOURCES = src/avl.c src/avl.h
libavl_la_LDFLAGS = -version-info 2:0:0
include_HEADERS = src/avl.h
dist_man_MANS = doc/avl.7 doc/avl_cmp.3 doc/avl_delete.3 doc/avl_fixup.3 doc/avl_index.3 doc/avl_insert.3 doc/avl_item_insert.3 doc/avl_node_init.3 doc/avl_search.3 doc/avl_tree_init.3
nobase_dist_doc_DATA = example/avlsort.c example/canmiss.c example/setdiff.c convert

SUBDIRS = . src example