#!/usr/bin/make -R

CXX ?= g++

OFLAGS   ?= -g
IFLAGS   ?= -I..
DEFS     ?= -DHAVE_CONFIG_H
CFLAGS   ?= $(IFLAGS) $(DEFS) $(OFLAGS)
CXXFLAGS ?= $(CFLAGS) -ftemplate-depth-24

LD          = $(CXX)
LDFLAGS    ?= $(OFLAGS) -L..
STATICLIBS ?= ../.libs/libmoot.a
LIBS       ?= -lz -lrecode -lexpat

KNOWN_TARGETS = toklex tokio tokio hmm+exit dummyhmm
TARGETS = dummyhmm

all: $(TARGETS)

##-- clean
clean:
	rm -f *.o $(KNOWN_TARGETS)

##-- libs
libs: $(STATICLIBS)

../.libs/libmoot.a: ../libmoot.la

../libmoot.la:
	$(MAKE) -C .. libmoot.la

##-- linking
hmm+exit: hmm+exit.o $(STATICLIBS)
dummyhmm: dummyhmm.o $(STATICLIBS)

##-- patterns: .o
%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

##-- patterns: default
.DEFAULT:
	$(LD) $(LDFLAGS) -o $@ $^ $(STATIC_LIBS) $(LIBS)

%: %.o $(STATICLIBS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
