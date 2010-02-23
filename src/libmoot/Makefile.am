## File: src/libmoot/Makefile.am
## Package: moot
## Description:
##   + source-level automake file for DWDS tagger
##
## Process this file with Automake to create Makefile.in.
##-----------------------------------------------------------------------

##-----------------------------------------------------------------------
## subdirectories
##-----------------------------------------------------------------------
SUBDIRS = gnulib

##-----------------------------------------------------------------------
## primary targets
##-----------------------------------------------------------------------

## --- executable programs
#bin_PROGRAMS = ${prog_1} ... ${prog_N}

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
	\
	mootSuffixTrie.cc \
	\
	mootToken.cc \
	\
	mootTokenLexer.cc \
	mootTokenIO.cc \
	\
	mootExpatParser.cc \
	mootTokenExpatIO.cc \
	\
	mootXmlDoc.cc \
	mootTokenXmlDoc.cc \
	\
	mootGenericLexer.cc \
	mootPPLexer.cc \
	\
	mootNgrams.cc \
	mootNgramsLexer.cc \
	mootNgramsParser.cc \
	mootNgramsCompiler.cc \
	\
	mootLexfreqs.cc \
	mootLexfreqsLexer.cc \
	mootLexfreqsParser.cc \
	mootLexfreqsCompiler.cc \
	\
	mootClassfreqs.cc \
	mootClassfreqsLexer.cc \
	mootClassfreqsParser.cc \
	mootClassfreqsCompiler.cc \
	\
	mootHMM.cc \
	mootHMMTrainer.cc \
	mootEval.cc \
	mootDynHMM.cc \
	mootMIParser.cc \
	\
	mootUtils.cc \
	mootIO.cc

#	mootCHMM.cc
#	mootEvent.cc
#	mootEventLexer.cc

##----------------------------------------------------
## gnulib sources (hack!)

#libmoot_la_SOURCES += \
#	gnulib/asnprintf.c \
#	gnulib/asprintf.c \
#	gnulib/memrchr.c \
#	gnulib/printf-args.c \
#	gnulib/printf-parse.c \
#	gnulib/vasnprintf.c \
#	gnulib/vasprintf.c


##----------------------------------------------------
## Extra (possible) sources
#EXTRA_${prog_i}_SOURCES = 
#EXTRA_${lib_i}_la_SOURCES = 

fxx_cc_deps =
if !HAVE_OSFCN_H
fxx_cc_deps += osfcn.h
endif

## --- Additional dependencies
mootNgrams.lo: mootNgramsLexer.h mootNgramsParser.h 
mootNgrams.o: mootNgramsLexer.h mootNgramsParser.h
mootNgramsCompiler.lo: mootNgramsLexer.h mootNgramsParser.h 
mootNgramsCompiler.o: mootNgramsLexer.h mootNgramsParser.h
mootNgramsLexer.lo: mootNgramsLexer.h mootNgramsParser.h $(fxx_cc_deps)
mootNgramsLexer.o: mootNgramsLexer.h mootNgramsParser.h $(fxx_cc_deps)
mootNgramsLexer.h: $(FLEXXX_DEPS)
mootNgramsLexer.cc: $(FLEXXX_DEPS)
mootNgramsParser.h: $(BISONXX_DEPS)
mootNgramsParser.cc: $(BISONXX_DEPS)

mootLexfreqs.lo: mootLexfreqsLexer.h mootLexfreqsParser.h $(fxx_cc_deps)
mootLexfreqs.o: mootLexfreqsLexer.h mootLexfreqsParser.h $(fxx_cc_deps)
mootLexfreqsCompiler.lo: mootLexfreqsLexer.h mootLexfreqsParser.h $(fxx_cc_deps)
mootLexfreqsCompiler.o: mootLexfreqsLexer.h mootLexfreqsParser.h $(fxx_cc_deps)
mootLexfreqsLexer.lo: mootLexfreqsLexer.h mootLexfreqsParser.h $(fxx_cc_deps)
mootLexfreqsLexer.o: mootLexfreqsLexer.h mootLexfreqsParser.h $(fxx_cc_deps)
mootLexfreqsLexer.h: $(FLEXXX_DEPS)
mootLexfreqsLexer.cc: $(FLEXXX_DEPS)
mootLexfreqsParser.h: $(BISONXX_DEPS)
mootLexfreqsParser.cc: $(BISONXX_DEPS)

