# Makefile for moot data

include config.mak

##-- Globals: data
TEST = utest
TRAIN = utrain
MODEL = $(TRAIN)
LCMODEL = $(TRAIN).lc
label = label

EVAL = $(TEST).dmt
TRUTH = $(TEST).ttt
XML_TRUTH_PRE = $(TEST).ttt.pre-xml
XML_TRUTH = $(TEST).ttt.xml

##-- Morph: mootm
SHARE_MOOT ?= $(shell cd share-moot ; pwd -P)

MOOTM_BINDIR ?= $(shell cd $(SHARE_MOOT)/../../bin ; pwd -P)
MOOTM ?= $(MOOTM_BINDIR)/mootm
MOOTM_FLAGS ?= -m $(SHARE_MOOT)/moot-stts-nofeatures.fst -d1024

##-- Morph: MorphConsole
MORPHC_BINDIR ?= $(MOOTM_BINDIR)
MORPHC ?= $(MORPHC_BINDIR)/MorphConsole
MORPHC_FLAGS ?= \
	-b share-moot/fsm.new/moot.sym \
	-m share-moot/fsm.new/MIMorph.fst \
	-p -f

##-- Morph: Hack
MORPHHACK ?= ./morphhack.perl

##-- convert: hack
XMLIFY ?= ./xmlify.perl
OF_XML=xml,pretty

##-- Globals: programs: moot
SUFFIX_TRIE_LEN ?= 4
SUFFIX_TRIE_THRESH ?= 10
SUFFIX_THETA  ?= 0.1017023
MOOT_BINDIR   ?= $(shell cd .. ; pwd -P)
MOOTRAIN      ?= $(MOOT_BINDIR)/mootrain
MOOTHMM       ?= $(MOOT_BINDIR)/moot
MOOTHMM_FLAGS ?= -a$(SUFFIX_TRIE_LEN) -A$(SUFFIX_TRIE_THRESH) --trie-theta=$(SUFFIX_THETA)
#-d1024

MOOTEVAL      ?= $(MOOT_BINDIR)/mooteval

MOOTCOMPILE   ?= $(MOOT_BINDIR)/mootcompile
ZFLAGS        ?= -z3
MOOTCOMPILE_FLAGS ?= $(ZFLAGS) -a$(SUFFIX_TRIE_LEN)

MOOTDUMP      ?= $(MOOT_BINDIR)/mootdump

MOOTCHURN     ?= $(MOOT_BINDIR)/mootchurn
MOOTCHURN_FLAGS ?= -d1024


all:
	@echo "Usage: make TEST=test TRAIN=train label=label {save|test}"
	@echo "MOOTM_BINDIR=$(MOOTM_BINDIR)"
	@echo "MOOT_BINDIR=$(MOOT_BINDIR)"


##-- don't delete these
.SECONDARY:

##--- untagging

t: $(TEST).t

%.t: %.ttt
	cut -f 1 $< > $@

%.t: %.tt
	cut -f 1 $< > $@

%.t: %.mt
	cut -f 1 $< > $@

##-- morph analysis
mt: $(TEST).mt

##-- analysis: mootm
%.m0ttt: %.ttt $(MOOTM)
	$(MOOTM) -12 $(MOOTM_FLAGS) $< -o $@

%.mttt: %.m0ttt $(MORPHHACK)
	$(MORPHHACK) --best $< > $@

%.m0t: %.t $(MOOTM)
	$(MOOTM) $(MOOTM_FLAGS) $< -o $@

%.mt: %.m0t $(MORPHHACK)
	$(MORPHHACK) --nobest $< > $@

##-- analysis: MorphConsole
#%.m0t: %.t $(MORPHC)
#	$(MORPHC) $(MORPHC_FLAGS) $< > $@
#
#%.m1tt: %.m0t $(MORPHHACK)
#	$(MORPHHACK) --nobest $< > $@
#
#%.mttt: %.ttt
#	$(MAKE) $*.m1tt
#	cut -f2- $*.m1tt | paste $< - > $@
#
#%.mt: %.mttt
#	cut -f 1,3- $< > $@
#
#%.mt: %.m1tt
#	ln $< $@

##-- tagging, lower-case
lc-tt: $(TEST).lc.tt

lc-dmt: $(TEST).lc.dmt

%.lc.tt: %.lc.t $(LCMODEL).lex $(LCMODEL).123 $(MOOTHMM)
	$(MOOTHMM) -M$(LCMODEL).lex,$(LCMODEL).123 $(MOOTHMM_FLAGS) $< -o $@

%.lc.dmt: %.lc.mt $(LCMODEL).lex $(LCMODEL).123 $(MOOTHMM)
	$(MOOTHMM) -M$(LCMODEL).lex,$(LCMODEL).123 $(MOOTHMM_FLAGS) $< -o $@

