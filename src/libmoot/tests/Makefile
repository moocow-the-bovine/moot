#!/usr/bin/make -R

CXX ?= g++

CFLAGS   ?= -I.. -DHAVE_CONFIG_H -ftemplate-depth-24 -g
CXXFLAGS ?= $(CFLAGS)

LD         ?= $(CXX)
LDFLAGS    ?= -g -L..
STATICLIBS ?= ../.libs/libmoot.a
LIBS       ?= -lz -lrecode -lexpat

KNOWN_TARGETS = toklex tokio
TARGETS = tokio

all: $(TARGETS)

##-- clean
clean:
	rm -f *.o $(KNOWN_TARGETS)

##-- libs
libs: $(STATICLIBS)

../.libs/libmoot.a: ../libmoot.la

../libmoot.la:
	$(MAKE) -C .. libmoot.la


##-- patterns: .o
%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o "$@" "$<"

##-- patterns: default
.DEFAULT:
%: %.o $(STATICLIBS)
	$(LD) $(LDFLAGS) -o "$@" $^ $(LIBS)
