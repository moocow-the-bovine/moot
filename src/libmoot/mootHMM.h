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

#ifndef _MOOT_HMM_H
#define _MOOT_HMM_H

#ifdef __GNUC__
# include <float.h>
#endif // __GNUC__

#include <string.h>
#include <ctype.h>

#include <mootTypes.h>
#include <mootIO.h>
#include <mootZIO.h>
#include <mootToken.h>
#include <mootTokenIO.h>
#include <mootLexfreqs.h>
#include <mootClassfreqs.h>
#include <mootNgrams.h>
#include <mootEnum.h>

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

/** \def MOOT_PROB_ZERO
 * Probability lower-bound
 *
 * \def MOOT_PROB_ONE
 * Probability upper-bound
 */
#ifdef DBL_MAX
#  define MOOT_PROB_NEG  -DBL_MAX
#  define MOOT_PROB_ZERO -1E+38
#  define MOOT_PROB_ONE   0.0
# else //-- !DBL_MAX
#  define MOOT_PROB_NEG  -3E+38
#  define MOOT_PROB_ZERO -1E+38
#  define MOOT_PROB_ONE   0.0
#endif //-- /DBL_MAX

/**
 * \def MOOT_LEX_UNKNOWN_TOKENS
 * Define this to include real lexical entries for tokens with counts
 * <= UnknownLexThreshhold.  Not entirely correct, but it actually
 * seems to help.
 */
#define MOOT_LEX_UNKNOWN_TOKENS
//#undef MOOT_LEX_UNKNOWN_TOKENS

/**
 * \def MOOT_USE_TRIGRAMS
 * If defined, trigrams will be used in addition to uni- and bi-grams.
 */

/**
 * \def MOOT_RELAX
 * If defined, pre-analyses will be used only as "hints" ; otherwise,
 * they are considered hard restrictions.
 */

/**
 * \def MOOT_HASH_TRIGRAMS
 * If defined (and if MOOT_USE_TRIGRAMS is defined), then
 * trigrams will be stored in a hash table.
 * Otherwise, trigrams will be stored in a sparse and
 * memory-hogging (but fast) C array
 */

/**
 * \def MOOT_LEX_UNKNOWN_CLASSES
 * Define this to include real lexical-class entries for classes with counts
 * <= UnknownClassThreshhold.  Not entirely kosher, but it works well, so
 * what the hey...
 */
#define MOOT_LEX_UNKNOWN_CLASSES
//#undef MOOT_LEX_UNKNOWN_CLASSES


moot_BEGIN_NAMESPACE

/*--------------------------------------------------------------------------
 * mootHMM : HMM class
 *--------------------------------------------------------------------------*/

/**
 * \brief 1st-order Hidden Markov Model Tagger/Disambiguator class.
 *
 * All probabilities are stored internally as logarithms: this saves
 * us a bit of runtime, and helps avoid datatype underflows.
 */
class mootHMM {
public:
  /*---------------------------------------------------------------------*/
  /** \name Atomic Types */
  //@{

  /** Symbolic verbosity level typedef */
  typedef enum {
    vlSilent,     /**< Be silent */
    vlErrors,     /**< Report errors */
    vlWarnings,   /**< Report warnings */
    vlProgress,   /**< Report progess */
    vlEverything  /**< Report everything we can */
  } VerbosityLevel;


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

  /** Type for lexical probability lookup subtable: \c tagid=>log(p(·|tagid)) */
  typedef map<TagID,ProbT> LexProbSubTable;

  /**
   * Type for lexical-class probability lookup subtable:
   * \c tagid=>log(p(·|tagid))
   */
  typedef LexProbSubTable LexClassProbSubTable;

  /**
   * Type for lexical probability lookup table: \c tokid=>(tagid=>log(p(tokid|tagid)))
   */
  typedef vector<LexProbSubTable> LexProbTable;

  /**
   * Type for lexical-class probability lookup table:
   * \c classid=>(tagid=>log(p(classid|tagid)))
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
   * Type for uni- and bigram probability lookup table:
   * c-style 2d array: bigram probabilites \c log(p(tagid|ptagid))
   * indexed by \c ((ntags*ptagid)+tagid) , and
   * unigram probabilities \c log(p(tagid)) indexed by \c tagid .
   *
   * This winds up being a rather sparse table,
   * but it should fit well in memory even for large
   * (~= 2K tags) tagsets on contemporary machines,
   * and lookup is Just Plain Quick.
   */
  typedef ProbT *BigramProbTable;

#if defined(MOOT_USE_TRIGRAMS)
# if defined(MOOT_HASH_TRIGRAMS)
  /// \brief Key type for a uni-, bi-, or trigram
  class Trigram {
  public:

