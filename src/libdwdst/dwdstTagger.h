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

  /** Single POS tag string, with or without features */
  typedef FSMSymbolString TagString;

  /** Single morphological analysis, including features */
  typedef FSM::FSMWeightedSymbolVector MorphAnalysis;

  /** Set of POS tags, no features */
  typedef set<Tag> TagSet;

  /** Set of morphological analyses, including features */
  typedef set<MorphAnalysis> MorphAnalysisSet;

  /** Set of POS tag strings, with or without features */
  typedef set<FSMSymbolString> TagStringSet;

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

  /** Do automatic NEGRA-style dequoting ? (default = false) */
  bool do_dequote;
  
  /**
   * output verbosity level (0..3)
   *  0: silent (no output but warnings/errors)
   *  1: output tagged text
   *  2: (reserved for dwdst)
   *  3: output runtime FSMSymSpec errors
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

  /** hack: for FSMSymbolSpec: error messages */
  list<string> syms_msgs;

  /** pre-allocated temporary: for analysisStrings() */
  TagStringSet analysis_strings;

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
  inline const MorphAnalysisSet &tag_token(char *token = NULL)
  {
    //-- analyse
    tmp->fsm_clear();
    curtok_s = (char *)(token ? token : curtok);
    result = morph->fsm_lookup(curtok_s, tmp, true);

    //-- serialize
    analyses.clear();
    tmp->fsm_symbol_vectors(analyses, false);

    //-- track statistics
    if (track_statistics && analyses.empty()) nunknown++;

    //-- check for errors (hack)
    check_symspec_messages();

    return analyses;
  };

  /**
   * mid-level tagging utility: tag end-of-sentence i/o
   * Outputs EOS marker if flags permit.
   */
  inline void tag_eos(void)
  {
    // -- just output end-of-sentence marker
    if (verbose > 0) {
      if (want_mabbaw_format) {
	fputs(eos.c_str(), outfile);
	fputs("\n\n", outfile);
      }
      else {
	fputc('\n', outfile);
      }
    }
  };

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
   * mid-level tagging utility: stringify a single token
   * analysis-set.
   */
  inline TagStringSet &analyses_to_strings(MorphAnalysisSet *ans = NULL)
  {
    if (!ans) ans = &analyses;
    analysis_strings.clear();
    for (analyses_i = ans->begin(); analyses_i != ans->end(); analyses_i++) {
      //-- stringify this analysis
      analysis_str.clear();
      symbol_vector_to_string(analyses_i->istr, analysis_str);
      analysis_strings.insert(analysis_str);
    }
    return analysis_strings;
  };

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
				   MorphAnalysisSet *ans = NULL,
				   FILE *out = NULL)
  {
    if (!ans) ans = &analyses;
    if (!out) out = outfile ? outfile : stdout;
    
    fputs(token ? token : curtok, out);
    if (want_mabbaw_format) {
      /*-- ambiguous, strings, all features, mabbaw-style */
      fprintf(out, ": %d Analyse(n)\n", ans->size());
      for (analyses_i = ans->begin(); analyses_i != ans->end(); analyses_i++) {
	//-- stringify this analysis
	analysis_str.clear();
	symbol_vector_to_string(analyses_i->istr, analysis_str);
	//-- ... and print it
	fputc('\t', out);
	fputs(analysis_str.c_str(), out);
	fprintf(out, (analyses_i->weight ? "\t<%f>\n" : "\n"), analyses_i->weight);
      }
      fputc('\n',outfile);
    } else { /*-- want_mabbaw_format */
      /*-- ambiguous, strings, all features, one tok/line */
      for (analyses_i = ans->begin(); analyses_i != ans->end(); analyses_i++) {
	//-- stringify this analysis
	analysis_str.clear();
	symbol_vector_to_string(analyses_i->istr, analysis_str);
	//-- ... and print it
	fputc('\t', out);
	fputs(analysis_str.c_str(), out);
	if (analyses_i->weight) fprintf(out, "<%f>", analyses_i->weight);
      }
      fputc('\n',out);
    }
  };

  /** Convert symbol-vectors to pretty strings: general */
  inline void symbol_vector_to_string(const vector<FSMSymbol> &vec, FSMSymbolString &str)
  {
    if (do_dequote) {
      syms->symbol_vector_to_string(vec, str, want_avm, verbose > 2);
    } else {
      symbol_vector_to_string_dq(vec, str);
    }
  };

  /** Convert symbol-vectors to pretty strings: dequoting (ugly, but useful) */
  void symbol_vector_to_string_dq(const vector<FSMSymbol> &vec, FSMSymbolString &str);

  //-------------------------------------
  // debugging & error reporting etc.

  /** Hack: check, print, & clear messages associated with our FSMSymSpec */
  inline void check_symspec_messages(void);

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
