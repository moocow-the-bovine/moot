dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.5)

dnl Some handy macros
define([THE_PACKAGE_NAME],    [moot])
define([THE_PACKAGE_VERSION], [2.0.10-6])
define([THE_PACKAGE_MAINTAINER],  [moocow@cpan.org])

AC_INIT(THE_PACKAGE_NAME, THE_PACKAGE_VERSION, THE_PACKAGE_MAINTAINER)
dnl AC_INIT(moot, 2.0.0, moocow@cpan.org)

dnl source && aux dir
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR(m4)

dnl canonical target (sets $target, $target_(cpu|vendor|os) : used for bindist)
AC_CANONICAL_TARGET

dnl use automake
AM_INIT_AUTOMAKE(THE_PACKAGE_NAME, THE_PACKAGE_VERSION)

dnl use autoheader
AM_CONFIG_HEADER([src/libmoot/mootAutoConfig.h])

dnl default prefix
AC_PREFIX_DEFAULT(/usr/local)

dnl-- makes libtoolize keep m4 macros in tree
AC_CONFIG_MACRO_DIR([m4])

#-------------------------------------------------------------
# save user's *FLAGS
#USER_LIBS="$LIBS"
#USER_LDFLAGS="$LDFLAGS"
#USER_CPPFLAGS="$CPPFLAGS"
USER_CXXFLAGS="$CXXFLAGS"
USER_CFLAGS="$CFLAGS"

dnl
dnl check for programs
dnl
AC_PROG_CC

dnl for gnulib
gl_EARLY

dnl-- for gnulib
dnl + do this BEFORE AC_LANG([C++]); otherwise memrchr() check pukes)
gl_INIT

AC_PROG_CXX
AC_LANG([C++])

dnl v--- for static-only libraries (non-libtool)
dnl AC_PROG_RANLIB

dnl v--- needed if Makefile.am uses lex sources
dnl AM_PROG_LEX

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## restore user CFLAGS
##