%.lc.cmt: %.lc.mt $(LCMODEL).lex $(LCMODEL).123 $(LCMODEL).clx $(MOOTHMM)
	$(MOOTHMM) -M$(LCMODEL).lex,$(LCMODEL).123,$(LCMODEL).clx $(MOOTHMM_FLAGS) $< -o $@

##-- tagging
tt: $(TEST).tt

dmt: $(TEST).dmt

%.tt: %.t $(MODEL).lex $(MODEL).123 $(MOOTHMM)
	$(MOOTHMM) -M$(MODEL).lex,$(MODEL).123 $(MOOTHMM_FLAGS) $< -o $@

%.dmt: %.mt $(MODEL).lex $(MODEL).123 $(MOOTHMM)
	$(MOOTHMM) -M$(MODEL).lex,$(MODEL).123 $(MOOTHMM_FLAGS) $< -o $@

%.cmt: %.mt $(MODEL).lex $(MODEL).123 $(MODEL).clx $(MOOTHMM)
	$(MOOTHMM) -M$(MODEL).lex,$(MODEL).123,$(MODEL).clx $(MOOTHMM_FLAGS) $< -o $@

##-- eval
eval: $(TEST).tt.eval $(TEST).dmt.eval $(TEST).cmt.eval

%.tt.eval: %.tt $(TRUTH) $(MOOTEVAL)
	$(MOOTEVAL) -v4 -2 -o $@ $*.ttt $<

%.dmt.eval: %.dmt $(TRUTH) $(MOOTEVAL)
	$(MOOTEVAL) -v4 -2 -o $@ $*.ttt $<

%.cmt.eval: %.cmt $(TRUTH) $(MOOTEVAL)
	$(MOOTEVAL) -v4 -2 -o $@ $*.ttt $<

%.mttt.eval: %.mttt $(TRUTH) $(MOOTEVAL)
	$(MOOTEVAL) -v4 -2 -o $@ $*.ttt $<

##-- error counting
errors: $(EVAL).errors
cerrors: $(EVAL).cerrors

%.tt.errors: %.tt.eval
	grep '^.b' $< | sort | uniq -c | sort -r -n > $@

%.tt.cerrors:
	echo "WHOLE FILE" > $@

%.errors: %.eval
	grep '^.b:' $< | grep -v '^..:...:---' | sort | uniq -c | sort -r -n > $@

%.cerrors: %.eval classify-eval.perl
	./classify-eval.perl $< \
	| grep '^.b:' | grep -v '^..:...:---' | sort | uniq -c | sort -r -n > $@

%.ierrors: %.eval
	grep '^.' $< | grep -v '^..:...:---' $< | sort | uniq -c | sort -r -n > $@

%.icerrors: %.eval classify-eval.perl
	./classify-eval.perl $< \
	| grep '^.' | grep -v '^..:...:---' | sort | uniq -c | sort -r -n > $@

##-- save
save: save/$(label).stamp

unsave:
	rm -rf save/$(label).stamp save/$(label)

test: save unsave

save/%.stamp: \
	$(TEST).ttt $(TEST).mt $(TRAIN).model
	for ext in tt dmt cmt ; do \
	  $(MAKE) $(TEST).$$ext $(TEST).$$ext.eval ;\
	done
	mkdir -p save/$*
	mv \
	   $(TRAIN).lex $(TRAIN).123 $(TRAIN).clx \
	   $(TEST).tt $(TEST).tt.eval \
	   $(TEST).dmt $(TEST).dmt.eval \
	   $(TEST).cmt $(TEST).cmt.eval \
	  save/$*
	rm -f $(TRAIN).model
	touch $@

##-- models
model: $(MODEL).model

%.model: %.mttt $(MOOTRAIN)
	$(MOOTRAIN) -lnC $< -o $*
	touch $@

#%.model: %.ttt $(MOOTRAIN)
#	$(MOOTRAIN) -lnC $< -o $*
#	touch $@

%.lex: ; $(MAKE) $*.model

%.123: ; $(MAKE) $*.model

%.clx: ; $(MAKE) $*.model

##-- binary models

hmm: $(MODEL).hmm

chmm: $(MODEL).chmm

%.hmm: %.model $(MOOTCOMPILE)
	$(MOOTCOMPILE) $(MOOTCOMPILE_FLAGS) -o $@ "$*.lex,$*.123"

%.chmm: %.model $(MOOTDUMP)
	$(MOOTCOMPILE) $(MOOTCOMPILE_FLAGS) -o $@ "$*.lex,$*.123,$*.clx"


##-- dumps

