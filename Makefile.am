#-----------------------------------------------------------------------
# File: ./Makefile.am
# Package: dwdst
# Description:
#   + top-level automake file for DWDS tagger
#
# Process this file with Automake to create Makefile.in.
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
# Options & Subdirectories
#-----------------------------------------------------------------------

## --- automake options
AUTOMAKE_OPTIONS = foreign dist-bzip2

## --- recursion subdirectories
SUBDIRS = config src doc

## --- pseudo-deps for '.SUFFIXES'
SUFFIXES = .pod .txt

#-----------------------------------------------------------------------
# Variables: cleanup
#-----------------------------------------------------------------------

## --- mostlyclean: built by 'make' & commonly rebuilt
#MOSTLYCLEANFILES =

## --- clean: built by 'make'
#CLEANFILES =

## --- distclean: built by 'configure'
DISTCLEANFILES = \
	config.cache \
	config.log \
	config.status

## -- maintainerclean: built by maintainer / by hand
MAINTAINERCLEANFILES = *~ \
	$(PODS:.pod=.txt) \
	Makefile Makefile.in \
	aclocal.m4 \
	configure

#CVSCLEAN_SUBDIRS = $(SUBDIRS)

#CVSCLEANFILES = Makefile.in Makefile

#-----------------------------------------------------------------------
# Additional Variables & Rules: PODS
#-----------------------------------------------------------------------
PODS = README.pod

.pod.txt:
	pod2text $< $@


#-----------------------------------------------------------------------
# Variables: distribution
#-----------------------------------------------------------------------

## --- extra distribution files
EXTRA_DIST = $(PODS:.pod=.txt) autogen.sh configure

## --- recursion subdirectories for 'make dist'
DIST_SUBDIRS = $(SUBDIRS)

## --- dist-hook: when another 'Makefile.am' is overkill
#DISTHOOK_DIRS = foo
#DISTHOOK_FILES = foo/bar.txt foo/baz.txt
#dist-hook:
#	for d in $(DISTHOOK_DIRS); do\
#	  mkdir -p $(distdir)/$$d ;\
#	done
#	for f in $(DISTHOOK_FILES); do\
#	  cp -p $(srcdir)/$$f $(distdir)/$$f ;\
#	done


#-----------------------------------------------------------------------
# Rules: cleanup
#-----------------------------------------------------------------------
.PHONY: cvsclean cvsclean-hook

cvsclean: maintainer-clean ;

#cvsclean-hook: ;
#	test -z "$(CVSCLEAN_SUBDIRS)" ||\
#	  for d in $(CVSCLEAN_SUBDIRS) ; do\
#	    $(MAKE) -C $$d cvsclean ;\
#	  done
#	rm -f $(CVSCLEANFILES)
