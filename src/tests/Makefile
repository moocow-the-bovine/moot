CXX = g++
CXX_WFLAGS = -Wall
CXX_OFLAGS = -g
CXX_IFLAGS = \
	-I/usr/local/include/FSMlib \
	-I/usr/local/include/stlport \
	-I/usr/local/include/bumblelib \
	-I..
CXXFLAGS = $(CXX_WFLAGS) $(CXX_OFLAGS) $(CXX_IFLAGS)
LDFLAGS  = -L/usr/local/lib
LIBS     = -lFSMExt -lFSM -lFSMSymSpec -lbumble++ -lstlport_gcc -lpthread -lz -lm

SRCDIR ?= ..
FLEXXX ?= flex++
#FLEXXX_SKEL = $(SRCDIR)/flexskel.cc
#FLEXXX_HSKEL = $(SRCDIR)/flexskel.h
#FLEXXX_FLAGS = -8 -S$(FLEXXX_SKEL) -H$(FLEXXX_HSKEL)
FLEXXX_FLAGS = -8

BISONXX ?= bison++
#BISONXX_SKEL = $(SRCDIR)/bison.cc
#BISONXX_HSKEL = $(SRCDIR)/bison.h
#BISON_FLAGS = --debug -d -v
#BISONXX_FLAGS = $(BISONXX_DFLAGS) -d -v -S$(BISONXX_SKEL) -H$(BISONXX_HSKEL)
BISONXX_FLAGS = $(BISONXX_DFLAGS) -d -v


TARGETS = plextest parcpy

all: $(TARGETS)

# deps
plextest_DEPS = plextest.o dwdst_param_lexer.o
parcpy_DEPS = parcpy.o \
	dwdst_param_lexer.o \
	$(SRCDIR)/dwdst_trainer.o \
	$(SRCDIR)/dwdst.o \
	$(SRCDIR)/dwdst_lexer.o \
	$(SRCDIR)/postag_lexer.o

parcpy.o: parcpy.cc dwdst_param_lexer.h $(SRCDIR)/dwdst_trainer.h
plextest.o: plextest.cc dwdst_param_lexer.h

.SUFFIXES: .ll .yy .cc .h .o

plextest: $(plextest_DEPS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

parcpy: $(parcpy_DEPS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

$(SRCDIR)/%: ;
	$(MAKE) -C $(SRCDIR) $*

.cc.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.ll.cc: $(FLEXXX_SKEL)
	$(FLEXXX) $(FLEXXX_FLAGS) -h$*.h -o$*.cc $<

.ll.h: $(FLEXXX_HSKEL)
	$(FLEXXX) $(FLEXXX_FLAGS) -h$*.h -o$*.cc $<


clean:
	rm -f $(TARGETS) *.o *~ \
	  $(patsubst %.ll,%.cc,$(wildcard *.ll)) \
	  $(patsubst %.ll,%.h,$(wildcard *.ll))
