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
  typedef dwdstEnum<dwdstTokString,
		    hash<dwdstTokString>,
		    equal_to<dwdstTokString> >
          TokIDTable;

  /** Typedef for tag-id lookup table */
  typedef dwdstEnum<dwdstTagString,
		    hash<dwdstTagString>,
		    equal_to<dwdstTagString> >
          TagIDTable;

  /** Type for an identifier-pair.  Instances are keys of probability lookup tables */
  typedef pair<dwdstEnumID,dwdstEnumID> IDPair;

  /** Hash-function struct for IDPair, needed by hash_map */
  struct IDPairHashFcn {
  public:
    inline size_t operator()(const IDPair &x) const
    {
      return
	//(x.first<<5)-x.first + x.second;
	5*x.first + x.second;
    };
  };

  /** Equality struct for IDPair, needed by hash_map */
  struct IDPairEqualFcn {
  public:
    inline size_t operator()(const IDPair &x, const IDPair &y) const
    {
      return x.first == y.first && x.second == y.second;
    };
  };

  /** Key type for lexical probability table: IDPair(tokid,tagid) */
  typedef IDPair LexProbKey;

  /** Key type for bigram probability table: IDPair(tagid,prevtagid) */
  typedef IDPair BigramProbKey;

  /**
   * Type for unigram probability table.
   */
  typedef hash_map<TagID,ProbT> TagProbTable;

  /**
   * Type for (pair(id,id)->probability) lookup table.
   */
  typedef hash_map<IDPair,ProbT,IDPairHashFcn,IDPairEqualFcn> IDPairProbTable;

  /**
   * Type for lexical frequency lookup table:
   * (IDPair(TokID,TagID)->p(TokID|TagID))
   */
  typedef IDPairProbTable LexProbTable;

  /**
   * Type for bigram probability lookup table:
   * (IDPair(TagID,PrevTagID)->p(TagID|PrevTagID)))
   */
  typedef IDPairProbTable BigramProbTable;

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
   * Looks up and returns lexical probability: p(tokid|tagid)
   * given IDPair(tokid,tagid).
   */
  inline const ProbT wordp(const IDPair tok_tag_idpair) const
  {
    LexProbTable::const_iterator ti = lexprobs.find(tok_tag_idpair);
    return ti == lexprobs.end() ? 0 : ti->second;
  };

  /**
   * Looks up and returns lexical probability: p(tokid|tagid)
   * given tokid, tagid.
   */
  inline const ProbT wordp(const TokID tokid, const TagID tagid) const
  {
    return wordp(IDPair(tokid,tagid));
  };

  /**
   * Looks up and returns lexical probability: p(token|tag)
   * given token, tag.
   */
  inline const ProbT wordp(const dwdstTokString token, const dwdstTagString tag) const
  {
    return wordp(IDPair(tokids.name2id(token), tagids.name2id(tag)));
  };


  //------ public methods: mid-level: unigram probability lookup
  /**
   * Looks up and returns unigram probability: p(tagid).
   */
  inline const ProbT tagp(const TagID tagid) const
  {
    TagProbTable::const_iterator ti = ngprobs1.find(tagid);
    return ti == ngprobs1.end() ? 0 : ti->second;
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
   * Looks up and returns bigram probability: p(tagid|prevtagid),
   * given IDPair(tagid,prevtagid).
   */
  inline const ProbT tagp(const IDPair tag_prevtag_idpair) const
  {
    BigramProbTable::const_iterator ti = ngprobs2.find(tag_prevtag_idpair);
    return ti == ngprobs2.end() ? 0 : ti->second;
  };

  /**
   * Looks up and returns bigram probability: p(tagid|prevtagid),
   * given tagid, prevtagid.
   */
  inline const ProbT tagp(const TagID tagid, const TagID prevtagid)
  {
    return tagp(IDPair(tagid,prevtagid));
  };

  /**
   * Looks up and returns bigram probability: p(tag|prevtag), string-version.
   */
  inline const ProbT tagp(const dwdstTagString tag, const dwdstTagString prevtag)
  {
    return tagp(IDPair(tagids.name2id(tag), tagids.name2id(prevtag)));
  };

  //------ public methods: low-level: debugging
  /** Debugging method: dump HMM contents to a text file. */
  void txtdump(FILE *file);

};


#endif /* _DWDST_HMM_H */
