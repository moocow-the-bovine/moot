/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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

/*--------------------------------------------------------------------------
 * File: mootHMM.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger for DWDS project : Hidden Markov Model (Disambiguator): headers
 *--------------------------------------------------------------------------*/

#ifndef _moot_HMM_H
#define _moot_HMM_H

#include <float.h>
#include <string.h>
#include <ctype.h>

#include "mootTypes.h"
#include "mootToken.h"
#include "mootTokenIO.h"
#include "mootLexfreqs.h"
#include "mootNgrams.h"
#include "mootEnum.h"
#include "mootBinStream.h"

#include <zlib.h>

/**
 * \def moot_DEBUG_ARCP
 * Define this for verbose debugging information to stderr.
 * Actually, arcp() should never be called anymore, so this
 * probably won't produce any output at all...
 */
//#define moot_DEBUG_ARCP 1
#undef moot_DEBUG_ARCP

/**
 * \def moot_ADD_ONE_HACK
 * Define this to include the 'add one' hack to avoid float underflows.
 *
 * \warning this causes major problems with sparse data, and using
 * 'double' as our probability type seems to work just dandy without
 * this hack...
 */
//#define moot_ADD_ONE_HACK 1
#undef moot_ADD_ONE_HACK

/**
 * \def mootProbEpsilon
 * Constant representing a minimal probability.
 * Used for default constructor.
 */
//#define mootProbEpsilon FLT_EPSILON
//#define mootProbEpsilon 1e-32
#define mootProbEpsilon DBL_EPSILON

/**
 * \def moot_LEX_UNKNOWN_TOKENS
 * Define this to include real lexical entries for tokens with counts
 * <= UnknownLexThreshhold.
 */
#define moot_LEX_UNKNOWN_TOKENS

/**
 * \def moot_USE_TRIGRAMS
 * Define this to use trigrams (not yet implemented!)
 */
#undef moot_USE_TRIGRAMS

moot_BEGIN_NAMESPACE

/*--------------------------------------------------------------------------
 * mootHMM : HMM class
 *--------------------------------------------------------------------------*/

/**
 * \brief 1st-order Hidden Markov Model Tagger/Disambiguator class.
 */
class mootHMM {
public:
  /*---------------------------------------------------------------------*/
  /** \name Atomic Types */
  //@{

  /** Type for a single probability value */
  typedef double ProbT;
  //typedef float ProbT;

  /** Type for a tag-identifier. Zero indicates an unknown tag. */
  typedef mootEnumID TagID;

  /** Type for a token-identider. Zero indicates an unknown token. */
  typedef mootEnumID TokID;
  //@}


  /*---------------------------------------------------------------------*/
  /** \name Lookup-Table Types */
  //@{
  
  /** Typedef for token-id lookup table */
  typedef mootEnum<mootTokString,
		    hash<mootTokString>,
		    equal_to<mootTokString> >
          TokIDTable;

  /** Typedef for tag-id lookup table */
  typedef mootEnum<mootTagString,
		    hash<mootTagString>,
		    equal_to<mootTagString> >
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

#ifdef moot_USE_TRIGRAMS
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
   * All ViterbiPathNode pointers returned by any mootHMM method
   * call are de-allocated on clear(). 
   *
   * Don't rely on the data in your (ViterbiPathNode*)s
   * remaining the same over multiple mootHMM method calls:
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
  /** \name I/O Format Flags */
  //@{
  /**
   * Whether to ignore first analysis of each input token (re-analysis).
   * Default=false.
   */
  bool input_ignore_first_analysis;

  /*---------------------------------------------------------------------
   * Output format flags : used by moot::TokenWriter
   */
  /**
   * Whether to output only analyses 
   * corresponding to the 'best' PoS tag (true),
   * or for all given PoS tags (false=default)
   */
  bool output_best_only;
  //@}

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
#ifdef moot_USE_TRIGRAMS
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
  mootTokString    typnames[NTokTypes]; /**< Names of special tokens */
  //@}

