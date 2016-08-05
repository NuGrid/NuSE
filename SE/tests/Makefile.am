TESTS = f77 f77varargs overwrite
EXTRA_DIST = $(TESTS) unit
check_PROGRAMS = testio testbackup fsewrite
testio_SOURCES = testio.f
testbackup_SOURCES = testbackup.c
fsewrite_SOURCES = fsewrite.f
AM_CPPFLAGS = -I$(srcdir)/../include -I$(srcdir)/../src -DH5_USE_16_API
AM_FFLAGS = -I$(srcdir)/../include
AM_CFLAGS = -Wall
LDADD = ../src/libse.la @SYSTEMSTUB_LIB@

if HAVE_CHECK
TESTS += unit
check_PROGRAMS += secheck
secheck_SOURCES = main.c tests.c tests.h $(srcdir)/../include/SE.h
AM_CPPFLAGS += @CHECK_CPPFLAGS@
AM_LDFLAGS = @CHECK_LDFLAGS@
LDADD += @CHECK_LIBS@
endif
