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
 * File: mootNgrams.h
 * Author:  Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *    Class for storage & retrieval of N-Gram counts
 *============================================================================*/

#ifndef _moot_NGRAMS_H
#define _moot_NGRAMS_H

#include <mootTypes.h>

moot_BEGIN_NAMESPACE

/**
 * \brief Class for storage & retrieval of raw N-Gram frequencies.
 */
class mootNgrams {

public:
  //------ public typedefs

  /** Type for an N-Gram count */
  typedef CountT NgramCount;

  /** Type for tertiary (tag->trigramCount) lookup-table entry */
  typedef map<mootTagString,NgramCount> TrigramTable;

  /** Type for secondary lookup-table entry */
  class BigramEntry {
  public:
    CountT       count;   /**< bigram count */
    TrigramTable  freqs;   /**< map from tags to trigram counts beginning with this bigram */
  public:
    BigramEntry(const CountT bg_count=0) : count(bg_count) {};
  };

  /** Type for secondary lookup table */
  typedef map<mootTagString,BigramEntry> BigramTable;
  
  /** Type for primary lookup-table entry */
  class UnigramEntry {
  public:
    CountT       count;   /**< bigram count */
    BigramTable  freqs;   /**< map from tags to bigram entries beginning with this tag */
  public:
    UnigramEntry(const CountT ug_count=0) : count(ug_count) {};
  };

  /** Type for primary lookup-table */
  typedef map<mootTagString,UnigramEntry> NgramTable;

  /** Class for full trigram data: used by interface methods */
  class Ngram : public deque<mootTagString> {
  public:
    /** Default constructor */
    Ngram(void) {};
    /** Unigram constructor */
    Ngram(const mootTagString &tag1) {
      push_back(tag1);
    };
    /** Bigram constructor */
    Ngram(const mootTagString &tag1, const mootTagString &tag2) {
      push_back(tag1);
      push_back(tag2);
    };
    /** Trigram constructor */
    Ngram(const mootTagString &tag1,
	  const mootTagString &tag2,
	  const mootTagString &tag3) {
      push_back(tag1);
      push_back(tag2);
      push_back(tag3);
    };

    /** Default destructor */
    ~Ngram(void) {
      clear();
    };

    /*----------------
     * Accessors
     */
    /** Return the 1st tag of the trigram */
    const mootTagString &tag1(void) const { return (*this)[0]; } ;
    /** Return the 2nd tag of the trigram */
    const mootTagString &tag2(void) const { return (*this)[1]; };
    /** Return the 3rd tag of the trigram */
    const mootTagString &tag3(void) const { return (*this)[2]; };

    /*----------------
     * Manipulators
     */
    /** Push \c tag3 onto the end of the n-gram, shifting old \c tag1 off the front */
    void push(const mootTagString &tag_new=mootTagString("")) {
      if (size() >= 3) pop_front();
      push_back(tag_new);
    };

    /** Debug: string-form */
    string as_string(void) const {
      string s = "<";
      for (const_iterator i = begin(); i != end(); i++) {
	s.append(*i);
	s.push_back(',');
      }
      if (s.size() > 1) {
	s[s.size()-1] = '>';
      } else {
	s.push_back('>');
      }
      return s;
    };
  };

public:
  //------ public data
  NgramTable  ngtable;  /**< N-Gram to count lookup table */
  NgramCount  ugtotal;  /**< total number of unigrams */

public:
  //------ public methods
  /** Default constructor */
  mootNgrams(void) : ugtotal(0) {};

  /** Default destructor */
  ~mootNgrams() {
    clear(); 
  };

  //------ public methods: manipulation

  /** Clear the ngrams object */
  void clear(void) {
    ngtable.clear();
    ugtotal = 0;
  };

  //------ public methods: information
  /** Return the number of distinct stored bigrams */
  size_t n_bigrams(void);

  /** Return the number of distinct stored trigrams */
  size_t n_trigrams(void);

  /**
   * Add \c count to the current count for unigram <tag>.
   */
  inline void add_count(const mootTagString &tag, const NgramCount count)
  {
    ngtable[tag].count += count;
    ugtotal += count;
  };

  /**
   * Add \c count to the current count for bigram <tag1,tag2>
   * Does NOT add any unigram counts.
   */
  inline void add_count(const mootTagString &tag1,
			const mootTagString &tag2,
			const NgramCount count)
  {
    ngtable[tag1].freqs[tag2].count += count;
  };

