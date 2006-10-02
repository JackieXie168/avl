INSTALL ?= /usr/bin/install
LDCONFIG ?= /sbin/ldconfig
LIBTOOL ?= /usr/bin/libtool --quiet
#CC = /opt/tendra/bin/tcc -I/usr/include -Yc99 -Xs -D__gnuc_va_list=int
#LN = /opt/tendra/bin/tcc
CC = gcc
LN = gcc

# You may select DEPTHs (no indexing), COUNTs (slower) or both (more memory).
#CPPFLAGS += -DAVL_COUNT -DAVL_DEPTH

# Some suggestions: (-mtune= keeps the code i386 compatible)
#CFLAGS = -O2 -fomit-frame-pointer -pipe -march=i586 -Wall -g
CFLAGS = -Os -pipe -march=athlon-xp -Wall -Werror -ansi -pedantic
#CFLAGS = -O6 -fomit-frame-pointer -pipe -march=athlon-xp -Wall -ansi -pedantic
LDFLAGS = -s
#LDFLAGS = -g -pg

#CFLAGS ?= -O2 -fomit-frame-pointer -pipe -mtune=i686 -w
CFLAGS += -DHAVE_C99 -DHAVE_POSIX -Isrc

prefix ?= /usr/local
libdir ?= $(prefix)/lib
#includedir ?= $(prefix)/include
includedir ?= /usr/include

LIBRARIES = src/libavl.la
PROGRAMS = example/avlsort example/setdiff example/canmiss

default: $(LIBRARIES)

all: $(LIBRARIES) $(PROGRAMS)

%.o %.lo: %.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(CFLAGS) -c $^ -o $@

example/setdiff: example/setdiff.o
	$(LIBTOOL) --mode=link $(LN) $(LDFLAGS) $^ -o $@ $(LIBS) -Lsrc -lavl

example/avlsort: example/avlsort.o
	$(LIBTOOL) --mode=link $(LN) $(LDFLAGS) $^ -o $@ $(LIBS) -Lsrc -lavl

example/canmiss: example/canmiss.o
	$(LIBTOOL) --mode=link $(LN) $(LDFLAGS) $^ -o $@ $(LIBS) -Lsrc -lavl

src/libavl.la: src/avl.lo
	$(LIBTOOL) --mode=link $(LN) $(LDFLAGS) -rpath $(libdir) -version-number 2:0 $^ -o $@

clean:
	$(LIBTOOL) --mode=clean $(RM) $(LIBRARIES) $(PROGRAMS) src/*.lo example/*.o

install: all
	$(INSTALL) -d $(DESTDIR)$(includedir)
	$(INSTALL) -m 644 src/avl.h $(DESTDIR)$(includedir)
	$(INSTALL) -d $(DESTDIR)$(libdir)
	$(LIBTOOL) --mode=install $(INSTALL) -m 644 $(LIBRARIES) $(DESTDIR)$(libdir)
	$(LIBTOOL) --mode=finish $(DESTDIR)$(libdir)

.PHONY: clean install all default
.PRECIOUS: %.h %.c
