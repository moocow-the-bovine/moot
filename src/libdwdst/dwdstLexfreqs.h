/* -*- Mode: C++ -*- */

/*============================================================================
 * File: dwdstLexfreqs.h
 * Author:  Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *    Class for storage & retrieval of lexical frequencies
 *============================================================================*/

#ifndef _DWDST_LEXFREQS_H
#define _DWDST_LEXFREQS_H

#include <dwdstTypes.h>

/**
 * Class for storage & retrieval of N-Gram counts
 */
class dwdstLexfreqs {
public:
  //------ public typedefs

  /** Type for a single lexeme+tag co-occurrence count */
  typedef float LexfreqCount;

  /**
   * Type for the lexical total-frequency lookup table.
   */
  typedef hash_map<dwdstTokString,LexfreqCount> LexTotalTable;


  /**
   * Type for a single lexical frequency entry (subtable), string form.
   * Keys are tag-strings, values are counts.
   */
  typedef hash_map<dwdstTagString,LexfreqCount> LexfreqSubtable;

  /** Actual Lexeme->(Tag->Count) lookup table typedef */
  typedef
    hash_map<dwdstTokString,LexfreqSubtable *>
    LexfreqStringTable;

public:
  //------ public data
  /** lexeme->(tag->count) lookup table */
  LexfreqStringTable lftable;
  LexTotalTable      lftotals;

public:
  //------ public methods
  /** Default constructor */
  dwdstLexfreqs() {};
  ~dwdstLexfreqs() { clear(); }

  //------ public methods: manipulation

  /** Clear internal table(s) */
  void clear(void);

  /** Add 'count' to the current count for (token,tag), returns new count */
  inline LexfreqCount add_count(const dwdstTokString &token,
				const dwdstTagString &tag,
				const LexfreqCount count)
  {
    LexfreqStringTable::iterator ti = lftable.find(token);
    if (ti != lftable.end()) {
      if (!ti->second) ti->second = new LexfreqSubtable;
    } else {
      ti = lftable.insert(pair<dwdstTokString, LexfreqSubtable *>(token, new LexfreqSubtable)).first;
    }
    LexfreqSubtable::iterator sti = ti->second->find(tag);
    if (sti != ti->second->end()) {
      sti->second += count;
    } else {
      sti = ti->second->insert(pair<dwdstTagString, LexfreqCount>(tag, count)).first;
    }
    //-- adjust total
    LexTotalTable::iterator toti = lftotals.find(token);
    if (toti == lftotals.end()) {
      lftotals[token] = count;
    } else {
      toti->second += count;
    }
    return sti->second;
  }; //-- add_count()


  //------ public methods: lookup

  /** Returns current count for (token,tag), returns 0 if unknown */
  inline const LexfreqCount lookup(const dwdstTokString &token, const dwdstTagString &tag)
  {
    LexfreqStringTable::const_iterator ti = lftable.find(token);
    if (ti == lftable.end() || ti->second == NULL) return 0;
    LexfreqSubtable::const_iterator sti = ti->second->find(tag);
    if (sti == ti->second->end()) return 0;
    return sti->second;
  }; //-- lookup

  /** Returns current total count for token, returns 0 if unknown */
  inline const LexfreqCount lookup(const dwdstTokString &token)
  {
    LexTotalTable::const_iterator toti = lftotals.find(token);
    return toti == lftotals.end() ? 0 : toti->second;
  }; //-- lookup


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



#endif /* _DWDST_LEXFREQS_H */
