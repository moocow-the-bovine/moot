/*--------------------------------------------------------------------------
 * File: dwdst_trainer.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + Trainer for KDWDS PoS-tagger: the guts
 *--------------------------------------------------------------------------*/

#include <stdio.h>
#include <errno.h>

#include "FSMRegexCompiler.h"

#include "dwdstTypes.h"
#include "dwdstTrainer.h"
#include "dwdstTaglistLexer.h"
#include "dwdstParamCompiler.h"

using namespace std;

/*--------------------------------------------------------------------------
 * Constructor / Destructor
 *--------------------------------------------------------------------------*/

/*
 * dwdstTrainer::~dwdstTrainer()
 */
dwdstTrainer::~dwdstTrainer() {
    // -- clear any leftover training-temps
    cleanup_training_temps();

    // -- clear param table
    ngtable.clear();

    // -- clear alltags-list
    alltags.clear();
}

/*--------------------------------------------------------------------------
 * Public Methods: mid-level: tag-list file-reading
 *--------------------------------------------------------------------------*/

/**
 * Implicitly clears 'tagset'.
 * If 'filename' is NULL, 'tagset' is set to all categories according to 'syms' data member.
 */
set<FSMSymbolString> dwdstTrainer::read_taglist_file(set<FSMSymbolString> &tagset, const char *filename=NULL)
{
  // -- sanity check
  tagset.clear();

  if (!filename) {
    // -- default: all categories
    if (!syms) {
      fprintf(stderr, "dwdstTrainer::read_taglist_file(): no symbols loaded!\n");
      return tagset;
    }
    const set<FSMSymbolString> *symbols = syms->symbols();
    for (set<FSMSymbolString>::iterator p = symbols->begin(); p != symbols->end(); p++) {
      if (!syms->is_category(*p)) continue;
      tagset.insert("_"+(*p));
    }
  } else {
    // -- load from file
    FILE *pos_file;
    dwdstTaglistLexer lexer;
    FSMSymbolString s;
    int  tok;
    
    if (!(pos_file = fopen(filename,"r"))) {
      fprintf(stderr,"dwdstTrainer::read_taglist_file(): could not open file '%s' for read.\n",
	      filename);
      return tagset;
    }
    // -- parse the pos-tag file
    lexer.step_streams(pos_file,stdout);
    while ((tok = lexer.yylex()) != dwdstTaglistLexer::PTEOF) {
      switch (tok) {
      case dwdstTaglistLexer::POSTAG:
	s = (char *)lexer.yytext;
	tagset.insert(s);
      case dwdstTaglistLexer::PTEOF:
	break;
      default:
	fprintf(stderr,"dwdstTrainer::get_pos_tags(): Error in input file '%s'.\n",
		filename);
      }
    }
  }
  return tagset;
}


/*--------------------------------------------------------------------------
 * Public Methods: FSA Generation: Unknown-Analysis FSA
 *--------------------------------------------------------------------------*/

/**
 * Generates analysis-FSA for tokens unknown to KDWDS morphology.
 * Uses 'opentags' : set of open-class PoS tags.
 */
FSM *dwdstTrainer::generate_unknown_fsa()
{
  FSMRepresentation *base;
  FSMState q0;
  set<FSMState> pstates;
  set<FSMState>::iterator ps;

  // -- ensure that ufsa exists
  if (ufsa) ufsa->fsm_clear();
  else {
    ufsa = new FSM();
    ufsa->set_fsm_type(false,false);
  }

  // -- add start state
  base = ufsa->fsm_representation();
  q0 = base->set_start_state(base->add_state(base->new_state()));

  // -- return an FSA ambiguous between all tags in 'tagset'
  for (set<FSMSymbolString>::iterator p = opentags.begin(); p != opentags.end(); p++) {
    pstates = fsm_add_pos_arc(ufsa, q0, *p, FSM_default_cost_structure.freecost());
    for (ps = pstates.begin(); ps != pstates.end(); ps++) {
      base->mark_state_as_final(*ps);
    }
  }
  return ufsa;
}

/*--------------------------------------------------------------------------
 * Public Methods: FSA Generation: Diambiguation-FSA
 *  --> WORK IN PROGESS <--
 *--------------------------------------------------------------------------*/

/**
 * Generate a statistical PoS-disambiguation FSA based on internal
 * N-Gram tables.
 */
