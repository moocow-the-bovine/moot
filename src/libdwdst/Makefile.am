## File: src/Makefile.am
## Package: dwdst
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
#bin_PROGRAMS = dwdspp dwdst dwdst-fstgen dwdst-pargen

## --- libtool libraries
#lib_LTLIBRARIES = ${lib_1}.la ... ${lib_N}.la
lib_LTLIBRARIES = libdwdst.la

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
libdwdst_la_SOURCES = \
	dwdstTaggerLexer.cc \
	dwdstPPLexer.cc \
	dwdstNgrams.cc \
	dwdstNgramsLexer.cc \
	dwdstNgramsParser.cc \
	dwdstNgramsCompiler.cc \
	dwdstLexfreqs.cc \
	dwdstLexfreqsLexer.cc \
	dwdstLexfreqsParser.cc \
	dwdstLexfreqsCompiler.cc \
	dwdstMorph.cc \
	dwdstHMM.cc \
	dwdstCHMM.cc

libdwdst_old = \
	dwdstTagger.cc \
	dwdstTaglistLexer.cc \
	dwdstTrainer.cc \
	dwdstAlphabetLexer.cc \
	dwdstAlphabetParser.cc \
	dwdstAlphabetCompiler.cc \
	dwdstDisambiguator.cc


## --- Extra (possible) sources
#EXTRA_${prog_i}_SOURCES = 
#EXTRA_${lib_i}_la_SOURCES = 

## --- Additional dependencies
#dwdstTrainer.o: dwdstParamCompiler.h
#dwdstTrainer.lo: dwdstParamCompiler.h
dwdstNgrams.lo: dwdstNgramsLexer.h dwdstNgramsParser.h 
dwdstNgrams.o: dwdstNgramsLexer.h dwdstNgramsParser.h
dwdstNgramsCompiler.lo: dwdstNgramsLexer.h dwdstNgramsParser.h 
dwdstNgramsCompiler.o: dwdstNgramsLexer.h dwdstNgramsParser.h
dwdstNgramsLexer.lo: dwdstNgramsLexer.h dwdstNgramsParser.h
dwdstNgramsLexer.o: dwdstNgramsLexer.h dwdstNgramsParser.h
dwdstNgramsLexer.h: $(FLEXXX_DEPS)
dwdstNgramsLexer.cc: $(FLEXXX_DEPS)
dwdstNgramsParser.h: $(BISONXX_DEPS)
dwdstNgramsParser.cc: $(BISONXX_DEPS)

dwdstLexfreqs.lo: dwdstLexfreqsLexer.h dwdstLexfreqsParser.h 
dwdstLexfreqs.o: dwdstLexfreqsLexer.h dwdstLexfreqsParser.h
dwdstLexfreqsCompiler.lo: dwdstLexfreqsLexer.h dwdstLexfreqsParser.h 
dwdstLexfreqsCompiler.o: dwdstLexfreqsLexer.h dwdstLexfreqsParser.h
dwdstLexfreqsLexer.lo: dwdstLexfreqsLexer.h dwdstLexfreqsParser.h
dwdstLexfreqsLexer.o: dwdstLexfreqsLexer.h dwdstLexfreqsParser.h
dwdstLexfreqsLexer.h: $(FLEXXX_DEPS)
dwdstLexfreqsLexer.cc: $(FLEXXX_DEPS)
dwdstLexfreqsParser.h: $(BISONXX_DEPS)
dwdstLexfreqsParser.cc: $(BISONXX_DEPS)


#dwdstDisambiguator.o: dwdstAlphabetCompiler.h
#dwdstDisambiguator.lo: dwdstAlphabetCompiler.h
#dwdstAlphabetCompiler.lo: dwdstAlphabetLexer.h dwdstAlphabetParser.h 
#dwdstAlphabetCompiler.o: dwdstAlphabetLexer.h dwdstAlphabetParser.h
#dwdstAlphabetLexer.lo: dwdstAlphabetLexer.h dwdstAlphabetParser.h
#dwdstAlphabetLexer.o: dwdstAlphabetLexer.h dwdstAlphabetParser.h
#dwdstAlphabetLexer.h: $(FLEXXX_DEPS)
#dwdstAlphabetLexer.cc: $(FLEXXX_DEPS)
#dwdstAlphabetParser.h: $(BISONXX_DEPS)
#dwdstAlphabetParser.cc: $(BISONXX_DEPS)

