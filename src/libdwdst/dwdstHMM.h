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
#include <string.h>
#include <hash_map>

#include "dwdstTypes.h"
#include "dwdstLexfreqs.h"
#include "dwdstNgrams.h"
#include "dwdstEnum.h"

/**
 * Define this for verbose debugging information to stderr.
 */
//#define DWDST_DEBUG_ARCP 1
#undef DWDST_DEBUG_ARCP

/**
 * Define this to include the 'add one' hack to avoid float underflows
 * WARNING: this can cause problems with sparse data!
 */
//#define DWDST_ADD_ONE_HACK 1
#undef DWDST_ADD_ONE_HACK

/**
 * Constant representing a minimal probability.
 */
//#define dwdstProbEpsilon FLT_EPSILON
#define dwdstProbEpsilon DBL_EPSILON
//#define dwdstProbEpsilon 1e-32

/**
 * Define this to include real lexical entries for tokens with counts
 * below UnknownLexThreshhold.
 */
#define DWDST_LEX_UNKNOWN_TOKENS

/**
 * Define this to use trigrams (not yet implemented)
*/
#undef DWDST_USE_TRIGRAMS

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
  typedef double ProbT;
  //typedef float ProbT;

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

  /** Type for lexical probability lookup subtable: tagid->prob */
  typedef map<TagID,ProbT> LexProbSubTable;

  /** Type for lexical probability lookup table: tokid->(tagid->p(tokid|tagid)) */
  typedef vector<LexProbSubTable> LexProbTable;

  /**
   * Type for unigram probability table: c-style array:
   * probabilities indexed by numeric tag-id.
   */
  //typedef hash_map<TagID,ProbT> TagProbTable;
  //typedef vector<ProbT> TagProbTable;
  typedef ProbT *TagProbTable;

  /**
   * Type for bigram probability lookup table:
   * c-style 2d array: probabilites p(tag|ptag)
   * indexed by ((ntags*ptag)+tag).
   */
  //typedef vector<TagProbTable> BigramProbTable;
  typedef ProbT *BigramProbTable;

  /** Key type for a trigram */
  class Trigram {
  public:

    /** Utility struct for hash_map */
    struct HashFcn {
    public:
      inline size_t operator()(const Trigram &x) const
      {
	return
	  (0xdeece66d * ((0xdeece66d * x.tag1) + x.tag2)) + x.tag3;
      };
    };

    /** Utility struct for hash_map */
    struct EqualFcn {
    public:
      inline size_t operator()(const Trigram &x, const Trigram &y) const
      {
	return 
	  x.tag1==y.tag1 && x.tag2==y.tag2 && x.tag3==y.tag3;
	//x==y;
      };
    };

  public:
    TagID tag1;
    TagID tag2;
    TagID tag3;

  public:
    /** Constructor */
    Trigram(TagID t1=0, TagID t2=0, TagID t3=0) : tag1(t1), tag2(t2), tag3(t3) {};

    /** Destructor */
    ~Trigram(void) {};
  };

  /** Type for a trigram probability lookup table */
  typedef
    hash_map<Trigram,ProbT,Trigram::HashFcn,Trigram::EqualFcn>
    TrigramProbTable;


  /** Type for a Viterbi state-table entry (linked-list columns) */
  struct ViterbiNode {
    TagID tagid;                   /**< Tag-ID for this node */
    ProbT prob;                    /**< Probability of best path to this node */
    struct ViterbiNode *pth_prev;  /**< Previous node in best path */
    struct ViterbiNode *row_next;  /**< Next node in current column */
  }; //-- class Trigram

  /**
   * Type for a Viterbi state-table column.
   * A Viterbi state-table is completely represented by its (current)
   * final column.
   */
  struct ViterbiColumn {
    //TokID          tokid;    /**< Token-ID for this column */
    ViterbiNode   *nodes;    /**< Column nodes */
    ViterbiColumn *col_prev; /**< Previous column */
  };

  /**
   * Type for a Viterbi path.  It's faster to use
   * the (ViterbiNode*)s directly, if you can deal with reverse order...
   */
  typedef deque<ViterbiNode*> ViterbiPath;

public:
  //------ public data: various pragmatic constants
  TagID             start_tagid;  /**< Initial/final tag, used for bootstrapping */

  //------ public data: smoothing constants
  ProbT             nglambda1;    /**< Smoothing constant for unigrams */
  ProbT             nglambda2;    /**< Smoothing constant for bigrams */
