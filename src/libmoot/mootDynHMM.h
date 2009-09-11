/* -*- Mode: C++ -*- */
/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2009 by Bryan Jurish <moocow@ling.uni-potsdam.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
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
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
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


  //------------------------------------------------------------
  // Tagging: Top-level
  /** \name Top-level Tagging Interface (overrides) */
  //@{

  /** Top-level tagging interface: TokenIO layer.
   *  Really just a dup of mootHMM::tag_io which calls virtual tag_sentence()
   */
  void tag_io(TokenReader *reader, TokenWriter *writer);

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

  typedef std::map<TokStr,ProbT>      TokProbMap;     /**< lexical string submap:  w -> p(w|t) */
  typedef std::map<TagStr,ProbT>      TagProbMap;     /**< lexical string map:     t -> p(tag) */
  typedef std::map<TagStr,TokProbMap> TagTokProbMap;  /**< lexical string map:     t -> (w -> p(w|t)) */
  typedef std::map<TagStr,TokProbMap> TokTagProbMap;  /**< lexical string map:     w -> (t -> p(w|t)) */

public:
  //---------------------------------------------------------------------
  // Data
  TagStr         tagstr_new;  /**< ID to use for "missing" tags (default="@NEW") */
  TagTokProbMap  dynlex;      /**< lexical string map: tag -> (w -> p(w|tag)) */
  ProbT          wtflambda0;  /**< Raw pseudo-frequency smoothing constant (non-log) for f(w,t) */

  size_t         tagids_size_orig; /**< original size of tagids */

public:
  //---------------------------------------------------------------------
  ///\name Constructors etc.
  //@{
  mootDynLexHMM(void)
    : tagstr_new("@NEW"),
      tagids_size_orig(0)
  {};

  virtual ~mootDynLexHMM(void) {};
  //@}

  /*---------------------------------------------------------------------*/
  ///\name Tagging: Hooks
  //@{
  virtual void tag_hook_pre(mootSentence &sent);
  virtual void tag_hook_post(mootSentence &sent);
  //@}

  /*---------------------------------------------------------------------*/
  ///\name Tagging: Hooks: Low-level Utilities
  //@{
  
  /** Estimate pseudo-frequency for the tag associated with analysis \c a of token \c tok.
   *  Should record/adjust pseudo-frequency f = f(tok.text(),a.tag) by setting dynlex[a.tag][tok.text()] = f.
   *  May replace \c a.tag by \c tagstr_new if required.
   *
   *  Should be called only after dynlex_add_tokids() has been called for \c a.
   *
   *  Default implementation just sets \c f(w,t)=a.prob,
   *  clobbering any old value for \c f(w,t).
   */
  virtual void dynlex_add_analysis(const mootToken &tok, const mootToken::Analysis &a)
  {
    dynlex[a.tag][tok.text()] = a.prob;
  };

  /** Converts pseudo-frequency \a dynlex to mootHMM::lexprobs.
   *  Sets
   *  <code>lexprobs[w][t] = wlambda1 * log( (wtflambda0+dynlex(w,t)) / \sum_w(wtflambda0+dynlex(w,t)) )</code>
   */
  virtual void dynlex_populate_lexprobs(void);
  //@}
};

/*======================================================================
 * class mootDynLexHMM
 */
/** \brief mootDynHMM subclass for dynamic lexical probabilities using inverted p(tag|word) instead of p(word|tag) */
class mootDynILexHMM : public mootDynLexHMM {
public:
  /*---------------------------------------------------------------------*/
  ///\name Tagging: Hooks: Low-level Utilities
  //@{

  /** Converts pseudo-frequency \a dynlex to mootHMM::lexprobs.
   *  Sets
   *  <code>lexprobs[w][t] = wlambda1 * log( (wtflambda0+dynlex(w,t)) / \sum_t(wtflambda0+dynlex(w,t)) )</code>
   *
   *  Note that this is theoretically incorrect.
   */
  virtual void dynlex_populate_lexprobs(void);
  //@}

};

/*======================================================================
 * Utilities
 */

/** \brief Enum for built-in mootDynHMM estimator modes (subclasses) */
typedef enum {
  dheUnknown,  ///< unknown
  dheFreq,     ///< ~= "Freq" ~= mootDynLexHMM
  dheIFreq,    ///< ~= "IFreq" ~= mootDynILexHMM
  dheN         ///< placeholder
} DynHMMEstimator;

/** Generic constructor for built-in mootDynHMM subclasses */
mootDynHMM *newDynHMM(DynHMMEstimator which=dheFreq);

/** Generic constructor for built-in mootDynHMM subclasses, given estimator name */
mootDynHMM *newDynHMM(const std::string &which="Freq");

moot_END_NAMESPACE

#endif /* _MOOT_DYNHMM_H */
