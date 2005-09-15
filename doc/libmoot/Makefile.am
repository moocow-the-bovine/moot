## File: doc/libmoot/Makefile.am
## Package: moot
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
DOXYBASE = libmoot
DOXYFILE = $(DOXYBASE).doxy

## --- doxygen hack: doxy stamp files: dependencies
STAMPFILE = $(DOXYFILE:.doxy=.stamp)
TAGFILE = $(DOXYFILE:.doxy=.tag)

all-local: $(STAMPFILE) doxy-filter.cfg

#-----------------------------------------------------------------------
# doxygen hack: sources
#-----------------------------------------------------------------------

## -- directories for prerequisite-search
DOXY_INCLUDES = $(top_srcdir)/src/libmoot $(top_builddir)/src/libmoot
vpath %.h $(DOXY_INCLUDES)

DOXY_INPUTS = \
	mainpage.dox \
	$(foreach dir,$(DOXY_INCLUDES),$(wildcard $(dir)/*.h))

#	mootCHMM.h
#	mootMorph.h

#-----------------------------------------------------------------------
# Additional dependencies
#-----------------------------------------------------------------------

## --- doxygen hacks: dependencies
$(man3_MANS): $(STAMPFILE)

$(STAMPFILE): $(DOXY_INPUTS) doxy-filter.cfg

#-----------------------------------------------------------------------
# doxygen hack: build rules (recurse in this directory)
#-----------------------------------------------------------------------
doxy-filter.cfg:
	echo -e '$$config_cppflags .= "$(patsubst %,-I%,$(DOXY_INCLUDES))";\n1;' > doxy-filter.cfg

builddir ?= $(top_builddir)/doc/libmoot

.doxy.stamp: doxy-filter.cfg
	@(if test -n "$(DOXYGEN)" ; then \
	  echo "$(DOXYGEN) $<" ;\
	  $(DOXYGEN) $< ;\
	  if test \( -n "$(DOC_LATEX_TARGETS)" -a -d ./latex \) ; then \
	    $(MAKE) -i -C ./latex $(DOC_LATEX_TARGETS) ;\
	  fi ;\
	  if test \( -d "$(builddir)/html" \) ; then \
	    $(INSTALL_DATA) @srcdir@/libmoot.css $(builddir)/html/libmoot.css ;\
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
CLEANFILES = $(STAMPFILE) $(TAGFILE) $(DOXYBASE).log \
	doxy-filter.log \
	doxy-filter.cfg

clean-local:
	rm -rf html latex man

## --- distclean: built by 'configure'
DISTCLEANFILES =

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
	mainpage.dox \
	doxy-filter.perl \
	libmoot.css \
	libmoot-header.html

## --- recursion subdirectories for 'make dist'
#DIST_SUBDIRS =

#-----------------------------------------------------------------------
# Rules: cleanup
#-----------------------------------------------------------------------
.PHONY: cvsclean

cvsclean: maintainer-clean ;
