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
  /** \name ID Lookup Tables */
  //@{
  TokIDEnum           tokids;     /**< Token-ID lookup table */
  TagIDEnum           tagids;     /**< Tag-ID lookup table */
  //ClassIDEnum       classids;   /**< Class-ID lookup table */

  //TokID             flavids[NTokFlavors];
  //@}

  /*---------------------------------------------------------------------*/
  /** \name Probability Lookup Tables */
  //@{
  size_t            n_tags;     /**< Number of known tags: used to compute lookup indices */
  size_t            n_toks;     /**< Number of known tokens: used for sanity checks */
  size_t            n_classes;  /**< Number of known lexical classes */

  //LexProbTable      lexprobs;   /**< Lexical probability lookup table */
  //LexClassProbTable lcprobs;    /**< Lexical-class probability lookup table */
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
  bool save(const char *filename, int compression_level=MOOT_DEFAULT_COMPRESSION);

  /** Load from a binary file */
  bool load(const char *filename=NULL);
  //@}

  //------------------------------------------------------------
  /** \name Compilation / Initialization */
  //@{
  /**
   * Top-level: load and compile a single model, and estimate all
   * smoothing constants.  Returns true on success, false on failure.
   */
  bool load_model(const std::string &modelname, const mootTagString &start_tag_str="__$");
  //...
  //@}

  //------------------------------------------------------------
  // Tagging: Top-level
  /** \name Top-level Tagging Interface */
  //@{
  
  /** Top-level tagging interface: TokenIO layer */
  void tag_io(TokenReader *reader, TokenWriter *writer);
  void tag_sentence(Sentence &sentence);
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
  void txtdump(FILE *file);
  //@}
};
