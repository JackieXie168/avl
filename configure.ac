# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(avl, 2.0.0, wsl-avl-bugs@fruit.je)

AM_INIT_AUTOMAKE()

AC_CONFIG_SRCDIR([src/avl.c])
dnl AC_CONFIG_HEADER([src/avl.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdint.h stdlib.h string.h sys/time.h], , [
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

# These need to become real tests
AC_SUBST(have_c99, 1)
AC_SUBST(have_posix, 1)

AC_CONFIG_FILES([Makefile src/avl.h])
AC_OUTPUT
