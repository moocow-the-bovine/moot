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

/*============================================================================
 * File: mootLexfreqs.h
 * Author:  Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *    Class for storage & retrieval of lexical frequencies (pair version)
 *============================================================================*/

#ifndef _moot_LEXFREQS_H
#define _moot_LEXFREQS_H

#include <mootTypes.h>
moot_BEGIN_NAMESPACE

/**
 * \brief Class for storage and retrieval of raw lexical frequencies.
 */
class mootLexfreqs {
public:
  //------ public typedefs

  /** Type for a single lexeme+tag co-occurrence count */
  typedef CountT LexfreqCount;

  /**
   * Type for frequency lookup subtables.
   */
  typedef map<mootTagString,LexfreqCount> LexfreqSubtable;

  /**
   * Type for frequency lookup table entries.
   */
  class LexfreqEntry {
  public:
    LexfreqCount     total;     /**< Total occurrences of this lexeme */
    LexfreqSubtable  freqs;     /**< Maps tags to occurrences of this lexeme with key tag */
  public:
    LexfreqEntry(const LexfreqCount tok_total=0) : total(tok_total) {};
    LexfreqEntry(const LexfreqCount tok_total, const LexfreqSubtable &tok_tagfreqs)
      : total(tok_total), freqs(tok_tagfreqs)
    {};
  };

  /**
   * Type for the lexical frequency lookup table.
   */
  typedef hash_map<mootTokString,LexfreqEntry> LexfreqTokTable;

  /** Lookup table: tag->Count(tag) */
  typedef hash_map<mootTagString,LexfreqCount> LexfreqTagTable;

public:
  //------ public data
  LexfreqTokTable    lftable;    /**< lexeme->(tag->count) lookup table */
  LexfreqTagTable    tagtable;   /**< tag->count lookup table */
  LexfreqCount       n_tokens;   /**< total number of tokens counted */

public:
  //------ public methods
  /** Default constructor */
  mootLexfreqs(size_t initial_bucket_count=0) : n_tokens(0)
  {
    if (initial_bucket_count != 0) {
      lftable.resize(initial_bucket_count);
    }
  };

  /** Default destructor */
  ~mootLexfreqs() {
    clear();
  }

  //------ public methods: manipulation

  /** Clear internal table(s) */
  void clear(void);

  /** Add 'count' to the current count for (token,tag) */
  inline void add_count(const mootTokString &text,
			const mootTagString &tag,
			const LexfreqCount count)
  {
    //-- adjust token-table
    LexfreqTokTable::iterator lfi = lftable.find(text);
    if (lfi == lftable.end()) {
      //-- new token
      lfi = lftable.insert(LexfreqTokTable::value_type(text,LexfreqEntry(count))).first;
      lfi->second.freqs[tag] = count;
    } else {
      //-- known token
      lfi->second.total += count;

      LexfreqSubtable::iterator lsi = lfi->second.freqs.find(tag);
      if (lsi == lfi->second.freqs.end()) {
	//-- unknown (tok,tag) pair
	lfi->second.freqs[tag] = count;
      } else {
	//-- known (tok,tag) pair: just add
	lsi->second += count;
      }
    }

    //-- adjust total tag-count
    LexfreqTagTable::iterator lftagi = tagtable.find(tag);
    if (lftagi != tagtable.end()) {
      lftagi->second += count;
    } else {
      tagtable[tag] = count;
    }

    //-- adjust total token-count
    n_tokens += count;
  };

  //------ public methods: lookup
  const LexfreqCount taglookup(const mootTagString &tag) const
  {
    LexfreqTagTable::const_iterator tagi = tagtable.find(tag);
    return tagi == tagtable.end() ? 0 : tagi->second;
  };

  //------ public methods: i/o

  /** Load n-grams from a TnT-style parameter file */
  bool load(char *filename);

  /** Load n-grams from a TnT-style parameter file (stream version) */
  bool load(FILE *file, char *filename = NULL);

  /** Save n-grams to a TnT-style paramater file */
  bool save(char *filename);

  /** Save n-grams to a TnT-style paramater file (stream version) */
  bool save(FILE *file, char *filename = NULL);
};


moot_END_NAMESPACE

#endif /* _moot_LEXFREQS_H */
