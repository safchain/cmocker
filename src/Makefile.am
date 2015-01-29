AM_CFLAGS = @GCOV_CFLAGS@ -I$(top_srcdir)/include

lib_LTLIBRARIES = libcmocker.la
libcmocker_la_SOURCES = cmocker.c
libcmocker_la_LIBADD = @GCOV_LIBS@

include_HEADERS = $(top_srcdir)/include/cmocker.h

CLEANFILES	= *~ *.gcno *.gcda *.gcov