##-----------------------------------------------------------------------
## headers
##-----------------------------------------------------------------------

## --- installable headers: @includedir@/@pkgname@
#pkginclude_HEADERS = ${hfile_1} ... ${hfile_N}
pkginclude_HEADERS = \
	dwdstBinIO.h \
	dwdstBinStream.h \
	dwdstEnum.h \
	dwdstTypes.h \
	dwdstTaggerLexer.h \
	dwdstPPLexer.h \
	dwdstNgramsLexer.h \
	dwdstNgramsParser.h \
	dwdstNgramsCompiler.h \
	dwdstNgrams.h \
	dwdstLexfreqsLexer.h \
	dwdstLexfreqsParser.h \
	dwdstLexfreqsCompiler.h \
	dwdstLexfreqs.h \
	dwdstHMM.h \
	dwdstCHMM.h \
	dwdstMorph.h

pkgheaders_old = \
	dwdstTagger.h \
	dwdstTrainer.h \
	dwdstTaglistLexer.h \
	dwdstAlphabetLexer.h \
	dwdstAlphabetParser.h \
	dwdstAlphabetCompiler.h \
	dwdstDisambiguator.h


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
AM_CXXFLAGS = $(DWDST_WFLAGS) $(DWDST_OFLAGS)

##-----------------------------------------------------------------------
## Additional variable: library versioning
##  + for -version-info $(CUR):$(REV):$(AGE) :
##  + base lib is created as libMYLIB.so.$(CUR).$(AGE).$(REV)
##-----------------------------------------------------------------------

## --- The most recent interface number that this library implements.
LIBCUR = 1

## --- The implementation number of the CURRENT interface.
LIBREV = 0

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

#libdwdst_la_LDFLAGS = @FSM_LFLAGS@
#libdwdst_la_LIBADD  = -lFSMExt @FSM_LIBS@

libdwdst_la_LDFLAGS = -version-info $(LIBCUR):$(LIBREV):$(LIBAGE)

#libdwdst_la_LDFLAGS =
#libdwdst_la_LIBADD  = -lFSMExt -lFSM -lFSMSymSpec -lbumble++ @STL_LIBS@ -lz

libdwdst_la_LIBADD = @FSMEXT_LIBS@

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
	dwdstTaggerLexer.cc dwdstTaggerLexer.h \
	dwdstPPLexer.cc dwdstPPLexer.h \
	dwdstTaglistLexer.cc dwdstTaglistLexer.h \
	dwdstAlphabetLexer.cc dwdstAlphabetLexer.h \
	dwdstAlphabetParser.cc dwdstAlphabetParser.h \
	dwdstNgramsLexer.cc dwdstNgramsLexer.h \
	dwdstNgramsParser.cc dwdstNgramsParser.h \
	dwdstLexfreqsLexer.cc dwdstLexfreqsLexer.h \
	dwdstLexfreqsParser.cc dwdstLexfreqsParser.h \
	gmon.out

##-----------------------------------------------------------------------
## Variables: distribution
##-----------------------------------------------------------------------

#	dwdst.sym

## --- extra distribution files
EXTRA_DIST = \
	dwdstTaggerLexer.ll \
	dwdstPPLexer.ll \
	dwdstNgramsLexer.ll \
	dwdstNgramsParser.yy \
	dwdstLexfreqsLexer.ll \
	dwdstLexfreqsParser.yy \
	flexskel.cc flexskel.h \
	bison.cc bison.h

dist_old = \
	dwdstTaglistLexer.ll \
	dwdstAlphabetLexer.ll \
	dwdstAlphabetParser.yy


#	dwdst.sym 

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