FSM *dwdstTrainer::generate_disambig_fsa()
{
  // -- for error reporting
  const char methodName[] = "dwdstTrainer::generate_disambig_fsa()";

  /*
    // -- Mecker!
    fprintf(stderr,"%s: not yet implemented!\n", methodName);
    return NULL;
  */

  // -- sanity check: symbols
  if (!syms) {
    fprintf(stderr,"%s: cannot generate disambiguation fsa without an FSMSymSpec!\n", methodName);
    return NULL;
  }
  // -- sanity check: kmax
  if (kmax <= 0) {
    for (NGramTable::iterator ngti = ngtable.begin(); ngti != ngtable.end(); ngti++) {
      if ((int)ngti->first.size() > kmax)
	kmax = ngti->first.size();
    }
    fprintf(stderr,"%s: using implicit kmax=%d.\n", methodName, kmax);
  }

#ifdef DWDST_DFSA_DEBUG
  fprintf(stderr, "%s: before erase(eos=%s), alltags.size()=%d\n", methodName, eos.c_str(), alltags.size());
#endif

  alltags.erase(eos); // -- HACK: temporarily remove EOS from the tagset

#ifdef DWDST_DFSA_DEBUG
  fprintf(stderr, "%s: after erase(eos=%s), alltags.size()=%d\n", methodName, eos.c_str(), alltags.size());
#endif

  // -- generate tag-to-symbol map for skeleton fsa
  tags2symbols.clear();
  if (!generate_tag_map()) return NULL;

#ifdef DWDST_DFSA_DEBUG
  fprintf(stderr, "dwdstTrainer::generate_tag_map(): generated %d skeleton tag-symbols.\n", tags2symbols.size());
#endif

  // -- generate the skeleton dfsa
  dfsa = generate_disambig_skeleton();
  if (!dfsa || !*dfsa) {
    if (dfsa) {
      delete dfsa;
      dfsa = NULL;
    }
    return NULL;
  }

#ifdef DWDST_DFSA_DEBUG
  // debug: save skeleton tag-labels
  FILE *labfile = fopen("dskeleton.lab","w");
  if (labfile) {
    fprintf(labfile,"<eps>\t0\n");
    for (dwdstStringToSymbolMap::iterator t2si =  tags2symbols.begin();
	 t2si != tags2symbols.end();
	 t2si++)
      {
	fprintf(labfile,"%s\t%d\n", t2si->first.c_str(), t2si->second);
      }
    fclose(labfile);
  } else {
    fprintf(stderr, "%s: could not open 'dskeleton.lab' for write: %s\n", methodName, strerror(errno));
  }
  // -- debug: save skeleton FSM
  if (!dfsa->fsm_save_to_binary_file("dskeleton.fsa", false)) {
    fprintf(stderr, "%s: save failed for skeleton FSA 'dskeleton.fsa'.\n", methodName);
  }
#endif

  // -- now substitute in the FSMs resulting from each tag's compilation
  //    as a regex.
  dfsa = expand_disambig_skeleton();
  if (!dfsa || !*dfsa) {
    if (dfsa) {
      delete dfsa;
      dfsa = NULL;
    }
    return NULL;
  }

  alltags.insert(eos);  // -- HACK: re-insert removed EOS back into the tagset
  return dfsa;
}

/**
 * Compile each key (pseudo-tag) in 'tags2symbols' as a regex,
 * and substitute the resultant FSM into the skeleton 'dfsa'.
 */
FSM *dwdstTrainer::expand_disambig_skeleton()
{
  // -- for error reporting
  const char methodName[] = "dwdstTrainer::expand_disambig_skeleton()";

  // -- setup a regex-compiler
  FSMRegexCompiler recomp;
  recomp.symspec = syms;
  recomp.allow_incomplete_categories = allow_incomplete_categories;
  //recomp.objname = (char *)methodName;
  recomp.verbose = verbose;
  recomp.theLexer.theLine = 0;

  // -- construct an FSMSubstitutionMap using tags2symbols
  FSM::FSMSubstitutionMap syms2fsms;
  for (dwdstStringToSymbolMap::iterator t2si = tags2symbols.begin(); t2si != tags2symbols.end(); t2si++) {
    recomp.theLexer.theLine++;
    recomp.theLexer.theColumn = 0;
    if (!recomp.parse_from_string(t2si->first.c_str())) {
      fprintf(stderr, "%s: could not compile pseudo-tag '%s' as a regular expression -- using <epsilon>!\n",
	      methodName, t2si->first.c_str());
      recomp.result_fsm = recomp.epsilon_fsm();
    }

    // -- generate a temporary substitution-map
    syms2fsms.clear();
    syms2fsms[t2si->second] = recomp.result_fsm;

    // -- do substition for *THIS PSEUDO-TAG ONLY* 
    dfsa->fsm_substitute(syms2fsms,FSM::FSMModeDestructive);

    // -- cleanup
    delete recomp.result_fsm;
    recomp.result_fsm = NULL;

    // -- sanity check
    if (!dfsa || !*dfsa) {
      fprintf(stderr, "%s: could not expand disambiguator skeleton for pseudo-tag '%s'!\n",
	      methodName, t2si->first.c_str());
      if (dfsa) {
	delete dfsa;
	dfsa = NULL;
      }
      return NULL;
    }
  }
  syms2fsms.clear();
  return dfsa;
}

