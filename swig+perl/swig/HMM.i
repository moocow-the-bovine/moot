/* -*- Mode: C++ -*- */
/*
 * File: HMM.i
 * Author: Bryan Jurish
 * Description: swig interface to moot: HMM
 */

%{
  using namespace moot;
%}

//-- Suppress SWIG warning
#pragma SWIG nowarn=SWIGWARN_PARSE_NESTED_CLASS


/*----------------------------------------------------------------------
 * Basic Types
 */
//%{ typedef mootHMM::VerbosityLevel HMMVerbosityLevel; %}
%inline %{
typedef enum {
  HMMvlSilent = mootHMM::vlSilent,     /**< Be silent */
  HMMvlErrors = mootHMM::vlErrors,     /**< Report errors */
  HMMvlWarnings = mootHMM::vlWarnings,   /**< Report warnings */
  HMMvlProgress = mootHMM::vlProgress,   /**< Report progess */
  HMMvlEverything = mootHMM::vlEverything  /**< Report everything we can */
} HMMVerbosityLevel;

typedef mootEnumID TagID;
typedef mootEnumID TokID;
typedef mootEnumID ClassID;
%}

/*----------------------------------------------------------------------
 * Class: mootHMM::TagIDTable
 */
%inline %{
typedef mootEnum<mootTokString> TokIDEnum;
typedef mootEnum<mootTagString> TagIDEnum;
%}
//class ClassIDEnum : public LexClassEnum;

/*----------------------------------------------------------------------
 * Class: mootHMM::LexProbTable ~ HMMLexProbTable
 */

//-- HMMLexProbSubTable
%inline %{
  //typedef mootHMM::LexProbSubTable HMMLexProbSubTable;
  typedef AssocVector<mootEnumID,ProbT> HMMLexProbSubTableT;
%}

//-- HMMLexProbTable
%template(HMMLexProbTable) std::vector<HMMLexProbSubTableT>;
%inline %{
  //typedef mootHMM::LexProbTable    HMMLexProbTable;
  typedef std::vector<HMMLexProbSubTableT >  HMMLexProbTableT;
%}

/*----------------------------------------------------------------------
 * Class: mootHMM::Trigram -> Trigram
 */
%{ typedef mootHMM::Trigram Trigram; %}
class Trigram {
public:
  TagID tag1;  ///< previous-previous tag_{i-2} or 0
  TagID tag2;  ///< previous tag: tag_{i-1} or 0
  TagID tag3;  ///< current tag: tag_i
public:
  Trigram(TagID t1=0, TagID t2=0, TagID t3=0);
  ~Trigram(void);
  %extend {
    size_t hash(void) { return Trigram::HashFcn().operator()(*$self); };
  };
};

/*----------------------------------------------------------------------
 * Class: mootHMM::TrigramProbHash TrigramProbHash
 */
%template(HMMNgramProbHash) hash_map<Trigram,ProbT,Trigram::HashFcn,Trigram::EqualFcn>;
%inline %{
  //typedef mootHMM::NgramProbHash HMMNgramProbHash;
  typedef hash_map<Trigram,ProbT,Trigram::HashFcn,Trigram::EqualFcn> HMMNgramProbHash;
%}

/*----------------------------------------------------------------------
 * Class: mootHMM
 */
%{ typedef mootHMM HMM; %}
class HMM {
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
   *  Default: false.
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
  //LexClass   uclass;
  //@}

  /*---------------------------------------------------------------------*/
  /** \name Smoothing Constants */
  //@{
  ProbT             nglambda1;    /**< (log) Smoothing constant for unigrams */
  ProbT             nglambda2;    /**< (log) Smoothing constant for bigrams */
  //#ifdef MOOT_USE_TRIGRAMS
  ProbT             nglambda3;    /**< (log) Smoothing constant for trigrams */
  //#endif
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
  TokIDEnum           tokids;     /**< Token-ID lookup table */
  TagIDEnum           tagids;     /**< Tag-ID lookup table */
  //ClassIDEnum       classids;   /**< Class-ID lookup table */

  //TokID             flavids[NTokFlavors];
  %extend {
    TokID flavid(mootTokenFlavor flav) { return $self->flavids[flav]; };
  }
  //@}

  /*---------------------------------------------------------------------*/
  /** \name Probability Lookup Tables */
  //@{
  size_t            n_tags;     /**< Number of known tags: used to compute lookup indices */
  size_t            n_toks;     /**< Number of known tokens: used for sanity checks */
  size_t            n_classes;  /**< Number of known lexical classes */

  HMMLexProbTableT  lexprobs;   /**< Lexical probability lookup table */
  //LexClassProbTable lcprobs;    /**< Lexical-class probability lookup table */

