## File: doc/programs/Makefile.am
## Package: moot
## Description:
##   + program-doc-level automake file for moocow's PoS tagger programs
##
## Process this file with Automake to create Makefile.in.
##-----------------------------------------------------------------------

##-----------------------------------------------------------------------
## subdirectories
##-----------------------------------------------------------------------
#SUBDIRS =

##-----------------------------------------------------------------------
## primary targets
##-----------------------------------------------------------------------

INDEX_SKEL = @DOC_PROG_INDEX_SKEL@

## --- manpages (section 1)
man1_MANS = @DOC_PROG_MAN1_TARGETS@

man5_MANS = @DOC_PROG_MAN5_TARGETS@

pkgdocprog_DATA = \
	@DOC_PROG_TXT_TARGETS@ \
	@DOC_PROG_HTML_TARGETS@ \
	@DOC_PROG_DVI_TARGETS@ \
	@DOC_PROG_PS_TARGETS@ \
	@DOC_PROG_PDF_TARGETS@

##-----------------------------------------------------------------------
## sources
##-----------------------------------------------------------------------

SUFFIXES = .skel .gog .pod .txt .1 .5 .html .tex .dvi .ps .pdf

##-----------------------------------------------------------------------
## local hooks
##-----------------------------------------------------------------------
all-local: $(man_MANS)

txt: $(moot_pods:.pod=.txt) $(INDEX_SKEL:.skel=.txt)

man: $(man1_MANS) $(MAN5_MANS)

html: $(moot_pods:.pod=.html) $(INDEX_SKEL:.skel=.html)

latex: $(moot_pods:.pod=.tex) $(INDEX_SKEL:.skel=.tex)

dvi-programs: $(moot_pods:.pod=.dvi) $(INDEX_SKEL:.skel=.dvi)

ps: $(moot_pods:.pod=.ps) $(INDEX_SKEL:.skel=.ps)

pdf: $(moot_pods:.pod=.pdf) $(INDEX_SKEL:.skel=.pdf)


##-----------------------------------------------------------------------
## pod compilation rules: Getopt::Gen
##-----------------------------------------------------------------------

.SECONDARY:

OPTGENFLAGS = -l

if HAVE_OPTGEN
mootconfig.pod: mootconfig.gog
	$(OPTGEN_PERL) $(OPTGENFLAGS) --nocfile --nohfile \
	--no-handle-help \
	--no-handle-version \
	--no-handle-rcfile \
	--no-handle-error \
	-F $* $<

.gog.pod:
	$(OPTGEN_PERL) $(OPTGENFLAGS) --nocfile --nohfile -F $* $<
endif

##-----------------------------------------------------------------------
## pod compilation rules: Getopt::Gen (from skel)
##-----------------------------------------------------------------------
OPTGEN_SKELFLAGS = \
	--nocfile --nohfile --nopod \
	--no-handle-help \
	--no-handle-version \
	--no-handle-rcfile \
	--no-handle-error

if HAVE_OPTGEN
.skel.pod:
	echo | $(OPTGEN_PERL) $(OPTGEN_SKELFLAGS) --template=$< - > $@ 2>/dev/null
endif

##-----------------------------------------------------------------------
## pod conversion rules
##-----------------------------------------------------------------------

POD2MANFLAGS = \
	--center="moot PoS Tagger" \
	--release="moot v$(VERSION)"
POD2HTMLFLAGS = --podpath="$(srcdir):." --htmlroot="." --css "mootprogs.css"

if HAVE_POD2MAN
.pod.1:
	$(POD2MAN) $(POD2MANFLAGS) --section=1 $< $@

.pod.5:
	$(POD2MAN) $(POD2MANFLAGS) --section=5 $< $@
endif

if HAVE_POD2TEXT
.pod.txt:
	$(POD2TEXT) $< $@
endif

if HAVE_POD2HTML
.pod.html:
	$(POD2HTML) $(POD2HTMLFLAGS) --infile=$< --outfile=$@
endif

if HAVE_POD2LATEX
.pod.tex:
	$(POD2LATEX) --full --out $@ $<
endif

##-----------------------------------------------------------------------
## latex compilation
##-----------------------------------------------------------------------
.tex.dvi:
	latex $*
	latex $*

.dvi.ps:
	dvips $*

.tex.pdf:
	pdflatex $*
	pdflatex $*

##-----------------------------------------------------------------------
## Variables: cleanup
##-----------------------------------------------------------------------

## --- mostlyclean: built by 'make' & commonly rebuilt
#MOSTLYCLEANFILES =

## --- clean:  built by 'make'
CLEANFILES = \
	*.dvi *.ps *.pdf \
	*.log *.aux *.toc *.idx *~~ *.tmp

if HAVE_OPTGEN
CLEANFILES += \
	$(moot_program_gogs:.gog=.pod) \
	$(INDEX_SKEL:.skel=.pod)
endif
if HAVE_POD2MAN
CLEANFILES += *.man *.1 *.5
endif
if HAVE_POD2HTML
CLEANFILES += *.html
endif
if HAVE_POD2TEXT
CLEANFILES += *.txt
endif
if HAVE_POD2LATEX
CLEANFILES += *.tex
endif

## --- distclean: built by 'configure'
DISTCLEANFILES = \
	*.gog acknowledge.pod

## -- maintainerclean: built by maintainer / by hand
MAINTAINERCLEANFILES = \
	$(moot_program_gogs:.gog=.pod) \
	$(INDEX_SKEL:.skel=.pod) \
	*~ .*~ Makefile Makefile.in \
	*.man *.1 *.5 *.html *.txt *.tex \
	$(CLEANFILES)

##-----------------------------------------------------------------------
## Variables: distribution
##-----------------------------------------------------------------------

## --- extra distribution files
EXTRA_DIST = \
	mootprogs.css \
	mootutils.skel \
	mootutils.pod \
	$(moot_program_gogs:.gog=.pod) \
	mootfiles.pod \
	mootconfig.pod \
	@DOC_PROG_TXT_TARGETS@ \
	@DOC_PROG_MAN1_TARGETS@ \
	@DOC_PROG_MAN5_TARGETS@ \
	@DOC_PROG_HTML_TARGETS@ \
	$(INDEX_SKEL:.skel=.pod) \
	$(INDEX_SKEL:.skel=.html) \
	$(INDEX_SKEL:.skel=.txt) \
	$(INDEX_SKEL:.skel=.1)

## --- recursion subdirectories for 'make dist'
#DIST_SUBDIRS = $(SUBDIRS)

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

##-----------------------------------------------------------------------
## Rules: cleanup
##-----------------------------------------------------------------------
.PHONY: cvsclean

cvsclean: maintainer-clean ;
