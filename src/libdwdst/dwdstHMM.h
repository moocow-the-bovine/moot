/* -*- Mode: C++ -*- */
/*--------------------------------------------------------------------------
 * File: dwdstHMM.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger for DWDS project : Hidden Markov Model (Disambiguator): headers
 *--------------------------------------------------------------------------*/

#ifndef _DWDST_HMM_H
#define _DWDST_HMM_H

#include <hash_map>

#include "dwdstTypes.h"
#include "dwdstLexfreqs.h"
#include "dwdstNgrams.h"
#include "dwdstEnum.h"

/*--------------------------------------------------------------------------
 * dwdstHMM : HMM class
 *--------------------------------------------------------------------------*/

/**
 * \brief 1st-order Hidden Markov Model Tagger/Disambiguator class.
 */
class dwdstHMM {
public:
  //------ public typedefs
  /** Type for a single probability value */
  typedef float ProbT;

  /** Type for a tag-identifier */
  typedef dwdstEnumID TagID;

  /** Type for a token-identider */
  typedef dwdstEnumID TokID;

  /** Typedef for token-id lookup table */
  typedef dwdstEnum<dwdstTokString, hash<dwdstTokString>, equal_to<dwdstTokString> > TokIDTable;

  /** Typedef for tag-id lookup table */
  typedef dwdstEnum<dwdstTagString, hash<dwdstTagString>, equal_to<dwdstTagString> > TagIDTable;

  /**
   * Type for a single lexical probability entry (subtable), numeric form.
   * Keys are tag-ids, values are counts.
   * Also used for unigram probabilities.
   */
  typedef hash_map<TagID,ProbT> TagProbTable;

  /**
   * Type for (TokID->(TagID->p(TokID|TagID))) lookup table.
   */
  typedef hash_map<TokID,TagProbTable *> LexProbTable;

  /**
   * Type for bigram (TagID->(prevTagID->p(TagID|PrevTagID))) lookup table
   */
  typedef hash_map<TagID,TagProbTable *> BigramProbTable;

public:
  // ------ public data: ID-lookup tables
  TokIDTable        tokids;     /** Token-ID lookup table */
  TagIDTable        tagids;     /** Tag-ID lookup table */

  // ------ public data: probability-lookup tables
  LexProbTable      lexprobs;   /**< Lexical probability lookup table */
  TagProbTable      ngprobs1;   /**< N-gram probability lookup table: unigrams */
  BigramProbTable   ngprobs2;   /**< N-gram probability lookup table: bigrams */
  
private:
  //------ private data
  
public:
  //------ public methods: constructor/destructor

  /** Default constructor */
  dwdstHMM(void) {};

  /** Construct & compile in one swell foop. */
  dwdstHMM(const dwdstLexfreqs &lexfreqs,
	   const dwdstNgrams &ngrams,
	   const dwdstLexfreqs::LexfreqCount unknownLexThreshhold=1)
    {
	compile(lexfreqs, ngrams);
    };

  /** Destructor */
  ~dwdstHMM(void) { clear(); }

  //------ public methods: reset / compile
  /**
   * Reset/clear the object.
   */
  void clear(void);

  /**
   * Compile probabilites from raw frequency counts in 'lexfreqs' and 'ngrams'.
   * Returns false on failure.
   */
  bool compile(const dwdstLexfreqs &lexfreqs,
	       const dwdstNgrams &ngrams,
	       const dwdstLexfreqs::LexfreqCount unknownLexThreshhold=1);


  //------ public methods: mid-level: lexical probability lookup
  /**
   * Looks up and returns lexical probability: p(tokid|tagid).
   */
  inline const ProbT wordp(const TokID tokid, const TagID tagid)
  {
    LexProbTable::const_iterator wi = lexprobs.find(tokid);
    if (wi == lexprobs.end() || wi->second == NULL) return 0;
    TagProbTable::const_iterator ti = wi->second->find(tagid);
    if (ti == wi->second->end()) return 0;
    return ti->second;
  };

  /**
   * Looks up and returns lexical probability: p(token|tag), string-version.
   */
  inline const ProbT wordp(const dwdstTokString token, const dwdstTagString tag)
  {
    return wordp(tokids.name2id(token), tagids.name2id(tag));
  };


  //------ public methods: mid-level: unigram probability lookup
  /**
   * Looks up and returns unigram probability: p(tagid).
   */
  inline const ProbT tagp(const TagID tagid)
  {
    TagProbTable::const_iterator ti = ngprobs1.find(tagid);
    if (ti == ngprobs1.end()) return 0;
    return ti->second;
  };

  /**
   * Looks up and returns unigram probability: p(tag), string-version.
   */
  inline const ProbT tagp(const dwdstTagString tag)
  {
    return tagp(tagids.name2id(tag));
  };


  //------ public methods: mid-level: bigram probability lookup
  /**
   * Looks up and returns bigram probability: p(tagid|prevtagid).
   */
  inline const ProbT tagp(const TagID tagid, const TagID prevtagid)
  {
    BigramProbTable::const_iterator ti = ngprobs2.find(tagid);
    if (ti == ngprobs2.end() || ti->second == NULL) return 0;
    TagProbTable::const_iterator pti = ti->second->find(prevtagid);
    if (pti == ti->second->end()) return 0;
    return pti->second;
  };

  /**
   * Looks up and returns bigram probability: p(tag|prevtag), string-version.
   */
  inline const ProbT tagp(const dwdstTagString tag, const dwdstTagString prevtag)
  {
    return tagp(tagids.name2id(tag), tagids.name2id(prevtag));
  };

};


#endif /* _DWDST_HMM_H */
