## File: doc/tutorial/Makefile.am
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

## --- manpages (section 1)
man1_MANS = moottut.1

pkgdoctut_DATA = \
	moottut.css \
	moottut.html \
	corpus.ttt \
	corpus.wdt \
	test.ttt \
	test.txt

##-----------------------------------------------------------------------
## sources
##-----------------------------------------------------------------------

SUFFIXES = .pod .txt .1 .html .tex .dvi .ps .pdf \
	   .123 .lex .clx .t .tt .ttt .mt .mtt .mttt .wd .wdt .mr .mrt

##-----------------------------------------------------------------------
## local hooks
##-----------------------------------------------------------------------
all-local: $(man_MANS)

txt: moottut.txt

man: moottut.1

html: moottut.html

latex: moottut.tex

dvi_doc: moottut.dvi

ps: moottut.ps

pdf: moottut.pdf


##-----------------------------------------------------------------------
## pod conversion rules
##-----------------------------------------------------------------------

POD2MANFLAGS = \
	--center="moot PoS Tagger" \
	--release="moot v$(VERSION)"
#POD2HTMLFLAGS = --podpath="$(srcdir):." --htmlroot="."
POD2HTMLFLAGS = --podpath="$(srcdir):$(srcdir)/../programs:.:./../programs" --htmlroot="." --css="moottut.css"

if HAVE_POD2MAN
.pod.1:
	$(POD2MAN) $(POD2MANFLAGS) --section=1 $< $@
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
## Tagging
##-----------------------------------------------------------------------
.txt.t:
	../../src/programs/mootpp -v0 -o $@ $<

.ttt.123:
	../../src/programs/mootrain -v0 $<

.ttt.lex:
	../../src/programs/mootrain -v0 $<

.wdt.clx:
	../../src/programs/mootrain -v0 $<

.t.tt: corpus.123 corpus.lex
	../../src/programs/moot -v0 -L0 -Mcorpus.lex,corpus.123 -o $@ $< 

.mr.wd: corpus.123 corpus.lex corpus.clx
	../../src/programs/moot -v0 -L1 -Mcorpus -o $@ $< 

##-----------------------------------------------------------------------
## Variables: cleanup
##-----------------------------------------------------------------------w

## --- mostlyclean: built by 'make' & commonly rebuilt
#MOSTLYCLEANFILES =

## --- clean:  built by 'make'
CLEANFILES = \
	*.dvi *.ps *.pdf \
	*.log *.aux *.toc *.idx *~~ *.tmp \
	*.lex *.123

#test.mtt test.tt test.t

if HAVE_POD2MAN
CLEANFILES += *.man *.1 *.5
endif
if HAVE_POD2HTML
CLEANFILES += *.html
endif
if HAVE_POD2TEXT
CLEANFILES += moottut.txt
endif
if HAVE_POD2LATEX
CLEANFILES += *.tex
endif

## --- distclean: built by 'configure'
#DISTCLEANFILES =

## -- maintainerclean: built by maintainer / by hand
MAINTAINERCLEANFILES = \
	*.man *.1 *.5 *.html moottut.txt *.tex \
	*~ .*~ Makefile Makefile.in \
	$(CLEANFILES)

##-----------------------------------------------------------------------
## Variables: distribution
##-----------------------------------------------------------------------

## --- extra distribution files
EXTRA_DIST = \
	$(pkgdoctut_DATA) \
	moottut.1 \
	moottut.pod

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
