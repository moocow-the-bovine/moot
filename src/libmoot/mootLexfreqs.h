/* -*- Mode: C++ -*- */

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
   * Type for the lexical total-frequency lookup table.
   */
  typedef hash_map<mootTokString,LexfreqCount> LexfreqTotalTable;

  /** Type for keys of the lexeme+tag frequency lookup table */
  typedef pair<mootTokString,mootTagString> LexfreqKey;

  /** Helper struct (argh) */
  struct LexfreqKeyHashFcn {
  public:
    inline size_t operator()(const LexfreqKey &x) const
    {
      //size_t hv = hash<const mootTokString&>(x.first);
      //return hv + (hv<<5)-hv + hash<const mootTagStr&>(x.second);
      return
	hash<mootTokString>()(x.first) * hash<mootTagString>()(x.second);
    };
  };
  /** Another helper struct */
  struct LexfreqKeyEqualFcn {
  public:
    inline size_t operator()(const LexfreqKey &x, const LexfreqKey &y) const
    {
      return x.first == y.first && x.second == y.second;
    };
  };


  /** Lookup table:  Lexeme->(Tag->Count) */
  typedef
    hash_map<LexfreqKey,
	     LexfreqCount,
	     LexfreqKeyHashFcn,
	     LexfreqKeyEqualFcn>
    LexfreqStringTable;

  /** Lookup table: Tag->Count */
  typedef
  hash_map<mootTagString,LexfreqCount> LexfreqTagTable;

public:
  //------ public data
  /** lexeme->(tag->count) lookup table */
  LexfreqStringTable lftable;
  LexfreqTotalTable  lftotals;
  LexfreqTagTable    lftags;
  LexfreqCount       n_tokens;

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

  /** Add 'count' to the current count for pair(token,tag), returns new count */
  inline LexfreqCount add_count(const LexfreqKey &key, const LexfreqCount count)
  {
    //-- adjust count
    LexfreqStringTable::iterator ti = lftable.find(key);
    if (ti == lftable.end()) {
      ti = lftable.insert(pair<LexfreqKey,LexfreqCount>(key,count)).first;
    } else {
      ti->second += count;
    }
    //-- adjust total
    LexfreqTotalTable::iterator toti = lftotals.find(key.first);
    if (toti == lftotals.end()) {
      lftotals[key.first] = count;
    } else {
      toti->second += count;
    }
    //-- adjust tag-count
    LexfreqTagTable::iterator tagi = lftags.find(key.second);
    if (tagi == lftags.end()) {
      lftags[key.second] = count;
    } else {
      tagi->second += count;
    }
    //-- adjust total token-count
    n_tokens += count;
    //-- return
    return ti->second;
  };

  /** Add 'count' to the current count for (token,tag), returns new count */
  inline LexfreqCount add_count(const mootTokString &token,
				const mootTagString &tag,
				const LexfreqCount count)
  {
    return add_count(LexfreqKey(token,tag), count);
  }; //-- add_count()


  //------ public methods: lookup

  /** Returns current count for pair(token,tag), returns 0 if unknown */
  inline const LexfreqCount lookup(const LexfreqKey &key) const
  {
    LexfreqStringTable::const_iterator ti = lftable.find(key);
    return ti == lftable.end() ? 0 : ti->second;
  }; //-- lookup(key)

  /** Returns current count for (token,tag), returns 0 if unknown */
  inline const LexfreqCount lookup(const mootTokString &token, const mootTagString &tag) const
  {
    return lookup(LexfreqKey(token,tag));
  }; //-- lookup(token,tag)

  /** Returns current total count for token, returns 0 if unknown */
  inline const LexfreqCount lookup(const mootTokString &token) const
  {
    LexfreqTotalTable::const_iterator toti = lftotals.find(token);
    return toti == lftotals.end() ? 0 : toti->second;
  }; //-- lookup(token)


  /** Returns current total count for tag, returns 0 if unknown */
  inline const LexfreqCount taglookup(const mootTagString &tag) const
  {
    LexfreqTagTable::const_iterator tagi = lftags.find(tag);
    return tagi == lftags.end() ? 0 : tagi->second;
  }; //-- taglookup(tag)

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