#ifdef DWDST_USE_TRIGRAMS
  ProbT             nglambda3;    /**< Smoothing constant for trigrams */
#endif
  ProbT             wlambda1;     /**< Smoothing constant lexical probabilities */
  ProbT             wlambda2;     /**< Smoothing constant lexical probabilities */

  //------ public data: ID-lookup tables
  TokIDTable        tokids;     /** Token-ID lookup table */
  TagIDTable        tagids;     /** Tag-ID lookup table */

  //------ public data: probability-lookup tables
  size_t            n_tags;     /**< Number of known tags: used to compute lookup indices */
  size_t            n_toks;     /**< Number of known tokens: used for sanity checks */
  LexProbTable      lexprobs;   /**< Lexical probability lookup table */
  TagProbTable      ngprobs1;   /**< N-gram probability lookup table: unigrams */
  BigramProbTable   ngprobs2;   /**< N-gram probability lookup table: bigrams */
#ifdef DWDST_USE_TRIGRAMS
  TrigramProbTable  ngprobs3;   /**< N-gram probability lookup table: trigrams */
#endif

  //------ low-level data: for Viterbi algorithm
  ViterbiColumn     *vtable;    /**< Low-level state table for Viterbi algorithm */

  //------ statistics
  size_t             nsents;    /**< Number of sentenced processed */
  size_t             ntokens;   /**< Number of tokens processed */

private:
  //------ private data: recycling bins
  ViterbiNode   *trash_nodes;   /**< Recycling bin for Viterbi state-table nodes */
  ViterbiColumn *trash_columns; /**< Recycling bin for Viterbi state-table columns */

  //------ private data: for Viterbi
  TagID             vtagid;     /**< Current tag-id under consideration for viterbi_step() */
  ProbT             vbestpr;    /**< Best probability for viterbi_step() */
  ProbT             vtagpr;     /**< Probability for current tag-id for viterbi_step() */
  ProbT             vwordpr;    /**< Save word-probability */
  ViterbiNode      *vbestpn;    /**< Best previous node for viterbi_step() */

  ViterbiPath       vbestpath;  /**< Best path constant */

  //-- private data: for tagging
  vector<dwdstTokString> tokens;  /**< Temporarily stores input tokens for tag_stream() */
  set<TagID>             curtags; /**< Temporarily stores input token-tags for tag_stream() */