  /*---------------------------------------------------------------------*/
  /** \name Probability Lookup Tables */
  //@{
  size_t            n_tags;     /**< Number of known tags: used to compute lookup indices */
  size_t            n_toks;     /**< Number of known tokens: used for sanity checks */
  LexProbTable      lexprobs;   /**< Lexical probability lookup table */
  TagProbTable      ngprobs1;   /**< N-gram probability lookup table: unigrams */
  BigramProbTable   ngprobs2;   /**< N-gram probability lookup table: bigrams */
#ifdef moot_USE_TRIGRAMS
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
  //vector<mootTokString> tokens;  /**< Temporarily stores input tokens for tag_stream() */
  //set<TagID>             curtags; /**< Temporarily stores input token-tags for tag_stream() */
  //@}

public:
  /*---------------------------------------------------------------------*/
  /** \name Constructor / Destructor */
  //@{
  /** Default constructor */
  mootHMM(void)
    : input_ignore_first_analysis(false),
      output_best_only(false),
      start_tagid(0),
      unknown_lex_threshhold(1.0),
      nglambda1(mootProbEpsilon),
      nglambda2(1.0 - mootProbEpsilon),
      wlambda1(1.0 - mootProbEpsilon),
      wlambda2(mootProbEpsilon),
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
  ~mootHMM(void)
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
  bool save(mootBinStream::oBinStream &obs, const char *filename=NULL);

  /** Low-level: save guts to a binary stream */
  bool _bindump(mootBinStream::oBinStream &obs, const char *filename=NULL);

  /** Load from a binary file */
  bool load(const char *filename=NULL);

  /** Load from a binary stream */
  bool load(mootBinStream::iBinStream &ibs, const char *filename=NULL);

  /** Low-level: load guts from a binary stream */
  bool _binload(mootBinStream::iBinStream &ibs, const char *filename=NULL);
  //@}

  /*------------------------------------------------------------*/
  /** \name Accessors */
  //@{
  /** Set the unknown token name */
  inline void unknown_token_name(const mootTokString &name)
  {
    tokids.unknown_name(name);
    typnames[TokTypeUnknown] = name;
  };

  /** Set the unknown tag */
  inline void unknown_tag_name(const mootTokString &name)
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
  bool compile(const mootLexfreqs &lexfreqs,
	       const mootNgrams &ngrams,
	       const mootTagString &start_tag_str=mootTagString());

  /** Assign IDs for tokens and tags from lexfreqs: called by compile() */
  void assign_ids_lf(const mootLexfreqs &lexfreqs);

  /** Assign IDs for tokens and tags from ngrams: called by compile() */
  void assign_ids_ng(const mootNgrams   &ngrams);

  /**
   * Estimate ngram-smoothing constants: NOT called by compile().
   */
  bool estimate_lambdas(const mootNgrams &ngrams);

  /**
   * Estimate lexical smoothing constants: NOT called by compile().
   */
  bool estimate_wlambdas(const mootLexfreqs &lf);
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
  // Viterbi: clear

  /** \name High-level Viterbi algorithm API */
  //@{

  /** Clear Viterbi state table(s) */
  void viterbi_clear(void);

  //------------------------------------------------------------
  // Viterbi: single iteration: (mootToken)

  /**
   * Step a single Viterbi iteration, mootToken version.
   * Really just a wrapper for viterbi_step(TokID,set<TagID>).
   */
  inline void viterbi_step(const mootToken &token) {
    if (token.analyses().empty()) {
      viterbi_step(token.text());
    }
    else {
      set<TagID> tok_tagids;
      for (mootToken::AnalysisSet::const_iterator ani = token.analyses().begin();
	   ani != token.analyses().end();
	   ani = token.upper_bound(ani->tag))
	{
	  tok_tagids.insert(tagids.name2id(ani->tag));
	}
      viterbi_step(token2id(token.text()), tok_tagids);
    }
  };


  //------------------------------------------------------------
  // Viterbi: single iteration: (TokID)

  /**
   * Step a single Viterbi iteration, considering all known tags
   * for 'tokid' as possible analyses.  Faster in the case
   * where no futher information (i.e. set of possible tags) is
   * available.
   */
  inline void viterbi_step(TokID tokid);

  //------------------------------------------------------------
  // Viterbi: single iteration: (TokID,set<TagID>)

  /**
   * Step a single Viterbi iteration, considering only the tags
   * in 'tagids' -- useful if you have some a priori information
   * on the token.
   */
  inline void viterbi_step(TokID tokid, const set<TagID> &tag_ids);


  //------------------------------------------------------------
  // Viterbi: single iteration: (TokString)

  /**
   * \bold DEPRECATED in favor of viterbi_step(moot_token)
   *
   * Step a single Viterbi iteration, string version.
   * Really just a wrapper for viterbi_step(TokID tokid).
   */
  inline void viterbi_step(const mootTokString &token) {
    return viterbi_step(token2id(token));
  };

  //------------------------------------------------------------
  // Viterbi: single iteration: (TokString,set<TagString>)

  /**
   * \bold DEPRECATED
   *
   * Step a single Viterbi iteration, considering only the tags in 'tags'.
   */
  inline void viterbi_step(const mootTokString &token, const set<mootTagString> &tags)
  {
    set<TagID> tags_ids;
    for (set<mootTagString>::const_iterator tagi = tags.begin();
	 tagi != tags.end();
	 tagi++)
      {
	tags_ids.insert(tagids.name2id(*tagi));
      }
    return viterbi_step(token2id(token), tags_ids);
  };


