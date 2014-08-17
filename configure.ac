# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(libavl, 2.0.0, wsl-avl-bugs@fruit.je)

AM_INIT_AUTOMAKE()

AC_CONFIG_SRCDIR([src/avl.c])
AC_CONFIG_HEADERS([config.h])
dnl AC_CONFIG_HEADER([src/avl.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_LANG_CPLUSPLUS
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AM_PROG_CC_C_O
AM_PROG_LIBTOOL
AM_SANITY_CHECK

# Setup AM
AM_INIT_AUTOMAKE
AM_PROG_CC_C_O

# Checks for libraries.
AC_CHECK_LIB([c], [main])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([errno.h stdint.h stdlib.h string.h sys/stat.h sys/time.h sys/mman.h unistd.h], , [
	AC_MSG_FAILURE([Required system header files not found.])
	exit 1
])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_CHECK_FUNCS([strchr])

# These need to become real tests
AC_SUBST(have_c99, 1)
AC_SUBST(have_posix, 1)

AC_CONFIG_FILES([Makefile src/Makefile src/avl.h example/Makefile])
AC_OUTPUT
