.SUFFIXES: \
	.fsa .fst .tfst .tfsa .hfsa .hfst .lx \
	.afsa .afst \
	.sym .lab .scl .dot .ps \
	.par .apar .tpar .atpar \
	.pp .txt .xml \
	.tag .atag .ttag .attag \
	.tags \
	.tnt .ttnt \
	.alph .talph .vmap .tvmap \
	.k.sym .k.lab .kt.sym .kt.lab \
	.arc .tarc \
	.k.fst .kt.fst

.SECONDARY: \
	%.kt.sym %.k.sym .kt.sym .k.sym \
	%.alph %.talph .alph .talph \
	%.fst %.k.fst %.kt.fst .fst .k.fst .kt.fst \
	%.arc %.tarc .arc .tarc \
	%.tnt %.ttnt .tnt .ttnt \
	%.par %.tpar .par .tpar \
	%.alph %.talph .alph .talph \
	%.vmap %.tvmap .vmap .tvmap

ifeq ($(compat), fsm)
 COMPAT_FLAGS = --no-compat
else
 COMPAT_FLAGS =
endif


all:
	@echo "-----------------------------------------------------"
	@echo "You must specify either filenames or one of the"
	@echo "following variable/target combinations:"
	@echo ""
	@echo " Common Variables:"
	@echo "  sym=SYMFILE"
	@echo "  morph=MORPHFST"
	@echo ""
	@echo " Common Targets:"
	@echo "  re=REGEX (re|re-ps|re-gv)"
	@echo "  utags=TAGLIST ufst"
	@echo "  dfsa=DISAMBIGFSA [par=PARFILE] [atags=ALLTAGS] (dfsa|dfsa-ps|dfsa-gv)"
	@echo "  dskel=DISAMBIGSKEL [par=PARFILE] [atags=ALLTAGS] (dskel|dskel-ps|dskel-gv)"
	@echo "  word=WORD word"
	@echo "  word=WORD wtags"
	@echo ""
	@echo " Rule Targets:"
	@echo "  .pp.tnt | .pp.ttnt | .tnt.alph | .ttnt.talph"
	@echo "  [z=ZLEVEL] .alph.sym | .talph.sym"
	@echo "  [alph=ALPH] [z=ZLEVEL] .par.arc | .tpar.arc"
	@echo "  [alph=ALPH] [z=ZLEVEL] .arc.fst | .arc.fst"
	@echo "  [alph=ALPH] [z=ZLEVEL] [sym=SYM] .alph.vmap | .talph.tvmap"
	@echo "-----------------------------------------------------"

#-----------------------------------------------------------------------
# doc preprocessing
#-----------------------------------------------------------------------
DWDSPP = ../programs/dwdspp

.xml.pp:
	$(DWDSPP) -v0 $< -o $@

.txt.pp:
	$(DWDSPP) -v0 $< -o $@

#-----------------------------------------------------------------------
# doc tagging
#-----------------------------------------------------------------------
DWDST    = ../programs/dwdst
#sym     ?= dwdst.sym
#morph   ?= dwdst.fst
sym     ?= test.sym
morph   ?= test.fst

.pp.tag:
	$(DWDST) -D --symbols=$(sym) --morph=$(morph) $< -o $@

.pp.atag:
	$(DWDST) -D --avm --symbols=$(sym) --morph=$(morph) $< -o $@

.pp.ttag:
	$(DWDST) -D --tags-only --symbols=$(sym) --morph=$(morph) $< -o $@

.pp.attag:
	$(DWDST) -D --avm --tags-only --symbols=$(sym) --morph=$(morph) $< -o $@

#-----------------------------------------------------------------------
# parameter generation
#-----------------------------------------------------------------------
PARGEN = ../programs/dwdst-pargen

.pp.par:
	$(PARGEN) -v0 --symbols=$(sym) --morph=$(morph) $< -o $@

