CXX = g++
CXX_WFLAGS = -Wall
CXX_OFLAGS = -g
CXX_IFLAGS = \
	-I/usr/local/include/FSMlib \
	-I/usr/local/include/stlport \
	-I/usr/local/include/bumblelib \
	-I$(SRCDIR)
CXXFLAGS = $(CXX_WFLAGS) $(CXX_OFLAGS) $(CXX_IFLAGS)
LDFLAGS  = -L/usr/local/lib -L$(SRCDIR)/.libs
LIBS     = -static -ldwdst -lFSMExt -lFSM -lFSMSymSpec -lbumble++ -lstlport_gcc -lpthread -lz -lm

SRCDIR ?= ../libdwdst
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


TARGETS = parcpy trim

all: $(TARGETS)

# deps
#plextest_DEPS = plextest.o
parcpy_OBJS = parcpy.o
parcpy_DEPS = $(SRCDIR)/libdwdst.la

trim_OBJS = trim.o

#parcpy.o: parcpy.cc
#plextest.o: plextest.cc

.SUFFIXES: .ll .yy .cc .h .o

#plextest: $(plextest_DEPS)
#	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)


parcpy: $(parcpy_OBJS) $(parcpy_DEPS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(parcpy_OBJS) $(LIBS)

trim: $(trim_OBJS) $(trim_DEPS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(trim_OBJS) -lstlport_gcc -lpthread

$(SRCDIR)/libdwdst.la:
	$(MAKE) -C $(SRCDIR) libdwdst.la

$(SRCDIR)/%: ;
	$(MAKE) -C $(SRCDIR) $*



.cc.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<



.ll.cc: $(FLEXXX_SKEL)
	$(FLEXXX) $(FLEXXX_FLAGS) -h$*.h -o$*.cc $<

.ll.h: $(FLEXXX_HSKEL)
	$(FLEXXX) $(FLEXXX_FLAGS) -h$*.h -o$*.cc $<


# --- checking
.PHONY: pardiff_check

par ?= test.par

pardiff: parcpy $(par) pardiff_check

pardiff_check:
	grep -v '^%%' $(par) | sort - > $(par).in
	./parcpy < $(par).in | sort - > $(par).out
	@echo "--------------------------------------------"
	diff -sq $(par).in $(par).out
	@echo "--------------------------------------------"
	echo "rm $(par).in $(par).out"

clean:
	rm -f $(TARGETS) *.o *~ \
	  $(patsubst %.ll,%.cc,$(wildcard *.ll)) \
	  $(patsubst %.ll,%.h,$(wildcard *.ll)) \
	  $(patsubst %.yy,%.cc,$(wildcard *.yy)) \
	  $(patsubst %.yy,%.h,$(wildcard *.yy))
