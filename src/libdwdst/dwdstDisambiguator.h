/* -*- Mode: C++ -*- */
/*--------------------------------------------------------------------------
 * File: dwdstDisambiguator.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + Disambiguiating PoS tagger for DWDS project : shared headers
 *--------------------------------------------------------------------------*/

#ifndef _DWDST_DISAMBIGUATOR_H_
#define _DWDST_DISAMBIGUATOR_H_

/// Define this to enable verbose debugging
//#define DWDSTD_DEBUG
//#undef DWDSTD_DEBUG


#include <stdio.h>

#include <FSMTypes.h>
#include <FSMSymSpec.h>
#include <FSM.h>

#ifdef HAVE_CONFIG_H
/*# include "nopackage.h"*/
# include <dwdstConfig.h>
#endif

#include "dwdstTypes.h"
#include "dwdstParamCompiler.h"


/*--------------------------------------------------------------------------
 * dwdstDisambiguator : disambiguator class
 *--------------------------------------------------------------------------*/

/// High-level class to perform morphological analysis and PoS-tagging.
class dwdstDisambiguator {
public:
  // ------ public typedefs
  typedef enum {
    dClassLookupFailed=1,
    dSentenceLookupFailed=2,
    dSentenceLookupAmbig=4
  } dwdstdFlag;
public:
  // ------ public data

  /** The (output-)symspec to use for disambiguation */
  FSMSymSpec *syms;

  /** FST to use for disambiguation */
  FSM *dfst;

  /** name of the 'universal' ambiguity-class */
  //string bottom_class;
  string bottom;

  /** name of the 'universal' tag */
  //string bottom_tag;

  /** flags : see dwdstdFlag */
  unsigned flags;

  /** verbose output level (0..2)
   * Values:
   *  0 : quiet
   *  1 : enable nsentences, nunknown_classes, nunknown_sentences tracing
   *  2 : enable verbose errors messages to stderr
   */
  int verbose;

  /** number of sentences disambiguated */
  unsigned nsentences;

  /** number of unknown ambiguity-classes encountered */
  unsigned nunknown_classes;

  /** number of failed sentence-disambiguations */
  unsigned nunknown_sentences;

  /** number of ambiguous sentence-disambiguations */
  unsigned nambig_sentences;

  /** Mapping of input-symbol-vectors to ambiguity-classes */
  dwdstSymbolVector2SymbolMap class2sym;

  /** sentence buffer (input symbols converted to ambiguity-classes) */
  FSM::FSMSymbolVector sentbuf;

protected:
  // ------ protected data
  bool i_made_syms;
  bool i_made_dfst;

  // -- temporary variables
  /** pre-allocated temporary */
  FSM *result;
  /** pre-allocated temporary */
  FSM *tmp;

  /** pre-allocated buffer for lookups */
  FSM::FSMSymbolVector class_key;

  /** pre-allocated temporary for lookups */
  dwdstSymbolVector2SymbolMap::iterator cv2si;

  /** pre-allocated temporary for weighted-vector conversion */
  set<FSM::FSMWeightedSymbolVector>::const_iterator wsvsi;

  /** pre-allocated temporary for unweighted-vector conversion */
  set<FSM::FSMSymbolVector>::const_iterator usvsi;

  /** pre-allocated temporary for unweighted-single-tag conversion */
  set<FSMSymbol>::const_iterator ssi;

  /** pre-allocated temporary: output */
  set<FSM::FSMWeightedSymbolVector> nresults;
  /** pre-allocated temporary: output */
  set<FSM::FSMWeightedSymbolVector>::iterator nri;

  /** pre-allocated temporary: result buffer generation */
  FSM::FSMSymbolVector::const_iterator svi;

  /** pre-allocated result buffer: string output */
  vector<const FSMSymbolString *> string_result;
  /** pre-allocated result buffer: numeric output (fallback only!) */
  FSM::FSMSymbolVector vector_result;

public:
  // -- public methods: constructor/destructor
  /** constructor */
  dwdstDisambiguator(FSMSymSpec *mysyms=NULL, FSM *mydfst=NULL);
  /** destructor */
  ~dwdstDisambiguator();

  // -- initialization

  /** Load an (output-)symbols file */
  FSMSymSpec *load_symbols_file(char *filename);

  /** Load a disambiguation FST file */
  FSM *load_fsm_file(char *filename);

  /** Load an alphabet file */
  dwdstSymbolVector2SymbolMap *load_alphabet_file(char *filename,
						  FSMSymSpec *input_symbols,
						  bool use_short_classnames=true);

  // -- disambiguation methods

  /** Clear the sentence buffer */
  inline void new_sentence(void)
  {
    if (verbose>0) nsentences++;
    sentbuf.clear();
  }

  /**
   * Append the next token to the sentence buffer.
   * Note: none of the vectors in 'analyses' should be weighted:
   * really, this just means they should be sorted in 'istr' order.
   * Returns false if the analysis-class was not found: in this
   * case, 'BOTTOM' has been appended in place of the missing
   * ambiguity-class symbol.
   */
  bool append_token_analyses(const set<FSM::FSMWeightedSymbolVector> &analyses);

  /** Append the next token to the sentence buffer (slightly safer) */
  bool append_token_analyses(const set<FSM::FSMSymbolVector> &analyses);

  /** Append the next token to the sentence buffer (slightly safer yet) */
  bool append_token_analyses(const set<FSMSymbol> &analyses);

  /**
   * Disambiguate the current sentence buffer.
   * Each symbol in the vector returned should correspond to
   * a single input token.
   */
  vector<const FSMSymbolString *> &disambiguate_sentence_string(void);

  /**
   * Disambiguate the current sentence buffer.
   * Each symbol in the vector returned should correspond to
   * a single input token.
   */
  const FSM::FSMSymbolVector &disambiguate_sentence_vector(void);

  //-------------------
  // debugging methods
  /**
   * Dump the vector-to-symbol map to a C stream (default=stdout)
   */
   void dump_class_map(FILE *file=NULL);

  /**
   * Dump a symbol-vector to a C stream (default=stdout)
   */
   void dump_symbol_vector(const char *label, const FSM::FSMSymbolVector v, FILE *file=NULL);

  /**
   * Dump a sentence-vector to a C stream (default=stdout)
   */
   void dump_sentence_vector(const char *label, const FSM::FSMSymbolVector s, FILE *file=NULL);
};

#endif // _DWDST_TAGGER_H_
