## Makefile for moot data

include config.mak

##-- Globals: data
TEST = utest
TRAIN = utrain
MODEL = $(TRAIN)
LCMODEL = $(TRAIN).lc

##-- labelling
label_host ?= $(shell hostname)
label_date ?= $(shell date "+%Y-%m-%d@%H-%M-%S")
#label = label
label ?= $(label_date).$(label_host)

eval ?=dis.wdt

EVAL ?= $(TEST).$(MORPHHACK_INFIX).$(eval)
TRUTH = $(TEST).ttt
XML_TRUTH_PRE = $(TEST).ttt.pre-xml
XML_TRUTH = $(TEST).ttt.xml

##-- Morph: mootm
SHARE_MOOT ?= $(shell cd share-moot ; pwd -P)

MOOTM_BINDIR ?= $(shell cd $(SHARE_MOOT)/../../bin ; pwd -P)
MOOTM ?= $(MOOTM_BINDIR)/mootm
MOOTM_FLAGS ?= -a -m $(SHARE_MOOT)/mootm-stts-nofeatures.gfst -s $(SHARE_MOOT)/mootm-stts.lab -d1024 -e"%"

##-- Morph: MorphConsole
MORPHC_BINDIR ?= $(MOOTM_BINDIR)
MORPHC ?= $(MORPHC_BINDIR)/MorphConsole
MORPHC_FLAGS ?= \
	-b share-moot/fsm.new/moot.sym \
	-m share-moot/fsm.new/MIMorph.fst \
	-p -f

##-- Morph: Hack
## MORPHHACK : if "yes", morphhack.perl will not be applied; otherwise not
MORPHHACK ?=yes

ifeq "$(MORPHHACK)" "yes"
 MORPHHACK_INFIX =morphhack
else
 MORPHHACK_INFIX =no-morphhack
endif

MORPHHACK_PROG   =./morphhack.perl

MODEL_HACKED   =$(MODEL).$(MORPHHACK_INFIX)
LCMODEL_HACKED =$(LCMODEL).$(MORPHHACK_INFIX)
TEST_HACKED    =$(TEST).$(MORPHHACK_INFIX)
TRAIN_HACKED   =$(TRAIN).$(MORPHHACK_INFIX)


##-- convert: hack
XMLIFY ?= ./xmlify.perl
OF_XML=xml,pretty

##-- moot: suffix trie stuff
a ?= 0
A ?= 10
theta ?= 0.1017023
SUFFIX_TRIE_LEN ?= $(a)
SUFFIX_TRIE_THRESH ?= $(A)
SUFFIX_THETA  ?= $(theta)

##-- moot: beam stuff
Z ?= 1000
BEAM_CUTOFF ?= $(Z)

##-- Globals: programs: moot
MOOT_BINDIR   ?= $(shell cd .. ; pwd -P)
MOOTRAIN      ?= $(MOOT_BINDIR)/mootrain
MOOTHMM       ?= $(MOOT_BINDIR)/moot
MOOTHMM_FLAGS ?= \
	-Z$(BEAM_CUTOFF)

#	-a$(SUFFIX_TRIE_LEN) -A$(SUFFIX_TRIE_THRESH) --trie-theta=$(SUFFIX_THETA)
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

config:
	@echo "MOOTM_BINDIR=$(MOOTM_BINDIR)"
	@echo "MOOT_BINDIR=$(MOOT_BINDIR)"
	@echo "MORPHHACK=$(MORPHHACK)"
	@echo "MODEL=$(MODEL)"
	@echo "MODEL_HACKED=$(MODEL_HACKED)"
	@echo "TRUTH=$(TRUTH)"


##----------------------------------------------------------------------
##-- don't delete these
.SECONDARY:

##----------------------------------------------------------------------
##-- untagging

t: $(TEST).t

%.t: %.ttt
	cut -f 1 $< > $@

%.t: %.tt
	cut -f 1 $< > $@

%.t: %.mrt
	cut -f 1 $< > $@

##----------------------------------------------------------------------
##-- morph analysis (* -> .mrt, .mr.ttt)
mrt: $(TEST_HACKED).mrt
mrttt: mr-ttt
mr-ttt: $(TEST_HACKED).mr.ttt