/**
 * Generate a skeleton statistical PoS-disambiguation FSA based on internal
 * N-Gram tables.  The skeleton FSA uses a single symbol for each possible
 * tag.  Later, these symbols should be replaced by the sub-FSAs resulting
 * from compiling the 'tag' as a regex.
 * HACK: Pseudo-tag transitions in the skeleton are prefixed with
 * an epsilon-transition, which is assigned the generated weight -- otherwise,
 * we lose the weights when we call 'fsm_substitute()' [*grumble*].
 */
FSM *dwdstTrainer::generate_disambig_skeleton()
{
  // -- for error-reporting
  const char methodName[] = "dwdstTrainer::generate_disambig_skeleton()";

  // -- create & initialize dfsa
  if (dfsa) { delete dfsa; }
  dfsa = new FSM();
  dfsa->set_fsm_type(false,true);
  FSMRepresentation *rep = dfsa->fsm_representation();
  FSMWeight freecost = rep->cost_structure()->freecost();

  // -- generate a skeleton-FSA using a single-symbol for each pseudo-tag
  //    initialization: BOS
  FSMState q0 = rep->new_state();
  rep->add_state(q0);
  rep->set_start_state(q0);
  rep->mark_state_as_final(q0,freecost);

  NGramToStateMap nGram2State;
  theNgram.clear();
  theNgram.push_back(eos);
  nGram2State.clear();
  nGram2State[theNgram] = pair<FSMState,FSMState>(q0,q0);

  tagSetIterVector tagIters;
  tagIters.clear();

  int len;
  FSMState prevState;
  NGramVector prevNgram;

  for (len = 1; len < kmax; len++) {
    // -- len: n-gram length for this pass
    //    we ignore len=0, because that's BOS/EOS,
    //    and also  len=kmax, because we don't need states for those.

# ifdef DWDST_DFSA_DEBUG_VERBOSE
    fprintf(stderr, "%s: BOS-bootstrap len=%d.\n", methodName, len);
# endif

    for (tagIters_begin(tagIters,alltags,len);
	 !tagIters_done(tagIters,alltags);
	 tagIters_next(tagIters,alltags))
      {
	// -- build current nGram from tagIters
	theNgram.clear();
	if (len < kmax-1) theNgram.push_back(eos);
	for (tagSetIterVector::iterator tsi = tagIters.begin(); tsi != tagIters.end(); tsi++) {
	  theNgram.push_back(*(*tsi));
	}
	
# ifdef DWDST_DFSA_DEBUG_VERBOSE
	fprintf(stderr, "%s: BOS-bootstrap nGram=<", methodName);
	for (NGramVector::iterator dbg_ngvi = theNgram.begin(); dbg_ngvi != theNgram.end(); dbg_ngvi++) {
	  fprintf(stderr,"%s,",dbg_ngvi->c_str());
	}
	fprintf(stderr, ">\n");
# endif

	// -- add a non-final state for the current nGram's cost: [context="<cost>.nGram"]
	FSMState costState = rep->new_state();
	rep->add_state(costState);

	// -- add a final state for the current nGram itself: [context="<cost>nGram."]
	FSMState ngramState = rep->new_state();
	rep->add_state(ngramState);
	rep->mark_state_as_final(ngramState, freecost);

	// -- remember what we've done: we may be needing it later
	nGram2State[theNgram] = pair<FSMState,FSMState>(costState,ngramState);

	// -- add transitions from the 'preceeding' nGram's state, if it exists
	//    (BOS-bootstrap)
	//    States:
	//     prevState  : [context=".<cost>nGram."]
	//     costState  : [context="<cost>.nGram."]
	//     ngramState : [context="<cost>nGram."]
	//    Transits:
	//      prevState --(eps/0)--> costState --($tag/$cost)--> ngramState
	prevNgram = theNgram;
	prevNgram.pop_back();
	if (prevNgram[0] != eos) { prevNgram.push_front(eos); }
	NGramToStateMap::const_iterator pngi = nGram2State.find(prevNgram);
	if (pngi != nGram2State.end()) {
	  prevState                  = pngi->second.second;
	  FSMSymbolString  theTag    = theNgram[theNgram.size()-1];
	  FSMSymbol        tagSymbol = tags2symbols[theTag];
	  FSMWeight        arcCost   = disambigArcCost(prevNgram, theTag, uniGramCount);
	  rep->add_transition(prevState,  costState,   EPSILON,   EPSILON,  arcCost);
	  rep->add_transition(costState, ngramState, tagSymbol, tagSymbol, freecost);
	} else {
	  // -- this should never happen!
	  fprintf(stderr, "%s: missing state in BOS bootstrap!\n", methodName);
	  continue;
	}
      }
  }

  // -- now, add transitions for the various kmax-Grams,
  //    between the states for the (kmax-1)-Grams
  NGramVector nextNgram;

# ifdef DWDST_DFSA_DEBUG_VERBOSE
    fprintf(stderr, "%s: (kmax=%d)-linkup.\n", methodName, kmax);
# endif

  for (tagIters_begin(tagIters,alltags,kmax-1);
       !tagIters_done(tagIters,alltags);
       tagIters_next(tagIters,alltags))
    {
      // -- build (kmax-1)-Gram for source state from tagIters
      prevNgram.clear();
      if (len < kmax) {
	prevNgram.push_back(eos);
      }
      for (tagSetIterVector::iterator tsi = tagIters.begin(); tsi != tagIters.end(); tsi++) {
	prevNgram.push_back(*(*tsi));
      }

# ifdef DWDST_DFSA_DEBUG_VERBOSE
      fprintf(stderr, "%s: kmax-linkup prevNgram=<", methodName);
      for (NGramVector::iterator dbg_ngvi = prevNgram.begin(); dbg_ngvi != prevNgram.end(); dbg_ngvi++) {
	fprintf(stderr,"%s,",dbg_ngvi->c_str());
      }
      fprintf(stderr, ">\n");
# endif

      // -- source-state lookup
      NGramToStateMap::const_iterator pngi = nGram2State.find(prevNgram);
      if (pngi != nGram2State.end()) {      
	prevState = pngi->second.second;
      } else {
	// -- this should never happen
	fprintf(stderr, "%s: missing source-state in kmax-linkup!\n", methodName);
	continue;
      }

      // -- build (kmax-1)-Gram for destination state from prevNgram
      nextNgram = prevNgram;
      nextNgram.pop_front();
      for (set<FSMSymbolString>::iterator tsi = alltags.begin(); tsi != alltags.end(); tsi++) {

# ifdef DWDST_DFSA_DEBUG_VERBOSE
	fprintf(stderr, "%s:   + theTag='%s'\n", methodName, tsi->c_str());
# endif

	FSMSymbolString theTag = *tsi;
	nextNgram.push_back(theTag);
	NGramToStateMap::const_iterator nngi = nGram2State.find(nextNgram);
	if (nngi != nGram2State.end()) {
	  FSMState   costState  = nngi->second.first;
	  FSMWeight  arcCost    = disambigArcCost(prevNgram, theTag, uniGramCount);
	  rep->add_transition(prevState, costState,   EPSILON,  EPSILON,   arcCost);
# ifdef DWDST_DFSA_DEBUG_VERBOSE
	  fprintf(stderr, "%s:     > cost='%f'\n", methodName, arcCost);
# endif
	} else {
	  // -- this should never happen!
	  fprintf(stderr, "%s: missing sink-state in kmax-linkup!\n", methodName);
	}
	nextNgram.pop_back();
      }
    }

  return dfsa;
}


