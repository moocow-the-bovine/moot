## File: src/Makefile.am
## Package: moot
## Description:
##   + source-level automake file for DWDS tagger
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
#bin_PROGRAMS = dwdspp moot moot-fstgen moot-pargen

## --- libtool libraries
#lib_LTLIBRARIES = ${lib_1}.la ... ${lib_N}.la
lib_LTLIBRARIES = libmoot.la

## --- shared data in @pkgdatadir@
#pkgdata_DATA = ${data_1} ... ${data_N}

## --- manpages -- section is auto-detected
#man_MANS = ${man_1} ... ${man_N}

##-----------------------------------------------------------------------
## sources
##-----------------------------------------------------------------------

## --- Required sources
#${prog_i}_SOURCES =
#${lib_i}_la_SOURCES =
libmoot_la_SOURCES = \
	mootTaggerLexer.cc \
	mootPPLexer.cc \
	mootNgrams.cc \
	mootNgramsLexer.cc \
	mootNgramsParser.cc \
	mootNgramsCompiler.cc \
	mootLexfreqs.cc \
	mootLexfreqsLexer.cc \
	mootLexfreqsParser.cc \
	mootLexfreqsCompiler.cc \
	mootMorph.cc \
	mootHMM.cc \
	mootCHMM.cc \
	mootHMMTrainer.cc

libmoot_old = \
	mootTagger.cc \
	mootTaglistLexer.cc \
	mootTrainer.cc \
	mootAlphabetLexer.cc \
	mootAlphabetParser.cc \
	mootAlphabetCompiler.cc \
	mootDisambiguator.cc


## --- Extra (possible) sources
#EXTRA_${prog_i}_SOURCES = 
#EXTRA_${lib_i}_la_SOURCES = 

## --- Additional dependencies
mootNgrams.lo: mootNgramsLexer.h mootNgramsParser.h 
mootNgrams.o: mootNgramsLexer.h mootNgramsParser.h
mootNgramsCompiler.lo: mootNgramsLexer.h mootNgramsParser.h 
mootNgramsCompiler.o: mootNgramsLexer.h mootNgramsParser.h
mootNgramsLexer.lo: mootNgramsLexer.h mootNgramsParser.h
mootNgramsLexer.o: mootNgramsLexer.h mootNgramsParser.h
mootNgramsLexer.h: $(FLEXXX_DEPS)
mootNgramsLexer.cc: $(FLEXXX_DEPS)
mootNgramsParser.h: $(BISONXX_DEPS)
mootNgramsParser.cc: $(BISONXX_DEPS)

mootLexfreqs.lo: mootLexfreqsLexer.h mootLexfreqsParser.h 
mootLexfreqs.o: mootLexfreqsLexer.h mootLexfreqsParser.h
mootLexfreqsCompiler.lo: mootLexfreqsLexer.h mootLexfreqsParser.h 
mootLexfreqsCompiler.o: mootLexfreqsLexer.h mootLexfreqsParser.h
mootLexfreqsLexer.lo: mootLexfreqsLexer.h mootLexfreqsParser.h
mootLexfreqsLexer.o: mootLexfreqsLexer.h mootLexfreqsParser.h
mootLexfreqsLexer.h: $(FLEXXX_DEPS)
mootLexfreqsLexer.cc: $(FLEXXX_DEPS)
mootLexfreqsParser.h: $(BISONXX_DEPS)
mootLexfreqsParser.cc: $(BISONXX_DEPS)


#mootDisambiguator.o: mootAlphabetCompiler.h
#mootDisambiguator.lo: mootAlphabetCompiler.h
#mootAlphabetCompiler.lo: mootAlphabetLexer.h mootAlphabetParser.h 
#mootAlphabetCompiler.o: mootAlphabetLexer.h mootAlphabetParser.h
#mootAlphabetLexer.lo: mootAlphabetLexer.h mootAlphabetParser.h
#mootAlphabetLexer.o: mootAlphabetLexer.h mootAlphabetParser.h
#mootAlphabetLexer.h: $(FLEXXX_DEPS)
#mootAlphabetLexer.cc: $(FLEXXX_DEPS)
#mootAlphabetParser.h: $(BISONXX_DEPS)
#mootAlphabetParser.cc: $(BISONXX_DEPS)

##-----------------------------------------------------------------------
## headers
##-----------------------------------------------------------------------

## --- installable headers: @includedir@/@pkgname@
#pkginclude_HEADERS = ${hfile_1} ... ${hfile_N}
pkginclude_HEADERS = \
	mootBinIO.h \
	mootBinStream.h \
	mootEnum.h \
	mootTypes.h \
	mootTaggerLexer.h \
	mootPPLexer.h \
	mootNgramsLexer.h \
	mootNgramsParser.h \
	mootNgramsCompiler.h \
	mootNgrams.h \
	mootLexfreqsLexer.h \
	mootLexfreqsParser.h \
	mootLexfreqsCompiler.h \
	mootLexfreqs.h \
	mootHMM.h \
	mootMorph.h \
	mootCHMM.h \
	mootHMMTrainer.h

pkgheaders_old = \
	mootTagger.h \
	mootTrainer.h \
	mootTaglistLexer.h \
	mootAlphabetLexer.h \
	mootAlphabetParser.h \
	mootAlphabetCompiler.h \
	mootDisambiguator.h


