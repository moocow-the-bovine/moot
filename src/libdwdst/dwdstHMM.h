/* -*- Mode: C++ -*- */
/*--------------------------------------------------------------------------
 * File: dwdstHMM.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger for DWDS project : Hidden Markov Model (Disambiguator): headers
 *--------------------------------------------------------------------------*/

#ifndef _DWDST_HMM_H
#define _DWDST_HMM_H

#include <float.h>
#include <hash_map>

#include "dwdstTypes.h"
#include "dwdstLexfreqs.h"
#include "dwdstNgrams.h"
#include "dwdstEnum.h"

/**
 * Constant representing a minimal probability.
 */
#define dwdstProbEpsilon FLT_EPSILON

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

  /** Key type for bigram probability table: IDPair(prevtagid,tagid) */
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
   * (IDPair(PrevTagID,TagID)->p(TagID|PrevTagID)))
   */
  typedef IDPairProbTable BigramProbTable;

  /** Type for an unweighted state-sequence (read: "tag-sequence") */
  typedef vector<TagID>   StateSeq;

  /**
   * Type for a weighted state-sequence:
   * used for entries in the Viterbi state-table.
   */
  struct WeightedStateSeq {
  public:
    ProbT    prob;
    StateSeq path;
  };

  /**
   * Type for the Viterbi state-table.
   * Keys: TagID
   * Values: WeightedStateSeq : best pair (prob(path),path)
   *         such that 'path' ends in TagID.
   */
  typedef vector<WeightedStateSeq> ViterbiStateTable;

public:
  //------ public data: ID-lookup tables
  TokIDTable        tokids;     /** Token-ID lookup table */
  TagIDTable        tagids;     /** Tag-ID lookup table */

  //------ public data: probability-lookup tables
  LexProbTable      lexprobs;   /**< Lexical probability lookup table */
  TagProbTable      ngprobs1;   /**< N-gram probability lookup table: unigrams */
  BigramProbTable   ngprobs2;   /**< N-gram probability lookup table: bigrams */

  //------ public data: various pragmatic constants
  TagID             start_tagid;  /**< Initial tag, used for bootstrapping */

  //------ public data: smoothing constants
  ProbT             nglambda1;    /**< Smoothing constant for unigrams */
  ProbT             nglambda2;    /**< Smoothing constant for bigrams */
  ProbT             wlambda1;     /**< Smoothing constant lexical probabilities */
  ProbT             wlambda2;     /**< Smoothing constant lexical probabilities */

  //------ low-level data: for Viterbi algorithm
  ViterbiStateTable vtable;     /**< Low-level state table for Viterbi algorithm */
  ViterbiStateTable pvtable;    /**< Low-level state table for Viterbi algorithm (previous iter) */

private:
  //------ private data
  ProbT             vbestp;     /**< Best probability for viterbi_step() */
  //TagID             vbestt;     /**< Best tag for viterbi_best() */
  TagID             vtagid;     /**< Current tag-id under consideration for viterbi_step() */
  ProbT             vtagp;      /**< Probability for current tag-id for viterbi_step() */

  ViterbiStateTable::iterator  vsi;      /**< Viterbi state-table iterator */
  ViterbiStateTable::iterator pvsi;      /**< Viterbi state-table iterator */
  ViterbiStateTable::iterator pvsi_best; /**< Viterbi state-table iterator */
  
