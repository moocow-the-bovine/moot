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

dvi: $(moot_pods:.pod=.dvi) $(INDEX_SKEL:.skel=.dvi)

ps: $(moot_pods:.pod=.ps) $(INDEX_SKEL:.skel=.ps)

pdf: $(moot_pods:.pod=.pdf) $(INDEX_SKEL:.skel=.pdf)


##-----------------------------------------------------------------------
## pod compilation rules: Getopt::Gen
##-----------------------------------------------------------------------

.SECONDARY:

OPTGENFLAGS = -l

mootconfig.pod: mootconfig.gog
	optgen.perl $(OPTGENFLAGS) --nocfile --nohfile \
	--no-handle-help \
	--no-handle-version \
	--no-handle-rcfile \
	--no-handle-error \
	-F $* $<

.gog.pod:
	optgen.perl $(OPTGENFLAGS) --nocfile --nohfile -F $* $<

##-----------------------------------------------------------------------
## pod compilation rules: Getopt::Gen (from skel)
##-----------------------------------------------------------------------
OPTGEN_SKELFLAGS = \
	--nocfile --nohfile --nopod \
	--no-handle-help \
	--no-handle-version \
	--no-handle-rcfile \
	--no-handle-error

.skel.pod:
	echo | optgen.perl $(OPTGEN_SKELFLAGS) --template=$< - > $@ 2>/dev/null

##-----------------------------------------------------------------------
## pod conversion rules
##-----------------------------------------------------------------------

POD2MANFLAGS = \
	--center="moot PoS Tagger" \
	--release="moot v$(VERSION)"
POD2HTMLFLAGS = --podpath="$(srcdir):." --htmlroot="."

.pod.1:
	$(POD2MAN) $(POD2MANFLAGS) --section=1 $< $@

.pod.5:
	$(POD2MAN) $(POD2MANFLAGS) --section=5 $< $@

.pod.txt:
	$(POD2TEXT) $< $@

.pod.html:
	$(POD2HTML) $(POD2HTMLFLAGS) --infile=$< --outfile=$@

.pod.tex:
	$(POD2LATEX) --full --out $@ $<

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
	$(moot_program_gogs:.gog=.pod) \
	$(INDEX_SKEL:.skel=.pod) \
	*.man *.1 *.5 *.html *.txt *.tex *.dvi *.ps *.pdf \
	*.log *.aux *.toc *.idx *~~ *.tmp

## --- distclean: built by 'configure'
DISTCLEANFILES = \
	*.gog acknowledge.pod

## -- maintainerclean: built by maintainer / by hand
MAINTAINERCLEANFILES = \
	*~ .*~ Makefile Makefile.in \
	$(CLEANFILES)

##-----------------------------------------------------------------------
## Variables: distribution
##-----------------------------------------------------------------------

## --- extra distribution files
EXTRA_DIST = \
	mootfiles.pod \
	mootutils.skel \
	@DOC_PROG_TXT_TARGETS@ \
	@DOC_PROG_MAN1_TARGETS@ \
	@DOC_PROG_MAN5_TARGETS@ \
	@DOC_PROG_HTML_TARGETS@

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
