/* -*- Mode: C++ -*- */
/*--------------------------------------------------------------------------
 * File: dwdstTagger.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger for DWDS project : shared headers
 *--------------------------------------------------------------------------*/

#ifndef _DWDST_TAGGER_H_
#define _DWDST_TAGGER_H_

/// Define this to enable verbose debugging
#define DWDST_DEBUG
//#undef DWDST_DEBUG


#include <stdio.h>

#include <FSMTypes.h>
#include <FSMSymSpec.h>
#include <FSM.h>

#include "dwdstTypes.h"
#include "dwdstTaggerLexer.h"
//#include "dwdstDisambiguator.h"

/*--------------------------------------------------------------------------
 * dwdstTagger : tagger class
 *--------------------------------------------------------------------------*/

/// High-level class to perform morphological analysis and PoS-tagging.
class dwdstTagger {
public:
  // ------ public typedefs
  /** Single POS tag, without features */
  typedef FSMSymbol Tag;

  /** Single morphological analysis, including features */
  typedef FSM::FSMWeightedSymbolVector MorphAnalysis;

  /** Set of POS tags, no features */
  typedef set<Tag> TagSet;

  /** Set of morphological analyses, including features */
  typedef set<MorphAnalysis> MorphAnalysisSet;

public:
  // ------ public data
    
  /** the symspec to use for tagging */
  FSMSymSpec *syms;
    
  /** morphology FST to use for tagging */
  FSM *morph;
    
  /** disambiguator class for ?-style disambiguation : NULL for no disambiguation*/
  //dwdstDisambiguator *dis;
  
  /** lexer for tagging methods */
  dwdstTaggerLexer lexer;
  
  /** end-of-sentence marker for tagger output */
  string eos;
  
  // ------- flags

  /** output in AVM (madwds-"vector") mode? (default = false) */
  bool want_avm;
  
  /** output in old MA-BBAW format? (default = false) */
  bool want_mabbaw_format;

  /** track ntokens and nuknown? (default = false) */
  bool track_statistics;
  
  /**
   * output verbosity level (0..2)
   * 0: silent (no output but warnings/errors)
   * 1: output tagged text
   * default = 1
   */
  int verbose;

  // -- statistical data
  /** number of tokens processed */
  unsigned int ntokens;
  /** number of unknown tokens encountered */
  unsigned int nunknown;
  
private:
  // ------ private data
  bool i_made_syms;
  bool i_made_morph;
    //set<FSMSymbol>        fsm_tags_tmp;

public:
  // -- file/stream variables
  /** input stream for tagging */
  FILE *infile;
  /** output stream for tagging */
  FILE *outfile;

  /** name of input stream (for error reporting) */
  char *srcname;

  // -- temporary variables
  /** pre-allocated temporary: morphological analysis output FSA */
  FSM *result, *tmp;
  /** pre-allocated temporary: current token */
  FSMSymbolString curtok_s;
  /** pre-allocated temporary: current token */
  char *curtok;

  // -- more temporaries
  /** pre-allocated temporary: morphological analysis output (for tag_token()) */
  MorphAnalysisSet analyses;

  /** pre-allocated temporary: for print_analyses() */
  MorphAnalysisSet::iterator analyses_i;

  /** pre-allocated temporary: for print_analyses() */
  FSMSymbolString analysis_str;

public:
  // -- public methods: constructor/destructor
  /** constructor */
  dwdstTagger(FSMSymSpec *mysyms=NULL, FSM *mymorph=NULL);
  /** destructor */
  ~dwdstTagger();

  // -- public methods: loading

  /** load a morphological symbols-file */
  FSMSymSpec *load_morph_symbols(char *syms_file);

  /** load morphology FST */
  FSM *load_morph(char *morph_file) {
    return load_fsm_file(morph_file, &morph, &i_made_morph);
  }

  // -- low-level public methods: loading
  /** low-level FSTfile loading utility */
  FSM *load_fsm_file(char *fsm_file, FSM **fsm, bool *i_made_fsm=NULL);

  // -- public methods: tagging
  /** High-level tagging interface: string input */
  bool tag_strings(int argc, char **argv, FILE *out=stdout, char *infilename=NULL);

  /** High-level tagging interface: file input */
  bool tag_stream(FILE *in=stdin, FILE *out=stdout, char *infilename=NULL);


  // -- public methods: mid-level: tagging : i/o
  /**
   * mid-level tagging utility: tag a single token
   * 'token' defaults to 'curtok'.
   * Clears, populates, and returns 'analyses'.
   *
   * No printing initiated by this method.
   */
  inline const MorphAnalysisSet &tag_token(char *token = NULL);

  /**
   * mid-level tagging utility: tag end-of-sentence i/o
   * Outputs EOS marker if flags permit.
   */
  inline void tag_eos(void);

  // -- public methods: sanity checks
  /** tagging utility: sanity check */
  bool can_tag(void) const
  {
    return (syms
	    && morph
	    && *morph);
  }

  //-------------------------------------
  // low-level tagging utilities : output

  /**
   * Prints analyses to the currently selected output stream.
   * 'token' defaults to 'curtok',
   * 'an' defaults to 'analyses' member, 'out' defaults to 'outfile'
   * data member.
   *
   * This method does not check the 'verbose' flag -- do that
   * elsewhere.
   */
  inline void print_token_analyses(const char *token = NULL,
				   MorphAnalysisSet *an = NULL,
				   FILE *out = NULL);

  //-------------------------------------
  // debugging & error reporting etc.

  /** Convert a symbol-vector to a numeric string */
  string symbol_vector_to_ascii(const FSM::FSMSymbolVector v);

  /** Stringify a token-analysis-set (weighted-vector version) */
  string analyses_to_string(const set<FSM::FSMWeightedSymbolVector> &analyses);

  /** Stringify a token-analysis-set (weighted-string-version) */
  string analyses_to_string(const set<FSM::FSMStringWeight> &analyses);

  /** Stringify a token-analysis-set (numeric-tags version) */
  string analyses_to_string(const set<FSMSymbol> &analyses);

  /** Stringify a token-analysis-set (string-tags version) */
  string analyses_to_string(const set<FSMSymbolString> &analyses);

  /** Error reporting */
  void carp(char *fmt, ...);

};

#endif // _DWDST_TAGGER_H_
