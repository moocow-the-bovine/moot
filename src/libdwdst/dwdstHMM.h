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
#include <ctype.h>

#include "dwdstTypes.h"
#include "dwdstLexfreqs.h"
#include "dwdstNgrams.h"
#include "dwdstEnum.h"
#include "dwdstBinStream.h"

#include <zlib.h>

/**
 * \def DWDST_DEBUG_ARCP
 * Define this for verbose debugging information to stderr.
 * Actually, arcp() should never be called anymore, so this
 * probably won't produce any output at all...
 */
//#define DWDST_DEBUG_ARCP 1
#undef DWDST_DEBUG_ARCP

/**
 * \def DWDST_ADD_ONE_HACK
 * Define this to include the 'add one' hack to avoid float underflows
 * WARNING: this causes major problems with sparse data, and using
 * 'double' as our probability type seems to work just dandy without
 * this hack.
 */
//#define DWDST_ADD_ONE_HACK 1
#undef DWDST_ADD_ONE_HACK

/**
 * \def dwdstProbEpsilon
 * Constant representing a minimal probability.
 * Used for default constructor.
 */
//#define dwdstProbEpsilon FLT_EPSILON
//#define dwdstProbEpsilon 1e-32
#define dwdstProbEpsilon DBL_EPSILON

/**
 * \def DWDST_LEX_UNKNOWN_TOKENS
 * Define this to include real lexical entries for tokens with counts
 * <= UnknownLexThreshhold.
 */
#define DWDST_LEX_UNKNOWN_TOKENS

/**
 * \def DWDST_USE_TRIGRAMS
 * Define this to use trigrams (not yet implemented!)
 */
#undef DWDST_USE_TRIGRAMS

DWDST_BEGIN_NAMESPACE

/*--------------------------------------------------------------------------
 * dwdstHMM : HMM class
 *--------------------------------------------------------------------------*/

/**
 * \brief 1st-order Hidden Markov Model Tagger/Disambiguator class.
 */
class dwdstHMM {
public:
  /*---------------------------------------------------------------------*/
  /** \name Atomic Types */
  //@{

  /** Type for a single probability value */
  typedef double ProbT;
  //typedef float ProbT;

  /** Type for a tag-identifier. Zero indicates an unknown tag. */
  typedef dwdstEnumID TagID;

  /** Type for a token-identider. Zero indicates an unknown token. */
  typedef dwdstEnumID TokID;
  //@}


  /*---------------------------------------------------------------------*/
  /** \name Lookup-Table Types */
  //@{
  
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

  /**
   * Type for lexical probability lookup table: tokid->(tagid->prob),
   * where prob==p(tokid|tagid).
   */
  typedef vector<LexProbSubTable> LexProbTable;

  /**
   * Type for unigram probability table: c-style array:
   * probabilities indexed by numeric tag-id.
   */
  typedef ProbT *TagProbTable;
  //typedef vector<ProbT> TagProbTable;
  //typedef hash_map<TagID,ProbT> TagProbTable;

  /**
   * Type for bigram probability lookup table:
   * c-style 2d array: probabilites p(tag|ptag)
   * indexed by ((ntags*ptag)+tag).
   *
   * This winds up being a rather sparse table,
   * but it should fit well in memory even for large
   * (~= 2K tags) tagsets on contemporary machines,
   * and lookup is Just Plain Quick.
   */
  typedef ProbT *BigramProbTable;
  //typedef vector<TagProbTable> BigramProbTable;

#ifdef DWDST_USE_TRIGRAMS
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
#endif
  //@}

  /*---------------------------------------------------------------------*/
  /** \name Viterbi State-Table Types  */
  //@{

  /** Type for a Viterbi state-table entry (linked-list columns) */
  class ViterbiNode {
  public:
    TagID tagid;                   /**< Tag-ID for this node */
    ProbT prob;                    /**< Probability of best path to this node */
    struct ViterbiNode *pth_prev;  /**< Previous node in best path */
    struct ViterbiNode *row_next;  /**< Next node in current column */
  };