/**
 * Generate a 1-1 map from possible tags to symbols in this->tags2symbols.
 * Also calculates uniGramCount for use by disambigArcCost().
 */
bool dwdstTrainer::generate_tag_map()
{
  // -- build a symbol-map for all tags we know
  uniGramCount = 0;
  FSMSymbol symMax = syms->alphabet_size(); // -- avoid collisions with existing symbols
  for (set<FSMSymbolString>::iterator si = alltags.begin(); si != alltags.end(); si++) {
    tags2symbols[*si] = ++symMax;
    // -- count unigrams
    theNgram.clear();
    theNgram.push_back(*si);
    NGramTable::iterator ngfi;
    if ((ngfi = ngtable.find(theNgram)) != ngtable.end()) {
      uniGramCount += ngfi->second;
    }
  }
  return true;
}


/*--------------------------------------------------------------------------
 * Public Methods: weight-calculation utilities
 *--------------------------------------------------------------------------*/

/**
 * Returns the expected cost of transition from
 * state nGram to state <nGram,tagTo>.
 */
FSMWeight dwdstTrainer::disambigArcCost(NGramVector &nGram, FSMSymbolString &tagTo, float uniGramCount=0)
{
  NGramVector nGramTo = nGram;
  nGramTo.push_back(tagTo);

  // -- get raw count and number of fallbacks for nGramTo
  NGramCountFallbacksPair toCF = nGramCountFallbacks(nGramTo);

  NGramCountFallbacksPair fromCF;
  if (nGramTo.size() <= 1) {
    // -- fallback to unigram: special case
    fromCF.fallbacks() = 0;
    fromCF.count() = uniGramCount;
  } else {
    // -- normal case:
    //    now we set up the source-nGram, because
    //    nGramTo might have been shortened.
    NGramVector nGramFrom = nGramTo;
    nGramFrom.pop_back();
    fromCF = nGramCountFallbacks(nGramFrom);

    // -- sanity check
    if (nGramFrom.size() != nGramTo.size()-1) {
      fprintf(stderr,
	      "dwdstTrainer::disambigArcCost(): got %d-gram without prefix %d-gram.\n",
	      nGramTo.size(), nGramTo.size()-1);
    }
  }

  // -- now we can just do the math
  //    ArcCost(fromCF,toCF) := / fromFallbacks + toFallbacks + (toCount / fromCount) : if fromCount != 0
  //                            \ fromFallbacks + toFallbacks + 1.0                   : otherwise
  /*
    float FBsum = fromCF.fallbacks() + toCF.fallbacks();
    float Cquot = fromCF.count() ? (toCF.count() / fromCF.count()) : 1.0;
    return FBsum + (1.0 - Cquot);
  */
  return
    fromCF.fallbacks() + toCF.fallbacks() +
    1 - (fromCF.count() ? (toCF.count() / fromCF.count()) : 1.0);
}

