#----------------------------------------------------------------
# Global variables
#----------------------------------------------------------------
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


#----------------------------------------------------------------
# Targets
#----------------------------------------------------------------
TARGETS = parcpy trim

#----------------------------------------------------------------
# Rules: Top-level
#----------------------------------------------------------------
all: $(TARGETS)

#----------------------------------------------------------------
# Deps
#----------------------------------------------------------------
#plextest_DEPS = plextest.o
parcpy_OBJS = parcpy.o
parcpy_DEPS = $(SRCDIR)/libdwdst.la

trim_OBJS = trim.o

#parcpy.o: parcpy.cc
#plextest.o: plextest.cc

.SUFFIXES: .ll .yy .cc .h .o

#plextest: $(plextest_DEPS)
#	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)


#----------------------------------------------------------------
# C: program linking
#----------------------------------------------------------------
parcpy: $(parcpy_OBJS) $(parcpy_DEPS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(parcpy_OBJS) $(LIBS)

trim: $(trim_OBJS) $(trim_DEPS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(trim_OBJS) -lstlport_gcc -lpthread

$(SRCDIR)/libdwdst.la:
	$(MAKE) -C $(SRCDIR) libdwdst.la

$(SRCDIR)/%: ;
	$(MAKE) -C $(SRCDIR) $*


#----------------------------------------------------------------
# C: object compilation
#----------------------------------------------------------------
.cc.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<


#----------------------------------------------------------------
# flex++/bison++: C++ generation
#----------------------------------------------------------------
.ll.cc: $(FLEXXX_SKEL)
	$(FLEXXX) $(FLEXXX_FLAGS) -h$*.h -o$*.cc $<

.ll.h: $(FLEXXX_HSKEL)
	$(FLEXXX) $(FLEXXX_FLAGS) -h$*.h -o$*.cc $<

#----------------------------------------------------------------
# parameter file checking
#----------------------------------------------------------------
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

#----------------------------------------------------------------
# parameter compilation
#----------------------------------------------------------------
par ?= test.par
parbase = $(basename $(par))
dis = $(addsuffix .fsa,$(parbase))
.SUFFIXES: .par .apar .tpar .atpar .dps

.PHONY: dis dskel

dis: dskel

dskel: $(dis)
	@echo ">> N-Gram Parameter File = $(par)"
	@echo ">> Disambiguation-FSA    = $(dis)"

.par.fsa:
	( cd ../programs ;\
	  $(MAKE) dwdst-fstgen ;\
	  ./dwdst-fstgen -d -n ../tests/$(par) -F $(dis) )
	mv ../programs/$(dis) .
	mv ../programs/dskeleton.lab $(parbase).lab

.par.lab:
	( cd ../programs ;\
	  $(MAKE) dwdst-fstgen ;\
	  ./dwdst-fstgen -d -n ../tests/$(par) -F $(dis) )
	mv ../programs/$(dis) .
	mv ../programs/dskeleton.lab $(parbase).lab

### viewing
.PHONY: disgv

disps = $(parbase).dps

disps: $(disps)

$(disps): $(dis)
	FSMprint $(dis) | fsmcompile -F $(parbase).afsa
	fsmdraw -i $(parbase).lab $(parbase).afsa > $(parbase).dot
	dot -Tps $(parbase).dot > $@

disgv: $(disps)
	gv $(disps)

#----------------------------------------------------------------
# FSM stuff
#----------------------------------------------------------------
.SUFFIXES: \
	.fsa .fst .tfst .tfsa .hfsa .hfst .lx \
	.afsa .afst \
	.sym .lab .scl .dot .ps

sym    ?= test.sym
SYMFILE = $(sym)
LABFILE ?= $(SYMFILE:.sym=.lab)
SCLFILE ?= $(SYMFILE:.sym=.scl)

ifeq ($(compat), fsm)
COMPAT_FLAGS = --no-compat
else
COMPAT_FLAGS =
endif

#--------------------------------
# regex compiling
.PHONY: re re.fst are are.afst

re      ?= [<epsilon>]
#reflags ?= --debug --compress
#reflags ?= --debug
refile  ?= re.fst
arefile ?= are.afst

gvre: $(refile:.fst=.ps); gv $(refile:.fst=.ps)
re: $(refile)
$(refile): FSMcompre $(SYMFILE)
	FSMcompre $(COMPAT_FLAGS) -s $(SYMFILE) $(reflags) -F $@ "$(re)"

areflags =

gvare: are.ps; gv are.ps
are: $(arefile)
$(arefile): $(LABFILE) $(SCLFILE)
	lexcompre -l $(LABFILE) -S $(SCLFILE) -s $(re) -F $@

#--------------------------------
# lexicon compilation
.lx.fsa:
	FSMcomplex -s $(SYMFILE) -z -F $@ $<

.lx.fst:
	FSMcomplex -s $(SYMFILE) -z -F $@ $<

.lx.afsa: $(LABFILE) $(SCLFILE)
	lexcomplex -l $(LABFILE) -S $(SCLFILE) -F $@ $<

.lx.afst: $(LABFILE) $(SCLFILE)
	lexcomplex -l $(LABFILE) -S $(SCLFILE) -F $@ $<

#--------------------------------
# FSM compilation
.tfsa.fsa:
	FSMcompile -z -F $@ $<

.tfst.fst:
	FSMcompile -z -F $@ $<

.tfsa.hfsa:
	FSMcompile -z -H -F $@ $<

.tfst.hfst:
	FSMcompile -z -H -F $@ $<



#--------------------------------
# FSM drawing
.fsa.dot:
	FSMdraw $(COMPAT_FLAGS) -s $(SYMFILE) -o $@ $<

.fst.dot:
	FSMdraw $(COMPAT_FLAGS) -s $(SYMFILE) -o $@ $<


.afsa.dot: $(LABFILE)
	fsmdraw -i $(LABFILE) -o $(LABFILE) $< > $@

.afst.dot: $(LABFILE)
	fsmdraw -i $(LABFILE) -o $(LABFILE) $< > $@

.dot.ps:
	dot -Tps $< > $@

gv: $(fst:.fst=.ps)
	gv $^ &

agv: $(afst:.afst=.ps)
	gv $^ &

#--------------------------------
# label generation
.sym.lab:
	lexmakelab $*

.sym.scl:
	lexmakelab $*

#--------------------------------

#----------------------------------------------------------------
# cleanup
#----------------------------------------------------------------
clean:
	rm -f $(TARGETS) *.o *~ \
	  $(patsubst %.ll,%.cc,$(wildcard *.ll)) \
	  $(patsubst %.ll,%.h,$(wildcard *.ll)) \
	  $(patsubst %.yy,%.cc,$(wildcard *.yy)) \
	  $(patsubst %.yy,%.h,$(wildcard *.yy)) \
	  *.fsa *.fst *.hfsa *.hfst *.afsa *.afst \
	  *.lab *.scl *.ps *.dps *.dot
