## File: m4/Makefile.am
## Package:
## Description:
##   + automake file for 'm4' package-subdir
##
## Process this file with Automake to create Makefile.in.
##-----------------------------------------------------------------------

##-----------------------------------------------------------------------
## Variables: options
##-----------------------------------------------------------------------

##-----------------------------------------------------------------------
## Variables: cleanup
##-----------------------------------------------------------------------

## --- mostlyclean: built by 'make' & commonly rebuilt
#MOSTLYCLEANFILES = 

## --- clean:  built by 'make'
#CLEANFILES = 

## --- distclean: built by 'configure'
#DISTCLEANFILES = 

## -- maintainerclean: built by maintainer / by hand
MAINTAINERCLEANFILES = \
	*~ .*~ \
	Makefile Makefile.in \
	config.guess \
	config.sub \
	depcomp \
	install-sh \
	ltmain.sh \
	missing \
	mkinstalldirs \
	texinfo.tex

#	ylwrap


##-----------------------------------------------------------------------
## Variables: distribution
##-----------------------------------------------------------------------

## --- extra distribution files
EXTRA_DIST = \
	Makefile.in \
	\
	alloca.m4 \
	eoverflow.m4 \
	gnulib-cache.m4 \
	gnulib-comp.m4 \
	gnulib-tool.m4 \
	intmax_t.m4 \
	inttypes_h.m4 \
	longdouble.m4 \
	longlong.m4 \
	memrchr.m4 \
	onceonly_2_57.m4 \
	signed.m4 \
	size_max.m4 \
	stdint_h.m4 \
	vasnprintf.m4 \
	vasprintf.m4 \
	wchar_t.m4 \
	wint_t.m4 \
	xsize.m4

#	ylwrap
#	ltmain.sh
#	config.guess 
#	config.sub 
#	texinfo.tex

## --- recursion subdirectories for 'make dist'
#DIST_SUBDIRS = $(SUBDIRS)

#-----------------------------------------------------------------------
# Rules: cleanup
#-----------------------------------------------------------------------
.PHONY: cvsclean cvsclean-hook

cvsclean: maintainer-clean ;