/**
 * Get the count for nGram.  If it doesn't exist,
 * chop its first (oldest) element and get the count for 
 */
NGramCountFallbacksPair dwdstTrainer::nGramCountFallbacks(NGramVector &nGram)
{
  NGramCountFallbacksPair cfb;

  cfb.fallbacks() = 0;
  NGramTable::iterator ngfi;
  while (!nGram.empty() && (ngfi = ngtable.find(nGram)) == ngtable.end()) {
    // -- didn't find one -- shorten the nGram and adjust the fallback-penalty
    cfb.fallbacks() += 1.0;
    nGram.pop_front();
  }
  if (nGram.empty()) {
    // -- we didn't find *anything*
    cfb.fallbacks() += 1.0;
    cfb.count() = 0.0;
  } else {
    // -- we found something: get the count
    cfb.count() = ngtable[nGram];
  }
  return cfb;
}


/*--------------------------------------------------------------------------
 * Public Methods: low-level tag iteration utilities
 *--------------------------------------------------------------------------*/

/**
 * Initialize possible ${len}-gram generating tag-iterator vector
 * for \b tagSet.
 */
tagSetIterVector &dwdstTrainer::tagIters_begin(tagSetIterVector &tagIters,
					       set<FSMSymbolString> &tagSet,
					       int len)
{
  // -- initialize
  tagIters.clear();
  tagIters.reserve(len);
  for (int j = 0; j < len; j++) {
    tagIters.push_back(tagSet.begin());
  }
  return tagIters;
}

/**
 * Get next possible ${tagIters.size()}-gram iterator-vector for tagSet
 */
