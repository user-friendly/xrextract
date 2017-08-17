# SYNOPSIS
#
#   AX_PRECOMPILED_HEADERS([filename])
#
# DESCRIPTION
#
#   Enable precompiled headers.
#   Filename must be the header file, relative to the top source dir.
#
#   This macro calls:
#       
#       AC_SUBST(PCH_SOURCE)
#       AC_SUBST(PCH_SOURCE_FILENAME)
#       AC_SUBST(PCH_FILENAME)
#       AC_SUBST(CFLAGS_PCH)
#       AC_SUBST(CXXFLAGS_PCH)
#
# LICENSE
#
#   Copyright (c) 2014 Plamen Ivanov <qnq6666@gmail.com>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved.

AC_DEFUN([AX_PRECOMPILED_HEADERS],[
    m4_if([$1], [],
    [ax_pch_file="no"],
    [ax_pch_file="m4_normalize([$1])"])
    
    CFLAGS_PCH=""
    CXXFLAGS_PCH=""
    PCH_FILENAME=""
    PCH_SOURCE_FILENAME=""
    
    AC_MSG_CHECKING([whether precompiled headers are enabled])
    
    if test "x$ax_pch_file" = "xno"; then
        AC_MSG_RESULT(no)
    else
        AC_MSG_RESULT(yes)
        
        # Defaults to GCC's PCH extension.
        ax_pch_ext="gch"
        if test "x${CC%%lang*}" = "xc"; then
            ax_pch_ext="pch"
        fi
        
        if test "x${CXX%%lang*}" = "xc"; then
            ax_pch_ext="pch"
        fi
        
        # Source PCH's base name.
        PCH_SOURCE_FILENAME="$ax_pch_file"
        # Source PCH's filename relative to the source directory.
        
        PCH_SOURCE="$srcdir/$ax_pch_file"
        # PCH's base filename.
        PCH_FILENAME="$ax_pch_file.$ax_pch_ext"
        
        CFLAGS_PCH="-include ${PCH_SOURCE}"
        CXXFLAGS_PCH="-include ${PCH_SOURCE}"
        
        # PCH's source file existance & readability check.
        if test ! -r $PCH_SOURCE; then
            AC_MSG_ERROR([Source PCH file does not exist or not readable: ${PCH_SOURCE}])
        fi
    fi
    
    AC_SUBST(PCH_SOURCE)
    AC_SUBST(PCH_SOURCE_FILENAME)
    AC_SUBST(PCH_FILENAME)
    AC_SUBST(CFLAGS_PCH)
    AC_SUBST(CXXFLAGS_PCH)
])
