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

#ifdef HAVE_CONFIG_H
/*# include "nopackage.h"*/
# include <dwdstConfig.h>
#endif

/// Whether to load symspecs in AT&T-compatibility mode
#define DWDST_SYM_ATT_COMPAT true

/*--------------------------------------------------------------------------
 * dwdstTagger : tagger class
 *--------------------------------------------------------------------------*/

/// High-level class to perform morphological analysis and PoS-tagging.
class dwdstTagger {
public:
  // ------ public data

  /// the symspec to use for tagging
  FSMSymSpec *syms;

  /// morphology FST to use for tagging
  FSM *morph;

  /// hard-coded analysis FSA for unknown-token analyses
  FSM *ufsa;
  /// disambiguation FSA (not yet implemented)
  FSM *dfsa;

  /// lexer for tagging methods
  dwdstTaggerLexer lexer;

  /// end-of-sentence marker for tagger output
  string eos;

  /// word-separator for TnT-style output
  string wdsep;

  // ------- flags

  /// output in AVM ("vector") mode?
  bool want_avm;

  //bool want_numeric;
  //bool want_binary;

  /// output morphosyntactic features as well as PoS tags?
  bool want_features;

  /// output in TnT format?
  bool want_tnt_format;

  /// verbose output (enable ntokens and nunknown tracking)?
  bool verbose;

  // -- verbose data
  /// for verbose mode
  unsigned int ntokens;
  /// for verbose mode
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
  /// input file for tagging
  FILE *infile;
  /// output file for tagging
  FILE *outfile;

  // -- temporary variables
  /// pre-allocated temporary
  FSM *result, *tmp;
  /// pre-allocated temporary
  FSMSymbolString s;
  /// pre-allocated temporary
  char *token;

  // -- all features, string-mode
  /// pre-allocated temporary: all features
  set<FSM::FSMStringWeight> results;
  /// pre-allocated temporary: all features
  set<FSM::FSMStringWeight>::iterator ri;

  // -- tags only, strings
  /// pre-allocated temporary: tags-only
  set<FSMSymbolString> tagresults;
  /// pre-allocated temporary: tags-only
  set<FSMSymbolString>::iterator tri;


  // -- all features, numeric
  // -- tags only, numeric
  /// pre-allocated temporary: tags-only
  set<FSMSymbol> ntagresults;
  /// pre-allocated temporary: tags-only
  set<FSMSymbol>::iterator ntri;

public:
  // -- public methods: constructor/destructor
  /// constructor
  dwdstTagger(FSMSymSpec *mysyms=NULL, FSM *mymorph=NULL);
  /// destructor
  ~dwdstTagger();

  // -- public methods: loading

  /// load a symbol file
  FSMSymSpec *load_symbols(char *syms_file);

  /// load morphology FST
  FSM *load_morph(char *morph_file) {
    return load_fsm_file(morph_file, &morph, &i_made_morph);
  }

  /// load unknown FSA (see 'ufsa')
  FSM *load_unknown_fsa(char *ufsa_file) {
    return load_fsm_file(ufsa_file, &ufsa, &i_made_ufsa);
  }

  /// load disambiguation FSA (see 'disambig')
  FSM *load_disambig_fsa(char *dfsa_file) {
    return load_fsm_file(dfsa_file, &dfsa, &i_made_dfsa);
  }

  // -- low-level public methods: loading
  /// low-level FSTfile loading utility
  FSM *load_fsm_file(char *fsm_file, FSM **fsm, bool *i_made_fsm=NULL);

  // -- public methods: tagging
  /// High-level tagging interface: string input
  bool tag_strings(int argc, char **argv, FILE *out=stdout);
  /// High-level tagging interface: file input
  bool tag_stream(FILE *in=stdin, FILE *out=stdout);

  // -- public methods: mid-level: tagging : i/o
  /// low-level tagging utility: tag a single token
  inline void tag_token(void);

  // -- public methods: sanity checks
  /// tagging utility: sanity check
  bool can_tag(void) const { return morph && syms; }

  // -- public methods: low-level
  /// Extract PoS-tag labels from analysis-FSA 'fsa'.
  /// Really, just the input-labels of all arcs from q0 are extracted:
  /// i.e. no epsilon-checking is performed!
  set<FSMSymbol> *get_fsm_tags(FSM *fsa, set<FSMSymbol> *tags=NULL);

  /// Extract PoS-tag strings from analysis-FSA 'fsa'.
  /// Same caveats as for get_fsm_tags().
  set<FSMSymbolString> *get_fsm_tag_strings(FSM *fsa, set<FSMSymbolString> *tag_strings=NULL);

};

#endif // _DWDST_TAGGER_H_
