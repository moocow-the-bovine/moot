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
  FSMBaseRepresentation *urep;
  FSMState q0, qi;

  // -- ensure that ufsa exists
  if (ufsa) ufsa->fsm_clear();
  else {
    ufsa = new FSM();
    ufsa->set_fsm_type(false,false);
  }

  // -- add start state
  urep = ufsa->representation();
  q0 = urep->set_start_state(urep->add_state(urep->new_state()));

  // -- return an FSA ambiguous between all tags in 'tagset' -- no features!
  for (set<FSMSymbolString>::iterator p = opentags.begin(); p != opentags.end(); p++) {
    qi = urep->add_state(urep->new_state());
    urep->add_transition(q0, qi,
			 syms->symbolname_to_symbol(*p), 
			 syms->symbolname_to_symbol(*p),
			 FSM_default_cost_structure.freecost());
    urep->mark_state_as_final(qi);
    if (want_features) {
      FSMSymbolString catstr = *p;
      if (catstr[0] == '_') { catstr.erase(0,1); }
      const vector<FSMSymbolString> *feats = syms->features_of_category(catstr);
      FSMState qf;
      set<FSMState> pstates;
      pstates.insert(qi);

      for (vector<FSMSymbolString>::const_iterator f = feats->begin(); f != feats->end(); f++) {
	set<FSMSymbol> *values = syms->subtypes_of(*f);
	qf = urep->add_state(urep->new_state());
	for (set<FSMSymbol>::iterator v = values->begin(); v != values->end(); v++) {
	  for (set<FSMState>::iterator ps = pstates.begin(); ps != pstates.end(); ps++) {
	    urep->add_transition(*ps, qf, *v, *v, FSM_default_cost_structure.freecost());
	  }
	}
	pstates.insert(qf);
	urep->mark_state_as_final(qf);
      }
    }
  }
  return ufsa;
}

/*--------------------------------------------------------------------------
 * Public Methods: FSA Generation: Diambiguation-FSA
 *  --> NOT YET IMPLEMENTED!
 *--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------
 * Public Methods: Parameter-generation: Training
 *--------------------------------------------------------------------------*/
/*
 * bool dwds_tagger_trainer::train_from_stream(FILE *in, FILE *out)
 */
bool dwds_tagger_trainer::train_from_stream(FILE *in, FILE *out)
{
  int tok, eosi;
  bool is_eos;
  set<FSMSymbolString> *curtags;
  set<FSMSymbolString>::iterator t;

  set<FSMSymbolString> *curngrams = new set<FSMSymbolString>;
  set<FSMSymbolString> *nextngrams = new set<FSMSymbolString>;

  set<FSMSymbolString>::iterator g_old;
  set<FSMSymbolString>::iterator g_new;

  FSMSymbolStringQueue::iterator qi;

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

  // -- ye olde guttes
  lexer.step_streams(in,out);
  while ((tok = lexer.yylex()) != DTEOF) {
    // -- check for eos
    if (tok == EOS || tok == DTEOF) {
      eosi = kmax;
      is_eos = true;
    }
    else {
      eosi = 0;
      is_eos = false;
    }

    // -- eos run-up / -down
    for ( ; eosi >= 0; eosi--) {
      // -- tag it first
      if (!is_eos) {
	s = (char *)lexer.yytext;
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
      if (verbose) {
	ntokens++;
      }
    }
  }
  // -- cleanup string-sets
  for (eosi = 0; eosi < kmax; eosi++) {
    curtags = stringq.front();
    stringq.pop_front();
    curtags->clear();
    delete curtags;
  }
  delete curngrams;
  delete nextngrams;

  return true;
}

/*
 * bool dwds_tagger_trainer::train_from_strings(int argc, char **argv, FILE *out=stdout)
 */
bool dwds_tagger_trainer::train_from_strings(int argc, char **argv, FILE *out=stdout)
{
  int i;
  set<FSMSymbolString> *curtags;
  set<FSMSymbolString>::iterator t;

  set<FSMSymbolString> *curngrams = new set<FSMSymbolString>;
  set<FSMSymbolString> *nextngrams = new set<FSMSymbolString>;

  set<FSMSymbolString>::iterator g_old;
  set<FSMSymbolString>::iterator g_new;

  FSMSymbolStringQueue::iterator qi;

  // -- sanity check
  if (!can_tag()) {
    fprintf(stderr, "dwds_tagger_trainer::train_from_strings(): cannot run uninitialized trainer!\n");
    return false;
  }

  // -- initialize string-sets
  for (i = 0; i < kmax; i++) {
    curtags = new set<FSMSymbolString>();
    curtags->insert(eos);
    stringq.push_back(curtags);
  }

  // -- ye olde guttes
  for ( ; --argc >= -1*kmax; argv++) {

    // -- tag it first
    if (argc >= 0) {
      s = *argv;
      tmp->fsm_clear();
      result = morph->fsm_lookup(s,tmp,true);
    }
    // -- pop 'current' tag-string set (last in queue == oldest)
    curtags = stringq.back();
    stringq.pop_back();

    // -- get the 'next' tag-string set to the 'current' one
    curtags->clear();
    if (argc >= 0) {
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

#  ifdef DWDST_PARGEN_DEBUG
    // -- output (debug)
    fprintf(out, "%s", s.c_str());
    for (t = curtags->begin(); t != curtags->end(); t++) {
      fputc('\t', out);
      fputs(*(t->c_str()) == '_' ? t->c_str()+1 : t->c_str(), out);
      fprintf(out, "<%f>", curtags->size() >= 0 ? 1.0/(float)curtags->size() : 0);
    }
    fputc('\n',out);
#  endif // DWDST_PARGEN_DEBUG

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
    if (verbose) {
      ntokens++;
    }
  }

  // -- cleanup string-sets
  for (i = 0; i < kmax; i++) {
    curtags = stringq.front();
    stringq.pop_front();
    curtags->clear();
    delete curtags;
  }
  delete curngrams;
  delete nextngrams;

  return true;
}


/*--------------------------------------------------------------------------
 * Public Methods: Parameter generation: Parameter-File Output
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


/*--------------------------------------------------------------------------
 *--------------------------------------------------------------------------*/