.pp.apar:
	$(PARGEN) -v0 --avm --symbols=$(sym) --morph=$(morph) $< -o $@

.pp.tpar:
	$(PARGEN) -v0 --tags-only --symbols=$(sym) --morph=$(morph) $< -o $@

.pp.atpar:
	$(PARGEN) -v0 --avm --tags-only --symbols=$(sym) --morph=$(morph) $< -o $@

#-----------------------------------------------------------------------
# unknown-fst generation
#-----------------------------------------------------------------------
FSTGEN = ../programs/dwdst-fstgen
utags ?= dwdst-unknown.tags
ufst  ?= $(utags:.tags=.fst)

.PHONY: unknown

unknown: $(ufst)
ufst: $(ufst)

$(ufst): $(FSTGEN) $(sym) $(utags)
	$(FSTGEN) --unknown \
	  --symbols=$(sym) --open-class-list=$(utags) \
	  -F $@

#-----------------------------------------------------------------------
# disambig-fsa generation
#-----------------------------------------------------------------------
dfsa  ?= dwdst-disambig.fsa
dfst   = dfsa
atags ?= /dev/null
par   ?= $(dfsa:.fsa=.par)

.PHONY: disambig dfst dfsa nodfsa

nodfsa: ; rm -f $(dfsa)
disambig: $(dfsa)
dfst: $(dfsa)
dfsa: $(dfsa)
dfsa-ps: $(dfsa:.fsa=.ps)
dfsa-gv: dfsa-ps ; gv $(dfsa:.fsa=.ps) &

$(dfsa): $(FSTGEN) $(sym) $(utags) $(atags) $(par)
	$(FSTGEN) --disambig \
	  --symbols=$(sym) \
	  --open-class-list=$(utags) \
	  --pos-tag-list=$(atags) \
	  --ngram-parameters=$(par) \
	  -F $@


.PHONY: disambig dskel skel
dskel ?= dskeleton.fsa
slab  ?= dskeleton.lab

nodskel: ; rm -f $(dskel)
dskel: $(dskel)
skel: $(dskel)
dskel-ps: $(dskel:.fsa=.ps)
dskel-gv: dskel-ps ; gv $(dskel:.fsa=.ps) &

$(dskel): $(FSTGEN) $(sym) $(utags) $(atags) $(par)
	$(FSTGEN) --disambig --skeleton \
	  --labels=$(slab) \
	  --symbols=$(sym) \
	  --open-class-list=$(utags) \
	  --pos-tag-list=$(atags) \
	  --ngram-parameters=$(par) \
	  -F $@

$(dskel:.fsa=.ps): $(dskel) $(slab)
	FSMprint $< | \
	  fsmcompile | \
	  fsmdraw -i $(slab) -o $(slab) | \
	  dot -Tps > $@


dskel_e ?= $(dskel:%.fsa=%-e.fsa)

dskel-e: $(dskel_e)
skel-e: $(dskel_e)
dskel-e-ps: $(dskel_e:.fsa=.ps)
dskel-e-gv: dskel-e-ps ; gv $(dskel_e:.fsa=.ps) &

$(dskel_e): $(dskel)
	FSMprint $< | \
	fsmcompile | fsmrmepsilon - | fsmprint - | \
	FSMcompile -w - -F $@


$(dskel_e:.fsa=.ps): $(dskel_e) $(slab)
	FSMprint $< | \
	  fsmcompile | \
	  fsmdraw -i $(slab) -o $(slab) | \
	  dot -Tps > $@

#-----------------------------------------------------------------------
# epsilon removal
#-----------------------------------------------------------------------

%-e.fsa: %.fsa
	FSMprint $< | \
	fsmcompile | fsmrmepsilon - | fsmprint - | \
	FSMcompile - -F $@


#%-e.fsa: %.fsa
#	FSMrmepsilon $< -F $@