  /**
   * Type for a Viterbi state-table column.
   * A Viterbi state-table is completely represented by its (current)
   * final column.
   */
  class ViterbiColumn {
  public:
    //TokID          tokid;    /**< Token-ID for this column */
    ViterbiNode   *nodes;    /**< Column nodes */
    ViterbiColumn *col_prev; /**< Previous column */
  };

  /**
   * Type for a Viterbi path-node.  It's faster to use
   * the (ViterbiNode*)s directly, if you can deal
   * with reverse order.
   *
   * All relevant allocation (and de-allocation) is handled
   * internally:
   * All ViterbiPathNode pointers returned by any dwdstHMM method
   * call are de-allocated on clear(). 
   *
   * Don't rely on the data in your (ViterbiPathNode*)s
   * remaining the same over multiple dwdstHMM method calls:
   * get what you need, and then lose the nodes.
   */
  struct ViterbiPathNode {
  public:
    ViterbiNode      *node;      /** Corresponding state-table node */
    ViterbiPathNode  *path_next; /** Next node in this path */
  };
  //typedef deque<ViterbiNode*> ViterbiPath;
  //@}

public:
  /*---------------------------------------------------------------------*/
  /** \name Pragmatic Constants */
  //@{
  TagID             start_tagid;  /**< Initial/final tag, used for bootstrapping */
  ProbT             unknown_lex_threshhold;  /**< Unknown lexical threshhold */
  //@}

  /*---------------------------------------------------------------------*/
  /** \name Smoothing Constants */
  //@{
  ProbT             nglambda1;    /**< Smoothing constant for unigrams */
  ProbT             nglambda2;    /**< Smoothing constant for bigrams */
#ifdef DWDST_USE_TRIGRAMS
  ProbT             nglambda3;    /**< Smoothing constant for trigrams */
#endif
  ProbT             wlambda1;     /**< Smoothing constant lexical probabilities */
  ProbT             wlambda2;     /**< Smoothing constant lexical probabilities */
  //@}

  /*---------------------------------------------------------------------*/
  /** \name ID Lookup Tables */
  //@{
  TokIDTable        tokids;     /**< Token-ID lookup table */
  TagIDTable        tagids;     /**< Tag-ID lookup table */

  TokID             typids[NTokTypes]; /**< Token-type to Token-ID lookup table for non-alphas */
  dwdstTokString    typnames[NTokTypes]; /**< Names of special tokens */
  //@}

  /*---------------------------------------------------------------------*/
  /** \name Probability Lookup Tables */
  //@{
  size_t            n_tags;     /**< Number of known tags: used to compute lookup indices */
  size_t            n_toks;     /**< Number of known tokens: used for sanity checks */
  LexProbTable      lexprobs;   /**< Lexical probability lookup table */
  TagProbTable      ngprobs1;   /**< N-gram probability lookup table: unigrams */
  BigramProbTable   ngprobs2;   /**< N-gram probability lookup table: bigrams */
#ifdef DWDST_USE_TRIGRAMS
  TrigramProbTable  ngprobs3;   /**< N-gram probability lookup table: trigrams */
#endif
  //@}

  /*---------------------------------------------------------------------*/
  /** \name Viterbi State Data */
  //@{
  ViterbiColumn     *vtable;    /**< Low-level state table for Viterbi algorithm */
  //@}

  /*---------------------------------------------------------------------*/
  /** \name Statistics / Performance Tracking */
  //@{
  size_t             nsents;    /**< Number of sentenced processed */
  size_t             ntokens;   /**< Number of tokens processed */
  //@}

protected:
  /*---------------------------------------------------------------------*/
  /** \name Low-level: trash stacks */
  //@{
  ViterbiNode     *trash_nodes;     /**< Recycling bin for Viterbi state-table nodes */
  ViterbiColumn   *trash_columns;   /**< Recycling bin for Viterbi state-table columns */
  ViterbiPathNode *trash_pathnodes; /**< Recycling bin for Viterbi path-nodes */
  //@}

