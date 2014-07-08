/* -*- Mode: C++ -*- */
/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2014 by Bryan Jurish <moocow@cpan.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/*--------------------------------------------------------------------------
 * File: mootHMM.h
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moot PoS tagger : Hidden Markov Model (Disambiguator): headers
 *--------------------------------------------------------------------------*/

/**
\file mootHMM.h
\brief Hidden Markov Model tagger/disambiguator
*/

#ifndef _MOOT_HMM_H
#define _MOOT_HMM_H

#include <math.h>

#include <mootFlavor.h>
#include <mootTokenIO.h>
#include <mootZIO.h>
#include <mootBinHeader.h>
#include <mootUtils.h>

#include <mootClassfreqs.h>
//#include <mootLexfreqs.h> //-- included by mootClassfreqs.h

#include <mootSuffixTrie.h>
/*
#ifdef MOOT_ENABLE_SUFFIX_TRIE
# include <mootSuffixTrie.h>
#else
# include <mootEnum.h>
# include <mootAssocVector.h>
# include <mootTrieVector.h>
# include <mootNgrams.h>
#endif //--MOOT_ENABLE_SUFFIX_TRUE
*/

/**
 * \def MOOT_USE_TRIGRAMS
 * \deprecated{Trigrams are always the base n-gram type}
 *
 * Formerly: if defined, trigrams will be used in addition to uni- and bi-grams.
 *
 * This macro should always be defined for moot >= v2.0.7-0, for backwards-compatibility.
 * Old behavior (<tt>!defined(MOOT_USE_TRIGRAMS)</tt>) can be simulated by model tweaking.
 * Newer code should not use this macro, as it may disappear in future versions.
 */

/**
 * \def MOOT_RELAX
 * \deprecated{Use mootHMM::relax instead}
 *
 * Formerly: If defined, pre-analyses will be used only as "hints" ; otherwise,
 * they are considered hard restrictions.
 *
 * As of moot v2.0.7-0, this macro should always be defined, for backwards-compatibility.
 * Interpretation of pre-analyses is now controlled by the mootHMM::relax field.
 * Newer code should not use this macro, as it may disappear in future versions.
 */

/**
 * \def MOOT_LEX_UNKNOWN_TOKENS
 * Define this to include real lexical entries for tokens with counts
 * <= UnknownLexThreshhold.  Not entirely correct, but it actually
 * seems to help.
 */
#define MOOT_LEX_UNKNOWN_TOKENS
//#undef MOOT_LEX_UNKNOWN_TOKENS

/**
 * \def MOOT_LEX_UNKNOWN_CLASSES
 * Define this to include real lexical-class entries for classes with counts
 * <= UnknownClassThreshhold.  Not entirely kosher, but it works well, so
 * what the hey...
 */
#define MOOT_LEX_UNKNOWN_CLASSES
//#undef MOOT_LEX_UNKNOWN_CLASSES

/**
 * \def MOOT_LEX_NONALPHA
 * Define this to include real lexical entries for "special"
 * (pattern-matched) tokens.
 * TnT for example seems to do this.
 */
#define MOOT_LEX_NONALPHA
//#undef MOOT_LEX_NONALPHA

/**
 * \def MOOT_LEX_IS_TIEBREAKER
 * Define this to use lexical probabilities to break ties.
 */
