## File: src/programs/Makefile.am
## Package: dwdst
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
bin_PROGRAMS = dwdspp dwdst dwdst-fstgen dwdst-pargen

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

dwdst_SOURCES = \
	cmdutil.cc cmdutil.h \
	dwdst_main.cc \
	dwdst_cmdparser.cc dwdst_cmdparser.h

dwdst_fstgen_SOURCES = \
	dwdst-fstgen_main.cc \
	dwdst-fstgen_cmdparser.cc dwdst-fstgen_cmdparser.h

dwdst_pargen_SOURCES = \
	cmdutil.cc cmdutil.h \
	dwdst-pargen_main.cc \
	dwdst-pargen_cmdparser.cc dwdst-pargen_cmdparser.h

## --- Extra (possible) sources
#EXTRA_${prog_i}_SOURCES = 
#EXTRA_${lib_i}_la_SOURCES = 

## --- Additional dependencies
dwdspp_main.o: dwdspp_cmdparser.h
dwdst_main.o: dwdst_cmdparser.h
dwdst-fstgen_main.o: dwdst-fstgen_cmdparser.h
dwdst-pargen_main.o: dwdst-pargen_cmdparser.h

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
AM_CPPFLAGS = $(FSM_IFLAGS) -I@srcdir@/../libdwdst -I../libdwdst

## --- compiler flags (c++)
AM_CXXFLAGS = $(FSM_WFLAGS) $(FSM_OFLAGS)

##-----------------------------------------------------------------------
## linker flags
##   + library special handling of configure.in vars:
##-----------------------------------------------------------------------
#${prog_i}_LDFLAGS = -L. -static
#${prog_i}_LDADD   = ${non_src_file}.o -lsomelib
#dwdst_LDFLAGS = -L.

LDFLAGS_COMMON = @FSM_LFLAGS@ -L../libdwdst
LDADD_COMMON   = -ldwdst -lFSMExt @FSM_LIBS@

dwdst_LDFLAGS = $(LDFLAGS_COMMON)
dwdst_LDADD = $(LDADD_COMMON)

dwdst_fstgen_LDFLAGS = $(LDFLAGS_COMMON)
dwdst_fstgen_LDADD = $(LDADD_COMMON)

dwdst_pargen_LDFLAGS = $(LDFLAGS_COMMON)
dwdst_pargen_LDADD = $(LDADD_COMMON)

dwdspp_LDFLAGS = $(LDFLAGS_COMMON)
dwdspp_LDADD = $(LDADD_COMMON)

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
	dwdst.sym \
	dwdst.gog \
	dwdst-fstgen.gog \
	dwdst-pargen.gog \
	dwdspp.gog

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