  /*---------------------------------------------------------------------*/
  /** \name Low-level Viterbi data */
  //@{
  TagID             vtagid;     /**< Current tag-id under consideration for viterbi_step() */
  ProbT             vbestpr;    /**< Best probability for viterbi_step() */
  ProbT             vtagpr;     /**< Probability for current tag-id for viterbi_step() */
  ProbT             vwordpr;    /**< Save word-probability */
  ViterbiNode      *vbestpn;    /**< Best previous node for viterbi_step() */

  ViterbiPathNode  *vbestpath;  /**< For node->path conversion */
  //@}

  /*---------------------------------------------------------------------*/
  /** \name Low-level tagging data */
  //@{
  vector<dwdstTokString> tokens;  /**< Temporarily stores input tokens for tag_stream() */
  set<TagID>             curtags; /**< Temporarily stores input token-tags for tag_stream() */
  //@}

public:
  /*---------------------------------------------------------------------*/
  /** \name Constructor / Destructor */
  //@{
  /** Default constructor */
  dwdstHMM(void)
    : start_tagid(0),
      unknown_lex_threshhold(1.0),
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
      trash_pathnodes(NULL),
      vbestpn(NULL),
      vbestpath(NULL)
  {
    typnames[TokTypeAlpha] = "@UNKNOWN"; // -- ?!
    typnames[TokTypeCard]  = "@CARD";
    typnames[TokTypeCardPunct] = "@CARDPUNCT";
    typnames[TokTypeCardSeps]  = "@CARDSEPS";
    typnames[TokTypeUnknown] = "@UNKNOWN";
    for (TokID i = 0; i < NTokTypes; i++) { typids[i] = 0; }
  };

  /** Destructor */
  ~dwdstHMM(void)
  {
    clear();
  };
  //@}

  /*------------------------------------------------------------*/
  /**\name Binary load / save */
  //@{

  /** Save to a binary file */
  bool save(const char *filename, int compression_level=Z_DEFAULT_COMPRESSION);

  /** Save to a binary stream */
  bool save(dwdstBinStream::oBinStream &obs, const char *filename=NULL);

  /** Low-level: save guts to a binary stream */
  bool _bindump(dwdstBinStream::oBinStream &obs, const char *filename=NULL);

  /** Load from a binary file */
  bool load(const char *filename=NULL);

  /** Load from a binary stream */
  bool load(dwdstBinStream::iBinStream &ibs, const char *filename=NULL);

  /** Low-level: load guts from a binary stream */
  bool _binload(dwdstBinStream::iBinStream &ibs, const char *filename=NULL);
  //@}

  /*------------------------------------------------------------*/
  /** \name Accessors */
  //@{
  /** Set the unknown token name */
  inline void unknown_token_name(const dwdstTokString &name)
  {
    tokids.unknown_name(name);
    typnames[TokTypeUnknown] = name;
  };

  /** Set the unknown tag */
  inline void unknown_tag_name(const dwdstTokString &name)
  {
    tagids.unknown_name(name);
  };
  //@}

  /*------------------------------------------------------------*/
  /** \name Reset and clear */
  //@{
  /**
   * Reset/clear the object, freeing all dynamic data structures.
   * If 'wipe_everything' is false, ID-tables and constants will
   * spared.
   */
  void clear(bool wipe_everything=true);
  //@}


  //------------------------------------------------------------
  /** \name Compilation */
  //@{

  /**
   * Compile probabilites from raw frequency counts in 'lexfreqs' and 'ngrams'.
   * Returns false on failure.
   */
  bool compile(const dwdstLexfreqs &lexfreqs,
	       const dwdstNgrams &ngrams,
	       const dwdstTagString &start_tag_str=dwdstTagString());

  /** Assign IDs for tokens and tags from lexfreqs: called by compile() */
  void assign_ids_lf(const dwdstLexfreqs &lexfreqs);

  /** Assign IDs for tokens and tags from ngrams: called by compile() */
  void assign_ids_ng(const dwdstNgrams   &ngrams);

  /**
   * Estimate ngram-smoothing constants: NOT called by compile().
   */
  bool estimate_lambdas(const dwdstNgrams &ngrams);

  /**
   * Estimate lexical smoothing constants: NOT called by compile().
   */
  bool estimate_wlambdas(const dwdstLexfreqs &lf);
  //@}

