## File: doc/libmoot/Makefile.am
## Package: moot
## Description:
##   + automake file for documentation subdir for
##     libmoot PoS-tagger library
##
## Process this file with Automake to create Makefile.in.
##-----------------------------------------------------------------------

SUFFIXES = .doxy .stamp .tag .log

##-----------------------------------------------------------------------
## Conditional targets
##-----------------------------------------------------------------------

DOXY_MAN3_MANS=
DOXY_HTML_STATIC=
DOXY_INSTALL_SUBDIRS=
DOXY_LATEX_TARGETS=

if HAVE_DOXYGEN

##-- manpages
## + v2.0.8-3: Mon, 12 Apr 2010 09:34:50 +0200: man page install rule chokes new doxygen
##   (debian sid / doxygen v1.6.2; fedora 11 / doxygen v1.6.1)
#if DOC_WANT_MAN
#DOXY_MAN3_MANS +=man/man3/*
#endif

##-- html docs
if DOC_WANT_HTML
DOXY_HTML_STATIC     +=libmoot.css
DOXY_INSTALL_SUBDIRS +=html
endif

##-- LaTeX
if DOC_WANT_LATEX
if DOC_WANT_DVI
DOXY_LATEX_TARGETS +=refman.dvi
endif
if DOC_WANT_PS
DOXY_LATEX_TARGETS +=refman.ps
endif

if DOC_WANT_PDF
DOXY_LATEX_TARGETS +=refman.pdf
endif
endif
##--/DOC_WANT_LATEX

endif
##--/HAVE_DOXYGEN

##-----------------------------------------------------------------------
## Primaries: documentation (real)
##-----------------------------------------------------------------------

## --- recursion subdirectories
#SUBDIRS =

## --- doxygen hack: doxy config files
DOXYBASE = libmoot
DOXYFILE = $(DOXYBASE).doxy

## --- doxygen hack: doxy stamp files: dependencies
STAMPFILE = $(DOXYFILE:.doxy=.stamp)
TAGFILE   = $(DOXYFILE:.doxy=.tag)

## -- section 3 manpages
man3_MANS = $(DOXY_MAN3_MANS)

all-local: $(STAMPFILE)

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
DOXY_FILTER_DEPS=

if HAVE_DOXY_FILTER
DOXY_FILTER_DEPS += doxy-filter.cfg
endif

if HAVE_DOXYGEN
if DOC_WANT_MAN
$(man3_MANS): $(STAMPFILE)
endif

$(STAMPFILE): $(DOXY_INPUTS) $(DOXY_FILTER_DEPS)
endif


#-----------------------------------------------------------------------
# doxygen hack: build rules (recurse in this directory)
#-----------------------------------------------------------------------
doxy-filter.cfg:
	echo '$$config_cppflags .= "-w -C -x c++ $(patsubst %,-I%,$(DOXY_INCLUDES))";' > $@
	echo '$$config_cpp    = "$(CXXCPP)";' >> $@
	echo '$$config_tmpdir = "$(PWD)";'    >> $@
	echo '1;' >> $@

builddir ?= $(top_builddir)/doc/libmoot

if HAVE_DOXYGEN
.doxy.stamp: doxy-filter.cfg @srcdir@/libmoot.css
	$(DOXYGEN) $<
if DOC_WANT_LATEX
	if test \( -n "$(DOXY_LATEX_TARGETS)" -a -d ./latex \) ; then \
	  for t in $(DOXY_LATEX_TARGETS); do \
	    $(MAKE) -i -C ./latex $$t ;\
	  done;\
	fi
endif
##--/DOC_WANT_LATEX
if DOC_WANT_HTML
	if test \( -d "$(builddir)html" \) ; then \
	  $(INSTALL_DATA) @srcdir@/libmoot.css $(builddir)html/libmoot.css ;\
	fi
endif
##--/DOC_WANT_HTML
	 touch $@
else
##--!HAVE_DOXYGEN
.doxy.stamp:
	@echo "-------------------------------------------------------------------"
	@echo "doxygen support disabled:"
	@echo "not building library documentation from $(DOXYFILE)"
	@echo "-------------------------------------------------------------------"
	touch $@
endif
##--/HAVE_DOXYGEN

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
#DISTCLEANFILES =

## -- maintainerclean: built by maintainer / by hand
MAINTAINERCLEANFILES = \
	*~ .*~ Makefile Makefile.in \
	$(DOXYFILE) $(DOXYFILE:.doxy=.log)

##-----------------------------------------------------------------------
## installation rules: local
##-----------------------------------------------------------------------
.PHONY: install-data-local install-doc-sub install-latex-sub

##-- always install the .doxy file
pkgdoclib_DATA = $(DOXYBASE).doxy


if HAVE_DOXYGEN

pkgdoclib_DATA +=$(DOXYBASE).tag

$(TAGFILE): $(STAMPFILE)

install-data-local:
	if test -n "$(DOXY_INSTALL_SUBDIRS)"; then \
	  $(MAKE) $(AM_MAKEFLAGS) install-doc-sub ;\
	fi

install-doc-sub:
	  $(mkinstalldirs) $(DESTDIR)$(pkgdocdir)/$(DOXYBASE)
	  for d in $(DOXY_INSTALL_SUBDIRS) ; do \
	    if test -d "$$d" ; then \
	      $(mkinstalldirs) $(DESTDIR)$(pkgdocdir)/$(DOXYBASE)/$$d ;\
	      (cd "$$d" ;\
	       for f in * ; do \
	        $(INSTALL_DATA) "$$f" "$(DESTDIR)$(pkgdocdir)/$(DOXYBASE)/$$d/$$f" ;\
	       done) ;\
	    fi ;\
	  done

if DOC_WANT_LATEX
if DOC_WANT_DVI
pkgdoclib_DATA += latex/refman.dvi
endif
if DOC_WANT_PS
pkgdoclib_DATA += latex/refman.ps
endif
if DOC_WANT_PDF
pkgdoclib_DATA += latex/refman.pdf
endif
endif
##--/DOC_WANT_LATEX

endif
##--/HAVE_DOXYGEN

#-----------------------------------------------------------------------
# Variables: distribution
#-----------------------------------------------------------------------

## --- extra distribution files
EXTRA_DIST = \
	doxy-filter.perl \
	mainpage.dox \
	libmoot.css \
	libmoot-header.html

## --- recursion subdirectories for 'make dist'
#DIST_SUBDIRS =

#-----------------------------------------------------------------------
# Rules: cleanup
#-----------------------------------------------------------------------
.PHONY: cvsclean

cvsclean: maintainer-clean ;
