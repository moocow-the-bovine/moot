# Makefile for moot data

##-- Globals: data
TEST = utest
TRAIN = utrain
MODEL = $(TRAIN)
label = label

EVAL = $(TEST).dmtt
TRUTH = $(TEST).ttt

##-- Globals: programs
MOOTM = /usr/local/bin/mootm
MORPHHACK = ./morphhack.perl
MOOTM_FLAGS = -m /usr/local/share/moot/dwdst-stts-nofeatures.fst -d1024
MOOTRAIN = ../mootrain
MOOTHMM = ../moot
MOOTHMM_FLAGS =
MOOTEVAL = ../mooteval

all:
	@echo "Usage: make TEST=test TRAIN=train label=label save"


##-- don't delete these
.SECONDARY:

##--- untagging

t: $(TEST).t

%.t: %.ttt
	cut -f 1 $< > $@

%.t: $.tt
	cut -f 1 $< > $@

%.t: %.mtt
	cut -f 1 $< > $@

##-- analysis
mtt: $(TEST).mtt

%.mtt: %.mttt
	cut -f 1,3- $< > $@

%.m0ttt: %.ttt $(MOOTM)
	$(MOOTM) -12 $(MOOTM_FLAGS) $< -o $@

%.mttt: %.m0ttt $(MORPHHACK)
	$(MORPHHACK) --best $< > $@

%.m0tt: %.t $(MOOTM)
	$(MOOTM) $(MOOTM_FLAGS) $< -o $@

%.mtt: %.m0tt $(MORPHHACK)
	$(MORPHHACK) --nobest $< > $@

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
	grep '^.b' $< \
	| sort \
	| uniq -c \
	| sort -r -n \
	> $@

%.tt.cerrors:
	echo "WHOLE FILE" > $@

%.errors: %.eval
	grep '^.b:' $< \
	| grep -v '^..:...:---' \
	| sort \
	| uniq -c \
	| sort -r -n \
	> $@

%.cerrors: %.eval classify-eval.perl
	./classify-eval.perl $< \
	| grep '^.b:' $< \
	| grep -v '^..:...:---' \
	| sort \
	| uniq -c \
	| sort -r -n \
	> $@

%.ierrors: %.eval
	grep '^.' $< \
	| grep -v '^..:...:---' $< \
	| sort \
	| uniq -c \
	| sort -r -n \
	> $@

%.icerrors: %.eval classify-eval.perl
	./classify-eval.perl $< \
	| grep '^.' \
	| grep -v '^..:...:---' \
	| sort \
	| uniq -c \
	| sort -r -n \
	> $@

##-- save
save: save/$(label).stamp

unsave:
	rm -rf save/$(label).stamp

save/%.stamp: $(TEST).ttt $(TEST).mtt $(TRAIN).model
	$(MAKE) $(TEST).tt.eval  $(TEST).tt.errors
	$(MAKE) $(TEST).dmtt.eval $(TEST).dmtt.errors $(TEST).dmtt.cerrors
	$(MAKE) $(TEST).cmtt.eval $(TEST).cmtt.errors $(TEST).cmtt.cerrors
	mkdir -p save/$*
	mv \
	   $(TRAIN).lex $(TRAIN).123 $(TRAIN).clx \
	   $(TEST).tt $(TEST).tt.eval $(TEST).tt.errors \
	   $(TEST).dmtt $(TEST).dmtt.eval $(TEST).dmtt.errors $(TEST).dmtt.cerrors \
	   $(TEST).cmtt $(TEST).cmtt.eval $(TEST).cmtt.errors $(TEST).cmtt.cerrors \
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

##-- dumps

dump: $(MODEL).dump
cdump: $(MODEL).cdump

%.dump: %.model $(MOOTHMM)
	$(MOOTHMM) -M$*.lex,$*.123 -D -o $@

%.cdump: %.model $(MOOTHMM)
	$(MOOTHMM) -M$*.lex,$*.123,$*.clx -D -o $@

##-- cleanup
clean:
	rm -f *~ *.t *.tt *.dmtt

realclean: clean
	rm -f *.lex *.123 *.clx *.mtt *.mttt *.amtt
