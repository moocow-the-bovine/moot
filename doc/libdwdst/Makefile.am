## File: doc/libdwdst/Makefile.am
## Package: dwdst
## Description:
##   + automake file for documentation subdir for
##     KDWDS PoS-tagger library
##
## Process this file with Automake to create Makefile.in.
##-----------------------------------------------------------------------

SUFFIXES = .doxy .stamp .tag .log

##-----------------------------------------------------------------------
## Primaries: documentation
##-----------------------------------------------------------------------

## --- recursion subdirectories
#SUBDIRS =

## -- section 3 manpages
man3_MANS = @DOC_MAN3_MANS@

## --- doxygen hack: doxy config files
DOXYBASE = libdwdst
DOXYFILE = $(DOXYBASE).doxy

## --- doxygen hack: doxy stamp files: dependencies
STAMPFILE = $(DOXYFILE:.doxy=.stamp)
TAGFILE = $(DOXYFILE:.doxy=.tag)

all-local: $(STAMPFILE)

#-----------------------------------------------------------------------
# doxygen hack: sources
#-----------------------------------------------------------------------

## -- directories for prerequisite-search
vpath %.h @top_srcdir@/src/libdwdst @top_builddir@/src/libdwdst

DOXY_INPUTS = \
	dwdstParamLexer.h   dwdstTaggerLexer.h \
	dwdstConfig.h	    dwdstParamParser.h  dwdstTaglistLexer.h \
	dwdstPPLexer.h	    dwdstTagger.h       dwdstTrainer.h

#-----------------------------------------------------------------------
# Additional dependencies
#-----------------------------------------------------------------------

## --- doxygen hacks: dependencies
$(man3_MANS): $(STAMPFILE)

$(STAMPFILE): $(DOXY_INPUTS)

#-----------------------------------------------------------------------
# doxygen hack: build rules (recurse in this directory)
#-----------------------------------------------------------------------
.doxy.stamp:
	@(if test -n "$(DOXYGEN)" ; then \
	  echo "$(DOXYGEN) $<" ;\
	  $(DOXYGEN) $< ;\
	  if test \( -n "$(DOC_LATEX_TARGETS)" -a -d ./latex \) ; then \
	    $(MAKE) -i -C ./latex $(DOC_LATEX_TARGETS) ;\
	  fi ;\
	 else \
	  echo "-------------------------------------------------------------------" ;\
	  echo "doxygen not found or unspecified:" ;\
	  echo "cannot build documentation from $(DOXYFILE)" ;\
	  echo "-------------------------------------------------------------------" ;\
	 fi)
	 touch $@

#-----------------------------------------------------------------------
# Variables: cleanup
#-----------------------------------------------------------------------

## --- mostlyclean: built by 'make' & commonly rebuilt
#MOSTLYCLEANFILES =

## --- clean:  built by 'make'
CLEANFILES = $(STAMPFILE) $(TAGFILE) $(DOXYBASE).log

clean-local:
	rm -rf html latex man

## --- distclean: built by 'configure'
#DISTCLEANFILES =

## -- maintainerclean: built by maintainer / by hand
MAINTAINERCLEANFILES = \
	*~ .*~ Makefile Makefile.in \
	$(DOXYFILE) $(DOXYFILE:.doxy=.log)

#-----------------------------------------------------------------------
# install rules: multi-doxy hack
#-----------------------------------------------------------------------
.PHONY: install-data-local install-doc-sub install-latex-sub

install-data-local:
	if test -n "$(DOC_SUBDIRS)" ; then \
	  $(MAKE) $(AM_MAKEFLAGS) install-doc-sub ;\
	fi
	if test -n "$(DOC_LATEX_TARGETS)" ; then \
	  $(MAKE) $(AM_MAKEFLAGS) install-latex-sub ;\
	fi

install-doc-sub:
	  $(mkinstalldirs) $(DESTDIR)$(pkgdocdir)/$(DOXYBASE)
	  if test -e $(DOXYBASE).tag ; then \
	    $(INSTALL_DATA) $(TAGFILE) $(DESTDIR)$(pkgdocdir)/$(DOXYBASE)/$(DOXYBASE).tag ;\
	  fi
	  for d in $(DOC_SUBDIRS) ; do \
	    if test -d "$$d" ; then \
	      $(mkinstalldirs) $(DESTDIR)$(pkgdocdir)/$(DOXYBASE)/$$d ;\
	      (cd $$d ;\
	       for f in * ; do \
	        $(INSTALL_DATA) $$f $(DESTDIR)$(pkgdocdir)/$(DOXYBASE)/$$d/$$f ;\
	       done) ;\
	    fi ;\
	  done

install-latex-sub:
	  $(mkinstalldirs) $(DESTDIR)$(pkgdocdir)/$(DOXYBASE)
	  for f in $(DOC_LATEX_TARGETS) ; do \
	    $(INSTALL_DATA) latex/$$f $(DESTDIR)$(pkgdocdir)/$(DOXYBASE)/$$f ;\
	  done


#-----------------------------------------------------------------------
# Variables: distribution
#-----------------------------------------------------------------------

## --- extra distribution files
EXTRA_DIST = \
	doxy-filter.perl

## --- recursion subdirectories for 'make dist'
#DIST_SUBDIRS =

#-----------------------------------------------------------------------
# Rules: cleanup
#-----------------------------------------------------------------------
.PHONY: cvsclean

cvsclean: maintainer-clean ;