mootClassfreqs.lo: mootClassfreqsLexer.h mootClassfreqsParser.h $(fxx_cc_deps)
mootClassfreqs.o: mootClassfreqsLexer.h mootClassfreqsParser.h $(fxx_cc_deps)
mootClassfreqsCompiler.lo: mootClassfreqsLexer.h mootClassfreqsParser.h 
mootClassfreqsCompiler.o: mootClassfreqsLexer.h mootClassfreqsParser.h
mootClassfreqsLexer.lo: mootClassfreqsLexer.h mootClassfreqsParser.h $(fxx_cc_deps)
mootClassfreqsLexer.o: mootClassfreqsLexer.h mootClassfreqsParser.h $(fxx_cc_deps)
mootClassfreqsLexer.h: $(FLEXXX_DEPS)
mootClassfreqsLexer.cc: $(FLEXXX_DEPS)
mootClassfreqsParser.h: $(BISONXX_DEPS)
mootClassfreqsParser.cc: $(BISONXX_DEPS)

mootTokenLexer.lo: mootTokenLexer.h mootToken.h $(fxx_cc_deps)
mootTokenLexer.o: mootTokenLexer.h mootToken.h $(fxx_cc_deps)
mootTokenIO.lo: mootTokenLexer.h
mootTokenIO.o: mootTokenLexer.h

mootPPLexer.lo: mootPPLexer.h $(fxx_cc_deps)
mootPPLexer.o: mootPPLexer.h $(fxx_cc_deps)

mootEventLexer.lo: mootEventLexer.h mootEvent.h $(fxx_cc_deps)
mootEventLexer.o: mootEventLexer.h mootEvent.h $(fxx_cc_deps)
#mootEventIO.lo: mootEventIO.h mootEventLexer.h
#mootEventIO.o: mootEventIO.h mootEventLexer.h

#mootEventLexer.lo: mootEventLexer.h mootTokenParser.h $(fxx_cc_deps)
#mootEventLexer.o: mootEventLexer.h mootTokenParser.h $(fxx_cc_deps)
#mootEventLexer.h: $(FLEXXX_DEPS)
#mootEventLexer.cc: $(FLEXXX_DEPS)
#mootTokenParser.h: $(BISONXX_DEPS)
#mootTokenParser.cc: $(BISONXX_DEPS)

#mootDisambiguator.o: mootAlphabetCompiler.h
#mootDisambiguator.lo: mootAlphabetCompiler.h
#mootAlphabetCompiler.lo: mootAlphabetLexer.h mootAlphabetParser.h 
#mootAlphabetCompiler.o: mootAlphabetLexer.h mootAlphabetParser.h
#mootAlphabetLexer.lo: mootAlphabetLexer.h mootAlphabetParser.h $(fxx_cc_deps)
#mootAlphabetLexer.o: mootAlphabetLexer.h mootAlphabetParser.h $(fxx_cc_deps)
#mootAlphabetLexer.h: $(FLEXXX_DEPS)
#mootAlphabetLexer.cc: $(FLEXXX_DEPS)
#mootAlphabetParser.h: $(BISONXX_DEPS)
#mootAlphabetParser.cc: $(BISONXX_DEPS)

##----------------------------------------------------
## Additional rules

mootNoConfig.h: mootAutoConfig.h.in
	rm -f "$@"
	cp "$<" "$@"

osfcn.h: myosfcn.h
	cp $< $@

##-----------------------------------------------------------------------
## headers
##-----------------------------------------------------------------------