    /// Utility struct for hash_map
    struct HashFcn {
    public:
      inline size_t operator()(const Trigram &x) const
      {
	return
	  (0xdeece66d * ((0xdeece66d * x.tag1) + x.tag2)) + x.tag3;
      };
    };

    /// Utility struct for hash_map
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
    TagID tag1;  ///< previous-previous tag_{i-2} or 0
    TagID tag2;  ///< previous tag: tag_{i-1} or 0
    TagID tag3;  ///< current tag: tag_i

  public:
    /// Trigram constructor
    Trigram(TagID t1=0, TagID t2=0, TagID t3=0)
      : tag1(t1), tag2(t2), tag3(t3)
    {};

    /// Trigram destructor
    ~Trigram(void) {};
  };

  /// Type for a trigram probability lookup table
  /// : trigram(t1,t2,t3)->log(p(t3|<t1,t2>))
  typedef
    hash_map<Trigram,ProbT,
	     Trigram::HashFcn,
	     Trigram::EqualFcn>
    TrigramProbTable;

# else //! MOOT_HASH_TRIGRAMS

  /**
   * \brief Type for uni-, bi- and trigram probability lookup table.
   *
   * C-style 3d array:
   *
   * trigram probabilities \c log(p(tagid|pptagid,ptagid))
   * indexexed by \c ((n_tags*((ntags*pptagid)+ptagid))+tagid)
   *
   * bigram probabilites \c log(p(tagid|ptagid))
   * indexed by \c ((ntags*ptagid)+tagid)
   *
   * unigram probabilities \c log(p(tagid)) indexed by \c tagid .
   *
   * This winds up being a rather sparse table,
   * but it should fit well in memory even for small
   * (~= 200 tags) tagsets on contemporary machines,
   * and lookup is Just Plain Quick.
   */
  typedef ProbT* TrigramProbTable;
# endif // MOOT_HASH_TRIGRAMS

#endif // MOOT_USE_TRIGRAMS
  //@}

  /*---------------------------------------------------------------------*/
  /** \name Viterbi Trellis Types  */
  //@{

  /** \brief Type for a Viterbi trellis entry ("pillar") node
   *
   * Viterbi trellis is a
   * (reverse-linked-list of columns [words]
   *   (of linked-list rows [current-tags]
   *    (of linked-list "pillars" [previous-tags])))
   */
  class ViterbiNode {
  public:
    TagID tagid;                  ///< Current Tag-ID for this node
#ifdef MOOT_USE_TRIGRAMS
    TagID ptagid;                 ///< Previous Tag-ID for this node
#endif
    ProbT lprob;                  ///< log-Probability of best path to this node

    class ViterbiNode *pth_prev;  ///< Previous node in best path to this node
    class ViterbiNode *nod_next;  ///< Next previous-tag-node in current pillar
  };

#ifdef MOOT_USE_TRIGRAMS
  /** \brief Type for a Viterbi trellis row ("current tag") node
   *
   * A Viterbi trellis row is completely specified by its Tag-ID
   * and corresponding "pillar" of previous Tag-IDs.
   */
  class ViterbiRow {
  public:
    TagID  tagid;                 ///< Current Tag-ID for this node (redundant)
    class ViterbiNode *nodes;     ///< Trellis "pillar" node(s) for this row
    class ViterbiRow  *row_next;  ///< Next row
  };
#else
  typedef ViterbiNode ViterbiRow; ///< Alias for a Viterbi trellis row ("current tag") node
#endif


  /**
   * \brief Type for a Viterbi trellis column.
   *
   * A Viterbi trellis is completely represented by its (current)
   * final column (top of the stack).
   */
  class ViterbiColumn {
  public:
    ViterbiRow    *rows;     ///< Column rows
    ViterbiColumn *col_prev; ///< Previous column
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
    ViterbiNode      *node;      /** Corresponding pillar-level trellis node */
    ViterbiPathNode  *path_next; /** Next node in this path */
  };
  //@}


public:
  /*---------------------------------------------------------------------*/
  /** \name I/O-related Flags */
  //@{
  /**
   * Verbosity level.  See \c VerbosityLevel typedef.
   * Default=1.  Not yet respected by all warnings.
   */
  int verbose;

