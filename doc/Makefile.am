## File: doc/Makefile.am
## Package: moot
## Description:
##   + automake file for doc/ subdir of KDWDS PoS-tagger library
##
## Process this file with Automake to create Makefile.in.
##-----------------------------------------------------------------------

##-----------------------------------------------------------------------
## Primaries: headers
##-----------------------------------------------------------------------

## --- recursion subdirectories
SUBDIRS = libmoot programs tutorial

#-----------------------------------------------------------------------
# Variables: cleanup
#-----------------------------------------------------------------------

## --- mostlyclean: built by 'make' & commonly rebuilt
#MOSTLYCLEANFILES =

## --- clean:  built by 'make'
#CLEANFILES =

## --- distclean: built by 'configure'
#DISTCLEANFILES =

## -- maintainerclean: built by maintainer / by hand
MAINTAINERCLEANFILES = *~ .*~ Makefile Makefile.in

#-----------------------------------------------------------------------
# Variables: (un)install
#-----------------------------------------------------------------------
pkgdoc_UNINSTALL_FILES = libmoot programs tutorial README.txt
uninstall-local:
	for f in $(pkgdoc_UNINSTALL_FILES) ; do \
	   rm -rf $(DESTDIR)$(pkgdocdir)/$$f ;\
	done

#-----------------------------------------------------------------------
# Variables: distribution
#-----------------------------------------------------------------------

## --- extra distribution files
#EXTRA_DIST = $(man_MANS) $(pkgdata_DATA)

## --- recursion subdirectories for 'make dist'
#DIST_SUBDIRS =

#-----------------------------------------------------------------------
# Rules: cleanup
#-----------------------------------------------------------------------
.PHONY: cvsclean

cvsclean: maintainer-clean ;
