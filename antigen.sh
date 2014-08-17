#!/bin/sh
#
# An utility script to remove all generated files.
#
# Running autogen.sh will be required after running this script since
# the 'configure' script will also be removed.
#
# This script is mainly useful when testing autoconf/automake changes
# and as a part of their development process.
# If there's a Makefile, then run the 'distclean' target first (which
# will also remove the Makefile).
if test -f Makefile; then
  make distclean
fi
# Remove all tar-files (assuming there are some packages).
rm -f *.tar.* *.tgz
# Also remove the autotools cache directory.
rm -Rf autom4te.cache m4
# Remove rest of the generated files.
rm -f aclocal.m4 configure config.h.in depcomp install-sh missing compile config.guess config.sub ltmain.sh autoscan.log configure.scan
find . -name Makefile.in -exec rm -f {} \;
find . -name *~ -exec rm -f {} \;