  //------------------------------------------------------------
  // Viterbi: single iteration: (TokID,TagID,col=NULL)

  /**
   * Step a single Viterbi iteration, considering only the tag 'tagid'.
   */
  inline void viterbi_step(TokID tokid, TagID tagid, ViterbiColumn *col=NULL);

  //------------------------------------------------------------
  // Viterbi: single iteration: (TokString,TagString)

  /**
   * \bold DEPRECATED
   *
   * Step a single Viterbi iteration, considering only the tag 'tag': string version.
   */
  inline void viterbi_step(const mootTokString &token, const mootTagString &tag)
  {
    return viterbi_step(token2id(token), tagids.name2id(tag));
  };


  //------------------------------------------------------------
  // Viterbi: finish

  /**
   * Run final Viterbi iteration, using 'final_tagid' as the boundary tag
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
  inline ViterbiPathNode *viterbi_best_path(const mootTagString &tagstr)
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
#ifdef moot_ADD_ONE_HACK_BAD
	1.0 +  //-- add 1 to avoid float underflow (not here!)
#else
#endif // moot_ADD_ONE_HACK_BAD
	pnod->prob
	*
	( (nglambda1 * curtagp)
	  +
	  (nglambda2 * tagp(pnod->tagid, curtagid)) )
	;

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
  // Viterbi: fallback

  /**
   * Step a single Viterbi iteration, last-ditch effort: consider
   * all tags.  Implicitly called by other viterbi_step() methods.
   */
  inline void _viterbi_step_fallback(TokID tokid, ViterbiColumn *col);
  //@}

  //------------------------------------------------------------
  // public methods: low-level: token-type identification

  /** \name TnT compatibility hacks */
  //@{
  /** Get the TokID for a given token, uses type-based lookup */
  inline TokID token2id(const mootTokString &token) const
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
   * \bold DEPRECATED
   *
   * Looks up and returns lexical probability: p(token|tag)
   * given token, tag.
   */
  inline const ProbT wordp(const mootTokString token, const mootTagString tag) const
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
   * \bold DEPRECATED
   *
   * Looks up and returns unigram probability: p(tag), string-version.
   */
  inline const ProbT tagp(const mootTagString &tag) const
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
   * \bold DEPRECATED
   *
   * Looks up and returns bigram probability: p(tag|prevtag), string-version.
   */
  inline const ProbT tagp(const mootTagString &prevtag, const mootTagString &tag) const
  {
    return tagp(tagids.name2id(prevtag), tagids.name2id(tag));
  };

  //------------------------------------------------------------
  // public methods: low-level: trigram probability lookup
#ifdef moot_USE_TRIGRAMS
  /**
   * Looks up and returns trigram probability: p(tagid|prevtagid2,prevtagid1),
   * given Trigram(prevtagid2,prevtagid1,tagid)
   *
   * \bold NOT REALLY IMPLEMENTED.
   */
  inline const ProbT tagp(const Trigram &trigram) const
  {
    TrigramProbTable::const_iterator tgti = ngprobs3.find(trigram);
    return tgti == ngprobs3.end() ? 0 : tgti->second;
  };

  /**
   * Looks up and returns trigram probability: p(tagid|prevtagid2,prevtagid1),
   * given prevtagid2, prevtagid1, tagid.
   *
   * \bold NOT REALLY IMPLEMENTED.
   */
  inline const ProbT tagp(const TagID prevtagid2, const TagID prevtagid1, const TagID tagid) const
  {
    return tagp(Trigram(prevtagid2,prevtagid1,tagid));
  };

  /**
   * \bold DEPRECATED
   *
   * Looks up and returns trigram probability: p(tag|prevtag1,prevtag2), string-version.
   *
   * \bold NOT REALLY IMPLEMENTED.
   */
  inline const ProbT tagp(const mootTagString &prevtag2,
			  const mootTagString &prevtag1,
			  const mootTagString &tag)
    const
  {
    return tagp(tagids.name2id(prevtag2), tagids.name2id(prevtag1), tagids.name2id(tag));
  };
#endif /* moot_USE_TRIGRAMS */
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
   * Mid-level tagging interface: mark 'best' tags in sentence
   * structure: fills 'besttag' datum of each 'mootToken' element
   * of 'sentence'.
   */
  void tag_mark_best(mootSentence &sentence);
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

moot_END_NAMESPACE

#endif /* _moot_HMM_H */
