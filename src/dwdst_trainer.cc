/*--------------------------------------------------------------------------
 * File: dwdst_trainer.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + Trainer for KDWDS PoS-tagger: the guts
 *--------------------------------------------------------------------------*/

#include <stdio.h>
#include <errno.h>

#include "dwdst_trainer.h"

using namespace std;

/*--------------------------------------------------------------------------
 * Constructor / Destructor
 *--------------------------------------------------------------------------*/

/*
 * dwds_tagger_trainer::~dwds_tagger_trainer()
 */
dwds_tagger_trainer::~dwds_tagger_trainer()
{
    if (ufsa) ufsa->fsm_free();
    ufsa = NULL;
    if (dfsa) dfsa->fsm_free();
    dfsa = NULL;
}

/*--------------------------------------------------------------------------
 * Public Methods: mid-level: tag-list file-reading
 *--------------------------------------------------------------------------*/

/*
 * set<FSMSymbolString> dwds_tagger_trainer::read_taglist_file(set<FSMSymbolString> &tagset, const char *filename=NULL);
 *  + implicitly clears 'tagset'
 *  + if 'filename' is NULL, 'tagset' is set to all categories according to 'syms' data member
 */
set<FSMSymbolString> dwds_tagger_trainer::read_taglist_file(set<FSMSymbolString> &tagset, const char *filename=NULL)
{
  // -- sanity check
  tagset.clear();

  if (!filename) {
    // -- default: all categories
    if (!syms) {
      fprintf(stderr, "dwds_tagger_trainer::read_taglist_file(): no symbols loaded!\n");
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
    postag_lexer lexer;
    FSMSymbolString s;
    int  tok;
    
    if (!(pos_file = fopen(filename,"r"))) {
      fprintf(stderr,"dwds_tagger_trainer::read_taglist_file(): could not open file '%s' for read.\n",
	      filename);
      return tagset;
    }
    // -- parse the pos-tag file
    lexer.step_streams(pos_file,stdout);
    while ((tok = lexer.yylex()) != PTEOF) {
      switch (tok) {
      case POSTAG:
	s = (char *)lexer.yytext;
	tagset.insert(s);
      case PTEOF:
	break;
      default:
	fprintf(stderr,"dwds_tagger_trainer::get_pos_tags(): Error in input file '%s'.\n",
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
 * FSM *dwds_tagger_trainer::generate_unknown_fsa();
 *   + generates analysis-FSA for tokens unknown to KDWDS morphology
 *   + uses 'opentags' : set of open-class PoS tags
 */
FSM *dwds_tagger_trainer::generate_unknown_fsa()
{
  FSMBaseRepresentation *base;
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
  base = ufsa->representation();
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
 *  --> NOT YET IMPLEMENTED!
 *--------------------------------------------------------------------------*/

/*
 * FSM *dwds_tagger_trainer::generate_disambig_fsa();
 */
FSM *dwds_tagger_trainer::generate_disambig_fsa()
{
  fprintf(stderr,"dwds_tagger_trainer::generate_disambig_fsa(): not yet implemented!");
  abort();
  return NULL;
}


/*--------------------------------------------------------------------------
 * Public Methods: mid-/low-level FSA Generation: arc-addition
 *--------------------------------------------------------------------------*/

/*
 * set<FSMState>
 * dwds_tagger_trainer::fsm_add_pos_arc(FSM *fsm, FSMState qfrom, FSMSymbolString &pos,
 *                                      FSMWeight cost = FSM_default_cost_structure.freecost())
 *   + returns set of "final" states for the arc
 *   + honors the 'want_features' data-member
 */
set<FSMState>
dwds_tagger_trainer::fsm_add_pos_arc(FSM *fsm, const FSMState qfrom, const FSMSymbolString &pos,
				     const FSMWeight cost = FSM_default_cost_structure.freecost())
{
  FSMBaseRepresentation *base = fsm->representation();
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
 * bool dwds_tagger_trainer::train_from_stream(FILE *in, FILE *out)
 */
bool dwds_tagger_trainer::train_from_stream(FILE *in, FILE *out)
{
  int tok, eosi;

  // -- sanity check
  if (!can_tag()) {
    fprintf(stderr, "dwds_tagger_trainer::train_from_stream(): cannot run uninitialized trainer!\n");
    return false;
  }

  // -- initialize string-sets
  for (eosi = 0; eosi < kmax; eosi++) {
    curtags = new set<FSMSymbolString>();
    curtags->insert(eos);
    stringq.push_back(curtags);
  }

  // -- tmp instance-vars
  curngrams = new set<FSMSymbolString>;
  nextngrams = new set<FSMSymbolString>;

  // -- files
  infile = in;
  outfile = out;

  // -- ye olde guttes
  lexer.step_streams(in,out);
  while ((tok = lexer.yylex()) != DTEOF) {
    // -- check for eos
    if (tok == EOS || tok == DTEOF) {
      eosi = kmax;
      is_eos = true;
    } else {
      eosi = 0;
      is_eos = false;
    }

    // -- eos run-up / -down
    for ( ; eosi >= 0; eosi--) {
      token = (char *)lexer.yytext;
      train_next_token();
    }
  }
  // -- cleanup : string-sets
  for (eosi = 0; eosi < kmax; eosi++) {
    curtags = stringq.front();
    stringq.pop_front();
    curtags->clear();
    delete curtags;
  }
  // -- cleanup : ngram-sets
  delete curngrams;
  delete nextngrams;
  curngrams = NULL;
  nextngrams = NULL;
  // -- cleanup : files
  infile = NULL;
  outfile = NULL;

  return true;
}

/*
 * bool dwds_tagger_trainer::train_from_strings(int argc, char **argv, FILE *out=stdout)
 */
bool dwds_tagger_trainer::train_from_strings(int argc, char **argv, FILE *out=stdout)
{
  int i;

  // -- sanity check
  if (!can_tag()) {
    fprintf(stderr, "dwds_tagger_trainer::train_from_strings(): cannot run uninitialized trainer!\n");
    return false;
  }

  // -- tmp instance-vars
  curngrams = new set<FSMSymbolString>;
  nextngrams = new set<FSMSymbolString>;

  // -- files
  infile = NULL;
  outfile = out;

  // -- initialize string-sets
  for (i = 0; i < kmax; i++) {
    curtags = new set<FSMSymbolString>();
    curtags->insert(eos);
    stringq.push_back(curtags);
  }

  // -- ye olde guttes
  for ( ; --argc >= -1*kmax; argv++) {
    token = *argv;
    if (argc <= 0) is_eos = true;
    else is_eos = false;
    train_next_token();

    // -- verbosity
    if (verbose) ntokens++;
  }

  // -- cleanup string-sets
  for (i = 0; i < kmax; i++) {
    curtags = stringq.front();
    stringq.pop_front();
    curtags->clear();
    delete curtags;
  }
  delete curngrams;
  curngrams = NULL;
  delete nextngrams;
  nextngrams = NULL;

  infile = NULL;
  outfile = NULL;

  return true;
}


/*
 * inline void dwds_tagger_trainer::train_next_token(void);
 *   + does the whahosis for the next token
 */
inline void dwds_tagger_trainer::train_next_token(void)
{
    // -- tag it first
    if (!is_eos) {
      s = (char *)token;
      tmp->fsm_clear();
      result = morph->fsm_lookup(s,tmp,true);
    }

    // -- pop 'current' tag-string set (last in queue == oldest)
    curtags = stringq.back();
    stringq.pop_back();

    // -- get the 'next' tag-string set to the 'current' one
    curtags->clear();
    if (!is_eos) {
      if (want_features) {
	results.clear();
	tmp->fsm_strings(syms, &results, false, want_avm);
	// -- set curtags to full string results
	for (ri = results.begin(); ri != results.end(); ri++) {
	  curtags->insert(ri->istr);
	}
      } else {
	// -- all features
	  get_fsm_tag_strings(tmp,curtags);
      }
      // -- unknown token?
      if (curtags->empty()) get_fsm_tag_strings(ufsa,curtags);
    } else {
      // -- terminal EOS
      curtags->insert(eos);
    }
    
    // -- push 'current' tags onto the queue (front of queue == newest)
    stringq.push_front(curtags);
    
    // -- note all 'current' tags in "alltags"
    alltags.insert(curtags->begin(),curtags->end());
    
    // -- counting: i <= kmax -grams
    curngrams->clear();
    for (qi = stringq.begin(); qi != stringq.end(); qi++) {
      // -- grab next ngrams
      if (curngrams->empty()) *curngrams = **qi;
      else {
	// -- get iterator-current tags
	curtags = *qi;
	nextngrams->clear();
	for (g_old = curngrams->begin(); g_old != curngrams->end(); g_old++) {
	  for (g_new = curtags->begin(); g_new != curtags->end(); g_new++) {
	    nextngrams->insert(*g_new + wdsep + *g_old);
	  }
	}
	*curngrams = *nextngrams;
      }
      // -- ... and count them
      for (t = curngrams->begin(); t != curngrams->end(); t++) {
	if ((sci = strings2counts.find(*t)) != strings2counts.end()) {
	    strings2counts[*t] += curngrams->size() >= 0 ? 1.0/(float)curngrams->size() : 0;
	} else {
	    strings2counts[*t] = curngrams->size() >= 0 ? 1.0/(float)curngrams->size() : 0;
	}
      }
    }

    // -- verbosity
    if (verbose) ntokens++;
}


/*--------------------------------------------------------------------------
 * Public Methods: Parameter generation: Parameter-File Read/Write
 *--------------------------------------------------------------------------*/

/*
 * bool dwds_tagger_trainer::write_param_file(FILE *out=stdout);
 */
bool dwds_tagger_trainer::write_param_file(FILE *out=stdout)
{
  // -- sorted string-list
  set<FSMSymbolString> allstrings;
  for (sci = strings2counts.begin(); sci != strings2counts.end(); sci++) {
    allstrings.insert(sci->first);
  }

  fputs("%% Parameter File\n", out);
  for (set<FSMSymbolString>::iterator asi = allstrings.begin(); asi != allstrings.end(); asi++) {
    fprintf(out, "%s\t%f\n", asi->c_str(), strings2counts[*asi]);
  }
  return true;
}

/*
 *
 */
bool dwds_tagger_trainer::read_param_file(FILE *in=stdin)
{
  fprintf(stderr,"dwdst_tagger_trainer::read_param_file(): not yet implemented!\n");
  abort();
  return false;
}

/*--------------------------------------------------------------------------
 *--------------------------------------------------------------------------*/
