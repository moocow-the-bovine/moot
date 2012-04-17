/* -*- Mode: C++ -*- */
/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2009 by Bryan Jurish <moocow@cpan.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/*--------------------------------------------------------------------------
 * File: mootDynHMM.h
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moot PoS tagger : HMM: for large "tag"-sets with dynamic lexical probabilities
 *--------------------------------------------------------------------------*/

/**
\file mootDynHMM.h
\brief Hidden Markov Model tagger/disambiguator for dynamic lexical probabilities
*/

#ifndef _MOOT_DYNHMM_H
#define _MOOT_DYNHMM_H

#include <mootHMM.h>
#include <assert.h>

moot_BEGIN_NAMESPACE


/*======================================================================
 *Generic  Utilities
 */

/** \brief Enum for built-in mootDynHMM estimator modes (subclasses) */
typedef enum {
  dheUnknown,   ///< unknown
  dheFreq,      ///< ~= "Freq" ~= mootDynLexHMM
  dheBoltzmann, ///< ~= "Boltzmann" ~= mootDynLexHMM_Boltzmann
  dheMIParser,  ///< ~= "MIParser" ~= mootMIParser
  dheN          ///< placeholder
} DynHMMClassId;

/** \brief Generic user-level options structure for built-in mootDynHMM subclasses */
class mootDynHMMOptions {
public:
  //-- mootDynLexHMM
  bool        invert_lexp;     /**< see mootDynLexHMM::invert_lexp */
  std::string newtag_str;      /**< see mootDynLexHMM::newtag_str */
  ProbT       Ftw_eps;         /**< see mootDynLexHMM::Ftw_eps */
  //
  //-- mootDynLexHMM_Boltzmann
  ProbT       dynlex_base;     /**< see mootDynLexHMM_Boltzmann::dynlex_base */
  ProbT       dynlex_beta;     /**< see mootDynLexHMM_Boltzmann::dynlex_beta */
  //
  //-- mootMIParser
  bool        text_tags;        /**< see mootMIParser::text_tags */

public:
  mootDynHMMOptions(void)
    : invert_lexp(true),
      newtag_str("@NEW"),
      Ftw_eps(0.5),
      dynlex_base(2.0),
      dynlex_beta(1.0),
      text_tags(false)
  {};

  ~mootDynHMMOptions(void) {};
};

/** Generic constructor for built-in mootDynHMM subclasses */
class mootDynHMM *newDynHMM(DynHMMClassId which=dheFreq, const mootDynHMMOptions &opts=mootDynHMMOptions());

/** Generic constructor for built-in mootDynHMM subclasses, given subclass name */
class mootDynHMM *newDynHMM(const std::string &which="Freq", const mootDynHMMOptions &opts=mootDynHMMOptions());


/*======================================================================
 * mootDynHMM : "dynamic" HMM class
 */

/**
 * \brief abstract HMM subclass for use with dynamic lexical probabilities.
 */
class mootDynHMM: public mootHMM {
public:
  /*---------------------------------------------------------------------*/
  /* Types */

public:
  /*---------------------------------------------------------------------*/
  /* Data */

public:
  /*---------------------------------------------------------------------*/
  /** \name Constructors, etc. */
  //@{
  /** Default constructor sets hash_ngrams=true */
  mootDynHMM(void)
  { hash_ngrams=true; };

  /** Destructor */
  virtual ~mootDynHMM(void) {};

  /** Set generic user-level options. Default does nothing. */
  virtual void set_options(const mootDynHMMOptions &opts)
  {};
  //@}

  //------------------------------------------------------------
  // Utilities
  //@{
  /** clears lexprobs & tokids, leaves 'UNKNOWN' entry present but empty */
  inline void lex_clear(void) {
    lexprobs.clear();
    tokids.clear();         //-- leaves '@UNKNOWN' tok entry
    lexprobs.resize(1);     //-- re-insert empty entry for '@UNKNOWN' tok
    n_toks = tokids.size(); //-- sanity check
  };

  /** resize lexicon to fit the id \c tokid_max */
  inline void lex_resize(TokID tokid_max) {
    if (n_toks <= tokid_max) {
      tokids.resize(tokid_max+1);
      lexprobs.resize(tokid_max+1);
      n_toks = tokids.size();
    }
  };

  /** Get or insert an ID for token text \c tok_text.
   *  Implicitly resizes lexprobs, tokids, and sets n_toks */
  inline TokID lex_get_tokid(const mootTokString &tok_text)
  {
    TokID tokid = tokids.get_id(tok_text);
    lex_resize(tokid);
    return tokid;
  };

  /** Grow tagids to fit the id \c tagid_max.
   *  Only works if hash_ngrams==true
   */
  inline void tagset_resize(TagID tagid_max) {
    assert(hash_ngrams==true);
    if (n_tags <= tagid_max) {
      tagids.resize(tagid_max+1);
      n_tags = tagids.size();
    }
  };

