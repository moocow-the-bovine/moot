/* -*- Mode: C++ -*- */
/*--------------------------------------------------------------------------
 * File: dwdstMorph.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger for DWDS project : morphology
 *--------------------------------------------------------------------------*/

#ifndef _DWDST_MORPH_H_
#define _DWDST_MORPH_H_

/// Define this to enable verbose debugging
#define DWDST_MORPH_DEBUG
//#undef DWDST_MORPH_DEBUG


#include <stdio.h>

#include <FSMTypes.h>
#include <FSMSymSpec.h>
#include <FSM.h>

#include "dwdstTypes.h"
#include "dwdstTaggerLexer.h"


/*--------------------------------------------------------------------------
 * dwdstMorph : morphological tagger class
 *--------------------------------------------------------------------------*/

/// High-level class to perform morphological analysis.
class dwdstMorph {
public:
  /*------------------------------------------------------------
   * public typedefs
   */
  /** Single POS tag, without features */
  typedef FSMSymbol Tag;

  /** Single POS tag string, with or without features */
  typedef FSMSymbolString TagString;

  /** Single morphological analysis, including features */
  typedef FSM::FSMWeightedSymbolVector MorphAnalysis;

  /** Set of morphological analyses, including features */
  typedef set<MorphAnalysis> MorphAnalysisSet;

  /** Set of POS tag strings, with or without features */
  typedef set<FSMSymbolString> TagStringSet;

  /** Some handy values for the 'verbose' flag */
  typedef enum {
    vlSilent=0,       /**< Be as silent as possible */
    vlErrors=10,      /**< Output runtime errors with carp() */
    vlWarnings=20,    /**< Output runtime warnings */
    vlEverything=127  /**< Output everything */
  } VerbosityLevel;

public:
  /*------------------------------------------------------------
   * public data : guts
   */
    
  /** The symspec to use for morphological analysis. */
  FSMSymSpec *syms;
    
  /** Morphology FST to use for analysis. */
  FSM *mfst;

  /**
   * Tag-extraction FST:
   * tags = project(2,strings(mfst(w) ° tagfst))
   */
  FSM *xfst;

  /*------------------------------------------------------------
   * public data : filenames
   */

  string syms_filename;  /**< Name of the symbols-file (for "binary" save) */
  string mfst_filename;  /**< Name of the FST-file (for "binary" save) */
  string xfst_filename; /**< Name of the tag-extraction FST-file (for "binary" save) */

  /*------------------------------------------------------------
   * public data : flags
   */
  /** Generate output strings in AVM (madwds-"vector") mode? (default = false) */
  bool want_avm;

  /** Generate output strings in old MA-BBAW format? (default = false) */
  bool want_mabbaw_format;

  /** Do NEGRA-style dequoting in output-string generation ? (default = false) */
  bool do_dequote;
  
  /** verbosity level (0..2) : see enum dwdstMorph::VerbosityLevel */
  int verbose;

  /*------------------------------------------------------------
   * public data : statistics
   */

  /** number of tokens processed */
  unsigned int ntokens;

  /** number of unknown tokens encountered */
  unsigned int nunknown;
  
protected:
  /*------------------------------------------------------------
   * protected data : flags
   */
  bool i_made_syms;
  bool i_made_mfst;
  bool i_made_xfst;

protected:
  /*------------------------------------------------------------
   * protected data : temporaries
   */

  /** pre-allocated temporary: morphological analysis output FSA */
  FSM *result, *tmp, *tmpx;
  /** pre-allocated temporary: current token */
  FSMSymbolString curtok_s;
  /* pre-allocated temporary: current token */
  //char *curtok;

  /** pre-allocated temporary: morphological analysis output (for tag_token()) */
  MorphAnalysisSet analyses;

  /** pre-allocated temporaries: morphological analysis output (for extrac_tags() HACK) */
  MorphAnalysisSet xanalyses, xxanalyses;

  /** pre-allocated temporary: for analysess-strings conversion */
  MorphAnalysisSet::const_iterator anlsi;

  /** pre-allocated temporary: for print_token_analyses() */
  FSMSymbolString analysis_str;

  /** hack: for FSMSymbolSpec: error messages */
  list<string> syms_msgs;

  /** pre-allocated temporary: for analysisStrings() */
  TagStringSet analysis_strings;

public:
  /*------------------------------------------------------------
   * public methods: constructor / destructor
   */
  /** constructor */
  dwdstMorph(FSMSymSpec *mysyms=NULL, FSM *myfst=NULL, FSM *myxfst=NULL) :
    syms(mysyms), 
    mfst(myfst),
    xfst(myxfst),
    syms_filename("dwdstMorph.sym"),
    mfst_filename("dwdstMorph.fst"),
    xfst_filename("dwdstMorph_x.fst"),
    want_avm(false),
    want_mabbaw_format(false),
    do_dequote(false),
    verbose(vlErrors),
    ntokens(0),
    nunknown(0),
    i_made_syms(false),
    i_made_mfst(false),
    i_made_xfst(false),
    result(NULL),
    tmp(new FSM()),
    tmpx(new FSM())
  {};