## --- no-install headers
#noinst_HEADERS = nopackage.h

##-----------------------------------------------------------------------
## pre-compile rules: gengetopt ; flex++ ; bison++
##-----------------------------------------------------------------------

SRCDIR = @srcdir@

## --- flex++/bison++ flags
FLEXXX ?= flex++
FLEXXX_SKEL = $(SRCDIR)/flexskel.cc
FLEXXX_HSKEL = $(SRCDIR)/flexskel.h
FLEXXX_DEPS = $(FLEXXX_SKEL) $(FLEXXX_HSKEL)
FLEXXX_FLAGS = -8 -S$(FLEXXX_SKEL) -H$(FLEXXX_HSKEL)
#FLEXXX_FLAGS = -8

BISONXX ?= bison++
BISONXX_SKEL = $(SRCDIR)/bison.cc
BISONXX_HSKEL = $(SRCDIR)/bison.h
BISONXX_DEPS = $(BISONXX_SKEL) $(BISONXX_HSKEL)
BISONXX_FLAGS = $(BISONXX_DFLAGS) -d -v -S$(BISONXX_SKEL) -H$(BISONXX_HSKEL)
#BISONXX_FLAGS = -d -v
#BISONXX_FLAGS = --debug -d -v

.ll.h: $(FLEXXX_DEPS)
	$(FLEXXX) $(FLEXXX_FLAGS) -h$*.h -o$*.cc $<

.ll.cc: $(FLEXXX_DEPS)
	$(FLEXXX) $(FLEXXX_FLAGS) -h$*.h -o$*.cc $<

.yy.cc: $(BISONXX_DEPS)
	$(BISONXX) $(BISONXX_FLAGS) -o $*.cc -h $*.h  $<

.yy.h: $(BISONXX_DEPS)
	$(BISONXX) $(BISONXX_FLAGS) -o $*.cc -h $*.h  $<

##-----------------------------------------------------------------------
## compile flags
##-----------------------------------------------------------------------
#SRCDIR = @srcdir@
#SRCINCLUDE = $(SRCDIR)/../include

## --- preprocessor flags
AM_CPPFLAGS = $(FSM_IFLAGS)

## --- compiler flags (c++)
AM_CXXFLAGS = $(moot_WFLAGS) $(moot_OFLAGS)

##-----------------------------------------------------------------------
## Additional variable: library versioning
##  + for -version-info $(CUR):$(REV):$(AGE) :
##  + base lib is created as libMYLIB.so.$(CUR).$(AGE).$(REV)
##-----------------------------------------------------------------------

## --- The most recent interface number that this library implements.
LIBCUR = 1

## --- The implementation number of the CURRENT interface.
LIBREV = 1

## --- The difference between the newest and oldest interfaces that this
##     library implements.  In other words, the library implements all the
##     interface numbers in the range from number `CURRENT - AGE' to
##     `CURRENT'.
LIBAGE = 0

##-----------------------------------------------------------------------
## linker flags
##   + library special handling of configure.in vars:
##-----------------------------------------------------------------------
#${prog_i}_LDFLAGS = -L. -static
#${prog_i}_LDADD   = ${non_src_file}.o -lsomelib

#${lib_i}_la_LDFLAGS = -L. -version-info ${lib_i_current}:${lib_i_rev}:${lib_i_age}
#${lib_i}_la_LIBADD  = -lotherlib

#libmoot_la_LDFLAGS = @FSM_LFLAGS@
#libmoot_la_LIBADD  = -lFSMExt @FSM_LIBS@

libmoot_la_LDFLAGS = -version-info $(LIBCUR):$(LIBREV):$(LIBAGE)

#libmoot_la_LDFLAGS =
#libmoot_la_LIBADD  = -lFSMExt -lFSM -lFSMSymSpec -lbumble++ @STL_LIBS@ -lz

libmoot_la_LIBADD = @FSMEXT_LIBS@

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
	mootTaggerLexer.cc mootTaggerLexer.h \
	mootPPLexer.cc mootPPLexer.h \
	mootTaglistLexer.cc mootTaglistLexer.h \
	mootAlphabetLexer.cc mootAlphabetLexer.h \
	mootAlphabetParser.cc mootAlphabetParser.h \
	mootNgramsLexer.cc mootNgramsLexer.h \
	mootNgramsParser.cc mootNgramsParser.h \
	mootLexfreqsLexer.cc mootLexfreqsLexer.h \
	mootLexfreqsParser.cc mootLexfreqsParser.h \
	gmon.out

##-----------------------------------------------------------------------
## Variables: distribution
##-----------------------------------------------------------------------

#	moot.sym

## --- extra distribution files
EXTRA_DIST = \
	mootTaggerLexer.ll \
	mootPPLexer.ll \
	mootNgramsLexer.ll \
	mootNgramsParser.yy \
	mootLexfreqsLexer.ll \
	mootLexfreqsParser.yy \
	flexskel.cc flexskel.h \
	bison.cc bison.h

dist_old = \
	mootTaglistLexer.ll \
	mootAlphabetLexer.ll \
	mootAlphabetParser.yy


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
