.SUFFIXES: \
	.fsa .fst .tfst .tfsa .hfsa .hfst .lx \
	.afsa .afst \
	.sym .lab .scl .dot .ps \
	.par .apar .tpar .atpar \
	.pp .txt .xml \
	.tag .atag .ttag .attag \
	.tags

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
	@echo "  word=WORD word"
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
	$(DWDST) --symbols=$(sym) --morph=$(morph) $< -o $@

.pp.atag:
	$(DWDST) --avm --symbols=$(sym) --morph=$(morph) $< -o $@

.pp.ttag:
	$(DWDST) --tags-only --symbols=$(sym) --morph=$(morph) $< -o $@

.pp.attag:
	$(DWDST) --avm --tags-only --symbols=$(sym) --morph=$(morph) $< -o $@

#-----------------------------------------------------------------------
# parameter generation
#-----------------------------------------------------------------------
PARGEN = ../programs/dwdst-pargen

.pp.par:
	$(PARGEN) --symbols=$(sym) --morph=$(morph) $< -o $@

.pp.apar:
	$(PARGEN) --avm --symbols=$(sym) --morph=$(morph) $< -o $@

.pp.tpar:
	$(PARGEN) --tags-only --symbols=$(sym) --morph=$(morph) $< -o $@

.pp.atpar:
	$(PARGEN) --avm --tags-only --symbols=$(sym) --morph=$(morph) $< -o $@

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

.PHONY: disambig dfst dfsa

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

#-----------------------------------------------------------------------
# epsilon removal
#-----------------------------------------------------------------------

%-e.fsa: %.fsa
	FSMprint $< | \
	fsmcompile | fsmrmepsilon - | fsmprint - | \
	FSMcompile - -F $@


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
# cleanup
#-----------------------------------------------------------------------
clean:
	rm -f *.fsa *.hfsa *.afsa *.afst
	rm -f *.lab *.scl *.ps *.dot
	rm -f *.tag *.atag *.ttag *.attag
	rm -f *.par *.apar *.tpar *.atpar

#	rm -f $(filter-out dwdst.%, $(wildcard *~ *.fst *.hfst))