public:
  //------ public methods: constructor/destructor

  /** Default constructor */
  dwdstHMM(void)
    : start_tagid(0),
      nglambda1(dwdstProbEpsilon),
      nglambda2(1.0 - dwdstProbEpsilon),
      wlambda1(1.0 - dwdstProbEpsilon),
      wlambda2(dwdstProbEpsilon),
      n_tags(0),
      n_toks(0),
      ngprobs1(NULL),
      ngprobs2(NULL),
      vtable(NULL),
      nsents(0),
      ntokens(0),
      trash_nodes(NULL),
      trash_columns(NULL),
      vbestpn(NULL)
  {};

  /** Destructor */
  ~dwdstHMM(void)
  {
    clear();
  };


  //------------------------------------------------------------
  // public methods: access

  /** Set the unknown token name */
  inline void unknown_token_name(const dwdstTokString &name) { tokids.unknown_name(name); }

  /** Set the unknown tag */
  inline void unknown_tag_name(const dwdstTokString &name) { tagids.unknown_name(name); }

  //------------------------------------------------------------
  // public methods: reset / clear

  /**
   * Reset/clear the object, freeing all dynamic data structures.
   * If 'wipe_everything' is false, ID-tables and constants will
   * spared.
   */
  void clear(bool wipe_everything=true);


  //------------------------------------------------------------
  // public methods: compilation

  /**
   * Compile probabilites from raw frequency counts in 'lexfreqs' and 'ngrams'.
   * Returns false on failure.
   */
  bool compile(const dwdstLexfreqs &lexfreqs,
	       const dwdstNgrams &ngrams,
	       const dwdstTagString &start_tag_str=dwdstTagString(),
	       const dwdstLexfreqs::LexfreqCount unknownLexThreshhold=1);

  /** Assign IDs for tokens and tags from lexfreqs: called by compile() */
  void assign_ids_lf(const dwdstLexfreqs &lexfreqs,
		     const dwdstLexfreqs::LexfreqCount unknownLexThreshhold=1);

 /** Assign IDs for tokens and tags from ngrams: called by compile() */
  void assign_ids_ng(const dwdstNgrams   &ngrams);

  /** Estimate n-gram smoothing constants: NOT called by compile() */
  bool estimate_lambdas(const dwdstNgrams &ngrams);


  //------------------------------------------------------------
  // Viterbi utilities: nodes

  /** Returns a pointer to an unused ViterbiNode, possibly allocating a new one. */
  inline ViterbiNode *viterbi_get_node(void) {
    ViterbiNode *nod;
    if (trash_nodes != NULL) {
      nod = trash_nodes;
      trash_nodes = nod->row_next;
    } else {
      nod = new ViterbiNode();
    }
    return nod;
  };

  //------------------------------------------------------------
  // Viterbi utilities: columns

  /** Returns a pointer to an unused ViterbiColumn, possibly allocating a new one. */
  inline ViterbiColumn *viterbi_get_column(void) {
    ViterbiColumn *col;
    if (trash_columns != NULL) {
      col = trash_columns;
      trash_columns = col->col_prev;
    } else {
      col = new ViterbiColumn();
    }
    return col;
  };

  //------------------------------------------------------------
  // public methods: high-level: Viterbi: clear state tables

  /** Clear Viterbi state table(s) */
  inline void viterbi_clear(void)
  {
    //-- move to trash
    ViterbiColumn *col, *col_next;
    ViterbiNode   *nod, *nod_next;
    for (col = vtable; col != NULL; col = col_next) {
      col_next      = col->col_prev;
      col->col_prev = trash_columns;
      trash_columns = col;
      for (nod = col->nodes; nod != NULL; nod = nod_next) {
	nod_next      = nod->row_next;
	nod->row_next = trash_nodes;
	trash_nodes   = nod;
      }
    }

    //-- add BOS entry
    vtable = viterbi_get_column();
    //vtable->tokid = start_tokid;
    vtable->col_prev = NULL;
    nod = vtable->nodes = viterbi_get_node();
    nod->tagid = start_tagid;
    nod->prob  = 1.0;
    nod->row_next = NULL;
    nod->pth_prev = NULL;
  };




  //------------------------------------------------------------
  // public methods: high-level: Viterbi: single iteration: (TokID)

  /**
   * Step a single Viterbi iteration, considering all known tags
   * for 'tokid' as possible analyses.
   */
  inline void viterbi_step(TokID tokid)
  {
    //-- Get next column
    ViterbiColumn *col = viterbi_get_column();
    ViterbiNode   *nod;
    col->col_prev = vtable;
    col->nodes = NULL;

    //-- sanity check
    if (tokid >= n_toks) tokid = 0;

    //-- Get map of possible destination tags
    const LexProbSubTable &lps = lexprobs[tokid];

    //-- for each possible destination tag 'vtagid'
    for (LexProbSubTable::const_iterator lpsi = lps.begin(); lpsi != lps.end(); lpsi++)
      {
	vtagid  = lpsi->first;

	//-- ignore "unknown" tag
	if (vtagid == 0) continue;

	//-- get lexical probability: p(tok|tag) 
	vwordpr = ( (wlambda1 * lpsi->second) + wlambda2 );

	//-- find best previous tag by n-gram probabilites: store information in vbestpr,vbestpn
	viterbi_find_best_prevnode(vtagid, tagp(vtagid));

	//-- skip zero-probabilities
	//if (vbestpr <= 0) continue;

	//-- update state table column for current destination tag
	nod           = viterbi_get_node();
	nod->tagid    = vtagid;
	nod->prob     = vbestpr * vwordpr;
	nod->pth_prev = vbestpn;
	nod->row_next = col->nodes;
	col->nodes    = nod;
      }

    //-- add new column to state table
    vtable = col;
  };



  //------------------------------------------------------------
  // public methods: high-level: Viterbi: single iteration: (TokString)

  /**
   * Step a single Viterbi iteration, string version.
   * Really just a wrapper for viterbi_step(TokID tokid).
   */
  inline void viterbi_step(const dwdstTokString &token) {
    return viterbi_step(tokids.name2id(token));
  };


  //------------------------------------------------------------
  // public methods: high-level: Viterbi: single iteration: (TokID,set<TagID>)

  /**
   * Step a single Viterbi iteration, considering only the tags
   * in 'tagids'.
   */
  inline void viterbi_step(TokID tokid, const set<TagID> &tag_ids)
  {
    //-- Get next column
    ViterbiColumn *col = viterbi_get_column();
    ViterbiNode   *nod;
    col->col_prev = vtable;
    col->nodes = NULL;

    //-- sanity check
    if (tokid >= n_toks) tokid = 0;

    //-- Get map of possible destination tags
    const LexProbSubTable &lps = lexprobs[tokid];
    LexProbSubTable::const_iterator lpsi;

    //-- for each possible destination tag 'vtagid'
    for (set<TagID>::const_iterator tii = tag_ids.begin(); tii != tag_ids.end(); tii++)
      {
	vtagid  = *tii;

	//-- ignore "unknown" tag(s)
	if (vtagid >= n_tags || vtagid == 0) continue;

	//-- get lexical probability: p(tok|tag) 
	lpsi = lps.find(vtagid);
	if (lpsi != lps.end()) {
	  vwordpr = ( (wlambda1 * lpsi->second) + wlambda2 );
	} else {
	  vwordpr = wlambda2;
	}

	//-- find best previous tag by n-gram probabilites: store information in vbestpr,vbestpn
	viterbi_find_best_prevnode(vtagid, tagp(vtagid));

	//-- skip zero-probabilities
	//if (vbestpr <= 0) continue;

	//-- update state table column for current destination tag
	nod           = viterbi_get_node();
	nod->tagid    = vtagid;
	nod->prob     = vbestpr * vwordpr;
	nod->pth_prev = vbestpn;
	nod->row_next = col->nodes;
	col->nodes    = nod;
      }

    //-- add new column to state table
    vtable = col;
  };


  //------------------------------------------------------------
  // public methods: high-level: Viterbi: single iteration: (TokString,set<TagString>)

  /**
   * Step a single Viterbi iteration, considering only the tags in 'tags'.
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


  //------------------------------------------------------------
  // public methods: high-level: Viterbi: single iteration: (TokID,TagID)

  /**
   * Step a single Viterbi iteration, considering only the tag 'tagid'
   */
  inline void viterbi_step(TokID tokid, TagID tagid)
  {
    //-- Get next column
    ViterbiColumn *col = viterbi_get_column();
    ViterbiNode   *nod;
    col->col_prev = vtable;
    col->nodes = NULL;

    //-- sanity check
    if (tokid >= n_toks) tokid = 0;

    //-- for the destination tag 'vtagid'
    vtagid = tagid >= n_tags ? 0 : tagid;

    //-- get lexical probability: p(tok|tag) 
    vwordpr = ( (wlambda1 * wordp(tokid,tagid)) + wlambda2 );

    //-- find best previous tag by n-gram probabilites: store information in vbestpr,vbestpn
    viterbi_find_best_prevnode(vtagid, tagp(vtagid));

    //-- update state table column for current destination tag
    nod           = viterbi_get_node();
    nod->tagid    = vtagid;
    nod->prob     = vbestpr * vwordpr;
    nod->pth_prev = vbestpn;
    nod->row_next = col->nodes;
    col->nodes    = nod;

    //-- add new column to state table
    vtable = col;
  };


  //------------------------------------------------------------
  // public methods: high-level: Viterbi: single iteration: (TokString,TagString)

  /**
   * Step a single Viterbi iteration, considering only the tag 'tag': string version.
   */
  inline void viterbi_step(const dwdstTokString &token, const dwdstTagString &tag)
  {
    return viterbi_step(tokids.name2id(token), tagids.name2id(tag));
  };


  //------------------------------------------------------------
  // public methods: high-level: Viterbi: finish

  /**
   * Run final Viterbi iteration, using 'final_tagid' as the final tag
   */
  inline void viterbi_finish(const TagID final_tagid)
  {
    viterbi_step(0, final_tagid);
  };

  /**
   * Run final Viterbi iteration, using instance datum 'start_tagid' as the final tag.
   */
  inline void viterbi_finish(void)
  {
    viterbi_step(0, start_tagid);
  };



  //------------------------------------------------------------
  // public methods: mid-level: Viterbi: best node

  /**
   * Get best current node from Viterbi state tables, considering all
   * possible current tags.  The best full path to this node can be
   * reconstructed (in reverse order) by traversing the 'pth_prev'
   * pointers until (pth_prev==NULL).
   */
  inline ViterbiNode *viterbi_best_node(void)
  {
    ViterbiNode *pnod;
    vbestpr = -1.0;
    vbestpn = NULL;
    for (pnod = vtable->nodes; pnod != NULL; pnod = pnod->row_next) {
      if (pnod->prob > vbestpr) {
	vbestpr = pnod->prob;
	vbestpn = pnod;
      }
    }
    return vbestpn;
  };

  /**
   * Get best current path from Viterbi state tables resulting in tag 'tagid'.
   * The best full path to this node can be
   * reconstructed (in reverse order) by traversing the 'pth_prev'
   * pointers until (pth_prev==NULL).
   */
  inline ViterbiNode *viterbi_best_node(TagID tagid)
  {
    ViterbiNode *pnod;
    vbestpr = -1.0;
    for (pnod = vtable->nodes; pnod != NULL; pnod = pnod->row_next) {
      if (pnod->tagid == tagid) return pnod;
    }
    return NULL;
  };

  //------------------------------------------------------------
  // public methods: mid-level: node-to-path conversion

  /**
   * Useful utility: build a path (in input order) from a ViterbiNode
   */
  inline const ViterbiPath &viterbi_node_path(ViterbiNode *node)
  {
    vbestpath.clear();
    ViterbiNode *nod;
    for (nod = node; nod != NULL; nod = nod->pth_prev) {
      vbestpath.push_front(nod);
    }
    return vbestpath;
  };


  //------------------------------------------------------------
  // public methods: high-level: best path

  /** Get current best path (in input order), considering all current tags */
  inline const ViterbiPath &viterbi_best_path(void)
  {
    return viterbi_node_path(viterbi_best_node());
  };

  /** Get current best path (in input order), considering only tag 'tagid' */
  inline const ViterbiPath &viterbi_best_path(TagID tagid)
  {
    return viterbi_node_path(viterbi_best_node(tagid));
  };

  /** Get current best path (in input order), considering only tag 'tag' */
  inline const ViterbiPath &viterbi_best_path(const dwdstTagString &tagstr)
  {
    return viterbi_best_path(tagids.name2id(tagstr));
  };


  //------------------------------------------------------------
  // public methods: low-level: Viterbi
  /**
   * Find the best previous node from top column of 'vtable' for destination tag 'curtagid',
   * stores a pointer to the best previous node in 'vbestpn', and the
   * (adjusted) n-gram transition probability in 'vbestpr'.
   *
   * NOTE: lexical probabilites are ignored for this computation, since they're
   * constant for the current (token,tag) pair under consideration.
   */
  inline void viterbi_find_best_prevnode(TagID curtagid, ProbT curtagp=-1.0)
  {
    ViterbiNode *pnod;
    if (curtagp <= 0) curtagp = tagp(curtagid);

    vbestpr = -1.0;
    vbestpn = NULL;
    for (pnod = vtable->nodes; pnod != NULL; pnod = pnod->row_next) {
      vtagpr =
#ifdef DWDST_ADD_ONE_HACK
	(pnod->prob + 1.0)  //-- add 1 to avoid float underflow (not here!)
#else
	pnod->prob
#endif // DWDST_ADD_ONE_HACK
	*
	( (nglambda1 * curtagp)
	  +
	  (nglambda2 * tagp(pnod->tagid, curtagid)) );

      if (vtagpr > vbestpr) {
	vbestpr = vtagpr;
	vbestpn = pnod;
      }
    }
  };


  //------------------------------------------------------------
  // public methods: low-level: lexical probability lookup

  /**
   * Looks up and returns lexical probability: p(tokid|tagid)
   * given tokid, tagid.
   */
  inline const ProbT wordp(const TokID tokid, const TagID tagid) const
  {
    if (tokid >= lexprobs.size()) return 0;
    const LexProbSubTable &lps = lexprobs[tokid];
    LexProbSubTable::const_iterator lpsi = lps.find(tagid);
    return lpsi != lps.end() ? lpsi->second : 0;
  };

  /**
   * Looks up and returns lexical probability: p(token|tag)
   * given token, tag.
   */
  inline const ProbT wordp(const dwdstTokString token, const dwdstTagString tag) const
  {
    return wordp(tokids.name2id(token), tagids.name2id(tag));
  };


  //------------------------------------------------------------
  // public methods: low-level: unigram probability lookup

  /**
   * Looks up and returns unigram probability: p(tagid).
   */
  inline const ProbT tagp(const TagID tagid) const
  {
    return
      ngprobs1 && tagid < n_tags
      ? ngprobs1[tagid]
      : 0;
  };

  /**
   * Looks up and returns unigram probability: p(tag), string-version.
   */
  inline const ProbT tagp(const dwdstTagString &tag) const
  {
    return tagp(tagids.name2id(tag));
  };

  //------------------------------------------------------------
  // public methods: low-level: bigram probability lookup
  /**
   * Looks up and returns bigram probability: p(tagid|prevtagid),
   * given tagid, prevtagid.
   */
  inline const ProbT tagp(const TagID prevtagid, const TagID tagid) const
  {
    return
      ngprobs2 && prevtagid < n_tags && tagid < n_tags
      ? ngprobs2[(n_tags*prevtagid)+tagid]
      : 0;
  };

  /**
   * Looks up and returns bigram probability: p(tag|prevtag), string-version.
   */
  inline const ProbT tagp(const dwdstTagString &prevtag, const dwdstTagString &tag) const
  {
    return tagp(tagids.name2id(prevtag), tagids.name2id(tag));
  };

  //------------------------------------------------------------
  // public methods: low-level: trigram probability lookup