#-----------------------------------------------------------------------
# regex compiling
#-----------------------------------------------------------------------
SYMFILE = $(sym)
LABFILE ?= $(SYMFILE:.sym=.lab)
SCLFILE ?= $(SYMFILE:.sym=.scl)


.PHONY: re re.fst are are.afst

re      ?= [<epsilon>]
#reflags ?= --debug --compress
#reflags ?= --debug
refile  ?= re.fst
arefile ?= are.afst


re: $(refile)
re-ps: $(refile:.fst=.ps)
re-gv: $(refile:.fst=.ps); gv $(refile:.fst=.ps)
gvre: re-gv

$(refile):  $(SYMFILE)
	FSMcompre $(COMPAT_FLAGS) -s $(SYMFILE) $(reflags) -F $@ "$(re)"

areflags =

gvare: are.ps; gv are.ps
are: $(arefile)
$(arefile): $(LABFILE) $(SCLFILE)
	lexcompre -l $(LABFILE) -S $(SCLFILE) -s $(re) -F $@

#-----------------------------------------------------------------------
# lexicon compilation
#-----------------------------------------------------------------------
.lx.fsa:
	FSMcomplex -s $(SYMFILE) -z -F $@ $<

.lx.fst:
	FSMcomplex -s $(SYMFILE) -z -F $@ $<

.lx.afsa: $(LABFILE) $(SCLFILE)
	lexcomplex -l $(LABFILE) -S $(SCLFILE) -F $@ $<

.lx.afst: $(LABFILE) $(SCLFILE)
	lexcomplex -l $(LABFILE) -S $(SCLFILE) -F $@ $<


#-----------------------------------------------------------------------
# FSM compiling
#-----------------------------------------------------------------------
.tfsa.fsa:
	FSMcompile -z -F $@ $<

.tfst.fst:
	FSMcompile -t -z -F $@ $<

.tfsa.hfsa:
	FSMcompile -z -H -F $@ $<

.tfst.hfst:
	FSMcompile -t -z -H -F $@ $<

#-----------------------------------------------------------------------
# FSM drawing
#-----------------------------------------------------------------------
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

#-----------------------------------------------------------------------
# AT&T label generation
#-----------------------------------------------------------------------
.sym.lab:
	lexmakelab $*

.sym.scl:
	lexmakelab $*

#-----------------------------------------------------------------------
# word-analysis
#-----------------------------------------------------------------------
word ?= Wort

.PHONY: word _word
word: words/$(word).fsa
_word: words/_$(word).fsa

words/$(word).fsa: $(morph) $(sym)
	FSMlookup --symbols=$(sym) --fst=$(morph) -F "words/$(word).fsa" "$(word)"

words/_$(word).fsa: words/$(word).fsa $(sym)
	FSMcompre --symbols=$(sym) "[WordBoundary]" | \
	  FSMconcat - "words/$(word).fsa" -F "words/_$(word).fsa"

.PHONY: wordps wordgv _wordps _wordgv

wordps: words/$(word).ps
wordgv: wordps ; gv "words/$(word).ps"

_wordps: words/_$(word).ps
_wordgv: _wordps ; gv "words/_$(word).ps"