  /** Get or insert a tag-id for tag \c tagstr.
   *  Implicitly resizes tagids and sets n_tags
   */
  inline TagID get_tagid(const mootTagString &tagstr)
  {
    if (tagids.nameExists(tagstr))
      return tagids.name2id(tagstr);
    TagID tagid = tagids.get_id(tagstr);
    tagset_resize(tagid);
    return tagid;
  };
  //@}

  //------------------------------------------------------------
  /** \name Tagging: Hooks (API) */
  //@{

  /**
   * Prepare dynamic lexicon for tagging \c sent.
   * Default implementation does nothing.
   */ 
  virtual void tag_hook_pre(mootSentence &sent)
  {};

  /**
   * Cleanup dynamic lexicon after tagging \c sent.
   * Default implementation does nothing.
   */
  virtual void tag_hook_post(mootSentence &sent)
  {};
  //@}

  /*---------------------------------------------------------------------*/
  ///\name User-level Niceties
  //@{
  /** Write some debugging header information to an output stream
   *  Default implementation does nothing. */
  virtual void tw_put_info(moot::TokenWriter *tw)
  {
    tw->printf_raw("  +DynHMM class      : %s\n", "mootDynHMM (?)");
  };
  //@}

  //------------------------------------------------------------
  // Tagging: Top-level
  /** \name Top-level Tagging Interface (overrides) */
  //@{

  /** Top-level tagging interface: TokenIO layer.
   *  Really just a dup of mootHMM::tag_io which calls virtual tag_sentence()
   */
  virtual void tag_io(TokenReader *reader, TokenWriter *writer);

  /**
   * Top-level tagging interface: mootSentence
   * Just wraps mootHMM::tag_sentence() in calls to tag_hook_pre() and tag_hook_post().
   */
  virtual void tag_sentence(mootSentence &sentence);
  //@}
};


/*======================================================================
 * class mootDynLexHMM
 */
/** \brief mootDynHMM subclass for dynamic lexical probabilities */
class mootDynLexHMM : public mootDynHMM {
public:
  //---------------------------------------------------------------------
  // Types
  typedef mootTokString TokStr;  /**< useful alias */
  typedef mootTagString TagStr;  /**< useful alias */

  typedef std::map<TokStr,ProbT>      TokProbMap;     /**< lexical string (sub-)map:  w -> p(w|t) */
  typedef std::map<TagStr,ProbT>      TagProbMap;     /**< lexical string (sub-)map:  t -> p(t) */
  typedef std::map<TagStr,TokProbMap> TagTokProbMap;  /**< lexical string map:        t -> (w -> p(w|t)) */
  typedef std::map<TagStr,TagProbMap> TokTagProbMap;  /**< lexical string map:        w -> (t -> p(w|t)) */

public:
  //---------------------------------------------------------------------
  // Data
  /** 
   * If true (the default), dynamic lexical probabilities will be estimated
   * (incorrectly) as <code>p(w|t) := f(w,t)/f(w) == p(t|w)</code>, rather than 
   * <code>p(w|t) := f(w,t)/f(w)</code>.
   *
   * Despite incorrectness, true is the default value here, which at least makes some
   * sense for dynamic lexical maps which are functions of the
   * input token's text type (mootToken::text()).
   */
  bool           invert_lexp;

  TagStr         newtag_str;  /**< tag string to copy for "missing" tags (default="@NEW") */
  TagID          newtag_id;   /**< ID for "missing" tags */
  ProbT          newtag_f;    /**< Raw frequency for 'new' tag, if not already in model.  Default=0.5 */

  TagTokProbMap  Ftw;         /**< pseudo-frequency lexicon:       t -> (w -> f(w,t)) */
  TokProbMap     Fw;          /**< pseudo-frequency (sub-)lexicon: w -> f(w) */
  TokProbMap     Ft;          /**< pseudo-frequency (sub-)lexicon: t -> f(t) */
  ProbT          Ftw_eps;     /**< Raw pseudo-frequency smoothing constant (non-log) for f(w,t) */

  size_t         tagids_size_orig; /**< original size of tagids */

public:
  //---------------------------------------------------------------------
  ///\name Constructors etc.
  //@{
  mootDynLexHMM(void)
    : invert_lexp(true),
      newtag_str("@NEW"),
      newtag_id(0),
      Ftw_eps(0.5),
      tagids_size_orig(0)
  {};

  virtual ~mootDynLexHMM(void) {};

  /** Set generic user-level options. */
  virtual void set_options(const mootDynHMMOptions &opts)
  {
    invert_lexp = opts.invert_lexp;
    newtag_str  = opts.newtag_str;
    Ftw_eps     = opts.Ftw_eps;
  };
  //@}

