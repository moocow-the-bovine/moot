/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2005 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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
 * File: mootSuffixTrie.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moocow's PoS tagger : suffix trie
 *--------------------------------------------------------------------------*/

#ifndef _MOOT_SUFFIX_TRIE_H
#define _MOOT_SUFFIX_TRIE_H

#include <mootTypes.h>
#include <mootToken.h>
#include <mootLexfreqs.h>
#include <mootNgrams.h>
#include <mootEnum.h>
#include <mootAssocVector.h>
#include <mootTrieVector.h>

moot_BEGIN_NAMESPACE

/** Typedef for suffix trie data */
typedef AssocVector<mootEnumID,ProbT> SuffixTrieDataT;

/** \brief Top-level class for suffix tries */
//template<>
class SuffixTrie : public TrieVector<SuffixTrieDataT>
{
public:
  //------------------------------------------------------------
  // SuffixTrie: Static Data
  //static const size_t SuffixTrieDefaultMaxLen = 10;
  static const size_t SuffixTrieDefaultMaxLen = 0;

public:
  //------------------------------------------------------------
  // SuffixTrie: Types

  /** Typedef for a tag-id */
  typedef mootEnumID              TagID;

  /** Typedef for tag-id lookup table */
  typedef mootEnum<mootTagString> TagIDTable;

  /** Underlying trie template type */
  typedef TrieVector<SuffixTrieDataT>  TrieType;

public:
  //------------------------------------------------------------
  // SuffixTrie: data
  CountT        maxcount;  ///< raw frequency upper bound
  ProbT         theta;     ///< standard deviation of unigram MLEs


public:
  //------------------------------------------------------------
  // SuffixTrie: Methods

  //--------------------------------------------------
  /// \name Constructors etc.
  //@{
  /** Default constructor */
  SuffixTrie(size_t max_length =SuffixTrieDefaultMaxLen,
	     bool   use_case   =true,
	     size_t max_count  =10)
    : TrieType(max_length,use_case),
      maxcount(max_count)
  {};

  /** Destructor */
  ~SuffixTrie(void) {};
  //@}

  //--------------------------------------------------
  /// \name Compilation
  //@{
  /** Construct a suffix trie from a mootLexfreqs object
   *  and a mootEnum object for tagids */
  bool build(const mootLexfreqs &lf,
	     const mootNgrams   &ng,
	     const TagIDTable   &tagids,
	     TagID eos_tagid,
	     bool  verbose=false);

  /** Low-level compilation utilitiy:
   *  enqueue pending suffix arcs */
  bool _build_insert(const mootLexfreqs &lf);

  /** Low-level compilation utilitiy:
   *  assign count data to all trie nodes */
  bool _build_assoc(const mootLexfreqs &lf, const TagIDTable &tagids);

  /** Low-level compilation utilitiy:
   *  compute smoothing constants */
  bool _build_compute_theta(const mootLexfreqs &lf,
			    const mootNgrams   &ng,
			    const TagIDTable   &tagids,
			    TagID eos_tagid);

  /** Low-level compilation utilitiy:
   *  compute MLE probabilities P(tag|suffix) */
  bool _build_compute_mles(const mootLexfreqs &lf,
			   const mootNgrams   &ng,
			   const TagIDTable   &tagids,
			   TagID eos_tagid);

  /** Low-level compilation utilitiy:
   *  Bayesian inversion: compute P(suffix|t) = P(t|suffix)*P(suffix)/P(t)
   */
  bool _build_invert_mles(const mootNgrams &ng,
			  const TagIDTable &tagids,
			  TagID eos_tagid);
  //@}

  //--------------------------------------------------
  /// \name Lookup
  //@{
  /** Get first ancestor (or self) with non-empty data,
   * or end() on failure (read/write) */
  inline iterator find_ancestor_nonempty(iterator dtr, size_t *matchlen=NULL)
  {
    if (matchlen) *matchlen = 0;
    for ( ; dtr != end() && dtr->data.empty(); dtr=find_mother(*dtr)) {
      if (matchlen) (*matchlen)--;
    }
    return dtr;
  };

  /** Get first real ancestor (or self) with non-empty data,
   * or end() on failure (read-only) */
  inline const_iterator const_find_ancestor_nonempty(const_iterator dtr, size_t *matchlen=NULL)
    const
  {
    if (matchlen) *matchlen = 0;
    for ( ; dtr != end() && dtr->data.empty(); dtr=find_mother(*dtr)) {
      if (matchlen) (*matchlen)--;
    }
    return dtr;
  };


  /** Get reverse-longest match iterator with actual data, read/write */
  inline iterator rfind_longest_nonempty(const mootTokString &tokstr,
					 size_t              *matchlen=NULL)
  { return find_ancestor_nonempty(rfind_longest(tokstr,matchlen),matchlen); };

  /** Get reverse-longest match iterator with actual data, read-only */
  inline const_iterator rfind_longest_nonempty(const mootTokString &tokstr,
					       size_t              *matchlen=NULL)
    const
  { return const_find_ancestor_nonempty(rfind_longest(tokstr,matchlen),matchlen); };


  /** Get (log-) probability table tagid=>log(P(tokstr|tagid))
   *  based on longest matched suffix */
  inline const SuffixTrieDataT& sufprobs(const mootTokString &tokstr, size_t *matchlen=NULL)
    const
  {
    const_iterator ti = rfind_longest_nonempty(tokstr,matchlen);
    return (ti==end() ? default_data() : ti->data);
  };
  //@}

  //--------------------------------------------------
  /// \name Debug / Dump
  //@{
  void txtdump(FILE *out, const TagIDTable &tagids) const;
  //@}

}; //-- /class SuffixTrie

moot_END_NAMESPACE

#endif // _MOOT_SUFFIX_TRIE_H
