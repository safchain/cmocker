#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([cmocker], [0.9], [safchain@gmail.com])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE
LT_PREREQ([2.2])
LT_INIT([dlopen])
AC_CONFIG_SRCDIR([include/cmocker.h])

AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([limits.h stdlib.h string.h unistd.h])

PKG_CHECK_MODULES([CHECK], [check >= 0.9.8])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE

AM_PROG_CC_C_O

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([strdup])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 tests/Makefile])

AC_OUTPUT
