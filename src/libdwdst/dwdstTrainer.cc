/*--------------------------------------------------------------------------
 * File: dwdst_trainer.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + Trainer for KDWDS PoS-tagger: the guts
 *--------------------------------------------------------------------------*/

#include <stdio.h>
#include <errno.h>

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

/*
 * set<FSMSymbolString> dwdstTrainer::read_taglist_file(set<FSMSymbolString> &tagset, const char *filename=NULL);
 *  + implicitly clears 'tagset'
 *  + if 'filename' is NULL, 'tagset' is set to all categories according to 'syms' data member
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

/*
 * FSM *dwdstTrainer::generate_unknown_fsa();
 *   + generates analysis-FSA for tokens unknown to KDWDS morphology
 *   + uses 'opentags' : set of open-class PoS tags
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
 *  --> BAUSTELLE!
 *--------------------------------------------------------------------------*/

/*
 * FSM *dwdstTrainer::generate_disambig_fsa();
 */
FSM *dwdstTrainer::generate_disambig_fsa()
{
  // Mecker!
  /*
    fprintf(stderr,"dwdstTrainer::generate_disambig_fsa(): not yet implemented!");
    abort();
    return NULL;
  */
  if (dfsa) { delete dfsa; }
  dfsa = new FSM();
  FSMRepresentation rep = dfsa->fsm_representation();

  // -- build a symbol-map for all tags we know
  FSMSymbol symMax = 0;
  dwdstStringToSymbolMap str2sym;
  for (set<FSMSymbolString>::iterator si = alltags.begin(); si != alltags.end(); si++) {
    str2sym[*si] = ++symMax;
  }

  // -- generate a skeleton-FSA using a single-symbol for each pseudo-tag
  FSMState q0 = rep->new_state();
  rep->add_state(q0);
  rep->set_start_state(q0);

  map<NGramVector,FSMState> nGram2State;
  theNgram.clear();
  theNgram.push_back(eos);
  nGram2State.clear();
  nGram2State[theNgram] = q0;

  tagSetIterVector tagIters;
  tagIters.clear();

  int len, j;
  NGramVector prevNgram;
  FSMState theState, prevState;
  for (len = 1; len <= kmax; len++) {
    // -- len: n-gram length for this pass
    for (tagIters_begin(tagIters,alltags,len);
	 !tagIters_done(tagIters,alltags);
	 tagIters_next(tagIters))
      {
	// -- build current and previous nGrams from tagIters
	theNgram.clear();
	if (len < kmax) theNgram.push_back(eos);
	for (tagSetIterVector::tsi = tagIters.begin(); tsi != tagIters.end(); tsi++) {
	  theNgram.push_back(*(*tsi));
	}
	prevNgram = theNgram;
	prevNgram.pop_back();

	// -- add a state for the current nGram
	theState = rep->new_state();
	rep->add_State(theState);
	nGram2State[theNgram] = theState;

	// -- add a transition from the preceeding nGram's state, if it exists
	//    (BOS-bootstrap)
	if ((dwdstStringToSymbolMap::iterator pngi = nGram2State.find(prevNgram)) !=
	    nGram2State.end())
	  {
	    prevState = *pngi;
	    
	  }
      }
  }

  FSMSymbol sym;
  FSMState q;
  for (sym = 1; sym < symMax; sym++) {
    ; // continue!
  }

  return dfsa;
}

/*--------------------------------------------------------------------------
 * Public Methods: weight-calculation utilities
 *--------------------------------------------------------------------------*/

/**
 * Returns the expected cost of 
 */
FSMWeight dwdstTrainer::transitionCost(NGramVector &nGram)
{
  // continue!
  return 0;
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
tagSetIterVector &dwdstTrainer::tagIters_next(tagSetIterVector &tagIters) {
  for (int j = tagIters.size()-1; j >= 0; j--) {
    if (++tagIters[j] != alltags.end() || j == 0) {
      // -- just step this position
      break;
    }
    // -- reset this position and increment the next highest
    tagIters[j] = alltags.begin();
  }
  return tagIters;
}

/**
 * Are we done iterating yet?
 */
bool dwdstTrainer::tagIters_done(tagSetIterVector &tagIters, set<FSMSymbolString> tagSet)
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
