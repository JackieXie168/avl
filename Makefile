# You may select DEPTHs (no indexing), COUNTs (slower) or both (more memory).
#CPPFLAGS += -DAVL_COUNT -DAVL_DEPTH

LN ?= gcc
INSTALL ?= /usr/bin/install
LDCONFIG ?= /sbin/ldconfig

# Some suggestions: (-mcpu= generates i386 compatible code)
CC=gcc-4.0
#CFLAGS = -O2 -fomit-frame-pointer -pipe -march=i586 -Wall -g
#CFLAGS = -O6 -fomit-frame-pointer -pipe -march=i586 -Wall -ansi -pedantic
#CFLAGS = -O6 -fomit-frame-pointer -pipe -march=i586 -Wall -ansi -pedantic
CFLAGS = -g -pg -pipe -march=i586 -Wall -Werror -ansi -pedantic
CFLAGS ?= -O2 -fomit-frame-pointer -pipe -mcpu=i686 -w
CFLAGS += -DHAVE_C99 -DHAVE_POSIX
#LDFLAGS = -s

prefix ?= /usr/local
libdir ?= $(prefix)/lib
#includedir ?= $(prefix)/include
includedir ?= /usr/include

PROGRAMS = avlsort setdiff
LIBAVL = libavl.so.2.0
LIBRARIES = $(LIBAVL)

all: $(LIBAVL)

test: $(PROGRAMS)

setdiff: setdiff.o avl.o
	$(LN) $(LDFLAGS) $^ -o $@ $(LIBS)

avlsort: avlsort.o avl.o
	$(LN) $(LDFLAGS) $^ -o $@ $(LIBS)

$(LIBAVL): avl.o
	$(LN) -nostdlib -shared -Wl,-soname,libavl.so.1 $^ -o $@ -lc

clean:
	$(RM) *.o $(PROGRAMS) libavl.so.*

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