  /**
   * Print a dot for every \c ndots tokens processed if reporting progess.
   * Default=0 (no dot printing).
   */
  size_t ndots;
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
   * Default = \c "__$" .
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
  ProbT             nglambda1;    /**< (log) Smoothing constant for unigrams */
  ProbT             nglambda2;    /**< (log) Smoothing constant for bigrams */
#ifdef MOOT_USE_TRIGRAMS
  ProbT             nglambda3;    /**< (log) Smoothing constant for trigrams */
#endif
  ProbT             wlambda0;     /**< (log) Smoothing constant for lexical probabilities */
  ProbT             wlambda1;     /**< (log) Smoothing constant for lexical probabilities */

  ProbT             clambda0;     /**< (log) Smoothing constant for class probabilities */
  ProbT             clambda1;     /**< (log) Smoothing constant for class probabilities */
  //@}

  /*---------------------------------------------------------------------*/
  /** \name ID Lookup Tables */
  //@{
  TokIDTable        tokids;     /**< Token-ID lookup table */
  TagIDTable        tagids;     /**< Tag-ID lookup table */
  ClassIDTable      classids;   /**< Class-ID lookup table */

  /* TokenFlavor to TokenID lookup table for non-alphabetics */
  TokID             flavids[NTokFlavors];
  //@}

  /*---------------------------------------------------------------------*/
  /** \name Probability Lookup Tables */
  //@{
  size_t            n_tags;     /**< Number of known tags: used to compute lookup indices */
  size_t            n_toks;     /**< Number of known tokens: used for sanity checks */
  size_t            n_classes;  /**< Number of known lexical classes */

  LexProbTable      lexprobs;   /**< Lexical probability lookup table */
  LexClassProbTable lcprobs;    /**< Lexical-class probability lookup table */
#ifdef MOOT_USE_TRIGRAMS
  TrigramProbTable  ngprobs3;   /**< N-gram (log-)probability lookup table: trigrams */
#else
  BigramProbTable   ngprobs2;   /**< N-gram (log-)probability lookup table: bigrams */
#endif
  //@}

  /*---------------------------------------------------------------------*/
  /** \name Viterbi Trellis Data */
  //@{
  ViterbiColumn     *vtable;    /**< Low-level trellis structure for Viterbi algorithm */
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
  ViterbiNode     *trash_nodes;     /**< Recycling bin for Viterbi trellis nodes */
#ifdef MOOT_USE_TRIGRAMS
  ViterbiRow      *trash_rows;      /**< Recycling bin for Viterbi trellis rows */
#endif
  ViterbiColumn   *trash_columns;   /**< Recycling bin for Viterbi trellis columns */
  ViterbiPathNode *trash_pathnodes; /**< Recycling bin for Viterbi path-nodes */
  //@}

  /*---------------------------------------------------------------------*/
  /** \name Low-level data: temporaries */
  //@{
  TagID             vtagid;     /**< Current tag-id under consideration for viterbi_step() */
  ProbT             vbestpr;    /**< Best (log-)probability for viterbi_step() */
  ProbT             vtagpr;     /**< (log-)Probability for current tag-id for viterbi_step() */
  ProbT             vwordpr;    /**< Save (log-)word-probability */
  ViterbiNode      *vbestpn;    /**< Best previous node for viterbi_step() */

  ViterbiPathNode  *vbestpath;  /**< For node->path conversion */
  //@}

public:
  /*---------------------------------------------------------------------*/
  /** \name Constructor / Destructor */
  //@{
  /** Default constructor */
  mootHMM(void);

  /** Destructor */
  ~mootHMM(void) { clear(); };
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
  bool save(const char *filename, int compression_level=MOOT_DEFAULT_COMPRESSION);

  /** Save to a binary stream */
  bool save(mootio::mostream *obs, const char *filename=NULL);

  /** Low-level: save guts to a binary stream */
  bool _bindump(mootio::mostream *obs, const char *filename=NULL);

  /** Load from a binary file */
  bool load(const char *filename=NULL);

  /** Load from a binary stream */
  bool load(mootio::mistream *ibs, const char *filename=NULL);

