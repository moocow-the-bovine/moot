#!/bin/sh

#-----------------------------------------------------------------------
# File: autogen.sh
# Description:
#   + wrapper for m4 black-magic
#-----------------------------------------------------------------------

MY_ALDIRS="."
MY_AHDIRS="."
MY_AMDIRS="."
MY_ACDIRS="."

if test -n "$MY_ALDIRS"; then
 for d in $MY_ALDIRS ; do
    echo "(cd $d ; aclocal)"
    (cd $d ; aclocal)
 done
fi

if test -n "$MY_AHDIRS"; then
 for d in $MY_AHDIRS ; do
    echo "(cd $d ; autoheader)"
    (cd $d ; autoheader)
 done
fi

if test -n "$MY_AMDIRS"; then
 for d in $MY_AMDIRS ; do
    echo "(cd $d ; automake -a)"
    (cd $d ; automake -a)
 done
fi

if test -n "$MY_ACDIRS"; then
 for d in $MY_ACDIRS ; do
    echo "(cd $d ; autoconf)"
    (cd $d ; autoconf)
 done
fi

#echo "(./configure)"
#./configure $*
