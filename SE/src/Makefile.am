lib_LTLIBRARIES = libse.la
libse_la_SOURCES = SEopen.c SEprivateopen.c SEread.c SEwrite.c		\
SEquery.c SEprivatequery.c SEerror.c SEattr.c SEevol.c SEgva.c		\
$(srcdir)/../include/SEerror.h SEprivateopen.h SEprivatequery.h FSE.c	\
FSE.h FSEevol.h FSEgva.h
include_HEADERS = $(srcdir)/../include/SE.h			\
$(srcdir)/../include/SEevol.h $(srcdir)/../include/SEgva.h	\
$(srcdir)/../include/FSE.fi $(srcdir)/../include/FSE_f90.fi
AM_CPPFLAGS = -I$(srcdir)/../include -DH5_USE_16_API
AM_CFLAGS = -Wall
bin_PROGRAMS = touchse
touchse_LDADD = libse.la
