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

/*--------------------------------------------------------------------------
 * File: mootHMM.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moot PoS tagger : Hidden Markov Model (Disambiguator): headers
 *--------------------------------------------------------------------------*/

#ifndef _moot_HMM_H
#define _moot_HMM_H

#include <float.h>
#include <string.h>
#include <ctype.h>

#include "mootTypes.h"
#include "mootToken.h"
#include "mootLexfreqs.h"
#include "mootClassfreqs.h"
#include "mootNgrams.h"
#include "mootEnum.h"
#include "mootBinStream.h"

#include <zlib.h>

/**
 * \def moot_ADD_ONE_HACK
 * Define this to include the 'add one' hack to avoid float underflows.
 *
 * \warning this causes major problems with sparse data, and using
 * 'double' as our probability type seems to work just dandy without
 * this hack.  In other words, you should \b NEVER define this.
 */
#undef moot_ADD_ONE_HACK

/**
 * \def mootProbEpsilon
 * Constant representing a minimal probability.
 * Used for default constructor.  The standard value
 * should be appropriate for an IEEE 754-1985 float,
 * but we want our probabilities bigger, so we use
 * DBL_EPSILON if we can.
 */
//#define mootProbEpsilon FLT_EPSILON
//#define mootProbEpsilon 1e-32
#ifdef DBL_EPSILON
# define mootProbEpsilon DBL_EPSILON
#else
# define mootProbEpsilon 1.19209290E-07
#endif

/**
 * \def moot_LEX_UNKNOWN_TOKENS
 * Define this to include real lexical entries for tokens with counts
 * <= UnknownLexThreshhold.  Not entirely correct, but it actually
 * seems to help.
 */
#define moot_LEX_UNKNOWN_TOKENS
//#undef moot_LEX_UNKNOWN_TOKENS

/**
 * \def moot_USE_TRIGRAMS
 * Define this to use trigrams (not yet implemented everywhere!)
 */
#undef moot_USE_TRIGRAMS

/**
 * \def moot_LEX_UNKNOWN_CLASSES
 * Define this to include real lexical-class entries for classes with counts
 * <= UnknownClassThreshhold.  Not entirely kosher, but it works well, so
 * what the hey...
 */
