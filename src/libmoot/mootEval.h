/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2004 by Bryan Jurish <moocow@ling.uni-potsdam.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

/*--------------------------------------------------------------------------
 * File: mootEval.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moot PoS tagger : evaluator
 *--------------------------------------------------------------------------*/

#ifndef _moot_EVAL_H
#define _moot_EVAL_H

#include <mootTypes.h>
#include <mootToken.h>
#include <string>


moot_BEGIN_NAMESPACE

/*--------------------------------------------------------------------------
 * mootEval : evaluation class
 *--------------------------------------------------------------------------*/

/** Evaluator class */
class mootEval {
public:

  /** Enum for evaluation flags */
  typedef enum {
    MEF_None           = 0x0000, /**< no errors */
    //MEF_SentMismatch = 0x0001, /**< sentence lengths differ */
    MEF_TokMismatch    = 0x0002, /**< token-text mismatch */
    MEF_BestMismatch   = 0x0004, /**< best-tag mismatch */
    MEF_EmptyClass1    = 0x0008, /**< empty class in 1st token */
    MEF_EmptyClass2    = 0x0010, /**< empty class in 2nd token */
    MEF_ImpClass1      = 0x0020, /**< Impossible class in 1st token (besttag1 \not\in class1) */
    MEF_ImpClass2      = 0x0040, /**< Impossible class in 2nd token (besttag2 \not\in class2) */
    MEF_XImpClass1     = 0x0080, /**< X-Impossible class in 2nd token (besttag1 \not\in class2) */
    MEF_XImpClass2     = 0x0100  /**< X-Impossible class in 2nd token (besttag2 \not\in class1) */
  } evalStatus;

public:
  int status;                    /**< status flags for this object */

public:
  mootEval(void) : status(MEF_None) {};

  /** Clear the object */
  inline void clear(void) { status = MEF_None; };

  /**
   * Compare content of two mootToken objects : returns binary OR'd evalStatus flags
   * \note You must call this before any of the \c is*() methods will return
   * meaningful values.
   */
  int compareTokens(const mootToken &tok1, const mootToken &tok2);

  /** Compare content of two mootSentence objects */
  //list<mootEvalFlag> compareSentences(const mootSentence &sent1, const mootSentence &sent2);

  /*----------------------------------------------------
   * Accessors
   */
  /** True after compareTokens(tok1,tok2) iff tokens mismatchedw */
  inline bool isTokenMismatch(void) { return status&MEF_TokMismatch; };

  /** True after compareTokens(tok1,tok2) iff best-tags mismatched */
  inline bool isBestMismatch(void) { return status&MEF_BestMismatch; };

  /**
   * True after compareTokens(tok1,tok2) iff tok1 had empty analyses:
   * iff \f$ tok1.analyses() = \emptyset \f$
   */
  inline bool isEmptyClass1(void) { return status&MEF_EmptyClass1; };

  /**
   * True after compareTokens(tok1,tok2) iff tok2 had empty analyses:
   * iff \f$ tok2.analyses() = \emptyset \f$
   */
  inline bool isEmptyClass2(void) { return status&MEF_EmptyClass2; };

  /**
   * True after \c compareTokens(tok1,tok2) iff tok1 had impossible non-empty analysis-set:
   * iff \f$ (tok1.besttag() \not\in tok1.analyses()) \f$
   */
  inline bool isImpClass1(void) { return status&MEF_ImpClass1; };

  /**
   * True after \c compareTokens(tok1,tok2) iff tok2 had impossible non-empty analysis-set:
   * iff \f$ (tok2.besttag() \not\in tok2.analyses()) \$
   */
  inline bool isImpClass2(void) { return status&MEF_ImpClass2; };

  /**
   * True after \c compareTokens(tok1,tok2) iff tok1 had cross-impossible non-empty analysis-set:
   * iff \f$ (tok2.besttag() \not\in tok1.analyses()) \f$
   */
  inline bool isXImpClass1(void) { return status&MEF_XImpClass1; };

  /**
   * True after \c compareTokens(tok1,tok2) iff tok2 had cross-impossible non-empty analysis-set:
   * iff \f$ (tok1.besttag() \not\in tok2.analyses()) \$
   */
  inline bool isXImpClass2(void) { return status&MEF_XImpClass2; };


  /*----------------------------------------------------
   * Strinification
   */
  /**
   * Returns canonical string-form of a mootEvalFlag: this
   * string 3 fields of 2 ASCII characters each, each
   * character corresponding to one of the bits in
   * \c flags.  The output looks like:
   *
   * \c {:Tok:}{:Best:}":"{:Empty1:}{:Imp1:}{:XImp1:}":"{:Empty2:}{:Imp2:}{:XImp2:}
   *
   * where a literal '-' indicates an unset flag, and
   * any other character indicates that the flag is set.
   * The actual characters used are mnemonics for the
   * corresponding flags:
   *
   * \c (t|-)(b|-):(e|-)(i|-)(x|-):(e|-)(i|-)(x|-)
   */
  string status_string(void);
};


moot_END_NAMESPACE

#endif /* _moot_EVAL_H */
