#
# Configuration file for X-Rebirth data extractor.
#

AC_PREREQ([2.68])
AC_INIT([X-Rebirth data extractor], [0.1.alpha], [pivanov@foobar.ninja])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([config.h])
# M4 macros specific to this project.
AC_CONFIG_MACRO_DIR([m4-project])

# Init Automake.
AM_INIT_AUTOMAKE([1.15 subdir-objects readme-alpha no-dist no-dist-gzip -Wall])

# Add debug option.
AX_CHECK_ENABLE_DEBUG([no], [DEBUG])
AX_CHECK_ENABLE_PEDANTIC([no])

# Checks for programs.
AC_PROG_CXX([clang++ c++ g++ gcc])
AC_PROG_INSTALL

# Checks for languages.
AC_LANG([C++])
AX_CXX_COMPILE_STDCXX_14([noext], [mandatory])

# Checks for libraries.
AX_BOOST_BASE([1.63],
        [],
        [AC_MSG_ERROR([Minimal Boost library requirments not met.])
])

AX_BOOST_SYSTEM
AX_BOOST_FILESYSTEM
AX_BOOST_PROGRAM_OPTIONS

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT

echo \
"-------------------------------------------------

 ${PACKAGE_NAME} Version ${PACKAGE_VERSION}

 Prefix: '${prefix}'.
 Variables:
 
 ac_top_srcdir= ${ac_top_srcdir}
 
 -- Compiler
 CC: ${CC}
 CXX: ${CXX}
 CPP: ${CPP}
 CFLAGS: ${CFLAGS}
 CPPFLAGS: ${CPPFLAGS}
 CXXFLAGS: ${CXXFLAGS}

 -- Linker
 LDFLAGS: ${LDFLAGS}
 LIBOBJS: ${LIBOBJS}
 LIBS: ${LIBS}
 LTLIBOBJS: ${LTLIBOBJS}

 Now type 'make @<:@<target>@:>@'
   where the optional <target> is:
     all                - build all binaries

--------------------------------------------------"