  /** Low-level: load guts from a binary stream */
  bool _binload(mootio::mistream *ibs, const char *filename=NULL);
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
   * Top-level: load and compile a single model, and estimate all
   * smoothing constants.  Returns true on success, false on failure.
   *
   * @param modelname is a model name following the conventions in mootfiles(5)
   * @param start_tag_str is the string form of the boundary tag.
   * @param myname name to use for warnings/errors/info
   *
   * If you want to load multiple models, you will need to first
   * load the raw-freqency objects, then call the compile(),
   * estimate_*(), and compute_logprobs() methods yourself.
   */
  bool load_model(const string &modelname,
		  const mootTagString &start_tag_str="__$",
		  const char *myname="mootHMM::load_model()");

  /**
   * Compile
   * probabilites from raw frequency counts in 'lexfreqs' and 'ngrams'.
   * Returns false on failure.
   */
  bool compile(const mootLexfreqs &lexfreqs,
	       const mootNgrams &ngrams,
	       const mootClassfreqs &classfreqs,
	       const mootTagString &start_tag_str="__$");

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

  /** Pre-compute runtime probability tables: NOT called by compile(). */
  bool compute_logprobs(void);
  //@}

  //------------------------------------------------------------
  // Tagging: Top-level
  /** \name Top-level Tagging Interface */
  //@{

  /** Top-level tagging interface: TokenIO layer */
  void tag_io(TokenReader *reader, TokenWriter *writer)
  {
    int rtok;
    mootSentence *sent;
    while (reader && (rtok = reader->get_sentence()) != TokTypeEOF) {
      sent = reader->sentence();
      if (!sent) continue;
      tag_sentence(*sent);
      if (writer) writer->put_sentence(*sent);
    }
  };

  /** Top-level tagging interface: string input, file output */
  //void tag_strings(int argc, char **argv, FILE *out=stdout);

