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
dskel = dskeleton.fsa
.SUFFIXES: .par .apar .tpar .atpar .dps

.PHONY: nodskel nodis dskel dis

### generation: disambig-skeleton
nodskel: ; rm -f $(dskel)
dskel: $(dskel)
	@echo ">> N-Gram Parameter File    = $(par)"
	@echo ">> Disambiguation-Skeleton  = $(dis)"

$(dskel): $(par)
	( cd ../programs ;\
	  $(MAKE) dwdst-fstgen ;\
	  ./dwdst-fstgen -d -n ../tests/$(par) -F ../tests/$(dis) )
	mv ../programs/dskeleton.fsa $(dskel)
	mv ../programs/dskeleton.lab $(dskel:.fsa=.lab)

### generation: disambig-fsa
nodis: ; rm -f $(dis)
dis: $(dis)
	@echo ">> N-Gram Parameter File = $(par)"
	@echo ">> Disambiguation-FSA    = $(dis)"

.par.fsa:
	( cd ../programs ;\
	  $(MAKE) dwdst-fstgen ;\
	  ./dwdst-fstgen -d -n ../tests/$(par) -F ../tests/$(dis) )
	test -e ../programs/dskeleton.fsa && cp ../programs/dskeleton.fsa $(dskel) || true
	test -e ../programs/dskeleton.lab && cp ../programs/dskeleton.lab $(dskel:.fsa=.lab) || true

.par.lab:
	( cd ../programs ;\
	  $(MAKE) dwdst-fstgen ;\
	  ./dwdst-fstgen -d -n ../tests/$(par) -F ../tests/$(dis) )
	test -e ../programs/dskeleton.fsa && cp ../programs/dskeleton.fsa $(dskel) || true
	test -e ../programs/dskeleton.lab && cp ../programs/dskeleton.lab $(dskel:.fsa=.lab) || true

### viewing: disambig-fsa
.PHONY: disgv

disps = $(parbase).dps

disps: $(disps)
$(disps): $(dis) $(LABFILE)
	FSMdraw -s $(sym) $(dis) -o $(parbase).dot
	dot -Tps $(parbase).dot > $@
disgv: $(disps)
	gv $(disps)

### viewing: disambig-skeleton
dskelps = $(dskel:.fsa=.ps)
$(dskelps): $(dskel)
	FSMprint $< | fsmcompile -F $(dskel:.fsa=.afsa)
	fsmdraw -i  $(dskel:.fsa=.lab) $(dskel:.fsa=.afsa) > dskeleton.dot
	dot -Tps dskeleton.dot > $@
dskelgv: $(dskelps)
	gv $(dskelps)

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
reflags ?= --incomplete-categories
refile  ?= re.fst
arefile ?= are.afst

gvre: $(refile:.fst=.ps); gv $(refile:.fst=.ps)
re: $(refile)
$(refile): $(SYMFILE)
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