## --- installable headers: @includedir@/@pkgname@
#pkginclude_HEADERS = ${hfile_1} ... ${hfile_N}
pkginclude_HEADERS = \
	\
	moot.h \
	mootConfig.h \
	mootNoConfig.h \
	mootAutoConfig.h \
	\
	mootAssocVector.h \
	mootTrieVector.h \
	mootSuffixTrie.h \
	\
	mootArgs.h \
	\
	mootIO.h \
	mootCIO.h \
	mootZIO.h \
	mootBufferIO.h \
	mootCxxIO.h \
	mootBinIO.h \
	mootBinHeader.h \
	\
	mootToken.h \
	\
	mootTokenIO.h \
	mootTokenLexer.h \
	\
	mootExpatParser.h \
	mootTokenExpatIO.h \
	mootRecode.h \
	\
	mootXmlDoc.h \
	mootTokenXmlDoc.h \
	\
	mootEnum.h \
	mootTypes.h \
	mootSTLHacks.h \
	\
	mootPPLexer.h \
	mootGenericLexer.h \
	\
	mootNgrams.h \
	mootNgramsLexer.h \
	mootNgramsParser.h \
	mootNgramsCompiler.h \
	\
	mootLexfreqs.h \
	mootLexfreqsLexer.h \
	mootLexfreqsParser.h \
	mootLexfreqsCompiler.h \
	\
	mootClassfreqs.h \
	mootClassfreqsLexer.h \
	mootClassfreqsParser.h \
	mootClassfreqsCompiler.h \
	\
	mootHMM.h \
	mootHMMTrainer.h \
	mootEval.h \
	mootDynHMM.h \
	mootMIParser.h \
	\
	mootUtils.h \
	\
	@INSTALL_OSFCN_H@

#	mootCHMM.h 
#	mootTokenParser.h

EXTRA_HEADERS = osfcn.h

## --- no-install headers
#noinst_HEADERS = nopackage.h

##-----------------------------------------------------------------------
## pre-compile rules: gengetopt ; flex++ ; bison++
##-----------------------------------------------------------------------

SRCDIR = @srcdir@

## --- flex++/bison++ flags
#FLEXXX ?= flex++
FLEXXX_SKEL = $(SRCDIR)/flexskel.cc
FLEXXX_HSKEL = $(SRCDIR)/flexskel.h
FLEXXX_DEPS = $(FLEXXX_SKEL) $(FLEXXX_HSKEL)
FLEXXX_FLAGS = -8 -S$(FLEXXX_SKEL) -H$(FLEXXX_HSKEL)
#FLEXXX_FLAGS = -8

#BISONXX ?= bison++
BISONXX_SKEL = $(SRCDIR)/bison.cc
BISONXX_HSKEL = $(SRCDIR)/bison.h
BISONXX_DEPS = $(BISONXX_SKEL) $(BISONXX_HSKEL)
BISONXX_FLAGS = -S$(BISONXX_SKEL) -H$(BISONXX_HSKEL) -d -v
#BISONXX_FLAGS = -d -v
##BISONXX_FLAGS = --debug -d -v

if HAVE_FLEXXX
.ll.h: $(FLEXXX_DEPS)
	$(FLEXXX) $(FLEXXX_FLAGS) -h$*.h -o$*.cc $<

.ll.cc: $(FLEXXX_DEPS)
	$(FLEXXX) $(FLEXXX_FLAGS) -h$*.h -o$*.cc $<
endif

if HAVE_BISONXX
.yy.cc: $(BISONXX_DEPS)
	$(BISONXX) $(BISONXX_FLAGS) -o $*.cc -h $*.h  $<

.yy.h: $(BISONXX_DEPS)
	$(BISONXX) $(BISONXX_FLAGS) -o $*.cc -h $*.h  $<
endif

##-----------------------------------------------------------------------
## compile flags
##-----------------------------------------------------------------------
#SRCDIR = @srcdir@
#SRCINCLUDE = $(SRCDIR)/../include

## --- preprocessor flags
#AM_CPPFLAGS = $(FSM_IFLAGS)
AM_CPPFLAGS = -I$(SRCDIR)/gnulib

