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

#include "dwdstTaggerLexer.h"
#include "dwdstDisambiguator.h"

#ifdef HAVE_CONFIG_H
/*# include "nopackage.h"*/
# include <dwdstConfig.h>
#endif

/*--------------------------------------------------------------------------
 * dwdstTagger : tagger class
 *--------------------------------------------------------------------------*/

/// High-level class to perform morphological analysis and PoS-tagging.
class dwdstTagger {
public:
  // ------ public data

  /** the symspec to use for tagging */
  FSMSymSpec *syms;

  /** morphology FST to use for tagging */
  FSM *morph;

  /** hard-coded analysis FSA for unknown-token analyses (OBSOLETE) */
  FSM *ufsa;
  /** disambiguation FSA (not yet implemented) (OBSOLETE) */
  FSM *dfsa;

  /** disambiguator class for Kempe-style disambiguation : NULL for no disambiguation*/
  dwdstDisambiguator *dis;

  /** lexer for tagging methods */
  dwdstTaggerLexer lexer;

  /** end-of-sentence marker for tagger output */
  string eos;

  FSMSymbolString wordStart;    /**< beginning-of-word symbol, used for disambiguation (OBSOLETE). */
  FSMSymbolString wordEnd;      /**< end-of-word symbol, used for disambiguation (OBSOLETE). */

  // ------- flags

  /** output in AVM (madwds-"vector") mode? */
  bool want_avm;

  /** output numeric FSMSymbol sequences (dwdst-"vector" mode) ? */
  bool want_numeric;
  //bool want_binary;

  /** output morphosyntactic features as well as PoS tags? */
  bool want_features;

  /** output in TnT format? */
  bool want_tnt_format;

  /** for disambiguation: output only classes? */
  bool want_classes_only;

  /** verbose output level (0..2) (enable ntokens and nunknown tracking)? */
  int verbose;

  // -- verbose data
  /** for verbose mode */
  unsigned int ntokens;
  /** for verbose mode */
  unsigned int nunknown;

private:
  // ------ private data
  bool i_made_syms;
  bool i_made_morph;
  bool i_made_ufsa;
  bool i_made_dfsa;
  set<FSMSymbol>        fsm_tags_tmp;

protected:
  // -- file variables
  /** input file for tagging */
  FILE *infile;
  /** output file for tagging */
  FILE *outfile;

  /** name of input file (for error reporting) */
  char *srcname;

  // -- temporary variables
  /** pre-allocated temporary */
  FSM *result, *tmp;
  /** pre-allocated temporary */
  FSMSymbolString s;
  /** pre-allocated temporary */
  char *token;

  // -- all features, string-mode
  /** pre-allocated temporary: all features */
  set<FSM::FSMStringWeight> results;
  /** pre-allocated temporary: all features */
  set<FSM::FSMStringWeight>::iterator ri;

  // -- tags only, strings
  /** pre-allocated temporary: tags-only */
  set<FSMSymbolString> tagresults;
  /** pre-allocated temporary: tags-only */
  set<FSMSymbolString>::iterator tri;


  // -- all features, numeric
  /** pre-allocated temporary: numeric, all features */
  set<FSM::FSMWeightedSymbolVector> nresults;
  /** pre-allocated temporary: numeric, all features */
  set<FSM::FSMWeightedSymbolVector>::iterator nri;
  /** pre-allocated temporary: numeric */
  FSM::FSMSymbolVector::const_iterator svi;

  // -- tags only, numeric
  /** pre-allocated temporary: tags-only */
  set<FSMSymbol> ntagresults;
  /** pre-allocated temporary: tags-only */
  set<FSMSymbol>::iterator ntri;

  // -- disambiguating mode
  /** pre-allocated token-buffer: disambiguating-mode */
  vector<FSMSymbolString> sentence_tokens;

  /** pre-allocated token-buffer iterator: disambiguating-mode */
  vector<FSMSymbolString>::const_iterator sti;

  /** pre-allocated iterator: disambiguating-mode, strings */
  vector<const FSMSymbolString *>::const_iterator dsti;
  /* pre-allocated iterator: disambiguating-mode, numeric */
  FSM::FSMSymbolVector::const_iterator dnti;


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

  /** load unknown FSA (see 'ufsa') [OBSOLETE] */
  FSM *load_unknown_fsa(char *ufsa_file) {
    return load_fsm_file(ufsa_file, &ufsa, &i_made_ufsa);
  }

  /** load disambiguation FSA (see 'disambig') [OBSOLETE] */
  FSM *load_disambig_fsa(char *dfsa_file) {
    return load_fsm_file(dfsa_file, &dfsa, &i_made_dfsa);
  }

  //-------------------------------------
  // Kempe-style disambiguator: loading

  /** load disambiguation symbols-file */
  FSMSymSpec *load_disambig_symbols(char *filename)
  {
    if (!dis) dis = new dwdstDisambiguator();
    return dis->load_symbols_file(filename);
  }

  /** load disambiguation FST */
  FSM *load_disambig_fst(char *filename)
  {
    if (!dis) dis = new dwdstDisambiguator();
    return dis->load_fsm_file(filename);
  }
  /** load disambiguation alphabet */
  dwdstSymbolVector2SymbolMap *load_disambig_alphabet(char *filename,
						      bool use_short_classnames=true)
  {
    if (!dis) dis = new dwdstDisambiguator();
    return dis->load_alphabet_file(filename,syms,use_short_classnames);
  }

  // end Kempe-style disambiguator: loading
  //-------------------------------------


  // -- low-level public methods: loading
  /** low-level FSTfile loading utility */
  FSM *load_fsm_file(char *fsm_file, FSM **fsm, bool *i_made_fsm=NULL);

  // -- public methods: tagging
  /** High-level tagging interface: string input */
  bool tag_strings(int argc, char **argv, FILE *out=stdout, char *infilename=NULL);
  /** High-level tagging interface: file input */
  bool tag_stream(FILE *in=stdin, FILE *out=stdout, char *infilename=NULL);

  // -- public methods: mid-level: tagging : i/o
  /** low-level tagging utility: tag a single token */
  inline void tag_token(void);

  /** low-level tagging utility: tag end-of-sentence i/o */
  inline void tag_eos(void);

  // -- public methods: sanity checks
  /** tagging utility: sanity check */
  bool can_tag(void) const
  {
    return (syms
	    && morph
	    && *morph
	    && (!dis
		|| (dis->syms
		    && dis->dfst
		    && *dis->dfst
		    && !dis->class2sym.empty())));
  }

  // -- public methods: low-level
  /**
   * Extract PoS-tag labels from analysis-FSA 'fsa'.
   * Really, just the input-labels of all arcs from q0 are extracted:
   * i.e. no epsilon-checking is performed!
   */
  set<FSMSymbol> *get_fsm_tags(FSM *fsa, set<FSMSymbol> *tags=NULL);

  /**
   * Extract PoS-tag strings from analysis-FSA 'fsa'.
   * Same caveats as for get_fsm_tags().
   */
  set<FSMSymbolString> *get_fsm_tag_strings(FSM *fsa, set<FSMSymbolString> *tag_strings=NULL);

  //-------------------------------------
  // debugging & error reporting etc.

  /** Convert a symbol-vector to a numeric string */
  string symbol_vector_to_string(const FSM::FSMSymbolVector v);

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
