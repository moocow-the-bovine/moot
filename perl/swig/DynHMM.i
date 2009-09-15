/* -*- Mode: C++ -*- */
/*
 * File: DynHMM.i
 * Author: Bryan Jurish
 * Description: swig interface to moot: mootDynHMM.h
 */

%{
  using namespace moot;
%}

//-- Suppress SWIG warning
#pragma SWIG nowarn=SWIGWARN_PARSE_NESTED_CLASS

//==============================================================================
%inline %{
  typedef mootTagString TagStr;
  typedef mootTokString TokStr;
%}

//==============================================================================
%{ typedef mootDynHMM DynHMM; %}
class DynHMM: public HMM {
public:
  DynHMM(void);
  virtual ~DynHMM(void);

public:
  virtual void tag_hook_pre(Sentence &sent);
  virtual void tag_hook_post(Sentence &sent);
  virtual void tag_io(TokenReader *reader, TokenWriter *writer);
  virtual void tag_sentence(Sentence &sentence);
};

//==============================================================================
%{ typedef mootDynLexHMM DynLexHMM; %}
class DynLexHMM : public DynHMM {
public:
  //---------------------------------------------------------------------
  // Data
  bool           invert_lexp;

  mootTagString  newtag_str;  /**< tag string to copy for "missing" tags (default="@NEW") */
  TagID          newtag_id;   /**< ID for "missing" tags */
  ProbT          newtag_f;    /**< Raw frequency for 'new' tag, if not already in model.  Default=0.5 */

  //TagTokProbMap  Ftw;         /**< pseudo-frequency lexicon:       t -> (w -> f(w,t)) */
  //TokProbMap     Fw;          /**< pseudo-frequency (sub-)lexicon: w -> f(w) */
  //TokProbMap     Ft;          /**< pseudo-frequency (sub-)lexicon: t -> f(t) */
  ProbT          Ftw_eps;     /**< Raw pseudo-frequency smoothing constant (non-log) for f(w,t) */

  size_t         tagids_size_orig; /**< original size of tagids */

public:
  //---------------------------------------------------------------------
  ///\name Constructors etc.
  DynLexHMM(void);
  virtual ~DynLexHMM(void);

  //---------------------------------------------------------------------
  ///\name Compilation & initialization
  virtual bool load_model(const std::string &modelname, const mootTagString &start_tag_str="__$");

  //---------------------------------------------------------------------
  ///\name Tagging: Hooks
  virtual void tag_hook_pre(Sentence &sent);
  virtual void tag_hook_post(Sentence &sent);
};


//==============================================================================
%{ typedef mootDynLexHMM_Boltzmann DynLexHMM_Boltzmann; %}
class DynLexHMM_Boltzmann : public DynLexHMM {
public:
  /** Base of Maxwell-Boltzmann estimator (>1), default=2.
   *  A value of 1.0 gives a uniform output distribution.
   *  Greater values give lower-entropy output distributions.
   */
  ProbT dynlex_base;

  /**
   * "Temperature" coefficient of Maxwell-Boltzmann estimator (>0), default=1
   * A value of 0.0 gives a uniform output distribution.
   * Greater values give lower-entropy output distributions.
   */
  ProbT dynlex_beta;

public:
  DynLexHMM_Boltzmann(void);
};