  /**
   * Top-level tagging interface: mootSentence input & output (destructive).
   * Calling this method will (re-)populate the \c besttag
   * datum in the \c sentence argument.
   */
  inline void tag_sentence(mootSentence &sentence) {
    viterbi_clear();
    for (mootSentence::const_iterator si = sentence.begin();
	 si != sentence.end();
	 si++)
      {
	viterbi_step(*si);
	if (ndots && (ntokens % ndots)==0) fputc('.', stderr);
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
    if (token.toktype() != TokTypeVanilla) return; //-- ignore non-vanilla tokens
    ntokens++;
    LexClass tok_class;
    for (mootToken::Analyses::const_iterator ani = token.analyses().begin();
	 ani != token.analyses().end();
	 ani++)
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
    vbestpr = MOOT_PROB_NEG;
    vbestpn = NULL;

#ifdef MOOT_USE_TRIGRAMS
    ViterbiRow  *prow;
    for (prow = vtable->rows; prow != NULL; prow = prow->row_next) {
      for (pnod = prow->nodes; pnod != NULL; pnod = pnod->nod_next) {
	if (pnod->lprob > vbestpr) {
	  vbestpr = pnod->lprob;
	  vbestpn = pnod;
	}
      }
    }
#else // !MOOT_USE_TRIGRAMS
    for (pnod = vtable->rows; pnod != NULL; pnod = pnod->nod_next) {
      if (pnod->lprob > vbestpr) {
	vbestpr = pnod->lprob;
	vbestpn = pnod;
      }
    }
#endif // MOOT_USE_TRIGRAMS
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
    vbestpr = MOOT_PROB_NEG;
#ifdef MOOT_USE_TRIGRAMS
    ViterbiRow  *prow;
    vbestpn = NULL;
    for (prow = vtable->rows; prow != NULL; prow = prow->row_next) {
      if (prow->tagid == tagid) {
	for (pnod = prow->nodes; pnod != NULL; pnod = pnod->nod_next) {
	  if (pnod->lprob > vbestpr) {
	    vbestpr = pnod->lprob;
	    vbestpn = pnod;
	  }
	}
	return vbestpn;
      }
    }
#else // !MOOT_USE_TRIGRAMS
    for (pnod = vtable->rows; pnod != NULL; pnod = pnod->nod_next) {
      if (pnod->tagid == tagid) return pnod;
    }
#endif // MOOT_USE_TRIGRAMS
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

  /** Returns true iff @col is a valid (non-empty) Viterbi trellis column */
  inline bool viterbi_column_ok(const ViterbiColumn *col) const {
    return (col
	    && col->rows 
#ifdef MOOT_USE_TRIGRAMS
	    && col->rows->nodes
#endif
	    );
  };

  /**
   * Get and populate a new Viterbi-trellis row in column @col for destination Tag-ID
   * @curtagid with lexical (log-)probability @wordpr.
   * If @col is NULL (the default), a new column will be allocated.
   * Returns a pointer to the trellis column, or NULL on failure.
   */
  inline ViterbiColumn *viterbi_populate_row(TagID curtagid,
					     ProbT wordpr=MOOT_PROB_ONE,
					     ViterbiColumn *col=NULL)
  {
#ifdef MOOT_USE_TRIGRAMS
    ViterbiRow  *prow, *row = viterbi_get_row();
    ViterbiNode *pnod, *nod = NULL;

    if (!col) {
      col           = viterbi_get_column();
      col->rows     = NULL;
    }
    col->col_prev = vtable;
    row->nodes = NULL;

    for (prow = vtable->rows; prow != NULL; prow = prow->row_next) {
      vbestpr = MOOT_PROB_NEG;
      vbestpn = NULL;

      for (pnod = prow->nodes; pnod != NULL; pnod = pnod->nod_next) {
	vtagpr = pnod->lprob + tagp(pnod->ptagid, prow->tagid, curtagid);
	if (vtagpr > vbestpr) {
	  vbestpr = vtagpr;
	  vbestpn = pnod;
	}
      }

      //-- set node information
      nod = viterbi_get_node();
      nod->tagid    = curtagid;
      nod->ptagid   = prow->tagid;
      nod->lprob    = vbestpr + wordpr;
      //nod->row      = row;
      nod->pth_prev = vbestpn;
      nod->nod_next = row->nodes;

      row->nodes    = nod;
    }

    //-- set row information
    row->tagid    = curtagid;
    row->row_next = col->rows;
    col->rows     = row;

#else //! MOOT_USE_TRIGRAMS

    ViterbiNode *pnod, *nod = NULL;

    if (!col) {
      col           = viterbi_get_column();
      col->rows     = NULL;
    }
    col->col_prev = vtable;

    vbestpr = MOOT_PROB_NEG;
    vbestpn = NULL;

    for (pnod = vtable->rows; pnod != NULL; pnod = pnod->nod_next) {
      vtagpr = pnod->lprob + tagp(pnod->tagid, curtagid);
      if (vtagpr > vbestpr) {
	vbestpr = vtagpr;
	vbestpn = pnod;
      }
    }

    //-- set node/row information
    nod           = viterbi_get_node();
    nod->tagid    = curtagid;
    nod->lprob    = vbestpr + wordpr;
    nod->pth_prev = vbestpn;
    nod->nod_next = col->rows;

    //-- set row/col information
    nod->nod_next = col->rows;
    col->rows     = nod;

#endif // MOOT_USE_TRIGRAMS

    return col;
  };


  //------------------------------------------------------------
  // Viterbi: Low-level: clear best-path
  /** Clear internal @vbestpath temporary */
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
      nod         = trash_nodes;
      trash_nodes = nod->nod_next;
    } else {
      nod = new ViterbiNode();
    }
    return nod;
  };

  //------------------------------------------------------------
  // Viterbi: trash utilities: Rows
  /** Returns a pointer to an unused ViterbiRow, possibly allocating a new one. */
  inline ViterbiRow *viterbi_get_row(void) {
#ifdef MOOT_USE_TRIGRAMS
    ViterbiRow *row;
    if (trash_rows != NULL) {
      row        = trash_rows;
      trash_rows = row->row_next;
    } else {
      row = new ViterbiRow();
    }
    return row;
#else
    return viterbi_get_node();
#endif //MOOT_USE_TRIGRAMS
  };