##-- analysis: mootm (raw)

%.no-morphhack.mr.ttt: %.ttt $(MOOTM)
	$(MOOTM) -12 $(MOOTM_FLAGS) $< -o $@
%.no-morphhack.mrt: %.t $(MOOTM)
	$(MOOTM) $(MOOTM_FLAGS) $< -o $@

%.morphhack.mr.ttt: %.no-morphhack.mr.ttt $(MORPHHACK_PROG)
	$(MORPHHACK_PROG) --best $< > $@ || (rm -f $@; false)
%.morphhack.mrt: %.no-morphhack.mrt $(MORPHHACK_PROG)
	$(MORPHHACK_PROG) --nobest $< > $@

%.wd.ttt: %.mr.ttt
	rm -f $@
	ln -s $< $@


##----------------------------------------------------------------------
##-- tagging
##   OLD_SUFF	NEW_SUFF	FLAGS				NOTES
##   t		t		t=text
##   tt		tt		m=text,tagged
##   mt		mrt		mr=text,analyzed
##   mttt	mr.ttt		wd=text,tagged,analyzed
##   dmt	dis.wdt		wd=text,tagged,analyzed		disambiguation only, no model .clx
##   cmt	clx.wdt		wd=text,tagged,analyzed		disambiguation with class probs
##   vtt	S.wdt		wd=text,tagged,analyzed		saved ambiguities?

dmt: dis-wdt
cmt: clx-wdt
vtt: S-wdt

dis-tt: dis-wdt
clx-tt: clx-wdt
S-tt: S-wdt

tt: $(TEST).$(MORPHHACK_INFIX).tt
dis-wdt: $(TEST).$(MORPHHACK_INFIX).dis.wdt
clx-wdt: $(TEST).$(MORPHHACK_INFIX).clx.wdt
S-wdt: $(TEST).$(MORPHHACK_INFIX).S.wdt

nott:
	rm -f *.tt *.vtt *.dmt *.cmt *.tnt-tt *.tnt-vtt *.dis.wdt *.clx.wdt *.S.wdt

%.tt: %.t $(MODEL_HACKED).lex $(MODEL_HACKED).123 $(MOOTHMM)
	$(MOOTHMM) -M$(MODEL_HACKED).lex,$(MODEL_HACKED).123 $(MOOTHMM_FLAGS) $< -o $@

%.dis.wdt: %.mrt $(MODEL_HACKED).lex $(MODEL_HACKED).123 $(MOOTHMM)
	$(MOOTHMM) -M$(MODEL_HACKED).lex,$(MODEL_HACKED).123 $(MOOTHMM_FLAGS) $< -o $@

%.clx.wdt: %.mrt $(MODEL_HACKED).lex $(MODEL_HACKED).123 $(MODEL_HACKED).clx $(MOOTHMM)
	$(MOOTHMM) -M$(MODEL_HACKED).lex,$(MODEL_HACKED).123,$(MODEL_HACKED).clx $(MOOTHMM_FLAGS) $< -o $@

%.S.wdt: %.t $(MODEL_HACKED).lex $(MODEL_HACKED).123 $(MOOTHMM)
	$(MOOTHMM) -M$(MODEL_HACKED).lex,$(MODEL_HACKED).123 $(MOOTHMM_FLAGS) -S $< -o $@

##----------------------------------------------------------------------
##-- tagging, lower-case
lc-dmt: lc-dis-wdt
lc-cmt: lc-clx-wdt
lc-vtt: lc-S-wdt

lc-tt: $(TEST).lc.tt
lc-dis-wdt: $(TEST).lc.dis.wdt
lc-clx-wdt: $(TEST).lc.clx.wdt
lc-S-wdt: $(TEST).lc.S.wdt


%.lc.tt: %.lc.t $(LCMODEL_HACKED).lex $(LCMODEL_HACKED).123 $(MOOTHMM)
	$(MOOTHMM) -M$(LCMODEL_HACKED).lex,$(LCMODEL_HACKED).123 $(MOOTHMM_FLAGS) $< -o $@

