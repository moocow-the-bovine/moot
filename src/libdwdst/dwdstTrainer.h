/* -*- Mode: C++ -*- */
/*--------------------------------------------------------------------------
 * File: dwdstTrainer.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + Trainer for KDWDS PoS-tagger: shared headers
 *--------------------------------------------------------------------------*/

#ifndef _DWDST_TRAINER_H_
#define _DWDST_TRAINER_H_

//#define DWDST_DEBUG
//#undef DWDST_DEBUG
//#define DWDST_FSTGEN_DEBUG
//#undef DWDST_FSTGEN_DEBUG
//#define DWDST_PARGEN_DEBUG
//#undef DWDST_PARGEN_DEBUG

#ifdef HAVE_CONFIG_H
//# include "nopackage.h"
# include <dwdstConfig.h>
#endif

#include "dwdstTagger.h"
#include "dwdstTypes.h"

using namespace std;

/*--------------------------------------------------------------------------
 * dwdstTrainer : tagger-trainer class
 *--------------------------------------------------------------------------*/

/// High-level class to gather and format training data for a dwdstTagger.
class dwdstTrainer : public dwdstTagger {
public:
  // -- typedefs (see dwdstTypes.h)
private:
  // -- private data
public:
  // ----- public data: tagsets

  /// set of PoS tags for open-classes (used for unknown-FST generation)
  set<FSMSymbolString>  opentags;  

  /// set of all PoS tags (to be used for disambig-FST generation)
  set<FSMSymbolString>  alltags;


  // ----- public data: count parameters

  /// Maximum length of n-grams to count.
  /// Caveat: time and space complexity grows exponentially with kmax!
  int kmax;

  /// Table of n-gram counts indexed by n-grams.
  NGramTable            ngtable;

  /// Working data for n-ngram generation.
  FSMSymbolStringQueue  stringq;

private:
  NGramTable::iterator ngti;
  bool is_eos;

  // -- training variables
  set<FSMSymbolString> *curtags;
  set<FSMSymbolString> *tmptags, *swaptags;
  set<FSMSymbolString>::iterator cti;

  NGramVector      theNgram;
  set<NGramVector> *curngrams;
  set<NGramVector> *nextngrams;
  set<NGramVector> *tmpngrams;

  set<NGramVector>::iterator ngi;

  //FSMSymbolStringQueue::reverse_iterator qri;
  FSMSymbolStringQueue::iterator qi;
  set<FSMSymbolString>::iterator qii;


  // -- methods
public:
  // ----- public methods: constructor/destructor

  /// constructor
  dwdstTrainer(FSMSymSpec *mysyms=NULL, FSM *mymorph=NULL) {};

  /// destructor
  ~dwdstTrainer();

  // ----- public methods: FSM generation

  /// generate an analysis-FSA for unknown tokens
  FSM *generate_unknown_fsa();

  /// generate a disambiguation-FSA based on internal n-gram tables
  /// (not yet implemented!)
  FSM *generate_disambig_fsa();

  // ----- mid-/low-level methods: FSM generation

  /// low-level: add an PoS-"arc" to an FSA: this needs an overhaul!
  set<FSMState> fsm_add_pos_arc(FSM *fsm, const FSMState qfrom, const FSMSymbolString &pos,
				const FSMWeight cost = FSM_default_cost_structure.freecost());

  // ----- public methods: param-generation: (output-file = DEBUG)

  /// high-level: add a sentence to the training-data
  bool train_from_strings(int argc, char **argv, FILE *out=stdout);

  /// high-level: train from a raw-text stream
  bool train_from_stream(FILE *in=stdin, FILE *out=stdout);

  // ----- mid/low-level

  /// low-level: training for the 'current' token
  inline void train_next_token(void);

  /// low-level: training for end-of-sentence pseudo-token.
  inline void train_eos(void);


  // ----- public methods: param-generation: parameter-file read/write

  /// load a TnT-style parameter file (buggy!)
  bool load_param_file(FILE *in=stdin,const char *filename=NULL);  // NYI

  /// save a TnT-style parameter file
  bool save_param_file(FILE *out=stdout);


  // -- public methods: mid-level: listfile-reading

  /// Read in a list of all PoS tags from filename.
  /// Any PoS tags encountered during training do not need
  /// to be in this file.
  set<FSMSymbolString> get_all_pos_tags(const char *filename=NULL) {
    return read_taglist_file(alltags, filename); 
  };

  /// Read in set of 'open-class' tags from a file.
  /// These tags are used for unknown-analysis-FSA generation.
  set<FSMSymbolString> get_open_class_tags(const char *filename=NULL) {
    read_taglist_file(opentags, filename);
    alltags.insert(opentags.begin(),opentags.end());
    return opentags;
  };


  // --- public methods: low-level : taglist file reading

  /// Low-level: read a PoS-taglist file.
  set<FSMSymbolString> read_taglist_file(set<FSMSymbolString> &tagset, const char *filename=NULL);


  // --- public methods: low-level : initialize & cleanup

  /// Low-level initialization for training methods.
  /// You shouldn't need to call this yourself.
  inline bool init_training_temps(FILE *in=NULL, FILE *out=NULL);

  /// Low-level de-initialization for training methods.
  /// You shouldn't need to call this yourself.
  inline bool cleanup_training_temps();

  // -- public methods: low-level: tagset-iteration utilities
  /// Low-level possible tagset-ngram iteration initializer.
  tagSetIterVector &tagIters_begin(tagSetIterVector &tagIters,set<FSMSymbolString> &tagSet,int len);
  /// Low-level possible tagset-ngram iteration incrementer.
  tagSetIterVector &tagIters_next(tagSetIterVector &tagIters,set<FSMSymbolString> &tagSet);
  /// Low-level possible tagset-ngram iteration termination-test predicate.
  bool tagIters_done(tagSetIterVector &tagIters,set<FSMSymbolString> tagSet);

  // -- public methods: low-level: disambig-fsa utilities
  /// Low-level: get the cost of final transition for nGram.
  FSMWeight disambigArcCost(NGramVector &nGram, FSMSymbolString &tagTo);
  /// Low-level: get the count for a single nGram, using fallback strategy.
  NGramCountFallbacksPair nGramCountFallbacks(NGramVector &nGram);
};

#endif // _DWDST_TRAINER_H_



