## File: src/programs/Makefile.am
## Package: moot
## Description:
##   + src-level automake file for DWDS PoS tagger programs
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

## --- executable programs
#bin_PROGRAMS = ${prog_1} ... ${prog_N}
bin_PROGRAMS = dwdspp dwdsm dwdshmm moot mootrain

old_programs = moot-fstgen moot-pargen

## --- libtool libraries
#lib_LTLIBRARIES = ${lib_1}.la ... ${lib_N}.la

## --- shared data in @pkgdatadir@
#pkgdata_DATA = ${data_1} ... ${data_N}

## --- manpages -- section is auto-detected
#man_MANS = ${man_1} ... ${man_N}

##-----------------------------------------------------------------------
## sources
##-----------------------------------------------------------------------

SUFFIXES = \
	.gog _cmdparser.c _cmdparser.cc _cmdparser.h \
	.pod .txt .1 .html .tex

## --- Required sources
#${prog_i}_SOURCES =
#${lib_i}_la_SOURCES =
dwdspp_SOURCES = \
	cmdutil.cc cmdutil.h \
	dwdspp_main.cc \
	dwdspp_cmdparser.cc dwdspp_cmdparser.h

dwdsm_SOURCES = \
	cmdutil.cc cmdutil.h \
	dwdsm_main.cc \
	dwdsm_cmdparser.cc dwdsm_cmdparser.h

dwdshmm_SOURCES = \
	cmdutil.cc cmdutil.h \
	hmmutil.cc hmmutil.h \
	dwdshmm_main.cc \
	dwdshmm_cmdparser.cc dwdshmm_cmdparser.h

moot_SOURCES = \
	cmdutil.cc cmdutil.h \
	hmmutil.cc hmmutil.h \
	moot_main.cc \
	moot_cmdparser.cc moot_cmdparser.h

mootrain_SOURCES = \
	cmdutil.cc cmdutil.h \
	hmmutil.cc hmmutil.h \
	mootrain_main.cc \
	mootrain_cmdparser.cc mootrain_cmdparser.h


## --- Extra (possible)sources
#EXTRA_${prog_i}_SOURCES = 
#EXTRA_${lib_i}_la_SOURCES = 

## --- Additional dependencies
dwdspp_main.o: dwdspp_cmdparser.h
dwdsm_main.o: dwdsm_cmdparser.h
dwdshmm_main.o: dwdshmm_cmdparser.h hmmutil.h
moot_main.o: moot_cmdparser.h hmmutil.h
mootrain_main.o: mootrain_cmdparser.h hmmutil.h

##-----------------------------------------------------------------------
## headers
##-----------------------------------------------------------------------

## --- installable headers: @includedir@/@pkgname@
#pkginclude_HEADERS = ${hfile_1} ... ${hfile_N}

## --- no-install headers
noinst_HEADERS = config.h

##-----------------------------------------------------------------------
## pre-compile rules: gengetopt
##-----------------------------------------------------------------------

#.ggo.h:
#	gengetopt -u -F$* < $^
#	mv $*.c $*.cc

#.ggo.cc:
#	gengetopt -u -F$* < $^
#	mv $*.c $*.cc

##-----------------------------------------------------------------------
## pre-compile rules: Getopt::Gen
##-----------------------------------------------------------------------

.SECONDARY:

OPTGENFLAGS = -u -l

.gog_cmdparser.h:
	optgen.perl $(OPTGENFLAGS) --nopod -F $*_cmdparser $<
	mv $*_cmdparser.c $*_cmdparser.cc

.gog_cmdparser.cc:
	optgen.perl $(OPTGENFLAGS) --nopod -F $*_cmdparser $<
	mv $*_cmdparser.c $*_cmdparser.cc

.gog.pod:
	optgen.perl $(OPTGENFLAGS) --nocfile --nohfile -F $* $<

##-----------------------------------------------------------------------
## pod conversion
##-----------------------------------------------------------------------

POD2MANFLAGS = --center="KDWDS PoS Tagging Tools"

.pod.1:
	pod2man $(POD2MANFLAGS) --section=1 $< $@

.pod.txt:
	pod2text $< $@

.pod.html:
	pod2html --infile=$< --outfile=$@

.pod.tex:
	pod2latex --full --out $@ $<

##-----------------------------------------------------------------------
## compile flags
##-----------------------------------------------------------------------

## --- preprocessor flags
AM_CPPFLAGS = $(FSM_IFLAGS) -I@srcdir@/../libmoot -I../libmoot

## --- compiler flags (c++)
AM_CXXFLAGS = $(moot_WFLAGS) $(moot_OFLAGS)

##-----------------------------------------------------------------------
## linker flags
##   + library special handling of configure.in vars:
##-----------------------------------------------------------------------
#${prog_i}_LDFLAGS = -L. -static
#${prog_i}_LDADD   = ${non_src_file}.o -lsomelib
#moot_LDFLAGS = -L.

#LDFLAGS_COMMON = @FSM_LFLAGS@ -L../libmoot
LDFLAGS_COMMON = -L../libmoot

#LDADD_COMMON   = -lmoot -lFSMExt @FSM_LIBS@
#LDADD_COMMON   = -lmoot -lFSMExt -lFSM -lFSMSymSpec -lbumble++ @STL_LIBS@ -lz

LDADD_COMMON   = ../libmoot/libmoot.la

dwdspp_LDFLAGS = $(LDFLAGS_COMMON)
dwdspp_LDADD = $(LDADD_COMMON)

dwdsm_LDFLAGS = $(LDFLAGS_COMMON)
dwdsm_LDADD = $(LDADD_COMMON)

dwdshmm_LDFLAGS = $(LDFLAGS_COMMON)
dwdshmm_LDADD = $(LDADD_COMMON)

moot_LDFLAGS = $(LDFLAGS_COMMON)
moot_LDADD = $(LDADD_COMMON)

mootrain_LDFLAGS = $(LDFLAGS_COMMON)
mootrain_LDADD = $(LDADD_COMMON)

#${lib_i}_la_LDFLAGS = -L. -version-info ${lib_i_current}:${lib_i_rev}:${lib_i_age}
#${lib_i}_LIBADD     = -lotherlib

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
	*~ .*~ Makefile Makefile.in \
	*_cmdparser.cc *_cmdparser.h \
	*.pod *.man *.1 *.html *.txt *.tex

##-----------------------------------------------------------------------
## Variables: distribution
##-----------------------------------------------------------------------

## --- extra distribution files
EXTRA_DIST = \
	moot.gog \
	dwdsm.gog \
	dwdspp.gog \
	dwdshmm.gog \
	mootrain.gog

#	moot.sym

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