  //---------------------------------------------------------------------
  ///\name Compilation & initialization
  //@{
  /** load a binary or text mode */
  virtual bool load_model(const string &modelname,
			  const mootTagString &start_tag_str="__$",
			  const char *myname="mootDynLexHMM::load_model()",
			  bool  do_estimate_nglambdas=true,
			  bool  do_estimate_wlambdas=true,
			  bool  do_estimate_clambdas=true,
			  bool  do_build_suffix_trie=true,
			  bool  do_compute_logprobs=true);

  /** compile a text model; ensures newtag_id is defined */
  virtual bool compile(mootLexfreqs &lexfreqs,
		       mootNgrams &ngrams,
		       mootClassfreqs &classfreqs,
		       const mootTagString &start_tag_str="__$");
  //@}

  //---------------------------------------------------------------------
  ///\name Tagging: Hooks
  //@{
  virtual void tag_hook_pre(mootSentence &sent);
  virtual void tag_hook_post(mootSentence &sent);
  //@}

  //---------------------------------------------------------------------
  ///\name User-level Niceties
  //@{
  /** Write some debugging header information to an output stream */
  virtual void tw_put_info(moot::TokenWriter *tw);
  //@}

  //---------------------------------------------------------------------
  ///\name Tagging: Hooks: Low-level Utilities
  //@{
  /** Clear dynamic lexica */
  void dynlex_clear(void);

  /** Estimate pseudo-frequency for the tag associated with analysis \c a of token \c tok.
   *  Called by tag_hook_pre() for each vanilla (token,analysis) in the input sentence;
   *  returned value is used to (re-)populate Ftw, Fw, and Ft data fields for each input sentence.
   *
   *  Has no effect with default tag_hook_pre() if returned pseudo-frequency is <=0.
   *  Default implementation just returns <code>a.prob + Ftw_eps</code>
   *
   *  \returns pseudo-frequency f ~= f(tok.text(),a.tag).
   */
  virtual ProbT dynlex_analysis_freq(const mootToken &tok, const mootToken::Analysis &a)
  {
    return a.prob + Ftw_eps;
  };

  /** Converts pseudo-frequency fields Ftw, Ft, and Fw to mootHMM::lexprobs.
   *  Sets
   *  <code>lexprobs[w][t] = log( wlambda1 * Ftw(w,t)/Z(w,t) )</code>
   */
  virtual void dynlex_populate_lexprobs(void);
  //@}
};

/*======================================================================
 * class mootDynLexHMM_Boltzmann
 */
/**
 * \brief mootDynHMM subclass using a Maxwell-Boltzmann distribution to estimate f(w,t)
 * \details
 * Estimates <code>f(w,t) = dynlex_base^(-dynlex_beta * a.prob)</code>
 * for token text \c w and analysis \c a with tag \c t.
 *
 * This estimator is suitable for use with token analyses whose \c prob field contains
 * non-negative "costs" or "distances" associated with the token-tag pair.
 */
class mootDynLexHMM_Boltzmann : public mootDynLexHMM {
public:
  /** Base of Maxwell-Boltzmann estimator (>1), default=2.
   *  A value of 1.0 gives a uniform output distribution.
   *  Greater values give lower-entropy output distributions.
   */
  ProbT dynlex_base;

  /**
   * "Temperature" coefficient of Maxwell-Boltzmann estimator (>0), default=1
   * A value of 0.0 gives a uniform output distribution.
   * Greater values give lower-entropy output distributions.
   */
  ProbT dynlex_beta;

public:
  /** Constructor */
  mootDynLexHMM_Boltzmann(void)
    : dynlex_base(2),
      dynlex_beta(1)
  {};

  /** Set user-level options */
  virtual void set_options(const mootDynHMMOptions &opts)
  {
    mootDynLexHMM::set_options(opts);
    dynlex_base = opts.dynlex_base;
    dynlex_beta = opts.dynlex_beta;
  };

  /** Estimate pseudo-frequency for the tag associated with analysis \c a of token \c tok.
   *  This implementation returns
   *  <code>f(w,t) = Ftw_eps + dynlex_base^(-dynlex_beta * a.prob)</code>
   */
  virtual ProbT dynlex_analysis_freq(const mootToken &tok, const mootToken::Analysis &a)
  {
    return Ftw_eps + pow(dynlex_base, -dynlex_beta * a.prob);
  };

  /** User-level information */
  virtual void tw_put_info(moot::TokenWriter *tw)
  {
    mootDynLexHMM::tw_put_info(tw);
    tw->printf_raw("  +DynHMM class      : %s\n", "mootDynLexHMM_Boltzmann");
    tw->printf_raw("   dynlex_base       : %g\n", dynlex_base);
    tw->printf_raw("   dynlex_beta       : %g\n", dynlex_beta);
  };
};


moot_END_NAMESPACE

#endif /* _MOOT_DYNHMM_H */