  HMMNgramProbHash     ngprobsh;   /**< N-gram (log-)probability lookup table: hashed */
  //NgramProbArray    ngprobsa;   /**< N-gram (log-)probability lookup table: dense */

#ifdef MOOT_ENABLE_SUFFIX_TRIE
  //SuffixTrie        suftrie;    /**< string-suffix (log-)probability trie */
#endif
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

public:
  HMM(void);
  ~HMM(void);
  void clear(bool wipe_everything=true, bool unlogify=false);

  /*------------------------------------------------------------*/
  /**\name Binary load / save */
  //@{
  /** Save to a binary file */
  virtual bool save(const char *filename, int compression_level=MOOT_DEFAULT_COMPRESSION);

  /** Load from a binary file */
  virtual bool load(const char *filename=NULL);
  //@}

  //------------------------------------------------------------
  /** \name Compilation / Initialization */
  //@{
  /**
   * Top-level: load and compile a single model, and estimate all
   * smoothing constants.  Returns true on success, false on failure.
   */
  virtual bool load_model(const std::string &modelname, const mootTagString &start_tag_str="__$");

  /** Low-level utility: set a (raw,smoothed,log-) n-gram probability */
  void set_ngram_prob(ProbT p, TagID t1=0, TagID t2=0, TagID t3=0);
  //@}

  //------------------------------------------------------------
  // Tagging: Top-level
  /** \name Top-level Tagging Interface */
  //@{
  
  /** Top-level tagging interface: TokenIO layer */
  virtual void tag_io(TokenReader *reader, TokenWriter *writer);
  virtual void tag_sentence(Sentence &sentence);
  //...

  //------------------------------------------------------------
  // Tagging: Mid-Level
  /**
   * Mid-level tagging interface: dump verbose trace to \c sentence (destructive).
   * Calling this method will add verbose trace information as comments to \c sentence.
   * Same caveats as for tag_mark_best().
   */
  void tag_dump_trace(mootSentence &sentence);
  //...
  //@}


  //------------------------------------------------------------
  // Low-level: ID Lookup
  //@{
  TokID token2id(const mootTokString &token) const;
  //LexClass *tagset2lexclass(const mootTagSet &tagset, LexClass *lclass=NULL, bool add_tagids=false);
  //ClassID class2id(const LexClass &lclass, bool autopopulate=true, bool autocreate=true);
  //@}

  //------------------------------------------------------------
  /** \name Probability Lookup */
  //@{
  ProbT wordp(TokID tokid, TagID tagid) const;
  ProbT wordp(const mootTokString &tokstr, const mootTagString &tagstr) const;

  //ProbT classp(const ClassID classid, const TagID tagid) const;
  //ProbT classp(const LexClass &lclass, const mootTagString tag) const;

  /** unigram (log-)probability (smoothed) */
  ProbT tagp(const TagID tagid) const;
  ProbT tagp(const mootTagString &tagstr) const;

  /** bigram (log-)probability (smoothed) */
  ProbT tagp(const TagID tag1id, const TagID tag2id) const;
  ProbT tagp(const mootTagString &tag1str, const mootTagString &tag2str) const;

  /** trigram (log-)probability (smoothed) */
  ProbT tagp(const TagID tag1id, const TagID tag2id, const TagID tag3id) const;
  ProbT tagp(const mootTagString &tag1str, const mootTagString &tag2str, const mootTagString &tag3str) const;

  /**
   * Generic n-gram (log-)probability (smoothed)
   *
   * Looks up and returns raw n-gram (log-)probability: log(p(tagid|prevtagid2,prevtagid1)),
   * given Trigram(prevtagid2,prevtagid1,tagid), no fallback.
   *
   * (Smoothed) unigrams are keyed as Trigram(0,0,tagid), bigrams as Trigram(0,tag1id,tag2id).
   */
  const ProbT tagp(const Trigram &trigram, ProbT ProbZero=MOOT_PROB_ZERO) const;
  //@}

  //------------------------------------------------------------
  // Error Reporting
  /** \name Error reporting */
  //@{
  /** Error reporting */
  void carp(const char *fmt, ...);
  //@}

  //------------------------------------------------------------
  // Debugging
  //@{
  /** Debugging method: dump basic HMM contents to a text file. */
  %extend {
    void txtdump(const char *filename) {
      FILE *f = stdout;
      if (strcmp(filename,"-") != 0) f = fopen(filename,"wb");
      if (f == NULL) croak("HMM::txtdump(): open failed for file '%s'", filename);
      $self->txtdump(f);
      if (f != stdout) fclose(f);
    };
  }
  //@}
};