dump: $(MODEL).dump
cdump: $(MODEL).cdump

%.dump: %.model $(MOOTDUMP)
	$(MOOTDUMP) -o $@ "$*.lex,$*.123"

%.cdump: %.model $(MOOTDUMP)
	$(MOOTDUMP) -o $@ "$*.lex,$*.123,$*.clx"

hmm-dump: $(MODEL).hmm-dump
chmm-dump: $(MODEL).chmm-dump

%.hmm-dump: %.hmm $(MOOTDUMP)
	$(MOOTDUMP) -o $@ $<

%.chmm-dump: %.chmm $(MOOTDUMP)
	$(MOOTDUMP) -o $@ $<

##-- XML: conversion text->xml
%.pre-xml: % $(XMLIFY)
	$(XMLIFY) $< > $@

%.ttt.xml: %.ttt.pre-xml $(MOOTCHURN)
	$(MOOTCHURN) $(MOOTCHURN_FLAGS) -Inative,medium -O$(OF_XML),medium -o $@ $<

%.t.xml: %.t.pre-xml $(MOOTCHURN)
	$(MOOTCHURN) $(MOOTCHURN_FLAGS) -Inative,rare -O$(OF_XML),rare -o $@ $<

%.mt.xml: %.mt.pre-xml $(MOOTCHURN)
	$(MOOTCHURN) $(MOOTCHURN_FLAGS) -Inative,mediumrare -O$(OF_XML),mediumrare -o $@ $<

%.mttt.xml: %.mttt.pre-xml $(MOOTCHURN)
	$(MOOTCHURN) $(MOOTCHURN_FLAGS) -Inative,welldone -O$(OF_XML),welldone -o $@ $<

##-- XML: conversion xml->text
%.post-xml: %.xml $(MOOTCHURN)
	$(MOOTCHURN) $(MOOTCHURN_FLAGS) -Ixml,welldone -Onative,welldone -o $@ $<


##-- XML: models
xml_model: $(MODEL).xml.model

no_xml_model: ; rm -f $(MODEL).xml.model

%.xml.model: %.mttt.xml $(MOOTRAIN)
	$(MOOTRAIN) -Ixml,welldone -lnC $< -o $*.xml
	touch $@

%.xml.lex: ; $(MAKE) $*.xml.model

%.xml.123: ; $(MAKE) $*.xml.model

%.xml.clx: ; $(MAKE) $*.xml.model


##-- XML: tagging
tt_xml: $(TEST).tt.xml

dmt_xml: $(TEST).dmt.xml

cmt_xml: $(TEST).cmt.xml

%.tt.xml: %.t.xml $(MODEL).xml.lex $(MODEL).xml.123 $(MOOTHMM)
	$(MOOTHMM) -Ixml -O$(OF_XML),welldone -M$(MODEL).xml.lex,$(MODEL).xml.123 $(MOOTHMM_FLAGS) -o $@ $<

%.dmt.xml: %.mt.xml $(MODEL).xml.lex $(MODEL).xml.123 $(MOOTHMM)
	$(MOOTHMM) -Ixml -O$(OF_XML),welldone -M$(MODEL).xml.lex,$(MODEL).xml.123 $(MOOTHMM_FLAGS) -o $@ $<

%.cmt.xml: %.mt.xml $(MODEL).xml.lex $(MODEL).xml.123 $(MODEL).xml.clx $(MOOTHMM)
	$(MOOTHMM) -Ixml -O$(OF_XML),welldone -M$(MODEL).xml.lex,$(MODEL).xml.123,$(MODEL).xml.clx $(MOOTHMM_FLAGS) -o $@ $<

##-- XML: eval (by conversion)
xml_eval1: $(TEST).tt.xml.eval1 $(TEST).dmt.xml.eval1 $(TEST).cmt.xml.eval1

%.tt.xml.eval1: %.tt.post-xml $(XML_TRUTH_PRE) $(MOOTEVAL)
	$(MOOTEVAL) -v4 -2 -o $@ $*.ttt.pre-xml $<

%.dmt.xml.eval1: %.dmt.post-xml $(XML_TRUTH_PRE) $(MOOTEVAL)
	$(MOOTEVAL) -v4 -2 -o $@ $*.ttt.pre-xml $<

%.cmt.xml.eval1: %.cmt.post-xml $(XML_TRUTH_PRE) $(MOOTEVAL)
	$(MOOTEVAL) -v4 -2 -o $@ $*.ttt.pre-xml $<

%.mttt.xml.eval1: %.mttt.post-xml $(XML_TRUTH_PRE) $(MOOTEVAL)
	$(MOOTEVAL) -v4 -2 -o $@ $*.ttt.pre-xml $<

