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
 * File: mootHMMTrainer.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + Trainer for moocow's PoS-tagger: shared headers
 *--------------------------------------------------------------------------*/

#ifndef _moot_HMM_TRAINER_H_
#define _moot_HMM_TRAINER_H_

#include <stdio.h>

#include <set>
#include <deque>

#include "mootTypes.h"
#include "mootToken.h"
#include "mootLexfreqs.h"
#include "mootNgrams.h"

moot_BEGIN_NAMESPACE

using namespace std;

/*--------------------------------------------------------------------------
 * mootHMMTrainer : HMM trainer class
 *--------------------------------------------------------------------------*/

/// High-level class to gather training data for a mootHMM or mootCHMM.
class mootHMMTrainer {

public:
  /*-------------------------------------------------------------*/
  /// \name Training types
  //@{
  /** Type for an N-gram */
  typedef mootNgrams::Ngram      Ngram;

  /** Type for counts */
  typedef mootNgrams::NgramCount CountT;

  /** Type for current tag-sets */
  typedef set<mootTagString> TagSet;

public:
  /*-------------------------------------------------------------*/
  /// \name Training data
  //@{

  /** Lexical frequency data */
  mootLexfreqs lexfreqs;

  /** N-gram data */
  mootNgrams   ngrams;
  //@}

  /*-------------------------------------------------------------*/
  /// \name Flags
  //@{
  bool want_lexfreqs;  /**< Whether to gather lexical frequency data */
  bool want_ngrams;    /**< Whether to gather n-gram frequency data */
  //@}

  /*-------------------------------------------------------------*/
  /// \name Pragmatic constants
  //@{
  /** String indicating end-of-sentence */
  mootTagString eos_tag;

  /* Maximum length of n-grams for which to gather data (n<=kmax) */
  //unsigned short int kmax;
  //@}

protected:
  /*------------------------------------------------------------*/
  /// \name Runtime training state
  //@{
  /** Current n-gram window */
  Ngram ng;

  /** Stupid hack */
  bool last_was_eos;
  //@}

public:
  /*------------------------------------------------------------*/
  /// \name Constructor / destructor
  //@{
  /** Default constructor */
  mootHMMTrainer(void)
    : want_lexfreqs(true),
      want_ngrams(true),
      eos_tag("__$"),
      last_was_eos(false)
  {};

  /** Default destructor */
  ~mootHMMTrainer(void) {};
  //@}


  /*------------------------------------------------------------*/
  /// \name Reset / Clear
  //@{
  /** Reset / clear the object. */
  inline void clear(void)
  {
    lexfreqs.clear();
    ngrams.clear();
  };
  //@}

  /*------------------------------------------------------------*/
  /// \name Top-level training methods
  //@{
  /** Gather training data from a C-stream using mootTaggerLexer */
  bool train_from_stream(FILE *in=stdin, const char *filename="(unknown)");

  /** Gather training data from a file using mootTaggerLexer */
  bool train_from_file(const char *filename);
  //@}

  /*------------------------------------------------------------*/
  /// \name Mid-level training methods
  //@{
  /** Initialize training data  */
  void train_init(void);

  /** Initialize data for training a new sentence  */
  void train_bos(void);

  /** Gather training information for a single token, using mootToken */
  void train_token(const mootToken &curtok);

  /** Gather training information for a sentence boundary. */
  void train_eos(void);
  //@}

  /*------------------------------------------------------------*/
  /// \name Warnings / Errors
  //@{

  /** Error reporting */
  void carp(char *fmt, ...);

  //@}
};

moot_END_NAMESPACE

#endif /* _moot_HMM_TRAINER_H */
