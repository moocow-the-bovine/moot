# Makefile for moot data

all:
	@echo No default rules!

##-- don't delete these
.SECONDARY:

##--- untagging
%.t: %.ttt
	cut -f 1 $< > $@

%.t: $.tt
	cut -f 1 $< > $@

%.t: %.mtt
	cut -f 1 $< > $@

##-- analysis
MOOTM = /usr/local/bin/mootm
MOOTM_FLAGS = -m /usr/local/share/moot/dwdst-stts-nofeatures.fst
%.mtt: %.t $(MOOTM)
	$(MOOTM) $(MOOTM_FLAGS) $< -o $@

%.mttt: %.ttt $(MOOTM)
	$(MOOTM) -12 $(MOOTM_FLAGS) $< -o $@

##-- tagging
MOOTHMM = ../moothmm
MODEL = utrain
MOOTHMM_FLAGS =
%.tt: %.t $(MODEL).lex $(MODEL).123 $(MOOTHMM)
	$(MOOTHMM) -M$(MODEL) $(MOOTHMM_FLAGS) $< -o $@

%.dmtt: %.mtt $(MODEL).lex $(MODEL).123 $(MODEL).clx
	$(MOOTHMM) -M$(MODEL) $(MOOTHMM_FLAGS) $< -o $@

##-- models
MOOTRAIN = ../mootrain
%.lex: %.mttt $(MOOTHMM)
	$(MOOTRAIN) -lnC $< -o $*

%.123: %.mttt $(MOOTHMM)
	$(MOOTRAIN) -lnC $< -o $*

%.clx: %.mttt $(MOOTHMM)
	$(MOOTRAIN) -lnC $< -o $*

%.lex: %.ttt $(MOOTHMM)
	$(MOOTRAIN) -ln $< -o $*

%.123: %.ttt $(MOOTHMM)
	$(MOOTRAIN) -ln $< -o $*


##-- cleanup
clean:
	rm -f *~ *.t *.tt *.dmtt

realclean: clean
	rm -f *.lex *.123 *.clx *.mtt *.mttt *.amtt