##-- XML: eval (direct)
xml_eval: $(TEST).tt.xml.eval $(TEST).dmt.xml.eval $(TEST).cmt.xml.eval

%.tt.xml.eval: %.tt.xml $(XML_TRUTH) $(MOOTEVAL)
	$(MOOTEVAL) -v4 -2 -Ixml,medium -o $@ $*.ttt.xml $<

%.dmt.xml.eval: %.dmt.xml $(XML_TRUTH) $(MOOTEVAL)
	$(MOOTEVAL) -v4 -2 -Ixml,welldone -o $@ $*.ttt.xml $<

%.cmt.xml.eval: %.cmt.xml $(XML_TRUTH) $(MOOTEVAL)
	$(MOOTEVAL) -v4 -2 -Ixml,welldone  -o $@ $*.ttt.xml $<

%.mttt.xml.eval: %.mttt.xml $(XML_TRUTH) $(MOOTEVAL)
	$(MOOTEVAL) -v4 -2 -Ixml,welldone -o $@ $*.ttt.xml $<

##-- XML: check
xml_check: $(TEST).tt.xml-check $(TEST).dmt.xml-check $(TEST).cmt.xml-check

%.tt.xml-check: %.tt.post-xml %.tt $(MOOTEVAL)
	$(MOOTEVAL) -v2 -2 -Imedium -o $@ $*.tt $<

%.dmt.xml-check: %.dmt.post-xml %.dmt $(MOOTEVAL)
	$(MOOTEVAL) -v2 -2 -Iwelldone -o $@ $*.dmt $<

%.cmt.xml-check: %.cmt.post-xml %.cmt $(MOOTEVAL)
	$(MOOTEVAL) -v2 -2 -Iwelldone  -o $@ $*.cmt $<


##-- XML: save
xml_save: save/$(label).xml_stamp

xml_unsave:
	rm -rf save/$(label).xml_stamp save/$(label)/*xml*

xml_test: xml_save xml_unsave

save/%.xml_stamp: $(TEST).ttt $(TEST).mt.xml $(TRAIN).xml.model
	for ext in tt dmt cmt ; do \
	  $(MAKE) $(TEST).$$ext.xml $(TEST).$$ext.xml-check ;\
	done
	mkdir -p save/$*
	mv \
	   $(TEST).tt.xml \
	   $(TEST).dmt.xml \
	   $(TEST).cmt.xml \
	   \
	   $(TEST).tt.xml-check \
	   $(TEST).dmt.xml-check \
	   $(TEST).cmt.xml-check \
	  save/$*
	rm -f $(TRAIN).xml.model
	touch $@

##-- lower-case

lower: lc

lc: $(TEST).lc.ttt $(TRAIN).lc.ttt

nolower: nolc

nolc:
	rm -f *.lc.*

%.lc.ttt: %.ttt
	tr A-ZÄÖÜ a-zäöü < $< > $@

lc_model: $(LCMODEL)

%.lc.model: %.lc.ttt $(MOOTRAIN)
	$(MOOTRAIN) -Imedium -lnC $< -o $*.lc
	touch $@

##-- tnt-tagging

TNT_AFLAGS ?= -a$(SUFFIX_TRIE_LEN)
TNT_DFLAGS ?= -d4
TNT_UFLAGS ?= -u3
TNT_NFLAGS ?= -n3
TNT_FLAGS ?= $(TNT_AFLAGS) $(TNT_DFLAGS) $(TNT_UFLAGS) $(TNT_NFLAGS)

notnt:
	rm -f *.tnt-tt *.tnt-tt.*

%.lc.tnt-tt: %.lc.t $(LCMODEL).model
	tnt $(TNT_FLAGS) $(LCMODEL) $< > $@

%.tnt-tt: %.t $(MODEL).model
	tnt $(TNT_FLAGS) $(MODEL) $< > $@

%.tnt-tt.eval: %.tnt-tt %.ttt
	$(MOOTEVAL) -v3 -2 -Imedium $*.ttt $< -o $@

##-- cleanup
clean:
	rm -f tmp.res *~ \
	  *.xml-check \
	  *.tt *.dmt *.cmt *.lc.* \
	  *.tt.xml *.dmt.xml *.cmt.xml \
	  *.tnt-tt \
	  *.lex *.123 *.clx *.model \
	  *.hmm *.chmm *.dump *.cdump \
	  *.errors *.cerrors *.icerrors *.eval \
	  *.post-xml

realclean: clean
	rm -f \
	  *.t  *.t.xml \
	  *.mt *.m0t *.m1t *.amt \
	  *.mtt *.m0tt *.m1tt \
	  *.mttt *.m0ttt *.m1ttt \
	  *.pre-xml
