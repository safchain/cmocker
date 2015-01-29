TESTS = check_cmocker
check_PROGRAMS = check_cmocker
check_cmocker_SOURCES = check_cmocker.c
check_cmocker_CFLAGS = @CHECK_CFLAGS@ -I$(top_builddir)/include \
                        -I$(top_builddir)/../include
check_cmocker_LDADD = @CHECK_LIBS@ $(top_builddir)/src/libcmocker.la -lpthread

CLEANFILES = *~ *.gcno *.gcda *.gcov
