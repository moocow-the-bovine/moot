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

#include <stdio.h>
#include <mootTypes.h>
#include <mootToken.h>
#include <mootAssocVector.h>
#include <mootTrieVector.h>
#include <mootEnum.h>

moot_BEGIN_NAMESPACE

using namespace std;

/** Typedef for suffix trie data */
typedef AssocVector<mootEnumID,ProbT> SuffixTrieDataT;

/** \brief Top-level class for suffix tries */
template<>
class SuffixTrie : public TrieVector<SuffixTrieDataT>
{
public:
  //--------------------------------------------------
  // SuffixTrie: typedefs

  /** Typedef for a tag-id */
  typedef mootEnumID              TagID;

  /** Typedef for tag-id lookup table */
  typedef mootEnum<mootTagString> TagIDTable;

public:
  //--------------------------------------------------
  // SuffixTrie: data
  ProbT   theta; ///< standard deviation of unigram MLEs


  //--------------------------------------------------
  /// \name Compilation
  //@{
  /** Construct a suffix trie from a mootLexfreqs object
   *  and a mootEnum object for tagids */
  inline void compile(const mootLexfreqs &lf,
		      const TagIDTable &tagids,
		      bool  verbose=false)
  {
    theta = 0;
    //-- TODO!
  };
  //@}

  //--------------------------------------------------
  /// \name Lookup
  //@{
  /** Get probability p(tokstr|tagid) based on suffix information */
  inline ProbT sufprob(const mootTokStr &tokstr, TagID tagid) const
  {
    //-- TODO!
  };
  //@}
}; //-- /class SuffixTrie

moot_END_NAMESPACE

#endif // _MOOT_SUFFIX_TRIE_H