%.lc.S.wdt: %.lc.t $(LCMODEL_HACKED).lex $(LCMODEL_HACKED).123 $(MOOTHMM)
	$(MOOTHMM) -M$(LCMODEL_HACKED).lex,$(LCMODEL_HACKED).123 $(MOOTHMM_FLAGS) -S $< -o $@

%.lc.mrt: %.mrt
	tr A-ZÄÖÜ a-zäöü < $< > $@

%.lc.dis.wdt: %.lc.mrt $(LCMODEL_HACKED).lex $(LCMODEL_HACKED).123 $(MOOTHMM)
	$(MOOTHMM) -M$(LCMODEL_HACKED).lex,$(LCMODEL_HACKED).123 $(MOOTHMM_FLAGS) $< -o $@

%.lc.clx.wdt: %.lc.mrt $(LCMODEL_HACKED).lex $(LCMODEL_HACKED).123 $(LCMODEL_HACKED).clx $(MOOTHMM)
	$(MOOTHMM) -M$(LCMODEL_HACKED).lex,$(LCMODEL_HACKED).123,$(LCMODEL_HACKED).clx $(MOOTHMM_FLAGS) $< -o $@

##----------------------------------------------------------------------
##-- eval
dis-eval: dis-wdt-eval
clx-eval: clx-wdt-eval

tt-eval: $(TEST).$(MORPHHACK_INFIX).tt.eval
dis-wdt-eval: $(TEST).$(MORPHHACK_INFIX).dis.wdt.eval
clx-wdt-eval: $(TEST).$(MORPHHACK_INFIX).clx.wdt.eval
mr-ttt-eval: $(TEST).$(MORPHHACK_INFIX).mr.ttt.eval

eval: tt-eval dis-wdt-eval clx-wdt-eval

no-eval:
	rm -f *.eval

%.tt.eval: %.tt $(TRUTH) $(MOOTEVAL)
	$(MOOTEVAL) -Iwd -v4 -2 -o $@ $(TRUTH) $<

%.dis.wdt.eval: %.dis.wdt $(TRUTH) $(MOOTEVAL)
	$(MOOTEVAL) -Iwd -v4 -2 -o $@ $(TRUTH) $<

%.clx.wdt.eval: %.clx.wdt $(TRUTH) $(MOOTEVAL)
	$(MOOTEVAL) -Iwd -v4 -2 -o $@ $(TRUTH) $<

%.mr.ttt.eval: %.mr.ttt $(TRUTH) $(MOOTEVAL)
	$(MOOTEVAL) -Iwd -v4 -Iwd -2 -o $@ $(TRUTH) $<


##----------------------------------------------------------------------
##-- error counting
errors: $(EVAL).errors
cerrors: $(EVAL).cerrors
ierrors: $(EVAL).ierrors
icerrors: $(EVAL).icerrors


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
	grep '^..:' $< | grep -v '^..:...:---' | sort | uniq -c | sort -r -n > $@

%.icerrors: %.eval classify-eval.perl
	./classify-eval.perl $< \
	| grep '^..:' | grep -v '^..:...:---' | sort | uniq -c | sort -r -n > $@

##----------------------------------------------------------------------
##-- save tagging data
save: save-stamp
save-stamp: save/$(label).stamp

unsave:
	rm -rf save/$(label).stamp save/$(label)

test: save unsave

save/%.stamp: \
	$(TEST).ttt $(TEST_HACKED).mrt $(TRAIN_HACKED).model $(MOOT_BINDIR)/mootconfig
	for ext in tt dis.wdt clx.wdt ; do \
	  $(MAKE) $(TEST_HACKED).$$ext $(TEST_HACKED).$$ext.eval ;\
	done
	mkdir -p save/$*
	mv \
	   $(MODEL_HACKED).lex $(MODEL_HACKED).123 $(MODEL_HACKED).clx \
	   $(TEST_HACKED).tt $(TEST_HACKED).tt.eval \
	   $(TEST_HACKED).dis.wdt $(TEST_HACKED).dis.wdt.eval \
	   $(TEST_HACKED).clx.wdt $(TEST_HACKED).clx.wdt.eval \
	  save/$*
	rm -f $(MODEL_HACKED).model
	cp -f $(MOOT_BINDIR)/mootconfig save/$*/mootconfig
	touch $@

