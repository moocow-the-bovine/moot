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

SUFFIXES = .ggo

## --- Required sources
#${prog_i}_SOURCES =
#${lib_i}_la_SOURCES =
dwdspp_SOURCES = \
	cmdutil.cc \
	cmdutil.h \
	dwdspp_lexer.ll \
	dwdspp_main.cc \
	dwdspp_cmdparser.ggo

dwdst_SOURCES = \
	cmdutil.cc \
	cmdutil.h \
	dwdst_lexer.ll \
	dwdst.cc \
	dwdst.h \
	dwdst_main.cc \
	dwdst_cmdparser.ggo

dwdst_fstgen_SOURCES = \
	dwdst_lexer.ll \
	dwdst.cc \
	dwdst.h \
	dwdst_trainer.cc \
	dwdst_trainer.h \
	dwdst_fstgen_main.cc \
	dwdst_fstgen_cmdparser.ggo \
	postag_lexer.ll

dwdst_pargen_SOURCES = \
	cmdutil.cc \
	cmdutil.h \
	dwdst_lexer.ll \
	dwdst.cc \
	dwdst.h \
	dwdst_trainer.cc \
	dwdst_fstgen.h \
	dwdst_pargen_main.cc \
	dwdst_pargen_cmdparser.ggo \
	postag_lexer.ll

## --- Extra (possible) sources
#EXTRA_${prog_i}_SOURCES = 
#EXTRA_${lib_i}_la_SOURCES = 

## --- Additional dependencies
dwdspp_main.o: dwdspp_cmdparser.h dwdspp_lexer.h
dwdst_main.o: dwdst_cmdparser.h dwdst_lexer.h
dwdst_fstgen_main.o: dwdst_fstgen_cmdparser.h dwdst_lexer.h postag_lexer.h
dwdst_pargen_main.o: dwdst_pargen_cmdparser.h dwdst_lexer.h postag_lexer.h

##-----------------------------------------------------------------------
## headers
##-----------------------------------------------------------------------

## --- installable headers: @includedir@/@pkgname@
#pkginclude_HEADERS = ${hfile_1} ... ${hfile_N}

## --- no-install headers
noinst_HEADERS = nopackage.h

##-----------------------------------------------------------------------
## pre-compile rules: gengetopt ; flex++
##-----------------------------------------------------------------------

.ggo.h:
	gengetopt -u -F$* < $^
	mv $*.c $*.cc

.ggo.cc:
	gengetopt -u -F$* < $^
	mv $*.c $*.cc

.ll.h:
	flex++ -8 -h$*.h -o$*.cc $<

.ll.cc:
	flex++ -8 -h$*.h -o$*.cc $<

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
#LIBCUR = 0

## --- The implementation number of the CURRENT interface.
#LIBREV = 0

## --- The difference between the newest and oldest interfaces that this
##     library implements.  In other words, the library implements all the
##     interface numbers in the range from number `CURRENT - AGE' to
##     `CURRENT'.
#LIBAGE = 0

##-----------------------------------------------------------------------
## linker flags
##   + library special handling of configure.in vars:
##-----------------------------------------------------------------------
#${prog_i}_LDFLAGS = -L. -static
#${prog_i}_LDADD   = ${non_src_file}.o -lsomelib
#dwdst_LDFLAGS = -L.
dwdst_LDFLAGS = @FSM_LFLAGS@
dwdst_LDADD = @FSM_LIBS@

dwdst_fstgen_LDFLAGS = @FSM_LFLAGS@
dwdst_fstgen_LDADD = @FSM_LIBS@

dwdst_pargen_LDFLAGS = @FSM_LFLAGS@
dwdst_pargen_LDADD = @FSM_LIBS@

# dwdspp_LDFLAGS = @FSM_LFLAGS@
# dwdspp_LDADD = @FSM_LIBS@

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
	dwdst_lexer.cc dwdst_lexer.h \
	dwdspp_lexer.cc dwdspp_lexer.h \
	postag_lexer.cc postag_lexer.h \
	dwdst_cmdparser.cc dwdst_cmdparser.h \
	dwdst_fstgen_cmdparser.cc dwdst_fstgen_cmdparser.h \
	dwdst_pargen_cmdparser.cc dwdst_pargen_cmdparser.h \
	dwdspp_cmdparser.cc dwdspp_cmdparser.h

##-----------------------------------------------------------------------
## Variables: distribution
##-----------------------------------------------------------------------

## --- extra distribution files
EXTRA_DIST = \
	dwdst.sym \
	dwdst_lexer.cc dwdst_lexer.h \
	dwdspp_lexer.cc dwdspp_lexer.h \
	postag_lexer.cc postag_lexer.h \
	dwdst_cmdparser.cc dwdst_cmdparser.h \
	dwdst_fstgen_cmdparser.cc dwdst_fstgen_cmdparser.h \
	dwdst_pargen_cmdparser.cc dwdst_pargen_cmdparser.h \
	dwdspp_cmdparser.cc dwdspp_cmdparser.h

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
