## File: src/programs/Makefile.am
## Package: moot
## Description:
##   + src-level automake file for moot PoS tagger programs
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
#bin_PROGRAMS = mootpp mootm moothmm moot mootrain
#bin_PROGRAMS = mootpp moothmm moot mootrain
bin_PROGRAMS = mootpp mootrain mootcompile mootdump moot mooteval mootchurn moottaste mootdyn
#mootngsmooth
#mootconfig

#bin_SCRIPTS = ${script1} ... ${scriptN}
bin_SCRIPTS = mootconfig

## --- libtool libraries
#lib_LTLIBRARIES = ${lib_1}.la ... ${lib_N}.la

## --- shared data in @pkgdatadir@
#pkgdata_DATA = ${data_1} ... ${data_N}

## --- manpages -- section is auto-detected
#man_MANS = ${man_1} ... ${man_N}

##-----------------------------------------------------------------------
## headers
##-----------------------------------------------------------------------

## --- installable headers: @includedir@/@pkgname@
#pkginclude_HEADERS = ${hfile_1} ... ${hfile_N}

## --- no-install headers
noinst_HEADERS = config.h

##-----------------------------------------------------------------------
## sources
##-----------------------------------------------------------------------

SUFFIXES = \
	.gog _cmdparser.c _cmdparser.cc _cmdparser.h \
	.pod .txt .1 .html .tex

## --- Required sources
#${prog_i}_SOURCES =
#${lib_i}_la_SOURCES =

## --- Extra (possible)sources
#EXTRA_${prog_i}_SOURCES = 
#EXTRA_${lib_i}_la_SOURCES = 

##-----------------------------------------------------------------------
## linker flags
##   + library special handling of configure.in vars:
##-----------------------------------------------------------------------
#${prog_i}_LDFLAGS = -L. -static
#${prog_i}_LDADD   = ${non_src_file}.o -lsomelib
#moot_LDFLAGS = -L.

#LDFLAGS_COMMON = @FSM_LFLAGS@ -L../libmoot
LDFLAGS_COMMON = -L../libmoot

#LDADD_COMMON   = ../libmoot/libmoot.la @moot_LIBS@
LDADD_COMMON   = @MOOT_LIBMOOT_LDADD@ @moot_LIBS@


##-----------------------------------------------------------------------
## Variables: distribution
##-----------------------------------------------------------------------

## --- extra distribution files
EXTRA_DIST = acknowledge.pod

##~~~~~~~~~ NEW
##-----------------------------------------------------------------------
## Program-dependent vars, by program

#~~~~~~~~
#mootconfig_SOURCES = mootconfig.cc
#mootconfig_LDFLAGS = $(LDFLAGS_COMMON)
#mootconfig_LDADD =
EXTRA_DIST += mootconfig.gog

#~~~~~~~~
mootpp_SOURCES = \
	mootpp_main.cc \
	mootpp_cmdparser.cc mootpp_cmdparser.h
mootpp_main.o: mootpp_cmdparser.h
mootpp_LDFLAGS = $(LDFLAGS_COMMON)
mootpp_LDADD = $(LDADD_COMMON)
EXTRA_DIST += mootpp.gog

#~~~~~~~~
mootrain_SOURCES = \
	mootrain_main.cc \
	mootrain_cmdparser.cc mootrain_cmdparser.h
mootrain_main.o: mootrain_cmdparser.h
mootrain_LDFLAGS = $(LDFLAGS_COMMON)
mootrain_LDADD = $(LDADD_COMMON)
EXTRA_DIST += mootrain.gog

#~~~~~~~~
mootcompile_SOURCES = \
	mootcompile_main.cc \
	mootcompile_cmdparser.cc mootcompile_cmdparser.h \
	computils.h computils.cc
mootcompile_main.o: mootcompile_cmdparser.h computils.h
mootcompile_LDFLAGS = $(LDFLAGS_COMMON)
mootcompile_LDADD = $(LDADD_COMMON)
EXTRA_DIST += mootcompile.gog

#~~~~~~~~
mootdump_SOURCES = \
	mootdump_main.cc \
	mootdump_cmdparser.cc mootdump_cmdparser.h
