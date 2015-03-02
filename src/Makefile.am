AM_CFLAGS = @GCOV_CFLAGS@ -I$(top_srcdir)/include -Wall

lib_LTLIBRARIES = libcmocker.la
libcmocker_la_SOURCES = cmocker.c
libcmocker_la_LIBADD = @GCOV_LIBS@
libcmocker_la_LDFLAGS = -version-info 0:9:0

include_HEADERS = $(top_srcdir)/include/cmocker.h

CLEANFILES	= *~ *.gcno *.gcda *.gcov
