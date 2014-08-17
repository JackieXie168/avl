# Makefile.am
AUTOMAKE_OPTIONS= foreign

#Build in this directory:
noinst_PROGRAMS = avlsort canmiss setdiff 
avlsort_SOURCES = avlsort.c
canmiss_SOURCES = canmiss.c
setdiff_SOURCES = setdiff.c

INCLUDES = -I$(top_srcdir)/src

AM_CFLAGS= -g -O6 -fomit-frame-pointer -pipe -Wall -ansi -pedantic -fforce-mem -fforce-addr -pipe

avlsort_LDADD = $(top_srcdir)/src/libavl.la
canmiss_LDADD = $(top_srcdir)/src/libavl.la
setdiff_LDADD = $(top_srcdir)/src/libavl.la

CLEANFILES = *~