  /** destructor */
  ~dwdstMorph();


  /*------------------------------------------------------------
   * public methods: initialization
   */

  /** Load morphological symbols-file, "filename" defaults to 'syms_filename' */
  FSMSymSpec *load_morph_symbols(const char *filename=NULL);

  /** Load morphology FST, 'filename' defaults to 'mfst_filename' */
  FSM *load_morph_fst(const char *filename=NULL)
  {
    if (filename) mfst_filename = (const char *)filename;
    if (load_fsm_file(mfst_filename.c_str(), &mfst, &i_made_mfst))
      {
	if (syms) mfst->fsm_use_symbol_spec(syms);
      }
    return mfst;
  };

  /** Load tag-extraction FST, 'filename' defaults to 'xfst_filename' */
  FSM *load_tagx_fst(const char *filename=NULL)
  {
    if (filename) xfst_filename = (const char *)filename;
    if (load_fsm_file(xfst_filename.c_str(), &xfst, &i_made_xfst))
      {
	if (syms) xfst->fsm_use_symbol_spec(syms);
      }
    return xfst;
  };

  /*------------------------------------------------------------
   * public methods: top-level: tagging utilities
   */

  /** Top-level: tag tokens from a C-stream, using a dwdstTaggerLexer */
  bool tag_stream(FILE *in=stdin, FILE *out=stdout, char *srcname=NULL);

  /** Top-level: tag a C-array of token-strings */
  bool tag_strings(int argc, char **argv, FILE *out=stdout, char *srcname=NULL);

  /** Top-level: print something for EOS */
  inline void tag_print_eos(FILE *out)
  {
    if (want_mabbaw_format) {
      fputs("--EOS--\n\n", out);
    } else {
      fputc('\n', out);
    }
  };

  /*------------------------------------------------------------
   * public methods: low-level: initialization
   */

  /** Low-level FSTfile loading utility */
  FSM *load_fsm_file(const char *fsm_file, FSM **fsm, bool *i_made_fsm=NULL);

  /*------------------------------------------------------------
   * public methods: analysis
   */

  /**
   * mid-level tagging utility: tag a single token
   * 'token' defaults to 'curtok_s'.
   * Clears, populates, and returns 'analyses'.
   *
   * Don't even THINK about calling this method unlesss
   * can_tag() returns true.
   */
  inline const MorphAnalysisSet &tag_token(const char *token)
  {
    curtok_s = (const char *)token;
    return tag_token();
  };


  /**
   * mid-level tagging utility: tag a single token.
   * Clears, populates, and returns 'analyses'.
   *
   * Don't even THINK about calling this method unlesss
   * can_tag() returns true.
   */
  inline const MorphAnalysisSet &tag_token(const dwdstTokString &tok)
  {
    curtok_s = tok;
    return tag_token();
  };

  /**
   * mid-level tagging utility: tag the current token 'curtok_s'.
   * Clears, populates, and returns 'analyses'.
   *
   * Don't even THINK about calling this method unlesss
   * can_tag() returns true.
   */
  inline const MorphAnalysisSet &tag_token(void)
  {
    //-- analyze
    tmp->fsm_clear();
    result = mfst->fsm_lookup(curtok_s, tmp, true);

    //-- serialize, extracting tags
    analyses.clear();
    if (xfst) {
      extract_tags(*tmp, analyses);
    }
    else {
      //-- just serialize
      tmp->fsm_symbol_vectors(analyses, false);
    }

    //-- track statistics
    ntokens++;
    if (analyses.empty()) nunknown++;

    //-- check for errors (hack)
    check_symspec_messages();

    return analyses;
  };

  /*------------------------------------------------------------
   * public methods: tag-extraction
   */
  /**
   * This method should just be a pair of libFSM calls,
   * but since they don't work (HINT), it's big and slow...
   * Complaints to tom@ling.uni-potsdam.de.
   */
  inline void extract_tags(FSM &fsm, MorphAnalysisSet &as)
  {
    //-- QUACK: this should work, but it doesn't (no FSMModeDestructive for compose())
    //tmp->fsm_compose(*tagx, FSM::FSMModeDestructive);

    //-- KABOOM: this too should work, but it segfaults (no FSA->compose(FST) ?!)
    //*tmpx = tmp->fsm_compose(*tagx, FSM::FSMModeConstructive);

    //-- QUACK/KABOOM: followed by:
    //tmpx->fsm_symbol_vectors(as, false);

    //-- HACK: this *REALLY* ought to work, because I friggin wrote it!
    //   (unfortunately, it's dog slow!)
    xanalyses.clear();
    fsm.fsm_symbol_vectors(xanalyses, false);
    for (MorphAnalysisSet::const_iterator xai = xanalyses.begin(); xai != xanalyses.end(); xai++) {
      tmpx->fsm_clear();
      xfst->fsm_lookup_vector(xai->istr, tmpx, true);
      xxanalyses.clear();
      tmpx->fsm_symbol_vectors(xxanalyses, false);
      for (MorphAnalysisSet::const_iterator xxai = xxanalyses.begin(); xxai != xxanalyses.end(); xxai++)
	{
	  as.insert(MorphAnalysis(xai->istr, xxai->istr, xxai->weight+xai->weight));
	}
    }
  };

