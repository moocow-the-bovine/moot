# Makefile for moot data

include config.mak

##-- Globals: data
TEST = utest
TRAIN = utrain
MODEL = $(TRAIN)
label = label

EVAL = $(TEST).dmtt
TRUTH = $(TEST).ttt

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

##-- Globals: programs: moot
MOOTRAIN = ../mootrain
MOOTHMM = ../moot
#MOOTHMM_FLAGS = -d1024
MOOTHMM_FLAGS =
MOOTEVAL = ../mooteval
MOOTCOMPILE = ../mootcompile
MOOTDUMP = ../mootdump

all:
	@echo "Usage: make TEST=test TRAIN=train label=label {save|test}"


##-- don't delete these
.SECONDARY:

##--- untagging

t: $(TEST).t

%.t: %.ttt
	cut -f 1 $< > $@

%.t: %.tt
	cut -f 1 $< > $@

%.t: %.mtt
	cut -f 1 $< > $@

##-- morph analysis
mtt: $(TEST).mtt

##-- analysis: mootm
%.m0ttt: %.ttt $(MOOTM)
	$(MOOTM) -12 $(MOOTM_FLAGS) $< -o $@

%.mttt: %.m0ttt $(MORPHHACK)
	$(MORPHHACK) --best $< > $@

%.m0tt: %.t $(MOOTM)
	$(MOOTM) $(MOOTM_FLAGS) $< -o $@

%.mtt: %.m0tt $(MORPHHACK)
	$(MORPHHACK) --nobest $< > $@

##-- analysis: MorphConsole
#%.m0tt: %.t $(MORPHC)
#	$(MORPHC) $(MORPHC_FLAGS) $< > $@
#
#%.m1tt: %.m0tt $(MORPHHACK)
#	$(MORPHHACK) --nobest $< > $@
#
#%.mttt: %.ttt
#	$(MAKE) $*.m1tt
#	cut -f2- $*.m1tt | paste $< - > $@
#
#%.mtt: %.mttt
#	cut -f 1,3- $< > $@
#
#%.mtt: %.m1tt
#	ln $< $@


##-- tagging
tt: $(TEST).tt

dmtt: $(TEST).dmtt

%.tt: %.t $(MODEL).lex $(MODEL).lex $(MODEL).123 $(MODEL).clx $(MOOTHMM)
	$(MOOTHMM) -M$(MODEL).lex,$(MODEL).123 $(MOOTHMM_FLAGS) $< -o $@

%.dmtt: %.mtt $(MODEL).lex $(MODEL).lex $(MODEL).123 $(MOOTHMM)
	$(MOOTHMM) -M$(MODEL).lex,$(MODEL).123 $(MOOTHMM_FLAGS) $< -o $@

%.cmtt: %.mtt $(MODEL).lex $(MODEL).lex $(MODEL).123 $(MODEL).clx $(MOOTHMM)
	$(MOOTHMM) -M$(MODEL).lex,$(MODEL).123,$(MODEL).clx $(MOOTHMM_FLAGS) $< -o $@

##-- eval
eval: $(EVAL).eval

%.tt.eval: %.tt $(TRUTH) $(MOOTEVAL)
	$(MOOTEVAL) -v4 -2 -o $@ $*.ttt $<

%.dmtt.eval: %.dmtt $(TRUTH) $(MOOTEVAL)
	$(MOOTEVAL) -v4 -2 -o $@ $*.ttt $<

%.cmtt.eval: %.cmtt $(TRUTH) $(MOOTEVAL)
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
	$(TEST).ttt $(TEST).mtt $(TRAIN).model
	for ext in tt dmtt cmtt ; do \
	  $(MAKE) $(TEST).$$ext $(TEST).$$ext.eval ;\
	done
	mkdir -p save/$*
	mv \
	   $(TRAIN).lex $(TRAIN).123 $(TRAIN).clx \
	   $(TEST).tt $(TEST).tt.eval \
	   $(TEST).dmtt $(TEST).dmtt.eval \
	   $(TEST).cmtt $(TEST).cmtt.eval \
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
	$(MOOTCOMPILE) -o $@ "$*.lex,$*.123"

%.chmm: %.model $(MOOTDUMP)
	$(MOOTCOMPILE) -o $@ "$*.lex,$*.123,$*.clx"


##-- dumps

dump: $(MODEL).dump
cdump: $(MODEL).cdump

%.dump: %.model $(MOOTDUMP)
	$(MOOTDUMP) -o $@ "$*.lex,$*.123"

%.cdump: %.model $(MOOTDUMP)
	$(MOOTDUMP) -o $@ "$*.lex,$*.123,$*.clx"

##-- cleanup
clean:
	rm -f *~ *.t *.tt *.dmtt tmp.res

realclean: clean
	rm -f \
	  *.lex *.123 *.clx \
	  *.mtt *.m0tt *.m1tt *.amtt \
	  *.mttt *.m0ttt *.m1ttt \
	  *.model *.hmm *.chmm *.dump *.cdump \
	  *.errors *.cerrors *.icerrors *.eval
