/*--------------------------------------------------------------------------
 * File: dwdst_fstgen.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + FST auto-generator for KDWDS PoS-tagger: the guts
 *--------------------------------------------------------------------------*/

#include <stdio.h>
#include <errno.h>

#include "dwdst_fstgen.h"

using namespace std;

/*--------------------------------------------------------------------------
 * Constructor / Destructor
 *--------------------------------------------------------------------------*/

/*
 * dwds_fst_generator::~dwds_fst_generator()
 */
dwds_fst_generator::~dwds_fst_generator()
{
    if (ufsa) ufsa->fsm_free();
    ufsa = NULL;
    if (dfsa) dfsa->fsm_free();
    dfsa = NULL;
}

/*--------------------------------------------------------------------------
 * Public Methods: Listfile-reading
 *--------------------------------------------------------------------------*/

/*
 * set<FSMSymbolString> dwds_fst_generator::read_taglist_file(set<FSMSymbolString> &tagset, const char *filename=NULL);
 *  + implicitly clears 'tagset'
 *  + if 'filename' is NULL, 'tagset' is set to all categories according to 'syms' data member
 */
set<FSMSymbolString> dwds_fst_generator::read_taglist_file(set<FSMSymbolString> &tagset, const char *filename=NULL)
{
  // -- sanity check
  tagset.clear();

  if (!filename) {
    // -- default: all categories
    if (!syms) {
      fprintf(stderr, "dwds_fst_generator::read_taglist_file(): no symbols loaded!\n");
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
      fprintf(stderr,"dwds_fst_generator::read_taglist_file(): could not open file '%s' for read.\n",
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
	fprintf(stderr,"dwds_fst_generator::get_pos_tags(): Error in input file '%s'.\n",
		filename);
      }
    }
  }
  return tagset;
}


/*--------------------------------------------------------------------------
 * Unknown-Analysis FSA generation
 *--------------------------------------------------------------------------*/

/*
 * FSM *dwds_fst_generator::generate_unknown_fsa();
 *   + generates analysis-FSA for tokens unknown to KDWDS morphology
 *   + uses 'opentags' : set of open-class PoS tags
 */
FSM *dwds_fst_generator::generate_unknown_fsa()
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
 * Disambiguation-FST geneartion
 *  --> NOT YET IMPLEMENTED!
 *--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------
 *--------------------------------------------------------------------------*/
