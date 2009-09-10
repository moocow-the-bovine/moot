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

moot_BEGIN_NAMESPACE

/*--------------------------------------------------------------------------
 * mootDynHMM : HMM class
 */

/**
 * \brief HMM class for use with dynamic lexical probabilities.
 */
template<class LexProbFcn = class LPFUniform>
class mootDynHMM: public mootHMM {
public:
  /*---------------------------------------------------------------------*/
  /* Types */
  typedef LexProbFcn LexProbFunc; /**< Useful alias */

public:
  /*---------------------------------------------------------------------*/
  /* Data */
  LexProbFunc lpf;  /**< dynamic lexical probability estimator */

public:
  /*---------------------------------------------------------------------*/
  /** \name Constructors, etc. */
  //@{
  /** Default constructor sets hash_ngrams=true */
  mootDynHMM(void)
  { hash_ngrams=true; }

  /** Destructor */
  ~mootDynHMM(void);
  //@}

  //------------------------------------------------------------
  // Utilities
  //@{
  /** clears lexprobs & tokids, leaves 'UNKNOWN' entry present but empty */
  void lex_clear(void) {
    lexprobs.clear();
    tokids.clear();     //-- leaves '@UNKNOWN' tok entry
    lexprobs.resize(1); //-- re-insert empty entry for '@UNKNOWN' tok
  };
  //@}


  //------------------------------------------------------------
  // Tagging: Top-level
  /** \name Top-level Tagging Interface */
  //@{

  /** Top-level tagging interface: TokenIO layer */
  void tag_io(TokenReader *reader, TokenWriter *writer)
  {
    int rtok;
    mootSentence *sent;
    while (reader && (rtok = reader->get_sentence()) != TokTypeEOF) {
      sent = reader->sentence();
      if (!sent) continue;
      tag_sentence(*sent);
      if (writer) writer->put_sentence(*sent);
    }
  };

  /**
   * Top-level tagging interface: mootSentence input & output (destructive).
   * Calling this method will (re-)populate the \c besttag
   * datum in the \c sentence argument.
   */
  inline void tag_sentence(mootSentence &sentence) {
    lpf.prepare(*this, sentence);
    mootHMM::tag_sentence(sentence);
    lpf.cleanup(*this,sentence);
  };
  //@}
};

/*--------------------------------------------------------------------------
 * DynLexProbFcn
 */
/**
 * \brief API specification for mootDynHMM::LexProbFunc
 */
class DynLexProbFcn {
public:
  DynLexProbFcn(void) {};  /**< Default constructor */
  ~DynLexProbFcn(void) {}; /**< Default destructor */

  /** Add/adjust lexical probabilities in \c hmm prior to tagging \c sent */
  //-- TODO: break type-loops!
  void prepare(mootDynHMM &hmm, mootSentence &sent);

  /** Cleanup lexical probabilities in \c hmm after tagging \c sent */
  void cleanup(mootDynHMM &hmm, mootSentence &sent);
};

/*--------------------------------------------------------------------------
 * class mootLPFUniform
 */
/** \brief uniform p(w|tag) for mootDynHMM */
class LPFUniform : public DynLexProbFunc {
public:
  LPFUniform(void) {};
  ~LPFUNiform(void) {};

  void prepare(mootDynHMM &hmm, mootSentence &sent) {
    lex_clear();
    for (mootSentence::const_iterator si = sent.begin(); si != sent.end(); si++) {
      if (si->toktype() != TokTypeVanilla) continue; //-- ignore non-vanilla tokens

      //-- populate LexClass lc
      mootHMM::LexClass lc;
      for (mootToken::Analyses::const_iterator ani = token.analyses().begin(); ani != token.analyses().end(); ani++) {
	mootHMM::TagID tagid = hmm.tagids.name2id(ani->tag);
	lc.insert(tagid);
      }

      //-- populate tokid, lexprobs
      mootHMM::TokID tokid = hmm.tokids.get_id(tok.text());
      if (hmm.lexprobs.size() <= tokid) hmm.lexprobs.resize(tokid+1);
      if (lc.size() > 0) {
	ProbT log_pwgt = log(1.0) - log(lc.size());
	for (mootHMM::LexClass::const_iterator lci = lc.begin(); lci != lc.end(); lci++) {
	  hmm.lexprobs[tokid].insert(*lci, log_pwgt);
	}
      }
    }
  };

  void cleanup(mootDynHMM &hmm, mootSentence &sent) {};
};


moot_END_NAMESPACE

#endif /* _MOOT_DYNHMM_H */