#define moot_LEX_UNKNOWN_CLASSES
//#undef moot_LEX_UNKNOWN_CLASSES


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

  /**
   * Type for a single probability value.
   * We use \c double here, since our probabilties can get wee tiny small
   * for longish sentences.
   */
  typedef double ProbT;
  //typedef float ProbT;

  /** Type for a tag-identifier. Zero indicates an unknown tag. */
  typedef mootEnumID TagID;

  /** Type for a token-identider. Zero indicates an unknown token. */
  typedef mootEnumID TokID;

  /**
   * Typedef for a lexical ClassID. Zero indicates
   * either a previously unknown class or the empty class.
   */
  typedef mootEnumID ClassID;
  //@}

  /*------------------------------------------------------------
   * public typedefs : lexical classes
   */
  /// \name Lexical class types
  //@{
  /**
   * Type for a lexical-class aka "ambiguity class".  Intuitively, the
   * lexical class associated with a given token is just the set of all
   * a priori possible PoS tags for that that token.
   */
  typedef set<TagID> LexClass;

  /** Hash method: utility struct for hash_map<LexClass,...>. */
  struct LexClassHash {
  public:
    inline size_t operator()(const LexClass &x) const {
      size_t hv = 0;
      for (LexClass::const_iterator xi = x.begin(); xi != x.end(); xi++) {
	hv = 5*hv + *xi;
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
  //@}

  /*---------------------------------------------------------------------*/
  /** \name Lookup-Table Types */
  //@{
  
  /** Typedef for tag-id lookup table */
  typedef mootEnum<mootTagString,
		    hash<mootTagString>,
		    equal_to<mootTagString> >
          TagIDTable;

  /** Typedef for token-id lookup table */
  typedef mootEnum<mootTokString,
		    hash<mootTokString>,
		    equal_to<mootTokString> >
          TokIDTable;

  /** Typedef for class-id lookup table */
  typedef mootEnum<LexClass,
		   LexClassHash,
		   LexClassEqual>
          ClassIDTable;

  /** Type for lexical probability lookup subtable: \c tagid=>p(tagid) */
  typedef map<TagID,ProbT> LexProbSubTable;

  /**
   * Type for lexical-class probability lookup subtable:
   * \c tagid=>p(...tagid...)
   */
  typedef LexProbSubTable LexClassProbSubTable;

  /**
   * Type for lexical probability lookup table: \c tokid=>(tagid=>p(tokid|tagid))
   */
  typedef vector<LexProbSubTable> LexProbTable;

  /**
   * Type for lexical-class probability lookup table:
   * \c classid=>(tagid=>p(classid|tagid))
   * Really just an alias for \c LexProbSubtable: at
   * some point, we should capitalize on this and
   * make things spiffy boffo stomach-lurching fast,
   * but that requires more information than is
   * currently stored in our models (specifically,
   * foreknowledge of the token->class mapping for known
   * tokens), and an assumption that this mapping is
   * static, which it very well might not be at
   * some vaguely imagined unspecified future point
   * in time.
   */
  typedef LexProbTable LexClassProbTable;

  /**
   * Type for unigram probability table: c-style array:
   * probabilities indexed by numeric tag-id: \c tagid->p(tagid)
   */
  typedef ProbT *TagProbTable;
  //typedef vector<ProbT> TagProbTable;
  //typedef hash_map<TagID,ProbT> TagProbTable;

  /**
   * Type for bigram probability lookup table:
   * c-style 2d array: probabilites \c p(tagid|ptagid)
   * indexed by \c ((ntags*ptagid)+tagid).
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
    /** Trigram constructor */
    Trigram(TagID t1=0, TagID t2=0, TagID t3=0) : tag1(t1), tag2(t2), tag3(t3) {};
    /** Trigram destructor */
    ~Trigram(void) {};
  };

  /** Type for a trigram probability lookup table : trigram(t1,t2,t3)->p(t3|<t1,t2>)*/
  typedef
    hash_map<Trigram,ProbT,Trigram::HashFcn,Trigram::EqualFcn>
    TrigramProbTable;
#endif
  //@}

  /*---------------------------------------------------------------------*/
  /** \name Viterbi State-Table Types  */
  //@{

  /** Type for a Viterbi state-table entry (linked-list columns of linked-list rows) */
  class ViterbiNode {
  public:
    TagID tagid;                   /**< Tag-ID for this node */
    ProbT prob;                    /**< Probability of best path to this node */
    struct ViterbiNode *pth_prev;  /**< Previous node in best path to this node */
    struct ViterbiNode *row_next;  /**< Next tag-node in current column */
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
   * call are de-allocated on clear().  On viterbi_clear(),
   * they're wiped and tossed onto an internal trash-stack:
   * this is marginally faster than re-allocation.
   *
   * \warning Don't rely on the data in your (ViterbiPathNode*)s
   * remaining the same over multiple mootHMM method calls:
   * get what you need, and then lose the nodes.
   */
  struct ViterbiPathNode {
  public:
    ViterbiNode      *node;      /** Corresponding state-table node */
    ViterbiPathNode  *path_next; /** Next node in this path */
  };
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
  /** \name Useful Constants */
  //@{

  /**
   * Whether to use class probabilities (Default=true)
   * \warning Don't set this to true unless your input
   * files actually contain a priori analyses generated
   * by the same method on which you trained your model;
   * otherwise, expect abominable accuracy.
   */
  bool      use_lex_classes;

  /**
   * Boundary tag, used during compilation, viterbi_start(), and viterbi_finish()
   * This gets set by the \c start_tag_str argument to compile().
   * Whatever it is, it should be consistend with what you trained on.
   */
  TagID     start_tagid;

  /**
   * "Unknown" lexical threshhold: used during compilation to determine
   * whether a token's statistics are recorded as "pure" lexical probabilities
   * or as probabilities for the "unknown" token.  This is just a raw
   * count: the minimum number of times a token must have occurred in
   * the training data in order for us to record statistics about it
   * as "pure" lexical probabilities.  Default=1.
   */
  ProbT     unknown_lex_threshhold;

  /**
   * "Unknown" lexical-class threshhold: used during compilation to determine
   * whether a classes's statistics are recorded as "pure" class probabilities
   * or as probabilities for the "unknown" class.  This is just a raw
   * count: the minimum number of times a class must have occurred in
   * the training data in order for us to record statistics about it
   * as "pure" lexical-class probabilities.  Default=1
   */
  ProbT     unknown_class_threshhold;

  /**
   * LexClass to use for unknown tokens with no analyses.
   * This gets set at compile-time.  You can re-assign it
   * after that if you are so inclined.
   */
  LexClass   uclass;
  //@}

  /*---------------------------------------------------------------------*/
  /** \name Smoothing Constants */
  //@{
  ProbT             nglambda1;    /**< Smoothing constant for unigrams */
  ProbT             nglambda2;    /**< Smoothing constant for bigrams */
#ifdef moot_USE_TRIGRAMS
  ProbT             nglambda3;    /**< Smoothing constant for trigrams */
#endif
  ProbT             wlambda1;     /**< Smoothing constant for lexical probabilities */
  ProbT             wlambda2;     /**< Smoothing constant for lexical probabilities */
  ProbT             clambda1;     /**< Smoothing constant for class probabilities */
  ProbT             clambda2;     /**< Smoothing constant for class probabilities */
  //@}

  /*---------------------------------------------------------------------*/
  /** \name ID Lookup Tables */
  //@{
  TokIDTable        tokids;     /**< Token-ID lookup table */
  TagIDTable        tagids;     /**< Tag-ID lookup table */
  ClassIDTable      classids;   /**< Class-ID lookup table */

  TokID             typids[NTokTypes]; /**< TokenType to TokenID lookup table for non-alphabetics */
  //@}

  /*---------------------------------------------------------------------*/
  /** \name Probability Lookup Tables */
  //@{
  size_t            n_tags;     /**< Number of known tags: used to compute lookup indices */
  size_t            n_toks;     /**< Number of known tokens: used for sanity checks */
  size_t            n_classes;  /**< Number of known lexical classes */

  LexProbTable      lexprobs;   /**< Lexical probability lookup table */
  LexClassProbTable lcprobs;    /**< Lexical-class probability lookup table */
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
  size_t             nsents;      /**< Total number of sentenced processed */
  size_t             ntokens;     /**< Total number of tokens processed */
  size_t             nnewtokens;  /**< Total number of unknown-tokens processed */
  size_t             nunclassed;  /**< Number of classless tokens processed */
  size_t             nnewclasses; /**< Number of unknown-class tokens processed */
  size_t             nunknown;    /**< Number of totally unknown (token,class) pairs procesed */
  size_t             nfallbacks;  /**< Number of fallbacks in viterbi_step() */
  //@}

protected:
  /*---------------------------------------------------------------------*/
  /** \name Low-level data: trash stacks */
  //@{
  ViterbiNode     *trash_nodes;     /**< Recycling bin for Viterbi state-table nodes */
  ViterbiColumn   *trash_columns;   /**< Recycling bin for Viterbi state-table columns */
  ViterbiPathNode *trash_pathnodes; /**< Recycling bin for Viterbi path-nodes */
  //@}

  /*---------------------------------------------------------------------*/
  /** \name Low-level data: temporaries */
  //@{
  TagID             vtagid;     /**< Current tag-id under consideration for viterbi_step() */
  ProbT             vbestpr;    /**< Best probability for viterbi_step() */
  ProbT             vtagpr;     /**< Probability for current tag-id for viterbi_step() */
  ProbT             vwordpr;    /**< Save word-probability */
  ViterbiNode      *vbestpn;    /**< Best previous node for viterbi_step() */

  ViterbiPathNode  *vbestpath;  /**< For node->path conversion */
  //@}

public:
  /*---------------------------------------------------------------------*/
  /** \name Constructor / Destructor */
  //@{
  /** Default constructor */
  mootHMM(void)
    : input_ignore_first_analysis(false),
      output_best_only(false),
      use_lex_classes(true),
      start_tagid(0),
      unknown_lex_threshhold(1.0),
      unknown_class_threshhold(1.0),
      nglambda1(mootProbEpsilon),
      nglambda2(1.0 - mootProbEpsilon),
      wlambda1(1.0 - mootProbEpsilon),
      wlambda2(mootProbEpsilon),
      clambda1(1.0 - mootProbEpsilon),
      clambda2(mootProbEpsilon),
      n_tags(0),
      n_toks(0),
      n_classes(0),
      ngprobs1(NULL),
      ngprobs2(NULL),
      vtable(NULL),
      nsents(0),
      ntokens(0),
      nnewtokens(0),
      nunclassed(0),
      nnewclasses(0),
      nunknown(0),
      nfallbacks(0),
      trash_nodes(NULL),
      trash_columns(NULL), 
      trash_pathnodes(NULL),
      vbestpn(NULL),
      vbestpath(NULL)
  {
    for (TokID i = 0; i < NTokTypes; i++) { typids[i] = 0; }
  };

  /** Destructor */
  ~mootHMM(void)
  {
    clear();
  };
  //@}

  /*------------------------------------------------------------*/
  /** \name Reset / clear */
  //@{
  /**
   * Reset/clear the object, freeing all dynamic data structures.
   * If 'wipe_everything' is false, ID-tables and constants will
   * spared.
   */
  void clear(bool wipe_everything=true);
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
  /** Set the unknown token name : UNSAFE! */
  inline void unknown_token_name(const mootTokString &name)
  {
    tokids.unknown_name(name);
  };

  /** Set the unknown tag : this tag should never appear anyways */
  inline void unknown_tag_name(const mootTokString &name)
  {
    tagids.unknown_name(name);
  };

  /*
   * Set lexical class to use for tokens without user-specified analyses.
   * Really just an alias for 'uclass' datum.
   */
  inline void unknown_class_name(const mootTagSet &tagset)
  {
    tagset2lexclass(tagset,&uclass,false);
  };
  //@}


  //------------------------------------------------------------
  /** \name Compilation / Initialization */
  //@{
  /**
   * Compile probabilites from raw frequency counts in 'lexfreqs' and 'ngrams'.
   * Returns false on failure.
   */
  bool compile(const mootLexfreqs &lexfreqs,
	       const mootNgrams &ngrams,
	       const mootClassfreqs &classfreqs,
	       const mootTagString &start_tag_str=mootTagString());

  /** Assign IDs for tokens and tags from lexfreqs: called by compile() */
  void assign_ids_lf(const mootLexfreqs &lexfreqs);

  /** Assign IDs for tags from ngrams: called by compile() */
  void assign_ids_ng(const mootNgrams   &ngrams);

  /** Assign IDs for classes and tags from classfreqs: called by compile() */
  void assign_ids_cf(const mootClassfreqs &classfreqs);

  /** Compile "unknown" lexical class : called by compile() */
  void compile_unknown_lexclass(const mootClassfreqs &classfreqs);

  /** Estimate ngram-smoothing constants: NOT called by compile(). */
  bool estimate_lambdas(const mootNgrams &ngrams);

  /** Estimate lexical smoothing constants: NOT called by compile(). */
  bool estimate_wlambdas(const mootLexfreqs &lf);

  /** Estimate class smoothing constants: NOT called by compile(). */
  bool estimate_clambdas(const mootClassfreqs &cf);
  //@}

  //------------------------------------------------------------
  // Tagging: Top-level
  /** \name Top-level Tagging Interface */
  //@{

  /** Top-level tagging interface: file input & output */
  void tag_stream(FILE *in=stdin, FILE *out=stdout, char *srcname=NULL);

  /** Top-level tagging interface: string input, file output */
  void tag_strings(int argc, char **argv, FILE *out=stdout, char *infilename=NULL);

  /**
   * Top-level tagging interface: mootSentence input & output (destructive).
   * Calling this method will (re-)populate the \c besttag
   * datum in the \c sentence argument.
   */
  inline void tag_sentence(mootSentence &sentence) {
    viterbi_clear();
    for (mootSentence::const_iterator si = sentence.begin(); si != sentence.end(); si++) {
      viterbi_step(*si);
    }
    viterbi_finish();
    tag_mark_best(sentence);
    nsents++;
  };
  //@}

  /*====================================================================
   * VITERBI: Mid-level
   *====================================================================*/
  /** \name Mid-level Viterbi algorithm API */
  //@{

  //------------------------------------------------------------
  // Viterbi: Mid-level: clear
  /** Clear Viterbi state table(s) */
  void viterbi_clear(void);

  //------------------------------------------------------------
  // Viterbi: single iteration: (mootToken)
  /**
   * Step a single Viterbi iteration, \c mootToken version.
   * Really just a wrapper for \c viterbi_step(TokID,set<TagID>).
   */
  inline void viterbi_step(const mootToken &token) {
    ntokens++;
    LexClass tok_class;
    for (mootToken::AnalysisSet::const_iterator ani = token.analyses().begin();
	 ani != token.analyses().end();
	 ani = token.upper_bound(ani->tag))
      {
	tok_class.insert(tagids.name2id(ani->tag));
      }
    viterbi_step(token2id(token.text()), tok_class);
  };

  //------------------------------------------------------------
  // Viterbi: single iteration: (TokID,LexClass=set<ClassID>)
  /**
   * Step a single Viterbi iteration, considering only the tags
   * in \c lexclass -- useful if you have some a priori information
   * on the token.
   */
  inline void viterbi_step(TokID tokid, const LexClass &lexclass)
  {
    if (use_lex_classes) {
      if (lexclass.empty()) {
	nunclassed++;
	viterbi_step(tokid, 0, uclass);
      } else {
	//-- non-empty class : get ID (assign empty distribution if unknown)
	ClassID classid = class2id(lexclass,0,1);
	viterbi_step(tokid,classid,lexclass);
      }
    } else {
      //-- !use_lex_classes
      if (lexclass.empty()) {
	nunclassed++;
	viterbi_step(tokid);
      } else {
	viterbi_step(tokid,0,lexclass);
      }
    }
  };

  //------------------------------------------------------------
  // Viterbi: single iteration: (TokID,ClassID,LexClass)
  /**
   * Step a single Viterbi iteration, considering only the tags
   * in \c lclass
   */
  void viterbi_step(TokID tokid, ClassID classid, const LexClass &lclass);

  //------------------------------------------------------------
  // Viterbi: single iteration: (TokID)
  /**
   * Step a single Viterbi iteration, considering all known tags
   * for \c tokid as possible analyses.  May be faster in cases
   * where no futher information (i.e. set of possible tags) is
   * available.
   */
  void viterbi_step(TokID tokid);

  //------------------------------------------------------------
  // Viterbi: single iteration: (TokString)
  /**
   * \bold DEPRECATED in favor of \c viterbi_step(mootToken)
   *
   * Step a single Viterbi iteration, string version.
   * Really just a wrapper for \c viterbi_step(TokID tokid).
   */
  inline void viterbi_step(const mootTokString &token_text) {
    return viterbi_step(token2id(token_text));
  };

  //------------------------------------------------------------
  // Viterbi: single iteration: (TokString,set<TagString>)
  /**
   * \bold DEPRECATED
   *
   * Step a single Viterbi iteration, considering only the tags in \c tags.
   */
  inline void viterbi_step(const mootTokString &token_text, const set<mootTagString> &tags)
  {
    LexClass lclass;
    tagset2lexclass(tags,&lclass);
    viterbi_step(token2id(token_text), lclass);
  };

  //------------------------------------------------------------
  // Viterbi: single iteration: (TokID,TagID,col=NULL)
  /**
   * Step a single Viterbi iteration, considering only the tag \c tagid.
   */
  void viterbi_step(TokID tokid, TagID tagid, ViterbiColumn *col=NULL);

  //------------------------------------------------------------
  // Viterbi: single iteration: (TokString,TagString)
  /**
   * \bold DEPRECATED
   *
   * Step a single Viterbi iteration, considering only the tag \c tag : string version.
   */
  inline void viterbi_step(const mootTokString &token, const mootTagString &tag)
  {
    return viterbi_step(token2id(token), tagids.name2id(tag));
  };


  //------------------------------------------------------------
  // Viterbi: finish
  /**
   * Run final Viterbi iteration, using \c final_tagid as the boundary tag
   */
  inline void viterbi_finish(const TagID final_tagid)
  {
    viterbi_step(0, final_tagid);
  };

  /**
   * Run final Viterbi iteration, using instance datum \c start_tagid as the final tag.
   */
  inline void viterbi_finish(void)
  {
    viterbi_step(0, start_tagid);
  };

  /**
   * Mid-level tagging interface: mark 'best' tags in sentence
   * structure: fills \c besttag datum of each \c mootToken element
   * of \c sentence.  Before calling this method, you should
   * have done following:
   *
   * \li called \c viterbi_clear() to initialize the Viterbi trellis.
   * \li called \c viterbi_step(mootToken) once for each element of \c sentence.
   * \li called \c viterbi_finish() to push the boundary tag onto the Viterbi trellis.
   */
  void tag_mark_best(mootSentence &sentence);
  //@}


  //------------------------------------------------------------
  // Viterbi: Low/Mid-level: path utilities
  /** \name Low/Mid-Level Viterbi Path Utilties  */
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

  /**
   * Get best current node from Viterbi state tables, considering all
   * possible current tags (all rows in current column).  The best full
   * path to this node can be reconstructed (in reverse order) by
   * traversing the \c pth_prev pointers until \c (pth_prev==NULL) .
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
  // Viterbi: Low/Mid: node-to-path conversion
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
  // public methods: low-level: Viterbi

  /** \name Low-level Viterbi iteration utilities */
  //{@
  /**
   * Find the best previous node from top column of 'vtable' for destination tag 'curtagid',
   * stores a pointer to the best previous node in 'vbestpn', and the
   * (adjusted) n-gram transition probability in 'vbestpr'.
   *
   * \note lexical probabilites are ignored for this computation, since they're
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
  // Viterbi: Low-level: clear best-path
  /** Clear internal \c vbestpath temporary */
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
   * all tags in tagset.  Implicitly called by other viterbi_step()
   * methods.
   */
  void _viterbi_step_fallback(TokID tokid, ViterbiColumn *col);
  //@}


  //------------------------------------------------------------
  /** \name Low-level Trash-stack Utilities */

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
  // Viterbi: trash utilities: columns
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
  // Viterbi: trash utilities: path-nodes
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
  // Low-level: ID Lookup
  /** \name ID Lookup */
  //@{
  /** Get the TokID for a given token, using type-based lookup */
  inline TokID token2id(const mootTokString &token) const
  {
    TokenType typ = token2type(token);
    return typids[typ]==0 ? tokids.name2id(token) : typids[typ];
  };

  /**
   * Convert string-form tagsets to lexical classes.
   * If \c add_tagids is true, then tag-IDs will
   * be assigned as needed for the element tags.
   * If you specify \c NULL as the lexical class, a
   * new one will be allocated and returned (you must
   * then delete it yourself!)
   *
   * \note \c lclass is NOT cleared by this method.
   */
  inline LexClass *tagset2lexclass(const mootTagSet &tagset,
				   LexClass *lclass=NULL,
				   bool add_tagids=false)
  {
    if (!lclass) lclass = new LexClass();
    //-- ... for all tags in the class (utsi)
    for (mootTagSet::const_iterator tsi = tagset.begin();
	 tsi != tagset.end();
	 tsi++)
      {
	//-- lookup or assign a tag id
	TagID tagid = tagids.name2id(*tsi);
	if (add_tagids && tagid==0) tagid = tagids.insert(*tsi);

	//-- insert tagid into lexical class
	lclass->insert(tagid);
      }
    return lclass;
  };


  /**
   * Lookup the ClassID for the lexical-class \c lclass.
   * @param autopopulate if true, new classes will be autopopulated with uniform distributions (implies \c autocreate).
   * @param autocreate if true, new classes will be created and assigned class-ids.
   */
  inline ClassID class2id(const LexClass &lclass,
			  bool autopopulate=true,
			  bool autocreate=true)
  {
    ClassID cid = classids.name2id(lclass);
    if (cid == 0) {
      nnewclasses++;
      if (!autopopulate && !autocreate) return cid;  //-- map unknown classes to zero

      //-- previously unknown class: fill 'er up with default values
      cid = classids.insert(lclass);
      if (cid >= lcprobs.size()) {
	n_classes = cid+1;

	//-- resize() should really happen 2 lines down,
	//   but that might break something : test this at some point!
	lcprobs.resize(n_classes);
      }
      if (autopopulate) {
	LexClassProbSubTable &lcps = lcprobs[cid];
	if (!lclass.empty()) {
	  //-- non-empty class: restrict population to class-members
	  ProbT lcprob = 1.0/((ProbT)lclass.size());
	  for (LexClass::const_iterator lci = lclass.begin(); lci != lclass.end(); lci++) {
	    lcps[*lci] = lcprob;
	  }
	} else {
	  //-- empty class: use class for "unknown" token instead [HACK!]
	  const LexProbSubTable &lps = lexprobs[0];
	  ProbT lpprob = 1.0/((ProbT)lps.size());
	  for (LexProbSubTable::const_iterator lpsi = lps.begin(); lpsi != lps.end(); lpsi++) {
	    lcps[lpsi->first] = lpprob;
	  }
	}
      }
    }
    return cid;
  };
  //@}


  //------------------------------------------------------------
  /** \name Probability Lookup */
  //@{

  /*------------------------------------------------------------
   * Lexical Probability Lookup
   */
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

  /*------------------------------------------------------------
   * Lexical-Class Probability Lookup
   */
  /**
   * Looks up and returns lexical-class probability: p(classid|tagid)
   */
  inline const ProbT classp(const ClassID classid, const TagID tagid) const
  {
    if (classid >= lcprobs.size()) return 0;
    const LexClassProbSubTable &lps = lcprobs[classid];
    LexClassProbSubTable::const_iterator lpsi = lps.find(tagid);
    return lpsi != lps.end() ? lpsi->second : 0;
  };

  /**
   * \bold DEPRECATED
   *
   * Looks up and returns lexical-class probability: p(class|tag)
   * given class, tag -- no id auto-generation is performed!
   */
  inline const ProbT classp(const LexClass &lclass, const mootTagString tag) const
  {
    return classp(classids.name2id(lclass), tagids.name2id(tag));
  };

  /*------------------------------------------------------------
   * Unigram Probability Lookup
   */
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

  /*------------------------------------------------------------
   * Bigram Probability Lookup
   */
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

  /*------------------------------------------------------------
   * Trigram probability lookup
   */
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
  // Error Reporting

  /** \name Error reporting */
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