## --- compiler flags (c++)
AM_CXXFLAGS = $(moot_WFLAGS) $(moot_OFLAGS)

##-----------------------------------------------------------------------
## Additional variable: library versioning
##  + for -version-info $(CUR):$(REV):$(AGE) :
##  + base lib is created as libMYLIB.so.$(CUR).$(AGE).$(REV)
##-----------------------------------------------------------------------

## --- The most recent interface number that this library implements.
LIBCUR = 2

## --- The difference between the newest and oldest interfaces that this
##     library implements.  In other words, the library implements all the
##     interface numbers in the range from number `CURRENT - AGE' to
##     `CURRENT'.
LIBAGE = 0

## --- The implementation number of the CURRENT interface.
LIBREV = 0

##-----------------------------------------------------------------------
## linker flags
##   + library special handling of configure.in vars:
##-----------------------------------------------------------------------
#${prog_i}_LDFLAGS = -L. -static
#${prog_i}_LDADD   = ${non_src_file}.o -lsomelib

#${lib_i}_la_LDFLAGS = -L. -version-info ${lib_i_current}:${lib_i_rev}:${lib_i_age}
#${lib_i}_la_LIBADD  = -lotherlib

#libmoot_la_LDFLAGS = -version-info $(LIBCUR):$(LIBREV):$(LIBAGE)

#libmoot_la_LDFLAGS = -version-info $(LIBCUR):$(LIBREV):$(LIBAGE) -rpath $(libdir)
libmoot_la_LDFLAGS = -version-info $(LIBCUR):$(LIBREV):$(LIBAGE)

libmoot_la_LIBADD = gnulib/libgnu.la

##-----------------------------------------------------------------------
## Variables: cleanup
##-----------------------------------------------------------------------

## --- mostlyclean: built by 'make' & commonly rebuilt
#MOSTLYCLEANFILES =

## --- clean:  built by 'make'
CLEANFILES = osfcn.h

## --- distclean: built by 'configure'
#DISTCLEANFILES =

## -- maintainerclean: built by maintainer / by hand
MAINTAINERCLEANFILES = \
	*~ .*~ Makefile Makefile.in \
	\
	mootEventLexer.cc mootEventLexer.h \
	mootTokenLexer.cc mootTokenLexer.h \
	mootTokenParser.cc mootTokenParser.h \
	\
	mootPPLexer.cc mootPPLexer.h \
	\
	mootTaglistLexer.cc mootTaglistLexer.h \
	\
	mootAlphabetLexer.cc mootAlphabetLexer.h \
	mootAlphabetParser.cc mootAlphabetParser.h \
	\
	mootNgramsLexer.cc mootNgramsLexer.h \
	mootNgramsParser.cc mootNgramsParser.h \
	\
	mootLexfreqsLexer.cc mootLexfreqsLexer.h \
	mootLexfreqsParser.cc mootLexfreqsParser.h \
	\
	mootClassfreqsLexer.cc mootClassfreqsLexer.h \
	mootClassfreqsParser.cc mootClassfreqsParser.h \
	\
	gmon.out

##-----------------------------------------------------------------------
## Variables: distribution
##-----------------------------------------------------------------------

#	moot.sym

## --- extra distribution files
EXTRA_DIST = \
	mootNoConfig.h \
	mootPPLexer.ll \
	mootTokenLexer.ll \
	mootNgramsLexer.ll \
	mootNgramsParser.yy \
	mootLexfreqsLexer.ll \
	mootLexfreqsParser.yy \
	mootClassfreqsLexer.ll \
	mootClassfreqsParser.yy \
	$(FLEXXX_DEPS) \
	$(BISONXX_DEPS) \
	myosfcn.h \
	gnulib/config.h

dist_old = \
	mootTokenLexer.ll \
	mootTokenParser.yy \
	mootTaglistLexer.ll \
	mootAlphabetLexer.ll \
	mootAlphabetParser.yy \
	flexskel.cc flexskel.h \
	bison.cc bison.h


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