  /*------------------------------------------------------------
   * public methods: sanity checking
   */

  /** tagging utility: sanity check */
  inline bool can_tag(void) const
  {
    return (syms && mfst && *mfst);
  }


  /*------------------------------------------------------------
   * public methods: tagging utilities: string-generation
   */

  /**
   * Tagging utility: stringify a single token analysis-set.
   */
  inline TagStringSet &analyses_to_strings(MorphAnalysisSet *anls = NULL)
  {
    if (!anls) anls = &analyses;
    analysis_strings.clear();
    for (anlsi = anls->begin(); anlsi != anls->end(); anlsi++) {
      //-- stringify this analysis
      analysis_str.clear();
      symbol_vector_to_string(anlsi->istr, analysis_str);
      analysis_strings.insert(analysis_str);
    }
    return analysis_strings;
  };

  /*------------------------------------------------------------
   * public methods: tagging utilities: output
   */

  /**
   * Prints analyses to the specified output stream.
   * 'token' defaults to 'curtok',
   * 'ans' defaults to 'analyses' member, 'out' defaults to STDOUT.
   */
  inline void print_token_analyses(FILE *out = stdout,
				   const char *token = NULL,
				   MorphAnalysisSet *anls = NULL)
  {
    if (!anls) anls = &analyses;
    
    fputs(token ? token : curtok_s.c_str(), out);
    if (want_mabbaw_format) {
      /*-- ambiguous, strings, all features, mabbaw-style */
      fprintf(out, ": %d Analyse(n)\n", anls->size());
      for (anlsi = anls->begin(); anlsi != anls->end(); anlsi++) {
	//-- print separator
	fputc('\t', out);

	//-- print "output" string if present (it might be an extracted tag)
	if (!anlsi->ostr.empty()) {
	  analysis_str.clear();
	  symbol_vector_to_string(anlsi->ostr, analysis_str);
	  fputs(analysis_str.c_str(), out);
	  fputc(':', out);
	}

	//-- always print "input" string
	analysis_str.clear();
	symbol_vector_to_string(anlsi->istr, analysis_str);
	fputs(analysis_str.c_str(), out);

	//-- print weight
	fprintf(out, (anlsi->weight ? "\t<%f>\n" : "\n"), anlsi->weight);
      }
      fputc('\n', out);
    } else { /*-- want_mabbaw_format */
      /*-- ambiguous, strings, all features, one tok/line */
      for (anlsi = anls->begin(); anlsi != anls->end(); anlsi++) {
	//-- print separator
	fputc('\t', out);

	//-- print "output string" if present (it might be an extracted tag)
	if (!anlsi->ostr.empty()) {
	  analysis_str.clear();
	  symbol_vector_to_string(anlsi->ostr, analysis_str);
	  fputs(analysis_str.c_str(), out);
	  fputc(':', out);
	}

	//-- always print "input" string
	analysis_str.clear();
	symbol_vector_to_string(anlsi->istr, analysis_str);
	fputs(analysis_str.c_str(), out);

	//-- print weight
	if (anlsi->weight) fprintf(out, "<%f>", anlsi->weight);
      }
      fputc('\n',out);
    }
  };


  /*------------------------------------------------------------
   * public methods: low-level: tagging utilities: conversion
   */

  /** Convert symbol-vectors to pretty strings: general */
  inline void symbol_vector_to_string(const vector<FSMSymbol> &vec, FSMSymbolString &str) const
  {
    if (do_dequote) {
      symbol_vector_to_string_dq(vec, str);
    } else {
      syms->symbol_vector_to_string(vec, str, want_avm, verbose >= vlWarnings);
    }
  };

  /** Convert symbol-vectors to pretty strings: dequoting (ugly, but useful) */
  void symbol_vector_to_string_dq(const vector<FSMSymbol> &vec, FSMSymbolString &str) const;

  /*------------------------------------------------------------
   * public methods: low-level: errors/warnings
   */

  /** Hack: check, print, & clear messages associated with our FSMSymSpec */
  inline void check_symspec_messages(void)
  {
    if (syms->messages && !syms->messages->empty()) {
      if (verbose >= vlWarnings) {
	for (list<string>::const_iterator e = syms_msgs.begin(); e != syms_msgs.end(); e++) {
	  carp("%s\n",e->c_str());
	}
      }
      syms->messages->clear();
    }
  };

  /*------------------------------------------------------------
   * checksum
   */
  size_t checksum(void)
  {
    size_t crc = 0;
    if (mfst) {
      crc += mfst->fsm_no_of_states() + mfst->fsm_no_of_final_states() + mfst->fsm_no_of_transitions();
    }
    if (syms) {
      crc += syms->signs()->size() + syms->symbols()->size();
    }
    return crc;
  };

  /** Error reporting */
  void carp(char *fmt, ...) const;

};

#endif // _DWDST_MORPH_H_