mootdump_main.o: mootdump_cmdparser.h
mootdump_LDFLAGS = $(LDFLAGS_COMMON)
mootdump_LDADD = $(LDADD_COMMON)
EXTRA_DIST += mootdump.gog

#~~~~~~~~
moot_SOURCES = \
	moot_main.cc \
	moot_cmdparser.cc moot_cmdparser.h \
	computils.cc computils.h
moot_main.o: moot_cmdparser.h computils.h
moot_LDFLAGS = $(LDFLAGS_COMMON)
moot_LDADD = $(LDADD_COMMON)
EXTRA_DIST += moot.gog

#~~~~~~~~
mooteval_SOURCES = \
	mooteval_main.cc \
	mooteval_cmdparser.cc mooteval_cmdparser.h
mooteval_main.o: mooteval_cmdparser.h
mooteval_LDFLAGS = $(LDFLAGS_COMMON)
mooteval_LDADD = $(LDADD_COMMON)
EXTRA_DIST += mooteval.gog

#~~~~~~~~
mootchurn_SOURCES = \
	mootchurn_main.cc \
	mootchurn_cmdparser.cc mootchurn_cmdparser.h
mootchurn_main.o: mootchurn_cmdparser.h
mootchurn_LDFLAGS = $(LDFLAGS_COMMON)
mootchurn_LDADD = $(LDADD_COMMON)
EXTRA_DIST += mootchurn.gog

#~~~~~~~~
moottaste_SOURCES = \
	moottaste_main.cc \
	moottaste_cmdparser.cc moottaste_cmdparser.h
moottaste_main.o: moottaste_cmdparser.h
moottaste_LDFLAGS = $(LDFLAGS_COMMON)
moottaste_LDADD = $(LDADD_COMMON)
EXTRA_DIST += moottaste.gog

#~~~~~~~~
#mootngsmooth_SOURCES = \
#	mootngsmooth_main.cc \
#	mootngsmooth_cmdparser.cc mootngsmooth_cmdparser.h
#mootngsmooth_main.o: mootngsmooth_cmdparser.h
#mootngsmooth_LDFLAGS = $(LDFLAGS_COMMON)
#mootngsmooth_LDADD = $(LDADD_COMMON)
#EXTRA_DIST += mootngsmooth.gog

#~~~~~~~~
mootdyn_SOURCES = \
	mootdyn_main.cc \
	mootdyn_cmdparser.cc mootdyn_cmdparser.h
mootdyn_main.o: mootdyn_cmdparser.h
mootdyn_LDFLAGS = $(LDFLAGS_COMMON)
mootdyn_LDADD = $(LDADD_COMMON)
EXTRA_DIST += mootdyn.gog


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

if HAVE_OPTGEN

.gog_cmdparser.h:
	optgen.perl $(OPTGENFLAGS) --nopod -F $*_cmdparser $<
	mv $*_cmdparser.c $*_cmdparser.cc

.gog_cmdparser.cc:
	optgen.perl $(OPTGENFLAGS) --nopod -F $*_cmdparser $<
	mv $*_cmdparser.c $*_cmdparser.cc

.gog.pod:
	optgen.perl $(OPTGENFLAGS) --nocfile --nohfile -F $* $<

endif

##-----------------------------------------------------------------------
## pod conversion
##-----------------------------------------------------------------------

POD2MANFLAGS = --center="moocow's tagging tools"

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
AM_CPPFLAGS = -I@srcdir@/../libmoot -I@srcdir@/../libmoot/gnulib -I../libmoot

## --- compiler flags (c++)
AM_CXXFLAGS = $(moot_WFLAGS) $(moot_OFLAGS)

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
DISTCLEANFILES = \
	mootconfig

## -- maintainerclean: built by maintainer / by hand
MAINTAINERCLEANFILES = \
	*~ .*~ Makefile Makefile.in \
	*_cmdparser.cc *_cmdparser.h \
	$(filter-out acknowledge.pod,$(wildcard *.pod)) \
	*.man *.1 *.html *.txt *.tex

##-----------------------------------------------------------------------
## More: distribution
##-----------------------------------------------------------------------

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
