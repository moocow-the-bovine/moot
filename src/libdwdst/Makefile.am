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
	dwdstParamLexer.cc \
	dwdstParamParser.cc \
	dwdstParamCompiler.cc \
	dwdstTaglistLexer.cc \
	dwdstTagger.cc \
	dwdstTrainer.cc

## --- Extra (possible) sources
#EXTRA_${prog_i}_SOURCES = 
#EXTRA_${lib_i}_la_SOURCES = 

## --- Additional dependencies
dwdstTrainer.o: dwdstParamCompiler.h
dwdstTrainer.lo: dwdstParamCompiler.h
dwdstParamCompiler.lo: dwdstParamLexer.h dwdstParamParser.h
dwdstParamCompiler.o: dwdstParamLexer.h dwdstParamParser.h
dwdstParamLexer.lo: dwdstParamLexer.h dwdstParamParser.h
dwdstParamLexer.o: dwdstParamLexer.h dwdstParamParser.h

##-----------------------------------------------------------------------
## headers
##-----------------------------------------------------------------------

## --- installable headers: @includedir@/@pkgname@
#pkginclude_HEADERS = ${hfile_1} ... ${hfile_N}
pkginclude_HEADERS = \
	dwdstTypes.h \
	dwdstTagger.h \
	dwdstTaggerLexer.h \
	dwdstPPLexer.h \
	dwdstParamLexer.h \
	dwdstParamParser.h \
	dwdstParamCompiler.h \
	dwdstTrainer.h \
	dwdstTaglistLexer.h

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
FLEXXX_FLAGS = -8 -S$(FLEXXX_SKEL) -H$(FLEXXX_HSKEL)
#FLEXXX_FLAGS = -8

BISONXX ?= bison++
BISONXX_SKEL = $(SRCDIR)/bison.cc
BISONXX_HSKEL = $(SRCDIR)/bison.h
BISONXX_FLAGS = $(BISONXX_DFLAGS) -d -v -S$(BISONXX_SKEL) -H$(BISONXX_HSKEL)
#BISONXX_FLAGS = -d -v
#BISONXX_FLAGS = --debug -d -v

.ll.h:
	$(FLEXXX) $(FLEXXX_FLAGS) -h$*.h -o$*.cc $<

.ll.cc:
	$(FLEXXX) $(FLEXXX_FLAGS) -h$*.h -o$*.cc $<

.yy.cc:
	$(BISONXX) $(BISONXX_FLAGS) -o $*.cc -h $*.h  $<

.yy.h:
	$(BISONXX) $(BISONXX_FLAGS) -o $*.cc -h $*.h  $<

##-----------------------------------------------------------------------
## compile flags
##-----------------------------------------------------------------------
#SRCDIR = @srcdir@
#SRCINCLUDE = $(SRCDIR)/../include

## --- preprocessor flags
AM_CPPFLAGS = $(FSM_IFLAGS)

## --- compiler flags (c++)
AM_CXXFLAGS = $(FSM_WFLAGS) $(FSM_OFLAGS)

##-----------------------------------------------------------------------
## Additional variable: library versioning
##  + for -version-info $(CUR):$(REV):$(AGE) :
##  + base lib is created as libMYLIB.so.$(CUR).$(AGE).$(REV)
##-----------------------------------------------------------------------

## --- The most recent interface number that this library implements.
LIBCUR = 0

## --- The implementation number of the CURRENT interface.
LIBREV = 3

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
libdwdst_la_LDFLAGS = @FSM_LFLAGS@
libdwdst_la_LIBADD  = -lFSMExt @FSM_LIBS@

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
	dwdstTaglistLexer.cc dwdstTaglistLexer.h

##-----------------------------------------------------------------------
## Variables: distribution
##-----------------------------------------------------------------------

## --- extra distribution files
EXTRA_DIST = \
	dwdst.sym \
	dwdstTaggerLexer.ll \
	dwdstPPLexer.ll \
	dwdstTaglistLexer.ll \
	dwdstParamLexer.ll \
	dwdstParamParser.yy \
	flexskel.cc flexskel.h \
	bison.cc bison.h

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