  //------------------------------------------------------------
  /** \name Trash-stack Utilities */

  //@{
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
  // Viterbi utilities: path-nodes

  /** Returns a pointer to an unused ViterbiPathNode, possibly allocating a new one. */
  inline ViterbiPathNode *viterbi_get_pathnode(void) {
    ViterbiPathNode *pnod;
    if (trash_pathnodes != NULL) {
      pnod            = trash_pathnodes;
      trash_pathnodes = pnod->path_next;
    } else {
      pnod = new ViterbiPathNode();
    }
    return pnod;
  };
  //@}

  //------------------------------------------------------------
  // public methods: high-level: Viterbi: clear

  /** \name High-level Viterbi algorithm API */
  //@{

  /** Clear Viterbi state table(s) */
  inline void viterbi_clear(void)
  {
    //-- move to trash: state-table
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

    //viterbi_clear_bestpath();

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

    if (col->nodes == NULL) {
      //-- we might not have found anything...
      _viterbi_step_fallback(tokid, col);
    } else{
      //-- add new column to state table
      vtable = col;
    }
  };

  //------------------------------------------------------------
  // public methods: high-level: Viterbi: single iteration: (TokString)

  /**
   * Step a single Viterbi iteration, string version.
   * Really just a wrapper for viterbi_step(TokID tokid).
   */
  inline void viterbi_step(const dwdstTokString &token) {
    return viterbi_step(token2id(token));
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

    if (col->nodes == NULL) {
      //-- we might not have found anything...
      _viterbi_step_fallback(tokid, col);
    } else{
      //-- add new column to state table
      vtable = col;
    }
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
    return viterbi_step(token2id(token), tags_ids);
  };


  //------------------------------------------------------------
  // public methods: high-level: Viterbi: single iteration: (TokID,TagID)