#undef MOOT_LEX_IS_TIEBREAKER

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

  /** Type for a tag-identifier. Zero indicates an unknown tag. */
  typedef mootEnumID TagID;

  /** Type for a token-identider. Zero indicates an unknown token. */
  typedef mootEnumID TokID;

  /** Type for a flavor-identider. 0 indicates a "normal" (e.g. alphabetic) token */
  typedef mootEnumID FlavorID;

  /**
   * Typedef for a lexical ClassID. Zero indicates
   * either a previously unknown class or the empty class.
   */
  typedef mootEnumID ClassID;

  /** Symbolic verbosity level typedef (for backwards-compatibility) */
  typedef moot::VerbosityLevel VerbosityLevel;
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
      for (LexClass::const_iterator xi = x.begin(); xi != x.end(); ++xi) {
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
  //typedef map<TagID,ProbT> LexProbSubTable;
  typedef AssocVector<TagID,ProbT> LexProbSubTable;

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
   * Type for unigram probability lookup table:
   * c-style 1d array:
   * unigram probabilities \c log(p(tagid)) indexed by \c tagid .
   *
   * Not currently used.
   */
  typedef ProbT* UnigramProbTable;

  /// Tag-trigram key type for HMM probability lookup table (only used if hash_ngrams is true)
  class Trigram {
  public:
    TagID tag1;  ///< previous-previous tag_{i-2} or 0
    TagID tag2;  ///< previous tag: tag_{i-1} or 0
    TagID tag3;  ///< current tag: tag_i
    //
  public:
    /// Utility struct for hash_map
    struct HashFcn {
    public:
      inline size_t operator()(const Trigram &x) const {
	return (0xdeece66d * ((0xdeece66d * x.tag1) + x.tag2)) + x.tag3;
      };
    };
    //
    /// Utility struct for hash_map
    struct EqualFcn {
    public:
      inline size_t operator()(const Trigram &x, const Trigram &y) const {
	return 
	  x.tag1==y.tag1 && x.tag2==y.tag2 && x.tag3==y.tag3;
      };
    };
    //
  public:
    /// Trigram constructor
    Trigram(TagID t1=0, TagID t2=0, TagID t3=0)
      : tag1(t1), tag2(t2), tag3(t3)
    {};
    //
    /// Trigram destructor
    ~Trigram(void) {};
  };

  /// Type for a trigram probability lookup table
  /// : trigram(t1,t2,t3) -> log(p(t3|<t1,t2>))
  /// : trigram(0 ,t2,t3) ~  bigram(t2,t3)
  typedef
    hash_map<Trigram,ProbT,
	     Trigram::HashFcn,
	     Trigram::EqualFcn>
    TrigramProbHash;

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
  typedef ProbT* TrigramProbArray;

  typedef Trigram          NgramProbKey;    ///< Generic n-gram key: trigrams
  typedef TrigramProbHash  NgramProbHash;   ///< Generic n-gram probabilities: trigrams, hashed
  typedef TrigramProbArray NgramProbArray;  ///< Generic n-gram probabilities: trigrams, dense
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
    TagID ptagid;                 ///< Previous Tag-ID for this node
    ProbT lprob;                  ///< log-Probability of best path to this node

    class ViterbiNode *pth_prev;  ///< Previous node in best path to this node
    class ViterbiNode *nod_next;  ///< Next previous-tag-node in current pillar
  };

  /** \brief Type for a Viterbi trellis row ("current tag") node
   *
   * A Viterbi trellis row is completely specified by its Tag-ID
   * and corresponding "pillar" of previous Tag-IDs.
   */
  class ViterbiRow {
  public:
    TagID  tagid;                 ///< Current Tag-ID for this node (redundant)
    ProbT  wprob;                 ///< (log-)lexical probability p(word|tag)
    class ViterbiNode *nodes;     ///< Trellis "pillar" node(s) for this row
    class ViterbiRow  *row_next;  ///< Next row
  };

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
    ProbT          bbestpr;  ///< Best probability in column for beam search
    ProbT          bpprmin;  ///< Minimum previous probability for beam search
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

  /**
   * Add contents of Viterbi trellis to \a analyses members of mootToken elements on tag_mark_best()
   */
  bool save_ambiguities;

  /**
   * Add flavor names to \a analyses members of mootToken elements on tag_mark_best()
   */
  bool save_flavors;

  /**
   * Mark unknown tokens with a single analysis '*' on tag_mark_best()
   */
  bool save_mark_unknown;
  //@}

  /*---------------------------------------------------------------------*/
  /** \name Useful Constants */
  //@{

  /**
   * Whether to store tag n-gram probabilities in a slow
   * but memory-friendly hash, as opposed to a dense array.
   * \warning Using the default (false) requires O(n_tags^3)
   * memory space for the tag n-gram probability table.
   */
  bool      hash_ngrams;

  /**
   * Whether to interpret token pre-analyses as "hints"
   * (relax==true) or hard restrictions (relax==false).
   *  Default (as of moot version >= 2.0.8-1): true.
   */
  bool      relax;

  /**
   * Whether to use class probabilities (Default=true)
   * \warning Don't set this to true unless your input
   * files actually contain a priori analyses generated
   * by the same method on which you trained your model;
   * otherwise, expect abominable accuracy.
   */
  bool      use_lex_classes;

  /**
   * Whether to use heuristic regex-based token "flavors" (Default=true).
   */
  bool      use_flavors;

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
  ProbT             nglambda3;    /**< (log) Smoothing constant for trigrams */
  ProbT             wlambda0;     /**< (log) Smoothing constant for lexical probabilities */
  ProbT             wlambda1;     /**< (log) Smoothing constant for lexical probabilities */

  ProbT             clambda0;     /**< (log) Smoothing constant for class probabilities */
  ProbT             clambda1;     /**< (log) Smoothing constant for class probabilities */

  /**
   * (log) Beam-search width: during Viterbi search,
   * heuristically prune paths whose probability is <= 1/beamwd*p_best
   * A value of zero indicates no beam pruning.
   */
  ProbT             beamwd;
  //@}

  /*---------------------------------------------------------------------*/
  /** \name ID Lookup Tables */
  //@{
  TokIDTable        tokids;     /**< Token-ID lookup table */
  TagIDTable        tagids;     /**< Tag-ID lookup table */
  ClassIDTable      classids;   /**< Class-ID lookup table */
  mootTaster	    taster;     /**< regex-based flavor heuristics */
  //@}

  /*---------------------------------------------------------------------*/
  /** \name Probability Lookup Tables */
  //@{
  size_t            n_tags;     /**< Number of known tags: used to compute lookup indices */
  size_t            n_toks;     /**< Number of known tokens: used for sanity checks */
  size_t            n_classes;  /**< Number of known lexical classes */

  LexProbTable      lexprobs;   /**< Lexical probability lookup table */
  LexClassProbTable lcprobs;    /**< Lexical-class probability lookup table */

  NgramProbHash     ngprobsh;   /**< N-gram (log-)probability lookup table: hashed */
  NgramProbArray    ngprobsa;   /**< N-gram (log-)probability lookup table: dense */

