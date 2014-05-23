/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2014 Jurish <moocow@cpan.org>

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
#include <mootFlavor.h>

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
  LexfreqCount	     unknown_threshhold; /**< maximum frequency for special @UNKNOWN lexeme (default=1) */
  const mootTaster  *taster;		 /**< regex-based token flavor heuristics (default=builtin; NULL for none) */

public:
  //------ public methods
  /** Default constructor */
  mootLexfreqs(size_t initial_bucket_count=0)
    : n_tokens(0), unknown_threshhold(1.0), taster(&builtinTaster)
  {
    if (initial_bucket_count != 0)
      lftable.resize(initial_bucket_count);
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

  /** Remove entry for a word */
  void remove_word(const mootTokString &text);

  //------ public methods: lookup

  /** get total frequency of a text type ("token") */
  inline LexfreqCount f_word(const mootTokString &w) const
  {
    LexfreqTokTable::const_iterator wi = lftable.find(w);
    return wi == lftable.end() ? 0 : wi->second.count;
  };

  /** get frequency of a tag */
  inline LexfreqCount f_tag(const mootTagString &tag) const
  {
    LexfreqTagTable::const_iterator ti = tagtable.find(tag);
    return ti == tagtable.end() ? 0 : ti->second;
  };

  /** get total frequency of a (word,tag) pair */
  inline LexfreqCount f_word_tag(const mootTokString &w, const mootTagString &tag) const
  {
    LexfreqTokTable::const_iterator wi = lftable.find(w);
    if (wi == lftable.end()) return 0;
    LexfreqSubtable::const_iterator wti = wi->second.freqs.find(tag);
    return wti == wi->second.freqs.end() ? 0 : wti->second;
  };

  /**
   * Compute counts for 'special' pseudo-lexemes to the object.
   * These include all flavors defined by \a taster (if specified and non-null),
   * as well as the special @UNKNOWN token. 
   * You should have set \a taster before calling this method.
   *
   * \warning This method will \b NOT overwrite entries for any (pseudo-)lexeme
   * with a defined frequency greater than zero. Call remove_specials() first
   * if you want to re-compute all special entries.
   *
   * \param compute_unknown whether to also compute @UNKNOWN entry
   */
  void compute_specials(bool compute_unknown=true);

  /**
   * Remove entries for 'special' pseudo-lexemes from the object.
   * You should have set \a taster before calling this method.
   *
   * \param taster mootTaster for determining which lexemes to remove
   * \param compute_unknown whether to also remove @UNKNOWN entry
   */
  void remove_specials(bool remove_unknown=true);

  /**
   * Discount pseudo-frequencies for 'special' pseudo-lexemes.
   * \param zf_special total frequency mass to alot for 'special' pseudo-lexemes.
   */
  void discount_specials(CountT zf_special=1.0);

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
