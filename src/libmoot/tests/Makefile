#!/usr/bin/make -R

CXX ?= g++

OFLAGS   ?= -ggdb
IFLAGS   ?= -I..
DEFS     ?= -DHAVE_CONFIG_H
WFLAGS   ?= -Wall
CFLAGS   ?= $(WFLAGS) $(IFLAGS) $(DEFS) $(OFLAGS)
CXXFLAGS ?= $(CFLAGS)

LD          = $(CXX)
LDFLAGS    ?= $(OFLAGS) -L..
STATICLIBS ?= ../.libs/libmoot.a
LIBS       ?= -lz -lrecode -lexpat

KNOWN_TARGETS = toklex tokio tokio hmm+exit dummyhmm kmwio dummyhmm linetag taster
TARGETS = taster

all: $(TARGETS)

##-- config
config:
	@echo "OFLAGS=$(OFLAGS)"
	@echo "CXXFLAGS=$(CXXFLAGS)"
	@echo "LIBS=$(LIBS)"
	@echo "STATICLIBS=$(STATICLIBS)"

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

kmwio: kmwio.o $(STATICLIBS)

linetag: linetag.o $(STATICLIBS)

taster: taster.o $(STATICLIBS)

##-- patterns: .o
taster.o: taster.cc mootFlavor.h

%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

##-- literals: linking
hmm+exit: hmm+exit.o $(STATICLIBS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

toklex: toklex.o $(STATICLIBS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

##-- patterns: default
.DEFAULT:
	$(LD) $(LDFLAGS) -o $@ $^ $(STATICLIBS) $(LIBS)

%: %.o $(STATICLIBS)
	$(LD) $(LDFLAGS) -o $@ $^ $(STATICLIBS) $(LIBS)