#ifdef DWDST_USE_TRIGRAMS
  /**
   * Looks up and returns trigram probability: p(tagid|prevtagid2,prevtagid1),
   * given Trigram(prevtagid2,prevtagid1,tagid)
   */
  inline const ProbT tagp(const Trigram &trigram) const
  {
    TrigramProbTable::const_iterator tgti = ngprobs3.find(trigram);
    return tgti == ngprobs3.end() ? 0 : tgti->second;
  };

  /**
   * Looks up and returns trigram probability: p(tagid|prevtagid2,prevtagid1),
   * given prevtagid2, prevtagid1, tagid.
   */
  inline const ProbT tagp(const TagID prevtagid2, const TagID prevtagid1, const TagID tagid) const
  {
    return tagp(Trigram(prevtagid2,prevtagid1,tagid));
  };

  /**
   * Looks up and returns trigram probability: p(tag|prevtag1,prevtag2), string-version.
   */
  inline const ProbT tagp(const dwdstTagString &prevtag2,
			  const dwdstTagString &prevtag1,
			  const dwdstTagString &tag)
    const
  {
    return tagp(tagids.name2id(prevtag2), tagids.name2id(prevtag1), tagids.name2id(tag));
  };
#endif /* DWDST_USE_TRIGRAMS */

  //------------------------------------------------------------
  // public methods: top-level: tagging

  /** Top-level tagging interface: string input */
  //void tag_strings(int argc, char **argv, FILE *out=stdout, char *infilename=NULL);

  /** Top-level tagging interface: file input */
  void tag_stream(FILE *in=stdin, FILE *out=stdout, char *infilename=NULL);

  /** Top-level tagging interface: dump best path */
  inline void tag_print_best_path(FILE *out=stdout)
  {
    viterbi_best_path();
    for (size_t i = 0; i < tokens.size(); i++) {
      if (i >= vbestpath.size()) {
	carp("%s: Error: no tag for token number %u of sentence number %u: '%s'\n",
	     i, nsents, tokens[i].c_str());
	continue;
      }
      fputs(tokens[i].c_str(), out);
      fputc('\t', out);
      fputs(tagids.id2name(vbestpath[i+1]->tagid).c_str(), out);
      fputc('\n', out);
    }
    fputc('\n', out);
  };


  //------------------------------------------------------------
  // public methods: low-level: errors

  /** Error reporting */
  void carp(char *fmt, ...);


  //------------------------------------------------------------
  // public methods: low-level: debugging

  /** Debugging method: dump basic HMM contents to a text file. */
  void txtdump(FILE *file);

  /** Debugging method: dump current Viterbi state-table column to a text file */
  void viterbi_txtdump(FILE *file);
};


#endif /* _DWDST_HMM_H */