##----------------------------------------------------------------------
##-- models
model: $(MODEL_HACKED).model
nomodel:
	rm -f $(MODEL_HACKED).model

%.model: %.mr.ttt $(MOOTRAIN)
	$(MOOTRAIN) -lnC $< -o $*
	touch $@

%.lex: ; $(MAKE) $*.model

%.123: ; $(MAKE) $*.model

%.clx: ; $(MAKE) $*.model

##----------------------------------------------------------------------
##-- binary models

hmm: $(MODEL_HACKED).hmm

chmm: $(MODEL_HACKED).chmm

%.hmm: %.model $(MOOTCOMPILE)
	$(MOOTCOMPILE) $(MOOTCOMPILE_FLAGS) -o $@ "$*.lex,$*.123"

%.chmm: %.model $(MOOTDUMP)
	$(MOOTCOMPILE) $(MOOTCOMPILE_FLAGS) -o $@ "$*.lex,$*.123,$*.clx"

##----------------------------------------------------------------------
##-- dumps

dump: $(MODEL_HACKED).dump
cdump: $(MODEL_HACKED).cdump

%.dump: %.model $(MOOTDUMP)
	$(MOOTDUMP) -o $@ "$*.lex,$*.123"

%.cdump: %.model $(MOOTDUMP)
	$(MOOTDUMP) -o $@ "$*.lex,$*.123,$*.clx"

hmm-dump: $(MODEL_HACKED).hmm-dump
chmm-dump: $(MODEL_HACKED).chmm-dump

%.hmm-dump: %.hmm $(MOOTDUMP)
	$(MOOTDUMP) -o $@ $<

%.chmm-dump: %.chmm $(MOOTDUMP)
	$(MOOTDUMP) -o $@ $<

##----------------------------------------------------------------------
##-- XML: conversion text->xml
%.pre-xml: % $(XMLIFY)
	$(XMLIFY) $< > $@

%.ttt.xml: %.ttt.pre-xml $(MOOTCHURN)
	$(MOOTCHURN) $(MOOTCHURN_FLAGS) -Inative,medium -O$(OF_XML),medium -o $@ $<

%.t.xml: %.t.pre-xml $(MOOTCHURN)
	$(MOOTCHURN) $(MOOTCHURN_FLAGS) -Inative,rare -O$(OF_XML),rare -o $@ $<

%.mrt.xml: %.mrt.pre-xml $(MOOTCHURN)
	$(MOOTCHURN) $(MOOTCHURN_FLAGS) -Inative,mediumrare -O$(OF_XML),mediumrare -o $@ $<

%.mr.ttt.xml: %.mr.ttt.pre-xml $(MOOTCHURN)
	$(MOOTCHURN) $(MOOTCHURN_FLAGS) -Inative,welldone -O$(OF_XML),welldone -o $@ $<

##----------------------------------------------------------------------
##-- XML: conversion xml->text
%.post-xml: %.xml $(MOOTCHURN)
	$(MOOTCHURN) $(MOOTCHURN_FLAGS) -Ixml,welldone -Onative,welldone -o $@ $<

##----------------------------------------------------------------------
##-- XML: models
xml_model: $(MODEL).xml.model

no_xml_model: ; rm -f $(MODEL).xml.model

%.xml.model: %.mttt.xml $(MOOTRAIN)
	$(MOOTRAIN) -Ixml,welldone -lnC $< -o $*.xml
	touch $@

%.xml.lex: ; $(MAKE) $*.xml.model

%.xml.123: ; $(MAKE) $*.xml.model

%.xml.clx: ; $(MAKE) $*.xml.model

##----------------------------------------------------------------------
##-- XML: tagging :: TODO: suffix-tweaking!
tt_xml: $(TEST).tt.xml
dmt_xml: $(TEST).dmt.xml
cmt_xml: $(TEST).cmt.xml

%.tt.xml: %.t.xml $(MODEL).xml.lex $(MODEL).xml.123 $(MOOTHMM)
	$(MOOTHMM) -Ixml -O$(OF_XML),welldone -M$(MODEL).xml.lex,$(MODEL).xml.123 $(MOOTHMM_FLAGS) -o $@ $<