tagSetIterVector &dwdstTrainer::tagIters_next(tagSetIterVector &tagIters,
					       set<FSMSymbolString> &tagSet)
{
  for (int j = tagIters.size()-1; j >= 0; j--) {
    /*
      ++tagIters[j];
      // -- HACK: ignore EOS
      if (tagIters[j] != tagSet.end() && *(tagIters[j]) == eos) ++tagIters[j];
      // -- Try to just step this position
      if (tagIters[j] != tagSet.end() || j == 0) break;
    */

    // -- Try to just step this position
    if (++tagIters[j] != tagSet.end() || j == 0) break;

    // -- Reset this position and increment the next highest
    tagIters[j] = tagSet.begin();
  }
  return tagIters;
}

/**
 * Are we done iterating yet?
 */
bool dwdstTrainer::tagIters_done(tagSetIterVector &tagIters, set<FSMSymbolString> &tagSet)
{
  return tagIters[0] == tagSet.end();
}


/*--------------------------------------------------------------------------
 * Public Methods: mid-/low-level FSA Generation: arc-addition
 *--------------------------------------------------------------------------*/

/*
 * set<FSMState>
 * dwdstTrainer::fsm_add_pos_arc(FSM *fsm, FSMState qfrom, FSMSymbolString &pos,
 *                                      FSMWeight cost = FSM_default_cost_structure.freecost())
 *   + returns set of "final" states for the arc
 *   + honors the 'want_features' data-member
 */
set<FSMState>
dwdstTrainer::fsm_add_pos_arc(FSM *fsm, const FSMState qfrom, const FSMSymbolString &pos,
				     const FSMWeight cost = FSM_default_cost_structure.freecost())
{
  FSMRepresentation *base = fsm->fsm_representation();
  FSMState qi;
  set<FSMState> pstates;

  // -- return an FSA ambiguous between all tags in 'tagset' -- no features!
  qi = base->add_state(base->new_state());
  base->add_transition(qfrom, qi,
		       syms->symbolname_to_symbol(pos), 
		       syms->symbolname_to_symbol(pos),
		       cost);
  pstates.insert(qi);

  if (want_features) {
    FSMSymbolString catstr = pos;
    if (catstr[0] == '_') { catstr.erase(0,1); }
    const vector<FSMSymbolString> *feats = syms->features_of_category(catstr);
    FSMState qf; // -- feature-state

    for (vector<FSMSymbolString>::const_iterator f = feats->begin(); f != feats->end(); f++) {
      set<FSMSymbol> *values = syms->subtypes_of(*f);
      qf = base->add_state(base->new_state());
      for (set<FSMSymbol>::iterator v = values->begin(); v != values->end(); v++) {
	for (set<FSMState>::iterator ps = pstates.begin(); ps != pstates.end(); ps++) {
	  base->add_transition(*ps, qf, *v, *v, FSM_default_cost_structure.freecost());
	}
      }
      pstates.insert(qf);
    }
  }
  return pstates;
}


/*--------------------------------------------------------------------------
 * Public Methods: Parameter-generation: Training
 *--------------------------------------------------------------------------*/

/*
 * bool dwdstTrainer::train_from_stream(FILE *in, FILE *out)
 */
bool dwdstTrainer::train_from_stream(FILE *in, FILE *out)
{
  int tok;

  // -- sanity check
  if (!can_tag() || !init_training_temps(in,out)) {
    fprintf(stderr, "dwdstTrainer::train_from_stream(): cannot run uninitialized trainer!\n");
    return false;
  }

  // -- ye olde guttes
  lexer.step_streams(in,out);
  while ((tok = lexer.yylex()) != dwdstTaggerLexer::DTEOF) {
    switch (tok) {
    case dwdstTaggerLexer::EOS:
    case dwdstTaggerLexer::DTEOF:
      // -- don't do much on eos
      train_eos();
      break;
    default:
      // -- normal tokens
      token = (char *)lexer.yytext;
      train_next_token();
    }
  }
  return cleanup_training_temps();
}

/*
 * bool dwdstTrainer::train_from_strings(int argc, char **argv, FILE *out=stdout)
 */
bool dwdstTrainer::train_from_strings(int argc, char **argv, FILE *out=stdout)
{
  // -- sanity check
  if (!can_tag() || !init_training_temps(NULL,out)) {
    fprintf(stderr, "dwdstTrainer::train_from_strings(): cannot run uninitialized trainer!\n");
    return false;
  }

  // -- ye olde guttes
  for ( ; --argc >= 0; argv++) {
    token = *argv;
    train_next_token();
  }
  train_eos();

  return cleanup_training_temps();
}