public:
  //------ public methods: constructor/destructor

  /** Default constructor */
  dwdstHMM(void)
    : start_tagid(0),
      nglambda1(dwdstProbEpsilon),
      nglambda2(1.0 - dwdstProbEpsilon),
      wlambda1(1.0 - dwdstProbEpsilon),
      wlambda2(dwdstProbEpsilon)
  {};

  /** Construct & compile in one swell foop. */
  dwdstHMM(const dwdstLexfreqs &lexfreqs,
	   const dwdstNgrams &ngrams,
	   const dwdstTagString &start_tag_str=dwdstTagString(),
	   const dwdstLexfreqs::LexfreqCount unknownLexThreshhold=1)
    : start_tagid(0),
      nglambda1(dwdstProbEpsilon),
      nglambda2(1.0 - dwdstProbEpsilon),
      wlambda1(1.0 - dwdstProbEpsilon),
      wlambda2(dwdstProbEpsilon)
    {
	compile(lexfreqs, ngrams, start_tag_str, unknownLexThreshhold);
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
	       const dwdstTagString &start_tag_str=dwdstTagString(),
	       const dwdstLexfreqs::LexfreqCount unknownLexThreshhold=1);

  //------ public methods: high-level: Viterbi
  /** Clear Viterbi state table(s) */
  inline void viterbi_clear(void)
  {
    for (vsi = vtable.begin(); vsi != vtable.end(); vsi++) {
      vsi->path.clear();
      vsi->path.push_back(start_tagid);
      vsi->prob = 1.0;
    }
    for (vsi = pvtable.begin(); vsi != pvtable.end(); vsi++) {
      vsi->path.clear();
      vsi->path.push_back(start_tagid);
      vsi->prob = 1.0;
    }
  };


  /**
   * Step a single Viterbi iteration, considering all known tags
   * as possible analyses for tokid.
   */
  inline void viterbi_step(const TokID tokid)
  {
    vtable.swap(pvtable);

    //-- for each destination tag
    for (vtagid = 0             , vsi  = vtable.begin();
	 vtagid < tagids.size() , vsi != vtable.end();
	 vtagid ++              , vsi ++)
      {
	vbestp = -1.0;

	//-- find best previous tag
	for (pvsi = pvsi_best = pvtable.begin(); pvsi != pvtable.end(); pvsi++) {
	  vtagp = ( (1.0+pvsi->prob)
		    * arcp(pvsi->path.empty() ? start_tagid : pvsi->path.back(),
			   tokid,
			   vtagid) );
	  if (vtagid != 0 && vtagp > vbestp) {
	    vbestp = vtagp;
	    pvsi_best = pvsi;
	  }
	}

	//-- update state table for current destination tag
	vsi->prob = vbestp;
	vsi->path = pvsi_best->path;
	vsi->path.push_back(vtagid);
      }
    //-- et voila
  };

  /**
   * Step a single Viterbi iteration, considering all known tags
   * as possible analyses for token.
   */
  inline void viterbi_step(const dwdstTokString &token) {
    return viterbi_step(tokids.name2id(token));
  };


  /**
   * Step a single Viterbi iteration, considering only the tags
   * in 'tagids'.
   */
  inline void viterbi_step(const TokID tokid, const set<TagID> &tag_ids)
  {
    vtable.swap(pvtable);

    //-- for each destination tag
    for (set<TagID>::const_iterator tagsi = tag_ids.begin(); tagsi != tag_ids.end(); tagsi++)
      {
	if (*tagsi >= vtable.size()) continue; //-- ignore bad tags
	vtagid = *tagsi;
	vbestp = -1.0;

	//-- find best previous tag
	for (pvsi = pvsi_best = pvtable.begin(); pvsi != pvtable.end(); pvsi++) {
	  vtagp = ( (1.0+pvsi->prob)
		    * arcp(pvsi->path.empty() ? start_tagid : pvsi->path.back(),
			   tokid,
			   vtagid) );
	  if (vtagid != 0 && vtagp > vbestp) {
	    vbestp = vtagp;
	    pvsi_best = pvsi;
	  }
	}

	//-- update state table for current destination tag
	WeightedStateSeq &wsseq = vtable[vtagid];
	wsseq.prob = vbestp;
	wsseq.path = pvsi_best->path;
	wsseq.path.push_back(vtagid);
      }
    //-- et voila
  };

  /**
   * Step a single Viterbi iteration, considering only the tag 'tagid'
   */
  inline void viterbi_step(const TokID tokid, const TagID tagid)
  {
    vtable.swap(pvtable);

    //-- for the destination tag
    vtagid = tagid;
    vbestp = -1.0;
    if (tagid >= vtable.size()) vtagid = 0; //-- ignore bad tags

    //-- find best previous tag
    for (pvsi = pvsi_best = pvtable.begin(); pvsi != pvtable.end(); pvsi++) {
      vtagp = ( (1.0+pvsi->prob)
		* arcp(pvsi->path.empty() ? start_tagid : pvsi->path.back(),
		       tokid,
		       vtagid) );
      if (vtagid != 0 && vtagp > vbestp) {
	vbestp = vtagp;
	pvsi_best = pvsi;
      }
    }

    //-- update state table for current destination tag
    WeightedStateSeq &wsseq = vtable[vtagid];
    wsseq.prob = vbestp;
    wsseq.path = pvsi_best->path;
    wsseq.path.push_back(vtagid);
  };

  /**
   * Step a single Viterbi iteration, considering only the tag 'tag'
   */
  inline void viterbi_step(const dwdstTokString &token, const dwdstTagString &tag)
  {
    return viterbi_step(tokids.name2id(token), tagids.name2id(tag));
  };

  /**
   * Step a single Viterbi iteration, considering only the tags
   * in 'tags'.
   */
  inline void viterbi_step(const dwdstTokString &token, const set<dwdstTagString> &tags)
  {
    set<TagID> tags_ids;
    for (set<dwdstTagString>::const_iterator tagi = tags.begin();
	 tagi != tags.end();
	 tagi++)
      {
	tags_ids.insert(tagids.name2id(*tagi));
      }
    return viterbi_step(tokids.name2id(token), tags_ids);
  };


  /**
   * Get best current path from Viterbi state tables, considering all tags.
   */
  inline WeightedStateSeq &viterbi_best(void)
  {
    //-- find best current tag
    vbestp = -1.0;
    for (vsi = vtable.begin(); vsi != vtable.end(); vsi++) {
      if (vsi->prob > vbestp) {
	vbestp = vtagp;
	pvsi_best = vsi;
      }
    }
    return *pvsi_best;
  };

  /**
   * Get best current path from Viterbi state tables, considering only the tags in 'tag_ids'
   */
  inline WeightedStateSeq &viterbi_best(const set<TagID> &tag_ids)
  {
    //-- find best current tag
    vbestp = -1.0;
    vtagid = 0;
    for (set<TagID>::const_iterator tagsi = tag_ids.begin();
	 tagsi != tag_ids.end();
	 tagsi++)
      {
	if (*tagsi >= vtable.size()) {
	  //-- ignore bad tags
	  continue;
	}
	else if (vsi->prob > vbestp) {
	  vbestp = vtagp;
	  vtagid = *tagsi;
	}
      }
    return vtable[vtagid];
  };

  /**
   * Get best current path from Viterbi state tables, considering only the tags in 'tags'.
   */
  inline WeightedStateSeq &viterbi_best(const set<dwdstTagString> &tags)
  {
    set<TagID> tags_ids;
    for (set<dwdstTagString>::const_iterator tagi = tags.begin();
	 tagi != tags.end();
	 tagi++)
      {
	tags_ids.insert(tagids.name2id(*tagi));
      }
    return viterbi_best(tags_ids);
  };

  /**
   * Get best current path from Viterbi state tables resulting in tag 'tagid'.
   */
  inline const WeightedStateSeq &viterbi_best(const TagID tagid) const
  {
    return tagid >= tagids.size() ? vtable[0] : vtable[tagid];
  };

  /**
   * Get best current path from Viterbi state tables resulting in tag 'tag'.
   */
  inline const WeightedStateSeq &viterbi_best(const dwdstTagString &tag) const
  {
    return viterbi_best(tagids.name2id(tag));
  };


  //------ public methods: mid-level: arc probability lookup
  /**
   * Looks up and returns arc probability:
   * probability of transition from tag 'prevtagid' to tag 'tagid'
   * on token 'tokid'.
   */
  inline const ProbT arcp(const TagID prevtagid, const TokID tokid, const TagID tagid) const
  {
    //--DEBUG
    fprintf(stderr, "arcp(ptag=%u, tok=%u, tag=%u) : ", prevtagid, tokid, tagid);
    fprintf(stderr, "wordp(tok,tag)=%g ; tagp(tag)=%g ; tagp(ptag,tag)=%g\n",
	    wordp(tokid,tagid), tagp(tagid), tagp(prevtagid, tagid));
    //--/DEBUG

    return
      ( (wlambda1 * wordp(tokid, tagid)) + wlambda2 )
      * 
      ( (nglambda1 * tagp(tagid))
	+ //(prevtagid == start_tagid ? 0.0 : (nglambda2 * tagp(tagid, prevtagid))) )
	(nglambda2 * tagp(prevtagid, tagid)) );
  };

  /**
   * Looks up and returns arc probability:
   * probability of transition from tag 'prevtag' to tag 'tag'
   * on token 'tok'.
   */
  inline const ProbT arcp(const dwdstTagString &prevtag,
			  const dwdstTokString &token,
			  const dwdstTagString &tag) const
  {
    return arcp(tagids.name2id(prevtag), tokids.name2id(token), tagids.name2id(tag));
  };

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
    return wordp(tokids.name2id(token), tagids.name2id(tag));
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
  inline const ProbT tagp(const dwdstTagString tag) const
  {
    return tagp(tagids.name2id(tag));
  };


  //------ public methods: mid-level: bigram probability lookup
  /**
   * Looks up and returns bigram probability: p(tagid|prevtagid),
   * given IDPair(prevtagid,tagid).
   */
  inline const ProbT tagp(const IDPair prevtag_tag_idpair) const
  {
    BigramProbTable::const_iterator ti = ngprobs2.find(prevtag_tag_idpair);
    return ti == ngprobs2.end() ? 0 : ti->second;
  };

  /**
   * Looks up and returns bigram probability: p(tagid|prevtagid),
   * given tagid, prevtagid.
   */
  inline const ProbT tagp(const TagID prevtagid, const TagID tagid) const
  {
    return tagp(IDPair(prevtagid,tagid));
  };

  /**
   * Looks up and returns bigram probability: p(tag|prevtag), string-version.
   */
  inline const ProbT tagp(const dwdstTagString prevtag, const dwdstTagString tag) const
  {
    return tagp(tagids.name2id(prevtag), tagids.name2id(tag));
  };

  //------ public methods: low-level: debugging
  /** Debugging method: dump basic HMM contents to a text file. */
  void txtdump(FILE *file);

  /** Debugging method: dump main Viterbi table to a text file */
  void viterbi_txtdump(FILE *file);

};


#endif /* _DWDST_HMM_H */