%.dmt.xml: %.mt.xml $(MODEL).xml.lex $(MODEL).xml.123 $(MOOTHMM)
	$(MOOTHMM) -Ixml -O$(OF_XML),welldone -M$(MODEL).xml.lex,$(MODEL).xml.123 $(MOOTHMM_FLAGS) -o $@ $<

%.cmt.xml: %.mt.xml $(MODEL).xml.lex $(MODEL).xml.123 $(MODEL).xml.clx $(MOOTHMM)
	$(MOOTHMM) -Ixml -O$(OF_XML),welldone -M$(MODEL).xml.lex,$(MODEL).xml.123,$(MODEL).xml.clx $(MOOTHMM_FLAGS) -o $@ $<

##----------------------------------------------------------------------
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

##----------------------------------------------------------------------
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

##----------------------------------------------------------------------
##-- XML: check
xml_check: $(TEST).tt.xml-check $(TEST).dmt.xml-check $(TEST).cmt.xml-check

%.tt.xml-check: %.tt.post-xml %.tt $(MOOTEVAL)
	$(MOOTEVAL) -v2 -2 -Imedium -o $@ $*.tt $<

%.dmt.xml-check: %.dmt.post-xml %.dmt $(MOOTEVAL)
	$(MOOTEVAL) -v2 -2 -Iwelldone -o $@ $*.dmt $<

%.cmt.xml-check: %.cmt.post-xml %.cmt $(MOOTEVAL)
	$(MOOTEVAL) -v2 -2 -Iwelldone  -o $@ $*.cmt $<

##----------------------------------------------------------------------
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

##----------------------------------------------------------------------
##-- lower-case

lower: lc

lc: $(TEST).lc.ttt $(TRAIN).lc.ttt

nolower: nolc

nolc:
	rm -f *.lc.*

lc-tt: $(TEST).lc.tt

nolc-tt:
	rm -f *.lc.tt *.lc.dmt *.lc.tnt-tt *.lc.tnt-vtt

%.lc.ttt: %.ttt
	tr A-ZÄÖÜ a-zäöü < $< > $@

lc_model: $(LCMODEL).model

%.lc.model: %.lc.ttt $(MOOTRAIN)
	$(MOOTRAIN) -Imedium -lnC $< -o $*.lc
	touch $@

##----------------------------------------------------------------------
##-- tnt-tagging

TNT_AFLAGS ?= -a$(SUFFIX_TRIE_LEN)
TNT_DFLAGS ?= -d4
TNT_UFLAGS ?= -u3
TNT_NFLAGS ?= -n3
TNT_ZFLAGS ?= -Z$(BEAM_CUTOFF)
TNT_FLAGS ?= $(TNT_AFLAGS) $(TNT_DFLAGS) $(TNT_UFLAGS) $(TNT_NFLAGS) $(TNT_ZFLAGS)

notnt:
	rm -f *.tnt-tt *.tnt-tt.*

tnt-tt: $(TEST).tnt-tt

tnt-vtt: $(TEST).tnt-vtt

lc-tnt-tt: $(TEST).lc.tnt-tt

lc-tnt-vtt: $(TEST).lc.tnt-vtt

%.lc.tnt-tt: %.lc.t $(LCMODEL).model
	tnt $(TNT_FLAGS) $(LCMODEL) $< > $@

%.tnt-tt: %.t $(MODEL).model
	tnt $(TNT_FLAGS) $(MODEL) $< > $@

%.tnt-tt.eval: %.tnt-tt %.ttt
	$(MOOTEVAL) -v3 -2 -Imedium $*.ttt $< -o $@


%.lc.tnt-vtt: %.lc.t $(LCMODEL).model
	tnt $(TNT_FLAGS) -z0 -Z0 $(LCMODEL) $< > $@

%.tnt-vtt: %.t $(MODEL).model
	tnt $(TNT_FLAGS) -z0 -Z0 $(MODEL) $< > $@

%.tnt-vtt.eval: %.tnt-vtt %.ttt
	$(MOOTEVAL) -v3 -2 -Imedium $*.ttt $< -o $@

##----------------------------------------------------------------------
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