#ifdef MOOT_ENABLE_SUFFIX_TRIE
  SuffixTrie        suftrie;    /**< string-suffix (log-)probability trie */
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
  ViterbiRow      *trash_rows;      /**< Recycling bin for Viterbi trellis rows */
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

  //ProbT           bbestpr;   /**< Best current (log-)probability for beam pruning */
  //ProbT           bpprmin;   /**< Minimum previous probability for beam pruning */
  //@}

public:
  /*---------------------------------------------------------------------*/
  /** \name Constructor / Destructor */
  //@{
  /** Default constructor */
  mootHMM(void)
    : verbose(1),
      ndots(0),
      save_ambiguities(false),
      save_flavors(false),
      save_mark_unknown(false),
      hash_ngrams(false),
      relax(true),
      use_lex_classes(true),
      use_flavors(true),
      start_tagid(0),
      unknown_lex_threshhold(1.0),
      unknown_class_threshhold(1.0),
      nglambda1(mootProbEpsilon),
      nglambda2(1.0 - mootProbEpsilon),
      wlambda0(mootProbEpsilon),
      wlambda1(1.0 - mootProbEpsilon),
      clambda0(mootProbEpsilon),
      clambda1(1.0 - mootProbEpsilon),
      beamwd(1000),
      n_tags(0),
      n_toks(0),
      n_classes(0),
      ngprobsa(NULL),
      vtable(NULL),
      nsents(0),
      ntokens(0),
      nnewtokens(0),
      nunclassed(0),
      nnewclasses(0),
      nunknown(0),
      nfallbacks(0),
      trash_nodes(NULL),
      trash_rows(NULL),
      trash_columns(NULL), 
      trash_pathnodes(NULL),
      vbestpn(NULL),
      vbestpath(NULL)
  {
    //-- create special token entries
    unknown_token_name("@UNKNOWN");
    unknown_tag_name("UNKNOWN");
    uclass = LexClass();
  };


  /** Destructor */
  //~mootHMM(void) { clear(true,false); };
  virtual ~mootHMM(void) { clear(false,false); };
  //@}

  /*------------------------------------------------------------*/
  /** \name Reset / clear */
  //@{
  /**
   * Reset/clear the object, freeing all dynamic data structures.
   * If 'wipe_everything' is false, ID-tables and constants will
   * spared.
   */
  void clear(bool wipe_everything=true, bool unlogify=false);
  //@}

  /*------------------------------------------------------------*/
  /**\name Binary load / save */
  //@{
  /** Save to a binary file */
  bool save(const char *filename, int compression_level=MOOT_DEFAULT_COMPRESSION);

  /** Save to a binary stream */
  bool save(mootio::mostream *obs, const char *filename=NULL);

  /** Low-level: save guts to a binary stream */
  bool _bindump(mootio::mostream *obs, const mootBinIO::HeaderInfo &hdr, const char *filename=NULL);

  /** Load from a binary file */
  bool load(const char *filename=NULL);

  /** Load from a binary stream */
  bool load(mootio::mistream *ibs, const char *filename=NULL);

  /** Low-level: load guts from a binary stream */
  bool _binload(mootio::mistream *ibs, const mootBinIO::HeaderInfo &hdr, const char *filename=NULL);
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

  /**
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
   * @param do_estimate_nglambdas whether to estimate n-gram smoothing constants here
   * @param do_estimate_wlambdas whether to estimate lexical smoothing constants here
   * @param do_estimate_clambdas whether to estimate lexical class smoothing constants here
   * @param do_build_suffix_trie whether to build a suffix trie here
   * @param do_compute_logprobs whether to compute log-probabilities here
   *
   * If you want to load multiple models, you will need to first
   * load the raw-freqency objects, then call the \c compile(),
   * \c estimate_*(), \c build_suffix_trie(), and \c compute_logprobs()
   * methods yourself (i.e. set all of the \p do_* parameters to \c false).
   */
  virtual bool load_model(const string &modelname,
			  const mootTagString &start_tag_str="__$",
			  const char *myname="mootHMM::load_model()",
			  bool  do_estimate_nglambdas=true,
			  bool  do_estimate_wlambdas=true,
			  bool  do_estimate_clambdas=true,
			  bool  do_build_suffix_trie=true,
			  bool  do_compute_logprobs=true);

  /**
   * Compile
   * probabilites from raw frequency counts in 'lexfreqs' and 'ngrams'.
   * Returns false on failure.
   */
  virtual bool compile(const mootLexfreqs &lexfreqs,
		       const mootNgrams &ngrams,
		       const mootClassfreqs &classfreqs,
		       const mootTagString &start_tag_str="__$",
		       const mootTaster &mtaster=builtinTaster);

  /** Assign IDs for taster; called by compile().
   *  Allocates tokids for each flavor label and saves these in taster.
   */
  void assign_ids_fl(void);

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

  /** Build suffix trie for unknown-word handling: NOT called by compile(). */
  bool build_suffix_trie(const mootLexfreqs &lf,
			 const mootNgrams   &ng,
			 bool  verbose=false)
  {
#ifdef MOOT_ENABLE_SUFFIX_TRIE
    return suftrie.build(lf,ng,tagids,start_tagid,verbose);
#else
    return false;
#endif
  };

  /** Pre-compute runtime log-probability tables: NOT called by compile(). */
  bool compute_logprobs(void);

  /** Low-level utility: set a (raw) n-gram probability.  Used by compile() */
  inline void set_ngram_prob(ProbT p, TagID t1=0, TagID t2=0, TagID t3=0)
  {
    if (hash_ngrams) {   // +hash
      ngprobsh[Trigram(t1,t2,t3)] = p; 
    } else {             // -hash
      ngprobsa[(n_tags*((n_tags*t1)+t2))+t3] = p;
    }
  };
  //@}

  //------------------------------------------------------------
  // Tagging: Top-level
  /** \name Top-level Tagging Interface */
  //@{

  /**
   * Top-level tagging interface: mootSentence input & output (destructive).
   * Calling this method will (re-)populate the \c besttag
   * datum in the \c sentence argument.
   */
  void tag_sentence(mootSentence &sentence);

  /** Top-level tagging interface: TokenIO layer using sentence-level I/O */
  virtual void tag_io(TokenReader *reader, TokenWriter *writer);

  /** Top-level tagging interface: TokenIO layer using token-level I/O */
  virtual void tag_stream(TokenReader *reader, TokenWriter *writer);
  //@}

  /*====================================================================
   * VITERBI: Mid-level
   *====================================================================*/
  /** \name Mid-level Viterbi algorithm API */
  //@{

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /** Clear Viterbi state table(s) */
  void viterbi_clear(void);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Viterbi: single iteration: (mootToken)
  /**
   * Step a single Viterbi iteration, \c mootToken version.
   * Really just a wrapper for \c viterbi_step(TokID,set<TagID>).
   */
  inline void viterbi_step(const mootToken &token) {
    if (token.toktype() != TokTypeVanilla) return; //-- ignore non-vanilla tokens
    ++ntokens;
    LexClass tok_class;
    token2lexclass(token, tok_class);
    viterbi_step(token2id(token.text()), tok_class, token.text());
  };

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Viterbi: single iteration: (TokID,LexClass=set<ClassID>)
  /**
   * Step a single Viterbi iteration, considering only the tags
   * in \c lexclass -- useful if you have some a priori information
   * on the token.
   */
  inline void viterbi_step(TokID tokid,
			   const LexClass &lexclass,
			   const mootTokString &toktext="")
  {
    if (use_lex_classes) {
      if (lexclass.empty()) {
	++nunclassed;
	viterbi_step(tokid, 0, uclass, toktext);
      } else {
	//-- non-empty class : get ID (assign empty distribution if unknown)
	ClassID classid = class2id(lexclass,0,1);
	viterbi_step(tokid, classid, lexclass, toktext);
      }
    } else {
      //-- !use_lex_classes
      if (lexclass.empty()) {
	++nunclassed;
	viterbi_step(tokid, toktext);
      } else {
	viterbi_step(tokid, 0, lexclass, toktext);
      }
    }
  };

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Viterbi: single iteration: (TokID,ClassID,LexClass)
  /**
   * Step a single Viterbi iteration, considering only the tags
   * in \c lclass
   */
  void viterbi_step(TokID tokid,
		    ClassID classid,
		    const LexClass &lclass,
		    const mootTokString &toktext="");

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Viterbi: single iteration: (TokID)
  /**
   * Step a single Viterbi iteration, considering all known tags
   * for \c tokid as possible analyses.  May be faster in cases
   * where no futher information (i.e. set of possible tags) is
   * available.
   */
  void viterbi_step(TokID tokid, const mootTokString &toktext="");

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Viterbi: single iteration: (TokString)
  /**
   * \deprecated{prefer \c viterbi_step(mootToken)}
   *
   * Step a single Viterbi iteration, string version.
   * Really just a wrapper for \c viterbi_step(TokID tokid).
   */
  inline void viterbi_step(const mootTokString &token_text) {
    return viterbi_step(token2id(token_text), token_text);
  };

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Viterbi: single iteration: (TokString,set<TagString>)
  /**
   * \deprecated{prefer \c viterbi_step(mootToken)}
   *
   *  Step a single Viterbi iteration, considering only the tags in \c tags.
   */
  inline void viterbi_step(const mootTokString &token_text, const set<mootTagString> &tags)
  {
    LexClass lclass;
    tagset2lexclass(tags,&lclass);
    viterbi_step(token2id(token_text), lclass, token_text);
  };

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Viterbi: single iteration: (TokID,TagID,col=NULL)
  /**
   * \deprecated{prefer \c viterbi_step(mootToken)}
   *
   * Step a single Viterbi iteration, considering only the tag \c tagid.
   */
  void viterbi_step(TokID tokid, TagID tagid, ViterbiColumn *col=NULL);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Viterbi: single iteration: (TokString,TagString)
  /**
   * \deprecated{prefer \c viterbi_step(mootToken)}
   *
   * Step a single Viterbi iteration, considering only the tag \c tag : string version.
   */
  inline void viterbi_step(const mootTokString &toktext, const mootTagString &tag)
  {
    return viterbi_step(token2id(toktext), tagids.name2id(tag));
  };


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Viterbi: finish
  /**
   * Run final Viterbi iteration, using \c final_tagid as the boundary tag
   */
  inline void viterbi_finish(const TagID final_tagid)
  {
    viterbi_step(0, final_tagid);
  };

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /**
   * Run final Viterbi iteration, using instance datum \c start_tagid as the final tag.
   */
  inline void viterbi_finish(void)
  {
    viterbi_step(0, start_tagid);
  };

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Viterbi: finish
  /**
   * Run final Viterbi iteration, using \c final_tagid as the boundary tag
   */
  void viterbi_flush(TokenWriter *writer, mootSentence &toks, ViterbiNode *nod);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /**
   * Mid-level tagging interface: mark 'best' tags in sentence
   * structure for the current best Viterbi path as returned by viterbi_best_path().
   * Fills \c besttag datum of each \a TokTypeVanilla element
   * of \a sentence.  Before calling this method, you should
   * have done following:
   *
   * \li called \c viterbi_clear() to initialize the Viterbi trellis.
   * \li called \c viterbi_step(mootToken) once for each element of \c sentence.
   * \li called \c viterbi_finish() to push the boundary tag onto the Viterbi trellis.
   *
   * @param sentence (partial) sentence to mark; TokTypeVanilla elements should correspond 1:1 with the ViterbiColumn*s in \a vtable
   */
  inline void tag_mark_best(mootSentence &sentence)
  {
    tag_mark_best(viterbi_best_path(), sentence);
  };

  /**
   * Mid-level tagging interface: mark 'best' tags in sentence
   * structure for path \a pnod: fills \c besttag datum of each \a TokTypeVanilla element
   * of \a sentence.  Before calling this method, you should
   * have done following:
   *
   * \li called \c viterbi_clear() to initialize the Viterbi trellis.
   * \li called \c viterbi_step(mootToken) once for each element of \c sentence.
   * \li called \c viterbi_finish() to push the boundary tag onto the Viterbi trellis.
   *
   * @param pnod serialized best path for \a sentence
   * @param sentence (partial) sentence to mark; TokTypeVanilla elements should correspond 1:1 with the path in \a pnod
   */
  //@param skip_first if true (default), the first path node will be skipped (it's usually an implicit BOS marker) 
  void tag_mark_best(ViterbiPathNode *pnod, mootSentence &sentence);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /**
   * Mid-level tagging interface: dump verbose trace to \c sentence (destructive).
   * Calling this method will add verbose trace information as comments to \c sentence.
   * Same caveats as for tag_mark_best().
   */
  void tag_dump_trace(mootSentence &sentence, bool dumpPredict=false);

  //@}


  //------------------------------------------------------------
  // Viterbi: Low-Level: path utilities

  /** \name Low-Level Viterbi Path Utilties  */
  //@{

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /** Get current best path (in input order), considering all current tags */
  inline ViterbiPathNode *viterbi_best_path(void)
  {
    return viterbi_node_path(viterbi_best_node());
  };

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /** Get current best path (in input order), considering only tag 'tagid' */
  inline ViterbiPathNode *viterbi_best_path(TagID tagid)
  {
    return viterbi_node_path(viterbi_best_node(tagid));
  };

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /** Get current best path (in input order), considering only tag 'tag' */
  inline ViterbiPathNode *viterbi_best_path(const mootTagString &tagstr)
  {
    return viterbi_best_path(tagids.name2id(tagstr));
  };

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /**
   * Get best current node from Viterbi state tables, considering all
   * possible current tags (all rows in current column).  The best full
   * path to this node can be reconstructed (in reverse order) by
   * traversing the \c pth_prev pointers until \c (pth_prev==NULL) .
   */
  ViterbiNode *viterbi_best_node(void);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /**
   * Get best current path from Viterbi state tables resulting in tag 'tagid'.
   * The best full path to this node can be
   * reconstructed (in reverse order) by traversing the 'pth_prev'
   * pointers until (pth_prev==NULL).
   */
  ViterbiNode *viterbi_best_node(TagID tagid);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /** Check whether the current viterbi trellis is unambiguous
   *  Returns a pointer to the unique "best" current node in the Viterbi trellis
   *  if there is only one possible node currently under consideration after
   *  considering beam-pruning parameters, or NULL if not only a single
   *  node is currently active (i.e. if the best path can conceivably still
   *  "flop" away from the current best node.)
   *  \li returned node can be passed to e.g. viterbi_node_path()
   */
  ViterbiNode* viterbi_flushable_node(void);
 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /**
   * Useful utility: build a path (in input order) from a ViterbiNode.
   * See caveats for 'struct ViterbiPathNode' -- return value is non-const
   * for easy iteration.
   *
   * Uses 'vbestpath' to store constructed path.
   */
  ViterbiPathNode *viterbi_node_path(ViterbiNode *node);
  //@}

  //------------------------------------------------------------
  // Viterbi: low-level: iteration

  /** \name Low-level Viterbi iteration utilities */
  //{@

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /** Returns true iff \p col is a valid (non-empty) Viterbi trellis column */
  inline bool viterbi_column_ok(const ViterbiColumn *col) const
  {
    return (col && col->rows  && col->rows->nodes);
  };

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /**
   * Get and populate a new Viterbi-trellis row in column \p col for destination Tag-ID
   * \p curtagid with lexical (log-)probability \p wordpr.
   * If \p col is NULL (the default), a new column will be allocated.
   * \returns a pointer to the trellis column, or \c NULL on failure.
   *
   * If specified, \p probmin can be used to override beam-pruning
   * for non-NULL columns.
   */
  ViterbiColumn *viterbi_populate_row(TagID 		curtagid,
				      ProbT 		wordpr   =MOOT_PROB_ONE,
				      ViterbiColumn     *col	 =NULL,
				      ProbT 		probmin  =MOOT_PROB_NONE);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /** Clear internal \a vbestpath temporary */
  void viterbi_clear_bestpath(void);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /**
   * Step a single Viterbi iteration, last-ditch effort: consider
   * all tags in tagset.  Implicitly called by other viterbi_step()
   * methods.
   */
  void _viterbi_step_fallback(TokID tokid, ViterbiColumn *col);
  //@}


  //------------------------------------------------------------
  /** \name Low-level Viterbi trash-stack utilities */

  //@{

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /** Returns a pointer to an unused ViterbiNode, possibly allocating a new one. */
  inline ViterbiNode *viterbi_get_node(void)
  {
    ViterbiNode *nod;
    if (trash_nodes != NULL) {
      nod         = trash_nodes;
      trash_nodes = nod->nod_next;
    } else {
      nod = new ViterbiNode();
    }
    return nod;
  };

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /** Returns a pointer to an unused ViterbiRow, possibly allocating a new one. */
  inline ViterbiRow *viterbi_get_row(void)
  {
    ViterbiRow *row;
    if (trash_rows != NULL) {
      row        = trash_rows;
      trash_rows = row->row_next;
    } else {
      row = new ViterbiRow();
    }
    return row;
  };

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /** Returns a pointer to an unused ViterbiColumn, possibly allocating a new one. */
  inline ViterbiColumn *viterbi_get_column(void)
  {
    ViterbiColumn *col;
    if (trash_columns != NULL) {
      col           = trash_columns;
      trash_columns = col->col_prev;
    } else {
      col = new ViterbiColumn();
    }
    return col;
  };

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /** Returns a pointer to an unused ViterbiPathNode, possibly allocating a new one. */
  inline ViterbiPathNode *viterbi_get_pathnode(void)
  {
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

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /** Get the TokID for a given token, using type-based lookup */
  inline TokID token2id(const mootTokString &token) const
  {
#ifdef MOOT_LEX_NONALPHA
    TokID tokid = tokids.name2id(token);
    return tokid || !use_flavors ? tokid : taster.flavor_id(token);
#else
    TokID tokid = use_flavors ? taster.flavor_id(token) : 0;
    return tokid ? tokid : tokids.name2id(token);
#endif
  };

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /** Add \c tag fields of mootToken::analyses to \c tok_class */
  void token2lexclass(const mootToken &token, LexClass &tok_class) const;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
  LexClass *tagset2lexclass(const mootTagSet &tagset, LexClass *lclass=NULL, bool add_tagids=false);


  /**
   * Lookup the ClassID for the lexical-class \c lclass.
   * @param lclass lexical class whose ID is to be looked up
   * @param autopopulate if true, new classes will be autopopulated with uniform distributions (implies \c autocreate).
   * @param autocreate if true, new classes will be created and assigned class-ids.
   */
  ClassID class2id(const LexClass &lclass, bool autopopulate=true, bool autocreate=true);
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
    return lpsi != lps.end() ? lpsi->value() : MOOT_PROB_ZERO;
  };

  /**
   * \deprecated{prefer direct lookup}
   *
   * Looks up and returns lexical probability: p(tokstr|tagstr)
   * given token, tag.
   */
  inline const ProbT wordp(const mootTokString &tokstr, const mootTagString &tagstr) const
  {
    return wordp(token2id(tokstr), tagids.name2id(tagstr));
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
    return lpsi != lps.end() ? lpsi->value() : MOOT_PROB_ZERO;
  };

  /**
   * \deprecated{prefer direct lookup}
   *
   * Looks up and returns lexical-class probability: p(class|tag)
   * given class, tag -- no id auto-generation is performed!
   */
  inline const ProbT classp(const LexClass &lclass, const mootTagString &tagstr) const
  {
    return classp(classids.name2id(lclass), tagids.name2id(tagstr));
  };

  /*------------------------------------------------------------
   * Unigram Probability Lookup
   */
  /**
   * Looks up and returns unigram probability: p(tagid).
   */
  inline const ProbT tagp(const TagID tagid) const
  {
    return tagp(0,0,tagid);
  };

  /**
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
    return tagp(0,prevtagid,tagid);
  };

  /**
   * Looks up and returns bigram probability: log(p(tag|prevtag)), string-version.
   */
  inline const ProbT tagp(const mootTagString &prevtag, const mootTagString &tag) const
  {
    return tagp(tagids.name2id(prevtag), tagids.name2id(tag));
  };

  /*------------------------------------------------------------
   * Trigram probability lookup
   */
  /**
   * Looks up and returns raw trigram (log-)probability: log(p(tagid|prevtagid2,prevtagid1)),
   * given Trigram(prevtagid2,prevtagid1,tagid), no fallback.
   */
  inline const ProbT tagp(const Trigram &trigram, ProbT ProbZero=MOOT_PROB_ZERO) const
  {
    if (!hash_ngrams) return tagp(trigram.tag1, trigram.tag2, trigram.tag3);
    NgramProbHash::const_iterator ngpi = ngprobsh.find(trigram);
    return ngpi != ngprobsh.end() ? ngpi->second : ProbZero;
  };

  /**
   * Looks up and returns trigram (log-)probability: log(p(tagid|prevtagid2,prevtagid1)),
   * given prevtagid2, prevtagid1, tagid.
   * Falls back to (interpolated) (<n)-gram values if required.
   */
  inline const ProbT tagp(const TagID prevtagid2, const TagID prevtagid1, const TagID tagid) const
  {
    if (!hash_ngrams) { //-- -hash
      return
	ngprobsa && prevtagid2 < n_tags && prevtagid1 < n_tags && tagid < n_tags
	? ngprobsa[(n_tags*((n_tags*prevtagid2)+prevtagid1))+tagid]
	: MOOT_PROB_ZERO;
    } else {            //-- +hash
      //-- trigram as stored (pre-smoothed)
      Trigram ng(prevtagid2,prevtagid1,tagid);
      ProbT p = tagp(ng, 1);
      if (p != 1) return p;
      //-- fallback: bigram as stored (pre-smoothed)
      ng.tag1 = 0;
      p = tagp(ng, 1);
      if (p != 1) return p;
      //-- fallback: unigram as stored (pre-smoothed)
      ng.tag2 = 0;
      p = tagp(ng, 1);
      if (p != 1) return p;
      //-- fallback: "unknown" unigram (pre-smoothed)
      ng.tag3 = 0;
      return tagp(ng, MOOT_PROB_ZERO);
    }
  };

  /**
   * \deprecated{prefer direct lookup}
   *
   * Looks up and returns trigram (log-)probability: log(p(tag|prevtag1,prevtag2)), string-version.
   */
  inline const ProbT tagp(const mootTagString &prevtag2,
			  const mootTagString &prevtag1,
			  const mootTagString &tag)
    const
  {
    return tagp(tagids.name2id(prevtag2), tagids.name2id(prevtag1), tagids.name2id(tag));
  };
  //@}


  //------------------------------------------------------------
  // Error Reporting

  /** \name Error reporting */
  //@{
  /** Error reporting */
  void carp(const char *fmt, ...);
  //@}

  //------------------------------------------------------------
  // public methods: low-level: debugging

  /** \name Debugging */
  //@{
  /** Debugging method: dump basic HMM contents to a text file. */
  void txtdump(FILE *file, bool dump_constants=true, bool dump_lexprobs=true, bool dump_classprobs=true, bool dump_suftrie=true, bool dump_ngprobs=true);

  /** Debugging method: dump entire Viterbi trellis to a text file
   *  \deprecated in favor of viterbi_dump_trace()
   */
  void viterbi_txtdump(TokenWriter *w, int ncols=0);

  /** Debugging method: dump single Viterbi column to a text file
   *  \deprecated in favor of viterbi_dump_trace()
   */
  void viterbi_txtdump_col(TokenWriter *w, ViterbiColumn *col, int colnum=0);
  //@}
};

moot_END_NAMESPACE

#endif /* _MOOT_HMM_H */
