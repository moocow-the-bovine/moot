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

//#define DWDST_DFSA_DEBUG
//#undef DWDST_DFSA_DEBUG
//#define DWDST_DFSA_DEBUG_VERBOSE
//#undef DWDST_DFSA_DEBUG_VERBOSE

#if defined(DWDST_DFSA_DEBUG_VERBOSE) && !defined(DWDST_DFSA_DEBUG)
# define DWDST_DFSA_DEBUG
#endif

#include "dwdstTagger.h"
#include "dwdstTypes.h"

#ifdef HAVE_CONFIG_H
//# include "nopackage.h"
# include <dwdstConfig.h>
#endif

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
  FSMSymbolStringSet  opentags;  

  /// set of all PoS tags (to be used for disambig-FST generation)
  FSMSymbolStringSet  alltags;


  // ----- public data: count parameters

  /// Maximum length of n-grams to count.
  /// Caveat: time and space complexity grows exponentially with kmax!
  int kmax;


  /// Whether to allow for incomplete (non-total) categories
  /// when compiling regexes.
  bool allow_incomplete_categories;

  // ----- public data: mid-/low-level stuff

  /// Table of n-gram counts indexed by n-grams.
  NGramTable            ngtable;

  /// Working data for n-ngram generation.
  FSMSymbolStringQueue  stringq;

  /// Tag-to-symbol map for disambiguation fsa skeleton .
  dwdstStringToSymbolMap tags2symbols;

  /// Used by disambigArcCost() when fallback strategy fails.
  float uniGramCount;

private:
  NGramTable::iterator ngti;
  bool is_eos;

  // -- training variables
  FSMSymbolStringSet *curtags;
  FSMSymbolStringSet *tmptags, *swaptags;
  FSMSymbolStringSet::iterator cti;

  NGramVector     theNgram;
  NGramVectorSet *curngrams;
  NGramVectorSet *nextngrams;
  NGramVectorSet *tmpngrams;

  set<NGramVector>::iterator ngi;

  //FSMSymbolStringQueue::reverse_iterator qri;
  FSMSymbolStringQueue::iterator qi;
  FSMSymbolStringSet::iterator qii;


  // -- methods
public:
  // ----- public methods: constructor/destructor

  /// constructor
  dwdstTrainer(FSMSymSpec *mysyms=NULL, FSM *mymorph=NULL)
    : kmax(2), allow_incomplete_categories(true)
  {};

  /// destructor
  ~dwdstTrainer();


  // ----- public methods: FSM generation

  /// Generate an analysis-FSA for unknown tokens.
  FSM *generate_unknown_fsa();

  /// Generate a disambiguation-FSA based on internal n-gram tables.
  /// <b>WARNING: WORK IN PROGRESS!</b>
  FSM *generate_disambig_fsa();


  // ----- mid-/low-level methods: FSM generation

  /// low-level: add an PoS-"arc" to an FSA: this needs an overhaul!
  FSMStateSet fsm_add_pos_arc(FSM *fsm,
			      const FSMState qfrom,
			      const FSMSymbolString &pos,
			      const FSMWeight cost = FSM_default_cost_structure.freecost());

  // ----- mid-level: disambig-fsa utilities

  /// For disambiguation, generate 'tags2symbols', a map from known tags to FSMSymbols.
  /// Also computes 'uniGramCount' -- this behavior should change!
  bool generate_tag_map();

  /// Common preparations for disambig-generation
  bool generate_disambig_init();

  /// Generate a skeleton FSA for the disambiguator.
  FSM *generate_disambig_skeleton();

  /// For disambiguation, apply substitutions indicated by 'tags2symbols' to
  /// the skeleton disambiguation fsa in 'dfsa'.
  FSM *expand_disambig_skeleton();

  /// For skeleton-generation debugging.
  bool save_skeleton_labels(const char *labfilename=NULL);
  /// For skeleton-generation debugging.
  bool save_skeleton_labels(FILE *labfile=NULL);

  // -- low-level: disambig-fsa utilities
  /// Low-level: get the cost of final transition for nGram.
  FSMWeight disambigArcCost(NGramVector &nGram, FSMSymbolString &tagTo, float uniGramCost=0);
  /// Low-level: get the count for a single nGram, using fallback strategy.
  NGramCountFallbacksPair nGramCountFallbacks(NGramVector &nGram);


  // ----- public methods: param-generation: (output-file = DEBUG)

  /// high-level: add a sentence to the training-data
  bool train_from_strings(int argc, char **argv, FILE *out=stdout);

  /// high-level: train from a raw-text stream
  bool train_from_stream(FILE *in=stdin, FILE *out=stdout);


  // ----- mid/low-level

  /// low-level: lookup symbolName in the symspec, producing warnings etc.
  /// Used by training methods for word-boundary symbols.
  FSMSymbol get_symbol_value(const char *methodName, const FSMSymbolString &symbolName);

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
  FSMSymbolStringSet &get_all_pos_tags(const char *filename=NULL) {
    return read_taglist_file(alltags, filename); 
  };

  /// Read in set of 'open-class' tags from a file.
  /// These tags are used for unknown-analysis-FSA generation.
  FSMSymbolStringSet &get_open_class_tags(const char *filename=NULL) {
    read_taglist_file(opentags, filename);
    alltags.insert(opentags.begin(),opentags.end());
    return opentags;
  };


  // --- public methods: low-level : taglist file reading

  /// Low-level: read a PoS-taglist file.
  FSMSymbolStringSet &read_taglist_file(FSMSymbolStringSet &tagset,
					const char *filename=NULL);


  // --- public methods: low-level : initialize & cleanup

  /// Low-level initialization for training methods.
  /// You shouldn't need to call this yourself.
  inline bool init_training_temps(FILE *in=NULL, FILE *out=NULL);

  /// Low-level de-initialization for training methods.
  /// You shouldn't need to call this yourself.
  inline bool cleanup_training_temps();


  // -- public methods: low-level: tagset-iteration utilities

  /// Low-level possible tagset-ngram iteration initializer.
  tagSetIterVector &tagIters_begin(tagSetIterVector &tagIters, FSMSymbolStringSet &tagSet, int len);

  /// Low-level possible tagset-ngram iteration incrementer.
  tagSetIterVector &tagIters_next(tagSetIterVector &tagIters, FSMSymbolStringSet &tagSet);

  /// Low-level possible tagset-ngram iteration termination-test predicate.
  bool tagIters_done(tagSetIterVector &tagIters, FSMSymbolStringSet &tagSet);
};

#endif // _DWDST_TRAINER_H_



