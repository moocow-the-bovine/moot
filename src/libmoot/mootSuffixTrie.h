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
  SuffixTrie(void) : maxcount(10), theta(0) {};

  /** Destructor */
  ~SuffixTrie(void) {};
  //@}

  //--------------------------------------------------
  /// \name Compilation
  //@{
  /** Construct a suffix trie from a mootLexfreqs object
   *  and a mootEnum object for tagids */
  void compile(const mootLexfreqs &lf,
	       const TagIDTable   &tagids,
	       bool  verbose=false);
  //@}

  //--------------------------------------------------
  /// \name Lookup
  //@{
  /** Get (log-) probability table tagid=>log(P(tokstr|tagid))
   *  based on longest matched suffix */
  inline const SuffixTrieDataT& sufprobs(const mootTokString &tokstr) const
  {
    const_iterator ti = rfind_longest(tokstr);
    return (ti==end() ? default_data() : ti->data);
  };

  /** Get (log-) probability table tagid=>log(P(token.text()|tagid))
   *  based on longest matched suffix */
  inline const SuffixTrieDataT& sufprobs(const mootToken &token) const
  { return sufprobs(token.text()); };



  /** Get (log-) probability log(P(tokstr|tagid))
   *  based on longest matched suffix */
  inline ProbT sufprob(const mootTokString &tokstr, const TagID tagid) const
  {
    const_iterator ti = rfind_longest(tokstr);
    if (ti==end()) return MOOT_PROB_ZERO;
    SuffixTrieDataT::const_iterator tdi = ti->data.find(tagid);
    return (tdi==ti->data.end() ? MOOT_PROB_ZERO : tdi->value());
  };

  /** Get (log-) probability log(P(token.text()|tagid))
   *  based on longest matched suffix */
  inline ProbT sufprob(const mootToken &token, const TagID tagid) const
  { return sufprob(token.text(),tagid); };
  //@}
}; //-- /class SuffixTrie

moot_END_NAMESPACE

#endif // _MOOT_SUFFIX_TRIE_H
