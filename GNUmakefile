# You may select DEPTHs (no indexing), COUNTs (slower) or both (more memory).
#CPPFLAGS += -DAVL_COUNT -DAVL_DEPTH

LN ?= gcc
INSTALL ?= /usr/bin/install
LDCONFIG ?= /sbin/ldconfig

# Some suggestions: (-mcpu= generates i386 compatible code)
CFLAGS = -O9
#CFLAGS ?= -O2 -fomit-frame-pointer -pipe -mcpu=i686 -w
#CFLAGS = -O2 -fomit-frame-pointer -pipe -march=i586 -Wall -g
#CFLAGS = -O6 -fomit-frame-pointer -pipe -march=i586 -Wall -ansi -pedantic
#CFLAGS = -O6 -fomit-frame-pointer -pipe -march=i586 -Wall -ansi -pedantic
#CFLAGS = -O6 -march=k6 -fforce-mem -fforce-addr -pipe
#CFLAGS = -g -fomit-frame-pointer -pipe -march=i686 -Wall -ansi -pedantic
#CFLAGS = -g -pg -a -pipe -march=i686 -Wall
#LDFLAGS = -s

prefix ?= /usr/local
libdir ?= $(prefix)/lib
#includedir ?= $(prefix)/include
includedir ?= /usr/include

PROGRAMS = avlsort setdiff
LIBAVL = libavl.so.1.5
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
	$(RM) *.o $(PROGRAMS) libavl.*

install: all
	$(INSTALL) -d $(DESTDIR)$(includedir)
	$(INSTALL) -m 644 avl.h $(DESTDIR)$(includedir)
	$(INSTALL) -d $(DESTDIR)$(libdir)
	$(INSTALL) -m 644 $(LIBRARIES) $(DESTDIR)$(libdir)
	for i in $(LIBRARIES); do\
		$(LN) -sf $$i $(DESTDIR)$(libdir)/$${i%.*};\
		$(LN) -sf $${i%.*} $(DESTDIR)$(libdir)/$${i%.*.*};\
	done
	#-$(LDCONFIG)

.PHONY: clean install all
.PRECIOUS: %.h %.c