.PHONY: nowords
nowords:
	rm -f words/*

#-----------------------------------------------------------------------
# tags-only word-analysis
#-----------------------------------------------------------------------
word ?= Wort

.PHONY: wtags

wtags: wtags/$(word).fsa

wtags/$(word).re: $(morph) $(sym)
	./dwdst -D --symbols=$(sym) --morph=$(morph) -v 0 -t -w "$(word)" |\
	  grep "^	" |\
	  xargs perl -e 'print join("|",@ARGV), "\n";' > $@

wtags/$(word).fsa: wtags/$(word).re $(sym)
	FSMcompre --symbols=$(sym) "`cat $<`" |\
	FSMprint | fsmcompile | fsmrmepsilon | fsmprint |\
	FSMcompile -F $@

.PHONY: wtagsps wtagsgv

wtagsps: wtags/$(word).ps
wtagsgv: wtagsps ; gv "wtags/$(word).ps"

.PHONY: nowtags
nowtags:
	rm -f wtags/*


#-----------------------------------------------------------------------
# Kempe targets
#-----------------------------------------------------------------------

.pp.tnt:
	./dwdst -D -T -v0 --symbols=$(sym) --morph=$(morph) $< -o $@

.pp.ttnt:
	./dwdst -D -Tt -v0 --symbols=$(sym) --morph=$(morph) $< -o $@


.tnt.alph: KempeGen.pm kempe-alphagen.perl
	./kempe-alphagen.perl $< -o $@

.ttnt.talph: KempeGen.pm kempe-alphagen.perl
	./kempe-alphagen.perl $< -o $@

z ?= 1

.alph.k.sym: KempeGen.pm kempe-alpha2sym.perl
	./kempe-alpha2sym.perl -z $(z) $< -o $@

.talph.kt.sym: KempeGen.pm kempe-alpha2sym.perl
	./kempe-alpha2sym.perl -z $(z) $< -o $@

.par.arc: $(alph) KempeGen.pm kempe-arcgen.perl
	test -z "$(alph)" && alph="$*.alph" || alph="$(alph)" ;\
	make $$alph ;\
	./kempe-arcgen.perl -v 0 -z $(z) -a $$alph $< -o $@

.tpar.tarc: $(alph) KempeGen.pm kempe-arcgen.perl
	test -z "$(alph)" && alph="$*.talph" || alph="$(alph)" ;\
	make $$alph ;\
	./kempe-arcgen.perl -v 1 -z $(z) -a $$alph $< -o $@

asym ?= $(patsubst .alph,.sym,$(patsubst .talph,.alph,$(alph)))
alab ?= $(asym:.sym=.lab)

.arc.k.fst: $(alph) $(asym)
	test -z "$(alph)" && alph="$*.alph" || alph="$(alph)" ;\
	abase=`basename "$$alph" .alph` ;\
	test -z "$(alab)" && alab="$$abase.k.lab" || alab="$(alab)" ;\
	make $$alph $$alab ;\
	lexarclist -l $$alab $< -F $*.k.afst
	fsmprint $*.k.afst | FSMcompile -t -F $@
	rm $*.k.afst

.tarc.kt.fst: $(alph)
	test -z "$(alph)" && alph="$*.talph" || alph="$(alph)" ;\
	abase=`basename "$$alph" .talph` ;\
	test -z "$(alab)" && alab="$$abase.kt.lab" || alab="$(alab)" ;\
	make $$alph $$alab ;\
	lexarclist -l $$alab $< -F $*.kt.afst
	fsmprint $*.kt.afst | FSMcompile -t -F $@
	rm $*.kt.afst


.alph.vmap: $(SYMFILE) KempeGen.pm kempe-alpha2vmap.perl
	./kempe-alpha2vmap.perl -z $(z) -s $(SYMFILE) $< -o $@

.talph.tvmap: $(SYMFILE) KempeGen.pm kempe-alpha2vmap.perl
	./kempe-alpha2vmap.perl -z $(z) -s $(SYMFILE) $< -o $@

#-----------------------------------------------------------------------
# cleanup
#-----------------------------------------------------------------------
clean:
	rm -f *.fsa *.hfsa *.afsa *.afst
	rm -f *.lab *.scl *.ps *.dot
	rm -f *.tag *.atag *.ttag *.attag
	rm -f *.par *.apar *.tpar *.atpar
	rm -f *.tnt *.ttnt *.alph *.talph *.arc *.tarc
	rm -f *.k.fst *.kt.fst *.k.sym *.kt.sym *.k.lab *.kt.lab
	rm -f beforeepsclosure.fsm *.vmap *.tvmap

#	rm -f $(filter-out dwdst.%, $(wildcard *~ *.fst *.hfst))