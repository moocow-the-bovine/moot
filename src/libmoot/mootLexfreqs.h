/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2012 by Bryan Jurish <moocow@cpan.org>

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

/*============================================================================
 * File: mootLexfreqs.h
 * Author:  Bryan Jurish <moocow@cpan.org>
 * Description:
 *    Class for storage & retrieval of lexical frequencies (nested map<> version)
 *============================================================================*/

/**
\file mootLexfreqs.h
\brief HMM training data: lexical frequencies: raw
*/

#ifndef _moot_LEXFREQS_H
#define _moot_LEXFREQS_H

#include <mootToken.h>

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
    LexfreqCount     count;  /**< Total occurrences of this lexeme */
    LexfreqSubtable  freqs;  /**< Maps tags to occurrences of this lexeme with key tag */
  public:
    LexfreqEntry(const LexfreqCount tok_count=0)
      : count(tok_count)
    {};
    LexfreqEntry(const LexfreqCount tok_count,
		 const LexfreqSubtable &tok_tagfreqs)
      : count(tok_count), freqs(tok_tagfreqs)
    {};
    /** Reset to empty */
    void clear(void) {
      count = 0;
      freqs.clear();
    };
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
  void add_count(const mootTokString &text, const mootTagString &tag, const LexfreqCount count);

  //------ public methods: lookup

  /** get total frequency of a text type ("token") */
  inline LexfreqCount f_text(const mootTokString &text) const
  {
    LexfreqTokTable::const_iterator txti = lftable.find(text);
    return txti == lftable.end() ? 0 : txti->second.count;
  };

  /** get frequency of a tag */
  inline LexfreqCount f_tag(const mootTagString &tag) const
  {
    LexfreqTagTable::const_iterator tagi = tagtable.find(tag);
    return tagi == tagtable.end() ? 0 : tagi->second;
  };

  /** get total frequency of a (text,tag) pair */
  inline LexfreqCount f_text_tag(const mootTokString &text, const mootTagString &tag) const
  {
    LexfreqTokTable::const_iterator txti = lftable.find(text);
    if (txti == lftable.end()) return 0;
    const LexfreqSubtable& tagfreqs = txti->second.freqs;
    LexfreqSubtable::const_iterator tagi = tagfreqs.find(tag);
    return tagi == tagfreqs.end() ? 0 : tagi->second;
  };

  /**
   * Add counts for 'special' tokens to the object.
   * This should be called after you have added
   * all 'real' tokens.
   *
   * \see token2type().
   */
  void compute_specials(void);

  /**
   * Return the number of distinct (token,tag) pairs we've counted.
   */
  size_t n_pairs(void);

  //------ public methods: i/o

  /** Load data from a TnT-style parameter file */
  bool load(const char *filename);

  /** Load data from a TnT-style parameter file (stream version) */
  bool load(FILE *file, const char *filename = NULL);

  /** Save data to a TnT-style paramater file */
  bool save(const char *filename);

  /** Save data to a TnT-style paramater file (stream version) */
  bool save(FILE *file, const char *filename = NULL);
};


moot_END_NAMESPACE

#endif /* _moot_LEXFREQS_H */