### --- unmangle user's CFLAGS
if test "$CFLAGS" != "$USER_CFLAGS" ; then
  # autoconf likes to set this (to '-g -O2')... who knows why...
  AC_MSG_NOTICE([Restoring user's original CFLAGS value])
  CFLAGS="$USER_CFLAGS"
fi

### --- unmangle user's CXXFLAGS
if test "$CXXFLAGS" != "$USER_CXXFLAGS" ; then
  # autoconf likes to set this (to '-g -O2')... who knows why...
  AC_MSG_NOTICE([Restoring user's original CXXFLAGS value])
  CXXFLAGS="$USER_CXXFLAGS"
fi
##
## /restore user CFLAGS
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## debug ?
##
CONFIG_OPTIONS=""
AC_MSG_CHECKING([whether to build debug version])
AC_ARG_ENABLE(debug,
	AC_HELP_STRING([--enable-debug], [build debug version (default=no)]))

if test "$enable_debug" = "yes" ; then
   AC_MSG_RESULT(yes)

   if test "$GCC" == "yes" ; then
     AC_MSG_NOTICE([GNU C compiler detected: setting appropriate debug flags])
     moot_OFLAGS="-O0 -ggdb -g3"
   else
     AC_MSG_WARN([GNU C compiler not detected: you must use CFLAGS, CXXFLAGS, LDFLAGS to set debugging flags])
     moot_OFLAGS="-g"
   fi

   AC_DEFINE(MOOT_DEBUG_ENABLED,1,
	     [Define this to enable debugging code])
   DOXY_DEFINES="$DOXY_DEFINES MOOT_DEBUG_ENABLED=1"
   #CONFIG_OPTIONS="$CONFIG_OPTIONS DEBUG=1"
   CONFIG_OPTIONS="DEBUG=1"
else
  AC_MSG_RESULT(no)

  if test "$GCC" == "yes"; then
   case "$CFLAGS" in
    *-O* | *-g* )
      AC_MSG_NOTICE([CFLAGS appears already to contain optimization flags - skipping])
      moot_OFLAGS=""
      ;;
    *)
     AC_MSG_NOTICE([GNU C compiler detected: setting appropriate optimization flags])
     #moot_OFLAGS="-O2"
     #moot_OFLAGS="-O3 -fomit-frame-pointer -funroll-loops -finline-limit-100000"
     moot_OFLAGS="-O3 -funroll-loops" #-- "-O,-O2,-O3,-Os" all imply -fomit-frame-pointer
     ;;
   esac
  else
    AC_MSG_WARN([GNU C compiler not detected: you must use CFLAGS, CXXFLAGS, LDFLAGS to set optimization flags])
    moot_OFLAGS=""
  fi

  #CONFIG_OPTIONS="$CONFIG_OPTIONS DEBUG=0"
  CONFIG_OPTIONS="DEBUG=0"
fi

AC_SUBST(moot_OFLAGS)
##
## /debug ?
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^



#---------------------------------------------------------------
# short package includes
spkgincludedir="\${includedir}/AC_PACKAGE_NAME"
AC_SUBST(spkgincludedir)
# short package includes
#---------------------------------------------------------------

#---------------------------------------------------------------
# get real prefix
AC_CACHE_CHECK([for installation prefix], [ac_cv_install_prefix],
	[if test "$prefix" = "NONE" ; then
	   ac_cv_install_prefix="/usr/local"
	 else
           ac_cv_install_prefix="$prefix"
	 fi
	])
dnl #  ... and add it to our flags
dnl CPPFLAGS="$CPPFLAGS -I${ac_cv_install_prefix}/include"
dnl LDFLAGS="$LDFLAGS -L${ac_cv_install_prefix}/lib"


dnl check for strdup
AC_CHECK_FUNC(strdup,[AC_DEFINE(HAVE_STRDUP,1,[Define this if you have the strdup() function])])


dnl ---------------------------------------------------------------
dnl pkg-config : program
dnl
AC_ARG_VAR(PKG_CONFIG, [How to run the pkg-config program])
AC_ARG_VAR(PKG_CONFIG_PATH, [Directories to search for pkg-config])
if test -z "$PKG_CONFIG" ; then
  AC_PATH_PROG(PKG_CONFIG,pkg-config,[])
fi
dnl pkg-config: destination directory
AC_ARG_WITH(pkgconfig-dir,
	AC_HELP_STRING([--with-pkgconfig-dir=DIR],
		[install pkg-config metafile(s) in DIR (default=LIBDIR/pkgconfig)]),
	[ac_cv_pkgconfigdir="$withval"])
if test -z "$ac_cv_pkgconfigdir" ; then
  ac_cv_pkgconfigdir="\$(libdir)/pkgconfig"
fi
pkgconfigdir="$ac_cv_pkgconfigdir"
AC_SUBST(pkgconfigdir)
dnl
dnl pkg-config
dnl ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


dnl vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
dnl check for time headers (argh)
AC_CHECK_HEADERS([time.h sys/time.h],
	[],
	[AC_MSG_WARN([Required header file not found!])
    	 AC_MSG_WARN([Are the 'struct timeval' and 'time()' headers in your 'CPPFLAGS' environment variable?])
    	 AC_MSG_ERROR([Required headers not found -- aborting.])
	])
dnl time headers
dnl ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

dnl vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
dnl float,value headers
AC_CHECK_HEADERS([float.h values.h],[],[])
dnl time headers
dnl ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

dnl vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
dnl gnulib: useful functions
dnl AC_CHECK_FUNCS([memrchr vasprintf vprintf],[],[])

dnl -- goofy memrchr stuff with gcc-4.4
gl_FUNC_MEMRCHR
gl_FUNC_VASPRINTF

dnl-- for optgen.perl / user-specific config files
AC_CHECK_HEADERS([pwd.h],[],[])
AC_CHECK_FUNCS([getuid getpwuid],[],[])
dnl gnulib: useful functions
dnl ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

dnl vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
dnl check for locale and regex headers
AC_CHECK_HEADERS([regex.h],[],
	[AC_MSG_WARN([Required header file 'regex.h' not found!])
    	 AC_MSG_WARN([Is the directory containing 'regex.h' in your 'CPPFLAGS' environment variable?])
    	 AC_MSG_ERROR([Required header(s) not found -- aborting.])
	])
AC_CHECK_FUNCS([regcomp regexec],[],
	[AC_MSG_WARN([Required regular expression support not found!])
    	 AC_MSG_ERROR([Required function(s) not found -- aborting.])
	])

AC_CHECK_HEADERS([locale.h],[],
	[AC_MSG_WARN([locale support disabled])
	])
AC_CHECK_FUNCS([setlocale],[],
	[AC_MSG_WARN([locale support disabled])
	])
dnl regex headers
dnl ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


## ------------------------------------------------------------
## DISTCHECK: simulate missing development-only packages
AC_ARG_VAR(DISTCHECK,
	[(Maintainer): set to nonempty value to simulate missing development packages])
if test -n "$DISTCHECK" ; then
   test -z "$DOXY_FILTER" && DOXY_FILTER=no
   test -z "$OPTGEN_PERL" && OPTGEN_PERL=no
   test -z "$DOXYGEN" && DOXYGEN=no
   test -z "$POD2X" && POD2TEXT=no
   test -z "$POD2X" && POD2MAN=no
   test -z "$POD2X" && POD2HTML=no
   test -z "$POD2X" && POD2LATEX=no
   test -z "$FLEXXX" && FLEXXX=no
   test -z "$BISONXX" && BISONXX=no
   test -z "$OSFCN_H" && OSFCN_H=no
   test -z "$PERL" && PERL=no
   test -z "$PYTHON" && PYTHON=no
   test -z "$RE2C" && RE2C=no
fi
## /DISTCHECK
## ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## zlib
##
AC_ARG_ENABLE(zlib,
	AC_HELP_STRING([--disable-zlib],
	               [Disable zlib compression support]),
	[ac_cv_enable_zlib="$enableval"],
	[ac_cv_enable_zlib="yes"])

if test "$ac_cv_enable_zlib" != "no" ; then
 ##-- zlib: headers
 for h in zlib.h ; do
  AC_CHECK_HEADER($h,
	[moot_have_header="yes"], [moot_have_header="no"], [ ])
  if test "$moot_have_header" = "no" ; then
    AC_MSG_WARN([])
    AC_MSG_WARN([----------------------------------------------------------------])
    AC_MSG_WARN([                 zlib header '$h' not found!])
    AC_MSG_WARN([])
    AC_MSG_WARN([    Is the directory containing the zlib headers in your])
    AC_MSG_WARN([             'CPPFLAGS' environment variable?])
    AC_MSG_WARN([----------------------------------------------------------------])
    AC_MSG_WARN([])
    ac_cv_enable_zlib="no"
  fi
 done
fi

##-- zlib: distcheck
if test -n "$DISTCHECK"; then
  AC_MSG_WARN([DISTCHECK set: disabling zlib support])
  ac_cv_enable_zlib="no"
fi

if test "$ac_cv_enable_zlib" != "no" ; then
 ##-- zlib: library: compile
 AC_CHECK_LIB(z,deflate,[ac_cv_have_libz="yes"])
 if test "$ac_cv_have_libz" != "yes" ; then
    AC_MSG_WARN([])
    AC_MSG_WARN([----------------------------------------------------------------])
    AC_MSG_WARN([                   zlib library not found!])
    AC_MSG_WARN([])
    AC_MSG_WARN([         Is the directory containing libz.a in your])
    AC_MSG_WARN([                'LDFLAGS' environment variable?])
    AC_MSG_WARN([----------------------------------------------------------------])
    AC_MSG_WARN([])
    ac_cv_enable_zlib="no"
 else
   moot_LIBS="$moot_LIBS -lz"
 fi
fi

##-- zlib: config.h flag
if test "$ac_cv_enable_zlib" = "no" ; then
  AC_MSG_NOTICE([zlib compression support disabled.])
  CONFIG_OPTIONS="$CONFIG_OPTIONS ZLIB=0"
else
 ##-- ac_cv_enable_zlib != no
 AC_DEFINE(MOOT_ZLIB_ENABLED,1,
	   [Define this to enable zlib compression support])
 DOXY_DEFINES="$DOXY_DEFINES MOOT_ZLIB_ENABLED=1"
 CONFIG_OPTIONS="$CONFIG_OPTIONS ZLIB=1"
fi
dnl
dnl zlib
dnl ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


dnl ------------------------------------------------------------
dnl check for flex++bison++
dnl
AC_ARG_VAR(FLEXXX, [Path to Alain Coetmeur's flex++ program, "no" for none])
AC_ARG_VAR(BISONXX, [Path to Alain Coetmeur's bison++ program, "no" for none])

PKG_CHECK_MODULES(FBXX, flex++bison++ >= 0.0.5,
	[ac_cv_have_pkg_fbxx="yes"],
	[ac_cv_have_pkg_fbxx="no"])

if test "$ac_cv_have_pkg_fbxx" = "yes"; then
  PATH="$PATH:`$PKG_CONFIG --variable=prefix flex++bison++`/bin"
  export PATH
  ###--- libFSMext would already get us this, but it's being removed ---
  CPPFLAGS="$CPPFLAGS `$PKG_CONFIG --cflags-only-I flex++bison++`"
fi


dnl ------------------------------------------------------------
dnl check for flex++
dnl
##-- flex++: user flag or path search
if test -z "$FLEXXX" ; then
  if test "$ac_cv_have_pkg_fbxx" = "yes"; then
    FLEXXX="`$PKG_CONFIG --variable=prefix flex++bison++`/bin/flex++"
    test -x "$FLEXXX" || FLEXXX=""
  fi
  if test -z "$FLEXXX" ; then
    AC_PATH_PROG(FLEXXX,[flex++],[no])
  fi
fi
##-- flex++: Coetmeur's?
if test "$FLEXXX" != "no"; then
  AC_MSG_CHECKING([for Coetmeur's flex++])
  if $FLEXXX -? 2>&1 | grep -q coetmeur; then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
    FLEXXX="no"
  fi
fi
##-- flex++: notices/warnings
AC_MSG_NOTICE([setting FLEXXX=$FLEXXX])
if test "$FLEXXX" = "no"; then
  AC_MSG_WARN([Coetmeur's flex++ is missing, broken, or disabled])
  AC_MSG_WARN([- interpretation of .ll lexer source files disabled])
fi
##-- flex++: automake conditional
AM_CONDITIONAL(HAVE_FLEXXX,[test "$FLEXXX" != "no"])


dnl ------------------------------------------------------------
dnl check for bison++
dnl
##-- bison++: user flag or path search
if test -z "$BISONXX" ; then
  if test "$ac_cv_have_pkg_fbxx" = "yes" ; then
    BISONXX="`$PKG_CONFIG --variable=prefix flex++bison++`/bin/bison++"
    test -x "$BISONXX" || BISONXX=""
  fi
  if test -z "$BISONXX" ; then
    AC_PATH_PROG(BISONXX,[bison++],[no])
  fi
fi
##-- bison++: Coetmeur's?
if test "$BISONXX" != "no"; then
  AC_MSG_CHECKING([for Coetmeur's bison++])
  if $BISONXX -V 2>&1 | grep -q coetmeur; then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
    BISONXX="no"
  fi
fi
##-- bison++: notices & warnings
AC_MSG_NOTICE([setting BISONXX=$BISONXX])
if test "$BISONXX" = "no"; then
  AC_MSG_WARN([Coetmeur's bison++ is missing, broken, or disabled])
  AC_MSG_WARN([- interpretation of .yy parser source files disabled])
fi
##-- bison++: conditional
AM_CONDITIONAL(HAVE_BISONXX,[test "$BISONXX" != "no"])

YACC="$BISONXX"
AC_SUBST(YACC)
AC_SUBST(BISONXX)

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## check for osfcn.h (for flex++/bison++)
##
if test -z "$OSFCN_H"; then
 AC_CHECK_HEADER([osfcn.h], [OSFCN_H=yes], [OSFCN_H=no])
fi

AC_MSG_CHECKING([whether to install local osfcn.h])
if test "$OSFCN_H" != "yes"; then
  AC_MSG_RESULT([yes])
  INSTALL_OSFCN_H=osfcn.h
else
  AC_MSG_RESULT([no])
  INSTALL_OSFCN_H=""
fi
AM_CONDITIONAL(HAVE_OSFCN_H, [test "$OSFCN_H" = "yes"])
AC_SUBST(INSTALL_OSFCN_H)
## /osfcn.h
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

dnl vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
dnl check for recode
dnl
dnl recode: user-request
AC_ARG_ENABLE(recode,
	AC_HELP_STRING([--disable-recode],
	               [Disable librecode support]),
	[ac_cv_enable_recode="$enableval"],[ac_cv_enable_recode="yes"])

dnl recode: headers
if test "$ac_cv_enable_recode" != "no" ; then
 for h in recodext.h ; do
  AC_CHECK_HEADER($h,
	[moot_have_header="yes"], [moot_have_header="no"], [])
  if test "$moot_have_header" = "no" ; then
    AC_MSG_WARN([])
    AC_MSG_WARN([----------------------------------------------------------------])
    AC_MSG_WARN([             librecode header '$h' not found!])
    AC_MSG_WARN([])
    AC_MSG_WARN([    Is the directory containing the librecode headers in your])
    AC_MSG_WARN([             'CPPFLAGS' environment variable?])
    AC_MSG_WARN([----------------------------------------------------------------])
    AC_MSG_WARN([])
    ac_cv_enable_recode="no"
  fi
 done
fi ;##-- /ac_cv_enable_recode != no

##-- recode: distcheck
if test -n "$DISTCHECK"; then
  AC_MSG_WARN([DISTCHECK set: disabling librecode support])
  ac_cv_enable_recode="no"
fi


dnl recode: library: compile
if test "$ac_cv_enable_recode" != "no" ; then

 AC_CHECK_LIB(recode,recode_new_outer,[ac_cv_have_librecode="yes"])

 if test "$ac_cv_have_librecode" != "yes" ; then
   AC_MSG_WARN([])
   AC_MSG_WARN([----------------------------------------------------------------])
   AC_MSG_WARN([             librecode library not found!])
   AC_MSG_WARN([])
   AC_MSG_WARN([    Is the directory containing the librecode.a in your])
   AC_MSG_WARN([             'LDFLAGS' environment variable?])
   AC_MSG_WARN([----------------------------------------------------------------])
   AC_MSG_WARN([])
   ac_cv_enable_recode="no"
 else
  moot_LIBS="$moot_LIBS -lrecode"
 fi
fi ;##-- /ac_cv_enable_recode != no

dnl recode: config.h flag
if test "$ac_cv_enable_recode" = "no" ; then
  AC_MSG_NOTICE([librecode support disabled.])
  CONFIG_OPTIONS="$CONFIG_OPTIONS RECODE=0"
else
 ##-- ac_cv_enable_recode != no
 AC_DEFINE(MOOT_RECODE_ENABLED,1,
	   [Define this to enable librecode support (useful for XML)])
 DOXY_DEFINES="$DOXY_DEFINES MOOT_RECODE_ENABLED=1"
 CONFIG_OPTIONS="$CONFIG_OPTIONS RECODE=1"
fi
dnl recode
dnl ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


dnl vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
dnl check for expat
dnl
dnl expat: user-request
AC_ARG_ENABLE(expat,
	AC_HELP_STRING([--disable-expat],
	               [Disable expat XML support]),
	[ac_cv_enable_expat="$enableval"],[ac_cv_enable_expat="yes"])

dnl expat: headers
if test "$ac_cv_enable_expat" != "no" ; then
 for h in expat.h ; do
  AC_CHECK_HEADER($h,
	[moot_have_header="yes"], [moot_have_header="no"], [ ])
  if test "$moot_have_header" = "no" ; then
    AC_MSG_WARN([])
    AC_MSG_WARN([----------------------------------------------------------------])
    AC_MSG_WARN([             expat header '$h' not found!])
    AC_MSG_WARN([])
    AC_MSG_WARN([    Is the directory containing the expat headers in your])
    AC_MSG_WARN([             'CPPFLAGS' environment variable?])
    AC_MSG_WARN([----------------------------------------------------------------])
    AC_MSG_WARN([])
    ac_cv_enable_expat="no"
  fi
 done
fi ;##-- /ac_cv_enable_expat != no

##-- expat: distcheck
if test -n "$DISTCHECK"; then
  AC_MSG_WARN([DISTCHECK set: disabling expat support])
  ac_cv_enable_expat="no"
fi

dnl expat: library: compile
if test "$ac_cv_enable_expat" != "no" ; then

 AC_CHECK_LIB(expat,XML_DefaultCurrent,[ac_cv_have_libexpat="yes"])

 if test "$ac_cv_have_libexpat" != "yes" ; then
    AC_MSG_WARN([])
    AC_MSG_WARN([----------------------------------------------------------------])
    AC_MSG_WARN([                expat library not found!])
    AC_MSG_WARN([])
    AC_MSG_WARN([      Is the directory containing libexpat.a in your])
    AC_MSG_WARN([             'LDPFLAGS' environment variable?])
    AC_MSG_WARN([----------------------------------------------------------------])
    AC_MSG_WARN([])
    ac_cv_enable_expat="no"
 else
    moot_LIBS="$moot_LIBS -lexpat"
 fi
fi ;##-- /ac_cv_enable_xml != no

dnl expat: config.h flag
if test "$ac_cv_enable_expat" = "no" ; then
  AC_MSG_NOTICE([Expat XML support disabled.])
  CONFIG_OPTIONS="$CONFIG_OPTIONS EXPAT=0"
else
 ##-- ac_cv_enable_xml != no
 AC_DEFINE(MOOT_EXPAT_ENABLED,1,
	   [Define this to enable experimental expat XML support])
 DOXY_DEFINES="$DOXY_DEFINES MOOT_EXPAT_ENABLED=1"
 CONFIG_OPTIONS="$CONFIG_OPTIONS EXPAT=1"
fi
dnl expat
dnl ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


dnl vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
dnl check for libxml2
dnl
dnl libxml2: user-request
AC_ARG_ENABLE(libxml,
	AC_HELP_STRING([--enable-libxml],
	               [Enable experimental libxml2 XML support (default=no)]),
	[ac_cv_enable_libxml="$enableval"],[ac_cv_enable_libxml="no"])

dnl libxml2: pkg-config
if test "$ac_cv_enable_libxml" != "no" ; then
  PKG_CHECK_MODULES(LIBXML2, libxml-2.0 >= 2.6.7,
	[ac_cv_enable_libxml="yes"],
	[ac_cv_enable_libxml="no"])
fi

##-- libxml2: distcheck
if test -n "$DISTCHECK"; then
  AC_MSG_WARN([DISTCHECK set: disabling libxml2 support])
  ac_cv_enable_libxml="no"
fi

dnl libxml: config.h flag
if test "$ac_cv_enable_libxml" = "no" ; then
  AC_MSG_NOTICE([libxml2 XML support disabled.])
 CONFIG_OPTIONS="$CONFIG_OPTIONS LIBXML2=0"
else
 ##-- ac_cv_enable_xml != no
 AC_DEFINE(MOOT_LIBXML_ENABLED,1,
	   [Define this to enable experimental libxml2 XML support])
 CXXFLAGS="$CXXFLAGS $LIBXML2_CFLAGS"
 LDFLAGS="$LDFLAGS `$PKG_CONFIG --libs-only-L libxml-2.0`"
 moot_LIBS="$moot_LIBS `$PKG_CONFIG --libs-only-l libxml-2.0`"

 DOXY_DEFINES="$DOXY_DEFINES MOOT_LIBXML_ENABLED=1"
 CONFIG_OPTIONS="$CONFIG_OPTIONS LIBXML2=1"
fi
## /libxml2
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## suffix tries?
##
AC_ARG_ENABLE(suffix-trie,
	AC_HELP_STRING([--disable-suffix-trie],
	               [Disable all support for (buggy) suffix tries]),
	[ac_cv_enable_suffix_trie="$enableval"],
	[ac_cv_enable_suffix_trie="yes"])

if test "$ac_cv_enable_suffix_trie" != "no" ; then
  AC_DEFINE(MOOT_ENABLE_SUFFIX_TRIE,1,[Define this to enable support for suffix tries])
  DOXY_DEFINES="$DOXY_DEFINES MOOT_ENABLE_SUFFIX_TRIE=1"
  CONFIG_OPTIONS="$CONFIG_OPTIONS SUFFIX_TRIE=1"
else
  CONFIG_OPTIONS="$CONFIG_OPTIONS SUFFIX_TRIE=0"
fi
##
## /suffix tries
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


##------------------------------------------------------------
## check for perl
AC_ARG_VAR(PERL, [Path to your perl interpreter.])
if test -z "$PERL" ; then
  AC_PATH_PROG(PERL,[perl],[])
fi
AC_SUBST(PERL)
## /perl
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


dnl ------------------------------------------------------------
dnl check for optgen.perl
dnl
AC_ARG_VAR(OPTGEN_PERL, [Path to the 'optgen.perl' script (disable with OPTGEN_PERL=no)])

##-- test for working optgen.perl
AC_MSG_CHECKING([whether Getopt::Gen works])
  if $PERL -MGetopt::Gen -e'exit 0;' >>config.log 2>&1; then
  AC_MSG_RESULT([yes])
else
  AC_MSG_RESULT([no])
  AC_MSG_WARN([Getopt::Gen (or something it depends on) is broken!])
  AC_MSG_WARN([- you probably need to fix your Parse::Lex module])
  OPTGEN_PERL="no"
fi

if test -z "$OPTGEN_PERL" ; then
  AC_PATH_PROG(OPTGEN_PERL,[optgen.perl],[no])
fi
if test "$OPTGEN_PERL" = "no" ; then
  AC_MSG_WARN([optgen.perl script is missing, broken, or disabled.])
  AC_MSG_WARN([- regeneration of command-line parsers from .gog specifications disabled.])
  AC_MSG_WARN([- regeneration of POD documentation from .gog specifications disabled.])
fi

AC_SUBST(OPTGEN_PERL)
AC_MSG_NOTICE([setting OPTGEN_PERL=$OPTGEN_PERL])
AM_CONDITIONAL(HAVE_OPTGEN, [test "$OPTGEN_PERL" != "no"])
dnl
dnl /optgen.perl
dnl ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##------------------------------------------------------------
## re2c
AC_ARG_VAR(RE2C,[Path to your re2c executable.])
if test -z "$RE2C"; then
  AC_PATH_PROG(RE2C,[re2c],[])
else
  AC_MSG_NOTICE([using environment variable RE2C=$RE2C])
fi
AC_SUBST(RE2C)
AM_CONDITIONAL(HAVE_RE2C, [test -n "$RE2C" -a "$RE2C" != "no"])
## /re2c
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##------------------------------------------------------------
## check for python
AC_ARG_VAR(PYTHON, [Path to your python interpreter.])
if test -z "$PYTHON" ; then
  AC_PATH_PROG(PYTHON,[python],[])
else
  AC_MSG_NOTICE([using environment variable PYTHON=$PYTHON])
fi
AC_SUBST(PYTHON)
AM_CONDITIONAL(HAVE_PYTHON, [test -n "$PYTHON" -a "$PYTHON" != "no"])
## /python
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##------------------------------------------------------------
## re2c_ucl
AC_ARG_ENABLE([re2c-auto],
	AC_HELP_STRING([--disable-re2c-auto],[disable auto-generation of Unicode regexes]),
	[],
	[enable_re2c_auto=auto])

AC_MSG_CHECKING([whether to auto-generate Unicode regexes (re2c_ucl, re2c_tolower)])
if test -n "$PYTHON" -a "$PYTHON" != "no" -a -n "$PERL" -a "$PERL" != "no" -a -n "$RE2C" -a "$RE2C" != "no" -a "$enable_re2c_auto" != "no" ; then
   AC_MSG_RESULT(yes)
   enable_re2c_auto=yes
else
   AC_MSG_RESULT(no)
   enable_re2c_auto=no
fi
AM_CONDITIONAL(ENABLE_RE2C_AUTO, [test "$enable_re2c_auto" != "no"])
## /re2c_ucl
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## documentation
##
AC_ARG_WITH(docdir,
	AC_HELP_STRING([--with-docdir=DIR],
		[install documentation in DIR/AC_PACKAGE_NAME (default=DATADIR/doc)]),
	[docdir="$withval"],
	[docdir="\${datadir}/doc"])
pkgdocdir="\${docdir}/\${PACKAGE}"
pkgdocprogdir="\${docdir}/\${PACKAGE}/programs"
pkgdoctutdir="\${docdir}/\${PACKAGE}/tutorial"
pkgdoclibdir="\${docdir}/\${PACKAGE}/lib\${PACKAGE}"

AC_SUBST(docdir)
AC_SUBST(pkgdocdir)
AC_SUBST(pkgdocprogdir)
AC_SUBST(pkgdoctutdir)
AC_SUBST(pkgdoclibdir)

AC_ARG_WITH(doc-formats,
	AC_HELP_STRING([--with-doc-formats=LIST],
		       [Build documentation formats in LIST. \
                        Available formats: txt, man, html, dvi, ps, pdf, none.
	                Default='man html'.  Requires Perl, pod2xxx, doxygen.]),
	[ac_cv_doc_formats="$withval"])
AC_ARG_ENABLE(doc,
	AC_HELP_STRING([--disable-doc],[Synonym for --with-doc-formats="none"]),
	[enable_doc="$enableval"],[enable_doc="yes"])


AC_MSG_CHECKING([which documentation formats to build])
## -- set default doc formats if unspecified
if test -z "$ac_cv_doc_formats" ; then
  ac_cv_doc_formats="man html"
fi

## -- un-comma-tize the doc-formats
moot_doc_formats=`echo "$ac_cv_doc_formats" | sed 's/\,/ /g'`

if test "$enable_doc" != "yes" -o -z "$PERL" ; then
 ##-- disable all docs
 moot_doc_formats="none"
fi
AC_MSG_RESULT($moot_doc_formats)
#echo ">> post RESULT"


##-- doxygen: var
AC_ARG_VAR(DOXYGEN,[Path to doxygen documentation generator (disable by setting DOXYGEN=no)])

if test "$moot_doc_formats" != "none" ; then
  ##-- docs: prog doxygen
  if test -z "$DOXYGEN" ; then
    AC_PATH_PROG(DOXYGEN,doxygen,[no])
  fi

  ##-- report doxygen-check results
  if test -z "$DOXYGEN" -o "$DOXYGEN" = "no"; then
    DOXYGEN="no"
    AC_MSG_WARN([doxygen not found: library documentation will not be built!]) 
  fi
  AC_MSG_NOTICE([setting DOXYGEN=$DOXYGEN])

  ##-- doxygen: filter: doc/lib*/'doxy-filter.perl'
  ##
  AC_ARG_VAR(DOXY_FILTER,[doxygen input filter; "no" to disable])
  AC_MSG_CHECKING([for doxygen input filter])
  if test -z "$DOXY_FILTER"; then
    DOXY_SRCDIR="`find ${srcdir}/doc/ -name 'libmoot*' -print | head -n1`"
    if test -d "$DOXY_SRCDIR"; then
      DOXY_SRCDIR=`cd "$DOXY_SRCDIR"; pwd`
      DOXY_FILTER="$PERL $DOXY_SRCDIR/doxy-filter.perl"
    else
      DOXY_FILTER="no"
    fi
  fi
  AC_MSG_RESULT([$DOXY_FILTER])
  ##
  if test "$DOXY_FILTER" != "no"; then
    AC_MSG_CHECKING([whether doxygen input filter works])
    if test -n "$DOXY_FILTER" && $DOXY_FILTER </dev/null 2>&1 >>config.log ; then
      AC_MSG_RESULT([yes])
    else
      AC_MSG_RESULT([no])
      DOXY_FILTER="no"
    fi
  fi
  AC_MSG_NOTICE([setting DOXY_FILTER=$DOXY_FILTER])
  if test "$DOXY_FILTER" != "no"; then
    DOXY_INPUT_FILTER="$DOXY_FILTER"
  else
    DOXY_INPUT_FILTER=""
  fi
  AC_SUBST(DOXY_FILTER)
  AC_SUBST(DOXY_INPUT_FILTER)
  ##
  ##--/doxygen:filter

  ##-- doxygen: sources
  AC_SUBST(DOXYGEN_SOURCES)

  ##-- docs:pod2x
  ##
  AC_ARG_VAR(POD2TEXT, [Path to perl 'pod2text' script; "no" for none])
  AC_ARG_VAR(POD2MAN,  [Path to perl 'pod2man' script; "no" for none])
  AC_ARG_VAR(POD2HTML, [Path to perl 'pod2html' script; "no" for none])
  AC_ARG_VAR(POD2LATEX,[Path to perl 'pod2latex' script; "no" for none])
  if test -z "$POD2TEXT" ; then
     AC_PATH_PROG(POD2TEXT,pod2text,[no])
  fi
  if test -z "$POD2MAN"  ; then
     AC_PATH_PROG(POD2MAN,pod2man,[no])
  fi
  if test -z "$POD2HTML" ; then
     AC_PATH_PROG(POD2HTML,pod2html,[no])
  fi
  if test -z "$POD2LATEX"; then
     AC_PATH_PROG(POD2LATEX,pod2latex,[no])
  fi
  AC_SUBST(POD2TEXT)
  AC_SUBST(POD2MAN)
  AC_SUBST(POD2HTML)
  AC_SUBST(POD2LATEX)
  ##
  ##--/docs:pod2x

  ##-- docs: .gog sources
  ##
  DOC_MAN1_GOGS="\
	moot.gog \
	mootconfig.gog \
	mootpp.gog \
	mootrain.gog \
	mootcompile.gog \
	mootdump.gog \
	mooteval.gog \
	mootchurn.gog \
	moottaste.gog \
	mootdyn.gog \
	waste.gog \
	"
  for g in $DOC_MAN1_GOGS ; do
    AC_CONFIG_LINKS(doc/programs/${g}:src/programs/${g})  
  done
  DOC_GOGS="$DOC_MAN1_GOGS"
  AC_SUBST(DOC_MAN1_GOGS)
  AC_SUBST(DOC_GOGS)
  AC_CONFIG_LINKS(doc/programs/acknowledge.pod:src/programs/acknowledge.pod)
  AC_CONFIG_LINKS(doc/programs/verbose.pod:src/programs/verbose.pod)
  ##
  ##-- /doc: .gog sources

  ##-- index-skeleton (section 1)
  DOC_PROG_INDEX_SKEL="mootutils.skel"
  AC_SUBST(DOC_PROG_INDEX_SKEL)

  ##-- section 5 pods
  DOC_MAN5_PODS="mootfiles.pod"
  AC_SUBST(DOC_MAN5_PODS)

  ##-- collected pods
  DOC_PODS="$DOC_MAN5_PODS mootutils.pod `echo \"$moot_program_gogs\" | sed 's/\.gog/\.pod/g'`"
  AC_SUBST(DOC_PODS)

  ##-- set "CONFIG_DOC_WANT_*" variables
  ##   + test with automake conditionals "DOC_WANT_*"
  CONFIG_DOC_WANT_TXT="no"
  CONFIG_DOC_WANT_MAN="no"
  CONFIG_DOC_WANT_HTML="no"
  CONFIG_DOC_WANT_LATEX="no"
  CONFIG_DOC_WANT_DVI="no"
  CONFIG_DOC_WANT_PS="no"
  CONFIG_DOC_WANT_PDF="no"

  ##-- docs: parse user requests
  ##
  for fmt in $moot_doc_formats ; do
    case "$fmt" in
      txt)
        CONFIG_DOC_WANT_TXT="yes"
	;;
      man)
        CONFIG_DOC_WANT_MAN="yes"
	;;
      html)
        CONFIG_DOC_WANT_HTML="yes"
	;;
      dvi)
        CONFIG_DOC_WANT_LATEX="yes"
        CONFIG_DOC_WANT_DVI="yes"
	;;
      ps)
	CONFIG_DOC_WANT_LATEX="yes"
        CONFIG_DOC_WANT_PS="yes"
	;;
      pdf)
        CONFIG_DOC_WANT_LATEX="yes"
	CONFIG_DOC_WANT_PDF="yes"
	;;
      *)
	AC_MSG_WARN(ignoring unknown documentation format: $fmt)
	;;
    esac; # case "$fmt" in ...
  done; # for fmt in $moot_doc_formats ...

  ##-- automake conditionals: indicator values
  AC_SUBST(CONFIG_DOC_WANT_TXT)
  AC_SUBST(CONFIG_DOC_WANT_MAN)
  AC_SUBST(CONFIG_DOC_WANT_HTML)
  AC_SUBST(CONFIG_DOC_WANT_LATEX)
  AC_SUBST(CONFIG_DOC_WANT_DVI)
  AC_SUBST(CONFIG_DOC_WANT_PS)
  AC_SUBST(CONFIG_DOC_WANT_PDF)

  ##-- doxygen vars (compatibility hack)
  DOXY_WANT_MAN="$CONFIG_DOC_WANT_MAN"
  DOXY_WANT_HTML="$CONFIG_DOC_WANT_HTML"
  DOXY_WANT_LATEX="$CONFIG_DOC_WANT_LATEX"
  DOXY_WANT_HTMLHELP="NO"
  AC_SUBST(DOXY_WANT_MAN)
  AC_SUBST(DOXY_WANT_HTML)
  AC_SUBST(DOXY_WANT_LATEX)
  AC_SUBST(DOXY_WANT_HTMLHELP)


  ##-- check for tag-files (this needs an overhaul!)
  #for ext in mootm ; do
  #  extdocdir="`$PKG_CONFIG --variable=pkgdocdir ${ext}`"
  #  if test "$extdocdir" -a "$extdocdir" != "no" ; then
  #    exttagfiles="`find $extdocdir -name '*.tag'`"
  #    for exttag in $exttagfiles ; do
  #      exttagdir="`dirname $exttag`/html"
  #      if test -d "$exttagdir" ; then
  #        DOXY_TAGFILES="$DOXY_TAGFILES $exttag=$exttagdir"
  #      fi
  #    done
  #  fi
  #done 
  AC_SUBST(DOXY_TAGFILES)   
fi; # if "$moot_doc_formats" != "none" ...

##-- extra defines for doxygen
AC_SUBST(DOXY_DEFINES)

##-- conditionals
AM_CONDITIONAL(HAVE_DOXYGEN,     [test -n "$DOXYGEN"     -a "$DOXYGEN"     != "no"])
AM_CONDITIONAL(HAVE_DOXY_FILTER, [test -n "$DOXY_FILTER" -a "$DOXY_FILTER" != "no"])

AM_CONDITIONAL(HAVE_POD2TEXT,    [test -n "$POD2TEXT"  -a "$POD2TEXT"  != "no"])
AM_CONDITIONAL(HAVE_POD2MAN,     [test -n "$POD2MAN"   -a "$POD2MAN"   != "no"]) 
AM_CONDITIONAL(HAVE_POD2HTML,    [test -n "$POD2HTML"  -a "$POD2HTML"  != "no"])
AM_CONDITIONAL(HAVE_POD2LATEX,   [test -n "$POD2LATEX" -a "$POD2LATEX" != "no"])

AM_CONDITIONAL(DOC_WANT_TXT,     [test -n "$CONFIG_DOC_WANT_TXT"   -a "$CONFIG_DOC_WANT_TXT"   != "no"])
AM_CONDITIONAL(DOC_WANT_MAN,     [test -n "$CONFIG_DOC_WANT_MAN"   -a "$CONFIG_DOC_WANT_MAN"   != "no"])
AM_CONDITIONAL(DOC_WANT_HTML,    [test -n "$CONFIG_DOC_WANT_HTML"  -a "$CONFIG_DOC_WANT_HTML"  != "no"])
AM_CONDITIONAL(DOC_WANT_LATEX,   [test -n "$CONFIG_DOC_WANT_LATEX" -a "$CONFIG_DOC_WANT_LATEX" != "no"])
AM_CONDITIONAL(DOC_WANT_DVI,     [test -n "$CONFIG_DOC_WANT_DVI"   -a "$CONFIG_DOC_WANT_DVI"   != "no"])
AM_CONDITIONAL(DOC_WANT_PS,      [test -n "$CONFIG_DOC_WANT_PS"    -a "$CONFIG_DOC_WANT_PS"    != "no"])
AM_CONDITIONAL(DOC_WANT_PDF,     [test -n "$CONFIG_DOC_WANT_PDF"   -a "$CONFIG_DOC_WANT_PDF"   != "no"])

##
## /Documentation
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^



#---------------------------------------------------------------
# Binary distribution
AC_ARG_VAR(BINDIST_RELEASE, [Release number for binary distribution builds])
AC_ARG_VAR(BINDIST_CPU,     [Architecture for binary distribution builds])
AC_ARG_VAR(BINDIST_OS,      [Target OS for binary distribution builds])


test -z "$BINDIST_RELEASE" && BINDIST_RELEASE=1

## -- hack: canonicalize package-name to lower-case (for debian)
BINDIST_PKGNAME=`echo "$PACKAGE" | tr '[[:upper:]]' '[[:lower:]]'`

## -- hack: downgrade ix86 -> i386
if test -z "$BINDIST_CPU" ; then
 case "$target_cpu" in
  i[[3-9]]86)
	BINDIST_CPU=i386
	;;
  *)
	BINDIST_CPU="$target_cpu"
	;;
 esac
fi

test -z "$BINDIST_OS" && BINDIST_OS="$target_os"

AC_SUBST(BINDIST_PKGNAME)
AC_SUBST(BINDIST_RELEASE)
AC_SUBST(BINDIST_CPU)
AC_SUBST(BINDIST_OS)
# Binary distribution
#---------------------------------------------------------------


dnl ---------------------------------------------------------------
dnl cxxflags: template depth
dnl
#CXXFLAGS="$CXXFLAGS -ftemplate-depth-24"
dnl
dnl cxxflags: template depth
dnl ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## trigrams ?
## + v2.0.7 (Thu, 10 Sep 2009 10:12:54 +0200): eliminated
##   - old --disable-trigrams behavior can be simulated by model tweaking
AC_DEFINE(MOOT_USE_TRIGRAMS,1,[Define this to enable use of trigrams (should always be defined for moot >=v2.0.7-0)])
CONFIG_OPTIONS="$CONFIG_OPTIONS USE_TRIGRAMS=2"

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## ngram-hash ?
##  + v2.0.6-3 : moved to runtime option
AC_DEFINE(MOOT_HASH_NGRAMS,1,[Define this to enable sparse n-gram hash (should always be defined for moot >=v2.0.6-3)])
CONFIG_OPTIONS="$CONFIG_OPTIONS HASH_NGRAMS=2"

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## relax ?
## + v2.0.7 (Thu, 10 Sep 2009 10:12:54 +0200): moved to runtime option
AC_DEFINE(MOOT_RELAX,1,[Define this to enable relaxed interpretation of pre-analyses (should always be defined for moot >=v2.0.7-0)])
CONFIG_OPTIONS="$CONFIG_OPTIONS RELAX=2"

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## 32-bit ints?
AC_ARG_ENABLE([32bit-ints],
	AC_HELP_STRING([--disable-32bit-ints],[use native machine words instead of 32-bit integers (default=no)]),
	[],
	[enable_32bit_ints=auto])


AX_RUN_SIZEOF([int],[-1])
AX_RUN_SIZEOF([long int],[-1])
AX_RUN_SIZEOF([size_t],[-1])
if test "$SIZEOF_int" = 4 -a "$SIZEOF_long_int" = 4 -a "$SIZEOF_size_t" = 4 ; then
  native_32bit_ints=yes
  AC_DEFINE(MOOT_32BIT_NATIVE,1,[Define this if your system uses native 32-bit integers])
else
  native_32bit_ints=no
fi

AC_DEFINE(MOOT_32BIT_NATIVE,1,[Define this if your system uses native 32-bit integers])

AC_MSG_CHECKING([whether to force use of 32-bit integers])
if test "$enable_32bit_ints" != "no" ; then
   AC_MSG_RESULT(yes)

   ##-- check for specific types & sizes (breaks NoConfig.h)
   dnl AC_TYPE_INT32_T
   dnl AC_TYPE_UINT32_T

   if test "$enable_32bit_ints" = "yes" -o "$native_32bit_ints" != "yes" ; then
     AC_MSG_NOTICE([attempting to force use of 32-bit integers via preprocessor kludge])
     AC_DEFINE(MOOT_32BIT_FORCE,1,[Define this to force use of 32-bit integers for binary I/O])
   else
     AC_MSG_NOTICE([native word length is already 32-bits; no action required])
   fi
else
  AC_MSG_RESULT(no)
fi
## /32-bit ints
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## warnings ?
##
AC_MSG_CHECKING([whether to display compiler warnings])
AC_ARG_ENABLE(warnings,
	AC_HELP_STRING([--disable-warnings],[disable compiler warnings (default=no)]))

if test "$enable_warnings" != "no" ; then
   AC_MSG_RESULT(yes)
   moot_WFLAGS="-Wall -Wno-sign-compare -Wno-deprecated"
else
  AC_MSG_RESULT(no)
  moot_WFLAGS=""
fi
AC_SUBST(moot_WFLAGS)
##
## /warnings
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## static compiling?
##
AC_ARG_ENABLE([all-static],
	AC_HELP_STRING([--enable-all-static],[link everything statically (default=no)]),
	[ac_cv_enable_all_static="$enableval"],
	[ac_cv_enable_all_static="no"])

if test "$ac_cv_enable_all_static" = "yes" ; then
  MOOT_LIBMOOT_LDADD="\$(top_builddir)/src/libmoot/libmoot.la"
  LDFLAGS="-all-static $LDFLAGS"
else
  MOOT_LIBMOOT_LDADD="-lmoot"
fi
AC_SUBST(MOOT_LIBMOOT_LDADD)
## /static compiling?
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## configuration options
##
AC_SUBST(CONFIG_OPTIONS)
AC_SUBST(moot_LIBS)
##
## /config options
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

dnl v--- needed if Makefile.am uses _LTLIBRARIES targets
AC_PROG_LIBTOOL

dnl
dnl output
dnl
AC_CONFIG_FILES(src/libmoot/gnulib/Makefile)
AC_CONFIG_FILES(src/libmoot/Makefile)
AC_CONFIG_FILES(src/programs/Makefile)
AC_CONFIG_FILES(src/programs/mootconfig, [chmod 0755 src/programs/mootconfig])
AC_CONFIG_FILES(src/Makefile)
dnl AC_CONFIG_FILES(doc/libmoot/mainpage.dox)
dnl AC_CONFIG_FILES(doc/libmoot/doxy-filter.cfg)
dnl AC_CONFIG_FILES(doc/libmoot/doxy-filter.perl)
AC_CONFIG_FILES(doc/libmoot/libmoot.doxy doc/libmoot/Makefile)
AC_CONFIG_FILES(doc/programs/Makefile)
AC_CONFIG_FILES(doc/tutorial/Makefile)
AC_CONFIG_FILES(doc/Makefile)
AC_CONFIG_FILES(config/Makefile)
AC_CONFIG_FILES(m4/Makefile)
AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(moot.pc)
AC_CONFIG_FILES(moot.spec)
AC_OUTPUT