/*
 * bool dwdstTrainer::init_training_temps()
 */
inline bool dwdstTrainer::init_training_temps(FILE *in=NULL, FILE *out=NULL)
{
  int i;

  // -- ngram temps
  curngrams = new set<NGramVector>();
  nextngrams = new set<NGramVector>();

  // -- files
  infile = in;
  outfile = out;

  // -- initialize string-sets
  for (i = 0; i < kmax; i++) {
    stringq.push_back(new set<FSMSymbolString>());
  }
  tmptags = new set<FSMSymbolString>();
  return true;
}


/*
 * bool dwdstTrainer::cleanup_training_temps()
 */
inline bool dwdstTrainer::cleanup_training_temps()
{
  // -- cleanup ngram-temps
  theNgram.clear();
  if (curngrams) {
    curngrams->clear();
    delete curngrams;
    curngrams = NULL;
  }
  if (nextngrams) {
    nextngrams->clear();
    delete nextngrams;
    nextngrams = NULL;
  }
  tmpngrams = NULL;

  // -- cleanup string-sets
  if (tmptags) {
      tmptags->clear();
      delete tmptags;
      tmptags = NULL;
  }
  while (!stringq.empty()) {
    curtags = stringq.front();
    stringq.pop_front();
    if (curtags) {
      curtags->clear();
      delete curtags;
    }
  }
  curtags = NULL;

  infile = NULL;
  outfile = NULL;
  return true;
}

/*
 * void dwdstTrainer::train_eos()
 *   + update internal ngram tables for EOS pseudo-tokens
 */
inline void dwdstTrainer::train_eos(void)
{
  // -- count it!
  theNgram.clear();
  theNgram.push_back(eos);
  if (ngtable.find(theNgram) != ngtable.end()) {
    ngtable[theNgram] += 1;
  } else {
    ngtable[theNgram] = 1;
  }

  // -- reset string-sets to EOS
  for (qi = stringq.begin(); qi != stringq.end(); qi++) {
    if (*qi) {
      (*qi)->clear();
      //(*qi)->insert(eos);
    }
  }
}


/*
 * inline void dwdstTrainer::train_next_token(void);
 *   + update internal ngram tables for real text tokens
 */
