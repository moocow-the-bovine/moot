/* -*- Mode: C++ -*- */

/*
   libmoot version 1.0.4 : moocow's part-of-speech tagging library
   Copyright (C) 2003 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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
 *   + Trainer for KDWDS HMM PoS-tagger: shared headers
 *--------------------------------------------------------------------------*/

#ifndef _moot_HMM_TRAINER_H_
#define _moot_HMM_TRAINER_H_

#include <stdio.h>

#include <set>
#include <deque>

#include "mootTypes.h"
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
  typedef mootNgrams::NgramString Ngram;

  /** Type for counts */
  typedef mootNgrams::NgramCount CountT;

  /** Type for current tag-sets */
  typedef set<mootTagString> TagSet;

  /** Class for on-the-fly computation of n-gram sets */
  class NgramSet {
  public:
    //-- iteration
    struct ngIteratorItem {
      size_t pos;
      TagSet::iterator cur;
      TagSet::iterator begin;
      TagSet::iterator end;
    };

    typedef deque<ngIteratorItem>         ngIterator;
    typedef ngIterator::iterator          ngIterator2;
    typedef ngIterator::reverse_iterator  ngIterator2r;

  public:
    typedef deque<TagSet> ngsType;
  public:
    ngsType ngs;
  public:
    //-- construct / destroy
    NgramSet(size_t size=0) { ngs.resize(size); };
    ~NgramSet(void) {};

    //-- clear , size, resize
    inline void clear(void) { ngs.clear(); };
    inline const size_t size(void) { return ngs.size(); };
    inline void resize(size_t size) { ngs.resize(size); };

    //-- push, pop, step
    inline void push_front(const TagSet &ts) { ngs.push_front(ts); };
    inline void push_back(const TagSet &ts) { ngs.push_back(ts); };
    inline void pop_front(void) { ngs.pop_front(); };
    inline void pop_back(void) { ngs.pop_back(); };
    inline void step(const TagSet &ts)
    {
      ngs.pop_front();
      ngs.push_back(ts);
    };

    //-- iteration
    inline ngIterator iter_begin(size_t len=0)
    {
      ngIterator it(len <= ngs.size() ? len : ngs.size());
      return iter_begin(it,len);
    };

    inline ngIterator &iter_begin(ngIterator &it, size_t len=0)
    {
      it.resize(len <= ngs.size() ? len : ngs.size());
      ngIterator2 iti;
      ngsType::iterator ngsi;
      size_t pos;
      for (iti    = it.begin() ,  ngsi  = ngs.begin() , pos = 0;
	   iti   != it.end()  &&  ngsi != ngs.end();
	   iti++               ,  ngsi++              , pos++)
	{
	  iti->pos   = pos;
	  iti->begin = iti->cur = ngsi->begin();
	  iti->end   = ngsi->end();
	}
      return it;
    };

    inline bool iter_valid(ngIterator &it)
    {
      return !it.empty() && it.front().cur != it.front().end;
    };

    inline ngIterator &iter_next(ngIterator &it)
    {
      if (!iter_valid(it)) return it;

      ngIterator2r                 itr;
      for (itr = it.rbegin(); itr != it.rend(); itr++) {
	//-- easy increment
	itr->cur++;

	//-- check overflow
	if (itr->cur == itr->end &&  itr->pos != 0) {
	  itr->cur = itr->begin;
	  continue;
	}

	//-- done
	break;
      }
      return it;
    };

    inline Ngram &iter2ngram(ngIterator &it, Ngram &ng)
    {
      ng.resize(it.size());
      Ngram::iterator ngi;
      ngIterator2 iti;
      for (iti    = it.begin() ,  ngi  = ng.begin();
	   iti   != it.end()  &&  ngi != ng.end();
	   iti++               ,  ngi++)
	{
	  *ngi = *(iti->cur);
	}
      return ng;
    };

  };
  //@}

public:
  /*-------------------------------------------------------------*/
  /// \name Training data
  //@{

  /** Lexical frequency data */
  mootLexfreqs lexfreqs;

  /** N-gram data */
  mootNgrams ngrams;
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

  /** Maximum length of n-grams for which to gather data (n<=kmax) */
  unsigned short int kmax;
  //@}

protected:
  /*------------------------------------------------------------*/
  /// \name Runtime training state
  //@{
  /** Current n-grams */
  NgramSet ngset;

  /** Temporary ngram for counting */
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
      kmax(3),
      last_was_eos(false)
  {};

  /** Default destructor */
  ~mootHMMTrainer(void) {};
  //@}


  /*------------------------------------------------------------*/
  /// \name Reset / Clear
  //@{
  /** Reset / clear the object. */
  void clear(void)
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
  inline void train_init(void)
  {
    ngset.resize((size_t)kmax);
    ng.resize((size_t)kmax);
    train_bos();
    //-- count one EOS marker (TnT compatibility hack)
    if (want_ngrams) {
      ng.clear();
      ng.push_back(eos_tag);
      ngrams.add_count(ng,1);
    }
  };

  /** Initialize data for training a new sentence  */
  inline void train_bos(void)
  {
    if (want_ngrams) {
      for (NgramSet::ngsType::iterator ngsi = ngset.ngs.begin(); ngsi != ngset.ngs.end(); ngsi++) {
	ngsi->clear();
	ngsi->insert(eos_tag);
      }
    }
  };

  /** Gather training information for a single token. */
  inline void train_token(const mootTokString &curtok, const TagSet &curtags)
  {
    CountT count = 1.0 / ((CountT)curtags.size());
    TagSet::const_iterator cti;

    if (want_lexfreqs) {
      //-- count lexical frequencies
      for (cti = curtags.begin(); cti != curtags.end(); cti++) {
	lexfreqs.add_count(curtok, *cti, count);
      }
    }

    if (want_ngrams) _train_token_ng(curtags);
  };

  /** Gather ngram-training information for a single token. */
  inline void _train_token_ng(const TagSet &curtags)
  {
    if (!want_ngrams) return;

    //-- count kmax-grams: add in next tagset
    ngset.step(curtags);

    //-- count all current ngrams
    NgramSet::ngIterator ngsi;
    size_t len;
    CountT ngcount;
    for (len = 1; len <= kmax; len++) {
      //-- get count
      ngcount = 0;
      for (ngsi = ngset.iter_begin(len); ngset.iter_valid(ngsi); ngset.iter_next(ngsi)) {
	ngcount++;
      }
      ngcount = 1.0/ngcount; //-- normalize
    
      //-- count ngrams
      for (ngsi = ngset.iter_begin(len); ngset.iter_valid(ngsi); ngset.iter_next(ngsi)) {
	ngset.iter2ngram(ngsi,ng);
	//-- ignore redundant n-grams for the boundary tag
	if (len > 1 && ((ng[0] == eos_tag && ng[1] == eos_tag)
			|| (ng.back() == eos_tag && ng[len-2] == eos_tag)))
	  continue;
	ngrams.add_count(ng,ngcount);
      }
    }

    //-- hack
    last_was_eos = false;
  };

  /** Gather training information for a sentence boundary. */
  void train_eos(void)
  {
    if (want_ngrams && !last_was_eos) {
      TagSet eostags;
      eostags.insert(eos_tag);
      for (int i = 1; i < kmax; i++) {
	_train_token_ng(eostags);
      }
    }
    last_was_eos = true;
  };
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
