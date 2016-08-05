#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([se], [1.2], [gaber@lanl.gov])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([src/SEquery.c])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# Checks for programs.
AC_PROG_CC
AC_PROG_F77
AC_PROG_LIBTOOL
AC_F77_WRAPPERS

# AC_F77_WRAPPERS may fail to determine the Fortran name-mangling
# scheme if the chosen C compiler can't link object files compiled
# with the chosen Fortran compiler (due to, e.g., different default
# target architectures).  Check for the existence of F77_FUNC.
AC_CHECK_DECL(F77_FUNC,
	,
	[AC_MSG_ERROR([C and Fortran compilers are apparently incompatible])])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stddef.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([strerror])
AC_CHECK_LIB([SystemStubs], [printf@S|@LDBLStub],
             [SYSTEMSTUB_LIB="-lSystemStubs"], [SYSTEMSTUB_LIB=""])
AC_SUBST([SYSTEMSTUB_LIB])

# Find the 'check' tool, if it exists
AC_ARG_WITH([check],
    [AS_HELP_STRING([--with-check@<:@=PATH@:>@],
                    [prefix where check is installed @<:@default=auto@:>@])],
    [CHECK_LIBS="-lcheck"
    AS_IF([test $with_check != yes],
          [CHECK_CPPFLAGS="-I$with_check/include"
          CHECK_LDFLAGS="-L$with_check/lib"])

    save_CPPFLAGS=$CPPFLAGS
    save_LDFLAGS=$LDFLAGS
    save_LIBS=$LIBS

    CPPFLAGS="$CPPFLAGS $CHECK_CPPFLAGS"
    LDFLAGS="$LDFLAGS $CHECK_LDFLAGS"
    LIBS="$LIBS $CHECK_LIBS"

    AC_CHECK_HEADER([check.h], 
                    [AC_CHECK_LIB([check], [tcase_create], 
                                  [], [with_check=no])],
                    [with_check=no])

    CPPFLAGS=$save_CPPFLAGS
    LDFLAGS=$save_LDFLAGS
    LIBS=$save_LIBS],
    [with_check=no])
AM_CONDITIONAL([HAVE_CHECK], [test $with_check != no])
AC_SUBST([CHECK_CPPFLAGS])
AC_SUBST([CHECK_LDFLAGS])
AC_SUBST([CHECK_LIBS])

# Checks for libraries.
AC_ARG_WITH([hdf5],
    [AS_HELP_STRING([--with-hdf5=PATH],
                    [prefix where HDF5 is installed])],
    [AS_IF([test $with_hdf5 = yes], 
           [AC_MSG_ERROR([use --with-hdf5=PATH to specify HDF5 location])],
           [CPPFLAGS="$CPPFLAGS -I$with_hdf5/include"
	   LDFLAGS="$LDFLAGS -L$with_hdf5/lib"])],
    [with_hdf5=no])
AC_CHECK_HEADER([hdf5.h], [], [AC_MSG_ERROR([hdf5.h is required])])
AC_CHECK_LIB([hdf5], [H5open], [], [AC_MSG_ERROR([libhdf5 is required])])

# Final output
AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile])
AC_OUTPUT
