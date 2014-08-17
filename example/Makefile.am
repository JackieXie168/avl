# Makefile.am
AUTOMAKE_OPTIONS= foreign

#Build in this directory:
noinst_PROGRAMS = avlsort canmiss setdiff 

avlsort_SOURCES = avlsort.c
setdiff_SOURCES = setdiff.c
canmiss_SOURCES = canmiss.c

INCLUDES = -I$(top_srcdir)/src

#AM_CFLAGS= -g -O6 -fomit-frame-pointer -pipe -Wall -ansi -pedantic -fforce-mem -fforce-addr -pipe
AM_CFLAGS= -g -O6 -fomit-frame-pointer -pipe -Wall -ansi -pedantic -fforce-addr -pipe

avlsort_LDADD = $(top_srcdir)/libavl.la
setdiff_LDADD = $(top_srcdir)/libavl.la
canmiss_LDADD = $(top_srcdir)/libavl.la

CLEANFILES = *~