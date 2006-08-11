# You may select DEPTHs (no indexing), COUNTs (slower) or both (more memory).
#CPPFLAGS += -DAVL_COUNT -DAVL_DEPTH

INSTALL ?= /usr/bin/install
LDCONFIG ?= /sbin/ldconfig

# Some suggestions: (-mcpu= generates i386 compatible code)
#CC = /opt/tendra/bin/tcc -I/usr/include -Yc99 -Xs -D__gnuc_va_list=int
#LN = /opt/tendra/bin/tcc
CC = gcc
LN = gcc
#CFLAGS = -O2 -fomit-frame-pointer -pipe -march=i586 -Wall -g
CFLAGS = -Os -pipe -march=athlon-xp -Wall -Werror -ansi -pedantic
#CFLAGS = -O6 -fomit-frame-pointer -pipe -march=athlon-xp -Wall -ansi -pedantic
LDFLAGS = -s -fpic
#LDFLAGS = -g -pg

#CFLAGS ?= -O2 -fomit-frame-pointer -pipe -mtune=i686 -w
CFLAGS += -DHAVE_C99 -DHAVE_POSIX

prefix ?= /usr/local
libdir ?= $(prefix)/lib
#includedir ?= $(prefix)/include
includedir ?= /usr/include

PROGRAMS = avlsort setdiff canmiss
LIBAVL = libavl.la
LIBRARIES = $(LIBAVL)

default: $(LIBAVL)

all: $(LIBAVL) $(PROGRAMS)

setdiff: setdiff.o avl.o
	$(LN) $(LDFLAGS) $^ -o $@ $(LIBS)

avlsort: avlsort.o avl.o
	$(LN) $(LDFLAGS) $^ -o $@ $(LIBS)

canmiss: canmiss.o avl.o
	$(LN) $(LDFLAGS) $^ -o $@ $(LIBS)

avl.lo: avl.c
	libtool --mode=compile $(CC) $(CFLAGS) -c $^ -o $@

$(LIBAVL): avl.lo
	libtool --mode=link $(CC) $(LDFLAGS) -rpath /usr/local/lib -version-number 2:0 $^ -o $@

clean:
	$(RM) *.o $(PROGRAMS) libavl.so.* *.la *.lo

install: all
	$(INSTALL) -d $(DESTDIR)$(includedir)
	$(INSTALL) -m 644 avl.h $(DESTDIR)$(includedir)
	$(INSTALL) -d $(DESTDIR)$(libdir)
	$(INSTALL) -m 644 $(LIBRARIES) $(DESTDIR)$(libdir)
	for i in $(LIBRARIES); do\
		ln -sf $$i $(DESTDIR)$(libdir)/$${i%.*};\
		ln -sf $${i%.*} $(DESTDIR)$(libdir)/$${i%.*.*};\
	done
	#-$(LDCONFIG)

.PHONY: clean install all
.PRECIOUS: %.h %.c