  /**
   * Step a single Viterbi iteration, considering only the tag 'tagid'.
   */
  inline void viterbi_step(TokID tokid, TagID tagid, ViterbiColumn *col=NULL)
  {
    ViterbiNode   *nod;
    if (col==NULL) {
      //-- Get next column
      col = viterbi_get_column();
      col->col_prev = vtable;
      col->nodes = NULL;
    }

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
    return viterbi_step(token2id(token), tagids.name2id(tag));
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
  //@}


  //------------------------------------------------------------
  // public methods: mid-level: Viterbi: best node

  /** \name Mid-Level Viterbi State Utilties  */
  //@{

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
   * Useful utility: build a path (in input order) from a ViterbiNode.
   * See caveats for 'struct ViterbiPathNode' -- return value is non-const
   * for easy iteration.
   *
   * Uses 'vbestpath' to store constructed path.
   */
  inline ViterbiPathNode *viterbi_node_path(ViterbiNode *node)
  {
    viterbi_clear_bestpath();
    ViterbiPathNode *pnod; 
    for ( ; node != NULL; node = node->pth_prev) {
      pnod            = viterbi_get_pathnode();
      pnod->node      = node;
      pnod->path_next = vbestpath;
      vbestpath       = pnod;
    }
    return vbestpath;
  };
  //@}


  //------------------------------------------------------------
  // public methods: high-level: best path

  /** \name High-level Viterbi Path utilities */
  //@{

  /** Get current best path (in input order), considering all current tags */
  inline ViterbiPathNode *viterbi_best_path(void)
  {
    return viterbi_node_path(viterbi_best_node());
  };

  /** Get current best path (in input order), considering only tag 'tagid' */
  inline ViterbiPathNode *viterbi_best_path(TagID tagid)
  {
    return viterbi_node_path(viterbi_best_node(tagid));
  };

  /** Get current best path (in input order), considering only tag 'tag' */
  inline ViterbiPathNode *viterbi_best_path(const dwdstTagString &tagstr)
  {
    return viterbi_best_path(tagids.name2id(tagstr));
  };
  //@}


  //------------------------------------------------------------
  // public methods: low-level: Viterbi

  /** \name Low-level Viterbi utilities */
  //{@

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
  // public methods: low-level: Viterbi: clear best-path

  /** Clear Viterbi state table(s) */
  inline void viterbi_clear_bestpath(void)
  {
    //-- move to trash: path-nodes
    ViterbiPathNode *pnod, *pnod_next;
    for (pnod = vbestpath; pnod != NULL; pnod = pnod_next) {
      pnod_next       = pnod->path_next;
      pnod->path_next = trash_pathnodes;
      trash_pathnodes = pnod;
    }
    vbestpath = NULL;
  };

  //------------------------------------------------------------
  // public methods: viterbi: fallback

  /**
   * Step a single Viterbi iteration, last-ditch effort: consider
   * all tags.  Implicitly called by other viterbi_step() methods.
   */
  inline void _viterbi_step_fallback(TokID tokid, ViterbiColumn *col)
  {
    //-- sanity
    if (tokid >= n_toks) tokid = 0;
    if (col==NULL) {
      //-- Get next column
      col = viterbi_get_column();
      col->col_prev = vtable;
      col->nodes = NULL;
    }

    //-- variables
    ViterbiNode                     *nod;
    const LexProbSubTable           &lps = lexprobs[tokid];
    LexProbSubTable::const_iterator  lpsi;

    //-- for each possible destination tag 'vtagid' (except "UNKNOWN")
    for (vtagid = 1; vtagid < n_tags; vtagid++) {

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

    if (col->nodes == NULL) {
      //-- we STILL might not have found anything...
      viterbi_step(tokid, 0, col);
    } else{
      //-- add new column to state table
      vtable = col;
    }
  };
  //@}

  //------------------------------------------------------------
  // public methods: low-level: token-type identification

  /** \name TnT compatibility hacks */
  //@{
  /** Get the TokID for a given token, uses type-based lookup */
  inline TokID token2id(const dwdstTokString &token) const
  {
    TokenType typ = token2type(token);
    return typids[typ]==0 ? tokids.name2id(token) : typids[typ];
  };
  //@}


  /** \name Probability lookup */
  //@{
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
    return wordp(token2id(token), tagids.name2id(tag));
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
  //@}


  //------------------------------------------------------------
  // public methods: top-level: tagging

  /** \name Top-level tagging interface */
  //@{

  /** Top-level tagging interface: file input */
  void tag_stream(FILE *in=stdin, FILE *out=stdout, char *srcname=NULL);

  /** Top-level tagging interface: string input */
  void tag_strings(int argc, char **argv, FILE *out=stdout, char *infilename=NULL);

  /**
   * Top-level tagging interface: dump best path.
   * Requires populated 'tokens' vector)
   */
  inline void tag_print_best_path(FILE *out=stdout)
  {
    //-- populate 'vbestpath' with (ViterbiPathNode*)s
    ViterbiPathNode *pnod = viterbi_best_path();
    vector<dwdstTokString>::const_iterator toki;

    if (pnod) pnod = pnod->path_next;  //-- skip boundary tag

    for (toki = tokens.begin(); toki != tokens.end(); toki++) {
      fputs(toki->c_str(), out);
      fputc('\t', out);

      if (pnod && pnod->node) {
	fputs(tagids.id2name(pnod->node->tagid).c_str(), out);
	pnod = pnod->path_next;
      }
      else {
	//-- this should never actually happen, but it has...
	carp("%s: Error: no best tag for token '%s'!\n",
	     "dwdstHMM::tag_print_best_path()", toki->c_str());
	fputs(tagids.id2name(0).c_str(), out); //-- use 'unknown' tag
      }

      fputc('\n', out);
    }
    fputc('\n', out);
  };
  //@}


  //------------------------------------------------------------
  // public methods: low-level: errors

  /** \name Warnings / Errors */
  //@{

  /** Error reporting */
  void carp(char *fmt, ...);

  //@}

  //------------------------------------------------------------
  // public methods: low-level: debugging

  /** \name Debugging */
  //@{

  /** Debugging method: dump basic HMM contents to a text file. */
  void txtdump(FILE *file);

  /** Debugging method: dump current Viterbi state-table column to a text file */
  void viterbi_txtdump(FILE *file);

  //@}
};

DWDST_END_NAMESPACE

#endif /* _DWDST_HMM_H */
