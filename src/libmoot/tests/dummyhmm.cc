#include <mootHMM.h>
using namespace moot;
using namespace std;

/*======================================================================
 * myHMM (dummy mootHMM)
 */
class myHMM {
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
#else
    ProbT wprob;                  ///< (log-)lexical probability p(word|tag)
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
    ProbT  wprob;                 ///< (log-)lexical probability p(word|tag)
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
    ProbT          bbestpr;  ///< Best probability in column for beam search
    ProbT          bpprmin;  ///< Best previous probability for beam search
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
   * Add contents of Viterbi trellis to @analyses members of mootToken elements on tag_mark_best()
   */
  bool save_ambiguities;

  /**
   * Add flavor names to @analyses members of mootToken elements on tag_mark_best()
   */
  bool save_flavors;

  /**
   * Mark unknown tokens with a single analysis '*' on tag_mark_best()
   */
  bool save_mark_unknown;

  /**
   * Save Viterbi trellis on tag_sentence()
   */
  bool save_dump_trellis;
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

  SuffixTrie        suftrie;    /**< string-suffix (log-)probability trie */
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

  //ProbT           bbestpr;   /**< Best current (log-)probability for beam pruning */
  //ProbT           bpprmin;   /**< Minimum previous probability for beam pruning */
  //@}

public:
  /*---------------------------------------------------------------------*/
  /** \name Constructor / Destructor */
  //@{
  /** Default constructor */
  myHMM(void)
    : verbose(1),
      ndots(0),
      save_ambiguities(false),
      save_flavors(false),
      save_mark_unknown(false),
      save_dump_trellis(false),
      use_lex_classes(true),
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
#if !defined(MOOT_USE_TRIGRAMS)
      ngprobs2(NULL),
#elif !defined(MOOT_HASH_TRIGRAMS)
      ngprobs3(NULL),
#endif
      vtable(NULL),
      nsents(0),
      ntokens(0),
      nnewtokens(0),
      nunclassed(0),
      nnewclasses(0),
      nunknown(0),
      nfallbacks(0),
      trash_nodes(NULL),
#ifdef MOOT_USE_TRIGRAMS
      trash_rows(NULL),
#endif
      trash_columns(NULL), 
      trash_pathnodes(NULL),
      vbestpn(NULL),
      vbestpath(NULL)
  {
    //-- create special token entries
    for (TokID i = 0; i < NTokFlavors; i++) { flavids[i] = 0; }
    unknown_token_name("@UNKNOWN");
    unknown_tag_name("UNKNOWN");
    uclass = LexClass();
  };


  /** Destructor */
  ~myHMM(void) { clear(true,false); };
  //@}


  /*------------------------------------------------------------*/
  /** \name Accessors */
  //@{
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


  void clear(bool wipe_everything, bool unlogify)
  {
    //-- iterator variables
    ViterbiColumn *col, *col_next;
#ifdef MOOT_USE_TRIGRAMS
    ViterbiRow    *row, *row_next;
#endif
    ViterbiNode   *nod, *nod_next;

    //-- free trellis: columns, rows, and nodes
    for (col = vtable; col != NULL; col = col_next) {
      col_next      = col->col_prev;
#ifdef MOOT_USE_TRIGRAMS
      for (row = col->rows; row != NULL; row = row_next)
	{
	  row_next    = row->row_next;
	  for (nod = row->nodes; nod != NULL; nod = nod_next)
#else
	    for (nod = col->rows; nod != NULL; nod = nod_next)
#endif
	      {
		nod_next  = nod->nod_next;
		delete nod;
	      }
#ifdef MOOT_USE_TRIGRAMS
	  delete row;
	}
#endif
      delete col;
    }
    vtable = NULL;

    //-- free trashed trellis cols
    for (col = trash_columns; col != NULL; col = col_next) {
      col_next = col->col_prev;
      delete col;
    }
    trash_columns = NULL;

#ifdef MOOT_USE_TRIGRAMS
    //-- free trashed trellis rows
    for (row = trash_rows; row != NULL; row = row_next) {
      row_next = row->row_next;
      delete row;
    }
    trash_rows = NULL;
#endif

    //-- free trashed trellis nodes
    for (nod = trash_nodes; nod != NULL; nod = nod_next) {
      nod_next = nod->nod_next;
      delete nod;
    }
    trash_nodes = NULL;

    //-- free best-path nodes
    ViterbiPathNode *pnod, *pnod_next;
    for (pnod = vbestpath; pnod != NULL; pnod = pnod_next) {
      pnod_next = pnod->path_next;
      delete pnod;
    }
    vbestpath = NULL;

    //-- free trashed path nodes
    for (pnod = trash_pathnodes; pnod != NULL; pnod = pnod_next) {
      pnod_next       = pnod->path_next;
      delete pnod;
    }
    trash_pathnodes = NULL;

    //-- free n-gram probabilitiy table(s)
#ifdef MOOT_USE_TRIGRAMS
# ifdef MOOT_HASH_TRIGRAMS
    ngprobs3.clear();
# else
    if (ngprobs3) {
      free(ngprobs3);
      ngprobs3 = NULL;
    }
# endif
#else
    if (ngprobs2) {
      free(ngprobs2);
      ngprobs2 = NULL;
    }
#endif // MOOT_USE_TRIGRAMS

    //-- free lexical probabilities
    lexprobs.clear();

    //-- free lexical-class probabilities
    lcprobs.clear();

    //-- reset to default "empty" values
    vbestpn = NULL;
    nsents = 0;
    ntokens = 0;
    nnewtokens = 0;
    nunclassed = 0;
    nnewclasses = 0;
    nunknown = 0;

    //-- un-logify constants
    if (unlogify) {
      nglambda1 = exp(nglambda1);
      nglambda2 = exp(nglambda2);
#ifdef MOOT_USE_TRIGRAMS
      nglambda3 = exp(nglambda3);
#endif
      wlambda0 = exp(wlambda0);
      wlambda1 = exp(wlambda1);
      clambda0 = exp(clambda0);
      clambda1 = exp(clambda1);
      if (beamwd) beamwd = exp(beamwd);
    }

    if (wipe_everything) {
      //-- free id-tables
      tokids.clear();
      tagids.clear();

      for (int i = 0; i < NTokFlavors; i++) {
	flavids[i] = 0;
      }

      start_tagid = 0;
      unknown_lex_threshhold = 1;
      unknown_class_threshhold = 1;
      n_tags = 0;
      n_toks = 0;
      n_classes = 0;
    }
  };

};

/*======================================================================
 * MAIN
 */
int main(void) {
  myHMM   *hmm=NULL, *hmm2=NULL;
  mootHMM *mhmm=NULL;
  std::string s;

  hmm  = new myHMM;
  mhmm = new mootHMM;
  hmm2 = new myHMM;

  fprintf(stderr, "&mhmm.tokids=%p, &mhmm.tagids=%p\n", &(mhmm->tokids), &(mhmm->tagids));

  delete hmm;
  delete mhmm;
  delete hmm2;

  return 0;
}