inline void dwdstTrainer::train_next_token(void)
{
  // -- tag it first
  s = (char *)token;
  tmp->fsm_clear();
  result = morph->fsm_lookup(s,tmp,true);
  // -- verbosity
  if (verbose) ntokens++;

  // -- pop 'current' tag-string set (back of queue == oldest)
  curtags = stringq.back();
  stringq.pop_back();

  // -- get the 'next' tag-string set to the 'current' one
  curtags->clear();
  if (want_features) {
    // -- all features
    results.clear();
    tmp->fsm_strings(*syms, results, false, want_avm);
    
    // -- set curtags to full string results
    for (ri = results.begin(); ri != results.end(); ri++) {
      curtags->insert(ri->istr);
    }
  } else {
    // -- tags only
    get_fsm_tag_strings(tmp,curtags);
  }
    
  // -- unknown token?
  if (curtags->empty()) {
    get_fsm_tag_strings(ufsa,curtags);
  }

  // -- HACK: trim extraneous spaces from each string in 'curtags'
  tmptags->clear();
  for (cti = curtags->begin(); cti != curtags->end(); cti++) {
      tmptags->insert(cti->substr(cti->find_first_not_of(" ",0),
				  cti->find_last_not_of(" ")+1));
  }
  swaptags = curtags;
  curtags = tmptags;
  tmptags = swaptags;
    
  // -- push 'current' tags onto the queue (front of queue == newest)
  stringq.push_front(curtags);
    
  // -- note all 'current' tags in "alltags"
  alltags.insert(curtags->begin(),curtags->end());

#ifdef DWDST_PARGEN_DEBUG
  // -- report current stringq
  fprintf(stderr, "\n<DEBUG> stringq=<");
  for (FSMSymbolStringQueue::iterator dbgqi = stringq.begin(); dbgqi != stringq.end(); dbgqi++) {
    fprintf(stderr, "\n<DEBUG>   {");
    for (set<FSMSymbolString>::iterator dbgi = (*dbgqi)->begin(); dbgi != (*dbgqi)->end(); dbgi++) {
      fprintf(stderr, "%s,", dbgi->c_str());
    }
    fprintf(stderr, "},");
  }
  fprintf(stderr, "\n<DEBUG>  >\n");
#endif // DWDST_PARGEN_DEBUG

  // ----------------------------
  // counting: i <= kmax -grams
  // ----------------------------
  curngrams->clear();
  theNgram.clear();
  is_eos = false;
  for (qi = stringq.begin();  qi != stringq.end() && !is_eos; qi++) {
    if (curngrams->empty()) {
      // -- initialize current-ngrams (when (*qi) is NEWEST)
      //    : curngrams are initialized to NEWEST string-set
      //      and built back-to-front in 'natural' order (oldest..newest)
      for (qii = (*qi)->begin(); qii != (*qi)->end(); qii++) {
	curngrams->insert(NGramVector(1,*qii));
      }
    } else {
      // -- we already have some ngrams -- need to extend them
      nextngrams->clear();
      for (ngi = curngrams->begin(); ngi != curngrams->end(); ngi++) {
	if ((*qi)->empty()) {
	  // -- empty string-set: append eos
	  is_eos = true;
	  theNgram = *ngi;
	  theNgram.push_front(eos);
	  nextngrams->insert(theNgram);
	} else {
	  // -- non-empty string set: append members
	  for (qii = (*qi)->begin(); qii != (*qi)->end(); qii++) {
	    theNgram = *ngi;
	    theNgram.push_front(*qii);
	    nextngrams->insert(theNgram);
	  }
	}
      }
      // -- swap curngrams and nextngrams
      tmpngrams = curngrams;
      curngrams = nextngrams;
      nextngrams = tmpngrams;
    }

# ifdef DWDST_PARGEN_DEBUG
    fprintf(stderr, "\n<NG-DEBUG> curngrams={");
    for (ngi = curngrams->begin(); ngi != curngrams->end(); ngi++) {
      fprintf(stderr, "\n<NG-DEBUG>   <");
      theNgram = *ngi;
      for (NGramVector::iterator ngvi = theNgram.begin(); ngvi != theNgram.end(); ngvi++) {
	fprintf(stderr, "%s,", ngvi->c_str());
      }
      fprintf(stderr, ">,");
    }
    fprintf(stderr, "\n<NG-DEBUG>   }\n");
# endif

    // -- ... and count them
    for (ngi = curngrams->begin(); ngi != curngrams->end(); ngi++) {
      theNgram = *ngi;
      if (ngtable.find(theNgram) != ngtable.end()) {
	ngtable[theNgram] += curngrams->size() >= 0 ? 1.0/(float)curngrams->size() : 0;
      } else {
	ngtable[theNgram] = curngrams->size() >= 0 ? 1.0/(float)curngrams->size() : 0;
      }
    }
  }
}


/*--------------------------------------------------------------------------
 * Public Methods: Parameter generation: Parameter-File Read/Write
 *--------------------------------------------------------------------------*/

/*
 * bool dwdstTrainer::save_param_file(FILE *out=stdout);
 */
bool dwdstTrainer::save_param_file(FILE *out=stdout)
{
  for (ngti = ngtable.begin(); ngti != ngtable.end(); ngti++) {
    //for (NGramVector::const_reverse_iterator ngvi = ngti->first.rbegin(); ngvi != ngti->first.rend(); ngvi++)
    for (NGramVector::const_iterator ngvi = ngti->first.begin(); ngvi != ngti->first.end(); ngvi++)
      {
	fputs(ngvi->c_str(),out);
	fputc('\t',out);
      }
    fprintf(out,"%f\n",ngti->second);
  }
  return true;
}

/*
 * load_param_file(file,filename)
 *  + adds counts from given param file to internal table 'strings2counts'
 */
bool dwdstTrainer::load_param_file(FILE *in=stdin,const char *filename=NULL)
{
  /*//--  MECKER
  fprintf(stderr,"dwdst_tagger_trainer::load_param_file(): not yet implemented!\n");
  abort();
  return false;
  */

 dwdstParamCompiler parcomp;

 // -- setup param-compiler
 parcomp.objname = "dwdstTrainer::load_param_file";
 parcomp.ngtable = &ngtable;
 parcomp.alltags = &alltags;

 if (!parcomp.parse_from_file(in,filename)) {
   fprintf(stderr, "dwdstTrainer::load_param_file: could not load file '%s'.\n", filename);
   return false;
 }
 return true;
}

/*--------------------------------------------------------------------------
 *--------------------------------------------------------------------------*/