  //------------------------------------------------------------
  // Viterbi: trash utilities: columns
  /** Returns a pointer to an unused ViterbiColumn, possibly allocating a new one. */
  inline ViterbiColumn *viterbi_get_column(void) {
    ViterbiColumn *col;
    if (trash_columns != NULL) {
      col           = trash_columns;
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
    mootTokenFlavor flav = tokenFlavor(token);
    return flavids[flav]==0 ? tokids.name2id(token) : flavids[flav];
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
	  ProbT lcprob = log(1.0/((ProbT)lclass.size()));

	  for (LexClass::const_iterator lci = lclass.begin(); lci != lclass.end(); lci++) {
	    lcps[*lci] = lcprob;
	  }
	} else {
	  //-- empty class: use class for "unknown" token instead [HACK!]
	  const LexProbSubTable &lps = lexprobs[0];
	  ProbT lpprob = log(1.0/((ProbT)lps.size()));

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
    if (tokid >= lexprobs.size()) return MOOT_PROB_ZERO;
    const LexProbSubTable &lps = lexprobs[tokid];
    LexProbSubTable::const_iterator lpsi = lps.find(tagid);
    return lpsi != lps.end() ? lpsi->second : MOOT_PROB_ZERO;
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
    if (classid >= lcprobs.size()) return MOOT_PROB_ZERO;
    const LexClassProbSubTable &lps = lcprobs[classid];
    LexClassProbSubTable::const_iterator lpsi = lps.find(tagid);
    return lpsi != lps.end() ? lpsi->second : MOOT_PROB_ZERO;
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
#ifdef MOOT_USE_TRIGRAMS
      tagp(0,0,tagid);
#else
      ngprobs2 && tagid < n_tags
      ? ngprobs2[tagid]
      : MOOT_PROB_ZERO;
#endif // MOOT_USE_TRIGRAMS
  };

  /**
   * \bold DEPRECATED
   *
   * Looks up and returns unigram (log-)probability: log(p(tag)), string-version.
   */
  inline const ProbT tagp(const mootTagString &tag) const
  {
    return tagp(tagids.name2id(tag));
  };

  /*------------------------------------------------------------
   * Bigram Probability Lookup
   */
  /**
   * Looks up and returns bigram (log-)probability: log(p(tagid|prevtagid)),
   * given tagid, prevtagid.
   */
  inline const ProbT tagp(const TagID prevtagid, const TagID tagid) const
  {
    return
#ifdef MOOT_USE_TRIGRAMS
      tagp(0,prevtagid,tagid);
#else
      ngprobs2 && prevtagid < n_tags && tagid < n_tags
      ? ngprobs2[(n_tags*prevtagid)+tagid]
      : MOOT_PROB_ZERO;
#endif
  };

  /**
   * \bold DEPRECATED
   *
   * Looks up and returns bigram probability: log(p(tag|prevtag)), string-version.
   */
  inline const ProbT tagp(const mootTagString &prevtag, const mootTagString &tag) const
  {
    return tagp(tagids.name2id(prevtag), tagids.name2id(tag));
  };

  /*------------------------------------------------------------
   * Trigram probability lookup
   */
#ifdef MOOT_USE_TRIGRAMS
  /**
   * Looks up and returns trigram (log-)probability: log(p(tagid|prevtagid2,prevtagid1)),
   * given Trigram(prevtagid2,prevtagid1,tagid)
   *
   * \bold WORK IN PROGRESS
   */
#ifdef MOOT_HASH_TRIGRAMS
  inline const ProbT tagp(const Trigram &trigram, ProbT ProbZero=MOOT_PROB_ZERO) const
  {
    TrigramProbTable::const_iterator tgti = ngprobs3.find(trigram);
    return tgti != ngprobs3.end() ? tgti->second : ProbZero;
  };
#endif //MOOT_HASH_TRIGRAMS

  /**
   * Looks up and returns trigram (log-)probability: log(p(tagid|prevtagid2,prevtagid1)),
   * given prevtagid2, prevtagid1, tagid.
   *
   * \bold WORK IN PROGRESS
   */
  inline const ProbT tagp(const TagID prevtagid2, const TagID prevtagid1, const TagID tagid) const
  {
    return
#ifdef MOOT_HASH_TRIGRAMS
      tagp(Trigram(prevtagid2,prevtagid1,tagid))
#else
      ngprobs3 && prevtagid2 < n_tags && prevtagid1 < n_tags && tagid < n_tags
      ? ngprobs3[(n_tags*((n_tags*prevtagid2)+prevtagid1))+tagid]
      : MOOT_PROB_ZERO;
#endif
      ;
  };

  /**
   * \bold DEPRECATED
   *
   * Looks up and returns trigram (log-)probability: log(p(tag|prevtag1,prevtag2)), string-version.
   *
   * \bold WORK IN PROGRESS
   */
  inline const ProbT tagp(const mootTagString &prevtag2,
			  const mootTagString &prevtag1,
			  const mootTagString &tag)
    const
  {
    return tagp(tagids.name2id(prevtag2), tagids.name2id(prevtag1), tagids.name2id(tag));
  };
#endif // MOOT_USE_TRIGRAMS
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

#endif /* _MOOT_HMM_H */
