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
 * File: mootClassfreqs.h
 * Author:  Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *    Class for storage & retrieval of lexical-class frequencies (nested map<>)
 *============================================================================*/

#ifndef _moot_CLASSFREQS_H
#define _moot_CLASSFREQS_H

#include <mootTypes.h>
#include <mootLexfreqs.h>
moot_BEGIN_NAMESPACE

/**
 * \brief Class for storage and retrieval of raw lexical-class frequencies.
 */
class mootClassfreqs {
public:
  //------ public typedefs

  /**
   * Type for class-frequency lookup subtables. map: tagid->c(tokid,tagid)
   */
  typedef mootLexfreqs::LexfreqSubtable ClassfreqSubtable;

  /**
   * Type for frequency lookup table entries.
   */
  typedef mootLexfreqs::LexfreqEntry ClassfreqEntry;

  /** Type for a lexical class ("ambiguity set") */
  typedef mootTagSet LexClass;

  /** Hash method: utility struct for hash_map<LexClass,...>. */
  struct LexClassHash {
  public:
    inline size_t operator()(const LexClass &x) const {
      size_t hv = 0;
      hash<mootTagString> hasher;
      for (LexClass::const_iterator xi = x.begin(); xi != x.end(); xi++) {
	hv = 5*hv + hasher(*xi);
      }
      return hv;
    };
  };
  /** Equality predicate: utility struct for hash_map<LexClass,...>. */
  struct LexClassEqual {
  public:
    inline size_t operator()(const LexClass &x, const LexClass &y) const {
      return x==y;
    };
  };

  /**
   * Type for the lexical-class frequency lookup table.
   */
  typedef hash_map<LexClass,
		   ClassfreqEntry,
		   LexClassHash,
		   LexClassEqual>
          ClassfreqTable;

  /** Lookup table: tag->Count(tag) */
  typedef hash_map<mootTagString,CountT> TagfreqTable;

public:
  //------ public data
  ClassfreqTable    lctable;    /**< lexeme->(tag->count) lookup table */
  TagfreqTable      tagtable;   /**< tag->count lookup table */
  CountT            totalcount; /**< Sum of all counts */

public:
  //------ public methods
  /** Default constructor */
  mootClassfreqs(size_t initial_bucket_count=0) : totalcount(0)
  {
    if (initial_bucket_count != 0) {
      lctable.resize(initial_bucket_count);
    }
  };

  /** Default destructor */
  ~mootClassfreqs() {
    clear();
  }

  //------ public methods: manipulation

  /** Clear internal table(s) */
  void clear(void);

  /** Add 'count' to the current count for (lclass,tag) */
  inline void add_count(const LexClass &lclass,
			const mootTagString &tag,
			const CountT count)
  {
    //-- adjust token-table
    ClassfreqTable::iterator lci = lctable.find(lclass);
    if (lci == lctable.end()) {
      //-- new token
      lci = lctable.insert(ClassfreqTable::value_type(lclass,ClassfreqEntry(count))).first;
      lci->second.freqs[tag] = count;
    } else {
      //-- known token
      lci->second.count += count;

      ClassfreqSubtable::iterator lsi = lci->second.freqs.find(tag);
      if (lsi == lci->second.freqs.end()) {
	//-- unknown (tok,tag) pair
	lci->second.freqs[tag] = count;
      } else {
	//-- known (tok,tag) pair: just add
	lsi->second += count;
      }
    }

    //-- adjust total tag-count
    TagfreqTable::iterator lctagi = tagtable.find(tag);
    if (lctagi != tagtable.end()) {
      lctagi->second += count;
    } else {
      tagtable[tag] = count;
    }

    //-- adjust total token-count
    totalcount += count;
  };

  //------ public methods: lookup
  const CountT taglookup(const mootTagString &tag) const
  {
    TagfreqTable::const_iterator tagi = tagtable.find(tag);
    return tagi == tagtable.end() ? 0 : tagi->second;
  };

  /**
   * Return the number of distinct \c (class,tag) pairs we've counted.
   */
  size_t n_pairs(void);

  /**
   * Return total number of 'impossible' \c (class,tag) pairs we've counted.
   * Defined as:
   * \f[ \sum_{\{(class,tag)\ |\ tag \not\in class\}} \mathrm{count}(class,tag) \f]
   */
  size_t n_impossible(void);

  //------ public methods: i/o

  /** Load data from a text-format parameter file */
  bool load(char *filename);

  /** Load data from a text-format parameter file (stream version) */
  bool load(FILE *file, char *filename = NULL);

  /** Save data to a text-format paramater file */
  bool save(char *filename);

  /** Save data to a text-format paramater file (stream version) */
  bool save(FILE *file, char *filename = NULL);
};


moot_END_NAMESPACE

#endif /* _moot_CLASSFREQS_H */