  /**
   * Add \c count to the current count for trigram <tag1,tag2,tag3>
   * Does NOT add any bigram or unigram counts.
   */
  inline void add_count(const mootTagString &tag1,
			const mootTagString &tag2,
			const mootTagString &tag3,
			const NgramCount count)
  {
    ngtable[tag1].freqs[tag2].freqs[tag3] += count;
  };

  /**
   * Add \c count to the current count for
   * \c ngram -- length-dependent.
   */
  inline void add_count(const Ngram &ngram, const NgramCount count)
  {
    switch (ngram.size()) {
    case 0:
      break;
    case 1:
      add_count(ngram[0],count);
      break;
    case 2:
      add_count(ngram[0],ngram[1],count);
      break;
    case 3:
      add_count(ngram[0],ngram[1],ngram[2],count);
      break;
    default:
      break;
    }
  };


  /**
   * Add \c count to the current count for
   * <tag1>, <tag1,tag2>, and <tag1,tag2,tag3>
   * in \c ngram.... \c ngram may be shorter than
   * 3 tags, in which case counts are only
   * added for the elements present.
   */
  inline void add_counts(const Ngram &ngram, const NgramCount count)
  {
    size_t ngsize = ngram.size();
    if (ngsize < 1) return;

    NgramTable::iterator ngi1 = ngtable.find(ngram.tag1());
    if (ngi1 == ngtable.end()) {
      ngi1 = ngtable.insert(pair<mootTagString,UnigramEntry>(ngram.tag1(),UnigramEntry())).first;
    }
    ngi1->second.count += count;
    ugtotal += count;

    if (ngsize < 2) return;
    BigramTable::iterator ngi2 = ngi1->second.freqs.find(ngram.tag2());
    if (ngi2 == ngi1->second.freqs.end()) {
      ngi2 = ngi1->second.freqs.insert(pair<mootTagString,
				            BigramEntry>  (ngram.tag2(),BigramEntry())).first;
    }
    ngi2->second.count += count;

    if (ngsize < 3) return;
    TrigramTable::iterator ngi3 = ngi2->second.freqs.find(ngram.tag3());
    if (ngi3 == ngi2->second.freqs.end()) {
      ngi2->second.freqs[ngram.tag3()] = count;
    } else {
      ngi3->second += count;
    }
  };

  //------ public methods: lookup

  /** Returns current count for unigram, returns 0 if unknown */
  inline const NgramCount lookup(const mootTagString &tag) const
  {
    NgramTable::const_iterator ugi = ngtable.find(tag);
    return ugi == ngtable.end() ? 0 : ugi->second.count;
  };

  /** Returns current count for bigram <tag1,tag2>, returns 0 if unknown */
  inline const NgramCount lookup(const mootTagString &tag1, const mootTagString &tag2) const
  {
    NgramTable::const_iterator ugi = ngtable.find(tag1);
    if (ugi == ngtable.end()) return 0;
    BigramTable::const_iterator bgi = ugi->second.freqs.find(tag2);
    return bgi == ugi->second.freqs.end() ? 0 : bgi->second.count;
  };

  /** Returns current count for trigram <tag1,tag2,tag3>, returns 0 if unknown */
  inline const NgramCount lookup(const mootTagString &tag1,
				 const mootTagString &tag2,
				 const mootTagString &tag3)
    const
  {
    NgramTable::const_iterator ugi = ngtable.find(tag1);
    if (ugi == ngtable.end()) return 0;
    BigramTable::const_iterator bgi = ugi->second.freqs.find(tag2);
    if (bgi == ugi->second.freqs.end()) return 0;
    TrigramTable::const_iterator tgi = bgi->second.freqs.find(tag3);
    return tgi == bgi->second.freqs.end() ? 0 : tgi->second;
  };

  //------ public methods: i/o

  /** Load n-grams from a TnT-style parameter file */
  bool load(char *filename);

  /** Load n-grams from a TnT-style parameter file (stream version) */
  bool load(FILE *file, char *filename = NULL);

  /** Save n-grams to a TnT-style paramater file */
  bool save(char *filename, bool compact=false);

  /** Save n-grams to a TnT-style paramater file (stream version) */
  bool save(FILE *file, char *filename = NULL, bool compact=false);
};

moot_END_NAMESPACE

#endif /* _moot_NGRAMS_H */
