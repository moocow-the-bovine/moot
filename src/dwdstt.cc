/*--------------------------------------------------------------------------
 * File: dwdstt.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger-trainer for DWDS project : the guts
 *--------------------------------------------------------------------------*/

#include <stdio.h>
#include <errno.h>

#include "dwdstt.h"

using namespace std;

/*--------------------------------------------------------------------------
 * Constructor / Destructor
 *--------------------------------------------------------------------------*/

/*
 * dwds_tagger_trainer::~dwds_tagger_trainer()
 */
dwds_tagger_trainer::~dwds_tagger_trainer()
{
    strings2counts.clear();
    stringq.clear();
    if (ufsa) ufsa->fsm_free();
    ufsa = NULL;
}

/*--------------------------------------------------------------------------
 * Unknown-Analysis FSA generation
 *--------------------------------------------------------------------------*/

/*
 * set<FSMSymbolString> dwds_tagger_trainer::get_pos_tags(const char *filename)
 */
set<FSMSymbolString> dwds_tagger_trainer::get_pos_tags(const char *filename = NULL) {
  set<FSMSymbolString>::iterator p;

  // -- sanity check
  if (!syms) {
    fprintf(stderr, "dwds_tagger_trainer::get_pos_tags(): no symbols loaded!\n");
    return tagset;
  }

  tagset.clear();

  if (!filename) {
    // -- default: all categories
    const set<FSMSymbolString> *symbols = syms->symbols();
    for (p = symbols->begin(); p != symbols->end(); p++) {
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
      fprintf(stderr,"dwds_tagger_trainer::get_pos_tags(): could not open file '%s' for read.\n",
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

/*
 * FSM *dwds_tagger_trainer::generate_unknown_fsa();
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
  for (set<FSMSymbolString>::iterator p = tagset.begin(); p != tagset.end(); p++) {
    qi = urep->add_state(urep->new_state());
    urep->add_transition(q0, qi,
			 syms->symbolname_to_symbol(*p), 
			 syms->symbolname_to_symbol(*p),
			 FSM_default_cost_structure.freecost());
    urep->mark_state_as_final(qi);
  }

  return ufsa;
}




/*--------------------------------------------------------------------------
 * Training Methods
 *--------------------------------------------------------------------------*/

/*
 * bool dwds_tagger_trainer::train_from_stream(FILE *in, FILE *out)
 */
bool dwds_tagger_trainer::train_from_stream(FILE *in, FILE *out)
{
  int tok;

  // -- sanity check
  if (!can_tag()) {
    fprintf(stderr, "dwds_tagger_trainer::train_from_stream(): cannot run uninitialized trainer!\n");
    return false;
  }

  // -- HACK
  fprintf(stderr, "dwds_tagger_trainer::train_from_stream(): not yet implemented.\n");
  abort();

  lexer.step_streams(in,out);
  while ((tok = lexer.yylex()) != DTEOF) {
    switch (tok) {
    case EOS:
      /* do something spiffy */
      fputs(eos.c_str(), out);
      fputs("\n\n", out);
      break;

    case TOKEN:
    default:
      tmp->fsm_clear();
      results.clear();
      s = (char *)lexer.yytext;
      
      result = morph->fsm_lookup(s,tmp,true);  // v0.0.2 -- getting leaks here!
      tmp->fsm_strings(syms, &results, false, want_avm);
      
      fprintf(out, "%s: %d Analyse(n)\n", lexer.yytext, results.size());
      for (ri = results.begin(); ri != results.end(); ri++) {
	fputs("\t", out);
	fputs(ri->istr.c_str(), out);
	fprintf(out, (ri->weight ? "\t<%f>\n" : "\n"), ri->weight);
      }
      fputc('\n',out);
      
      // -- verbosity
      if (verbose) {
	ntokens++;
	if (results.empty()) nunknown++;
      }
    }
  }
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

  // -- sanity check
  if (!can_tag()) {
    fprintf(stderr, "dwds_tagger_trainer::train_from_strings(): cannot run uninitialized trainer!\n");
    return false;
  }

  // -- initialize string-sets
  for (i = 0; i < kmax; i++) {
    curtags = new set<FSMSymbolString>();
    curtags->insert(eos);
    stringq.push_front(curtags);
  }

  // -- ye olde guttes
  for ( ; --argc >= 0; argv++) {

    // -- tag it first
    s = *argv;
    tmp->fsm_clear();
    result = morph->fsm_lookup(s,tmp,true);

    // -- pop 'current' string set
    curtags = stringq.front();
    stringq.pop_front();

    // -- get the 'current' tags
    curtags->clear();
    get_fsm_tag_strings(tmp,curtags);

    // -- unknown token?
    if (curtags->empty()) get_fsm_tag_strings(ufsa,curtags);

    // -- push 'current' tags onto the queue
    stringq.push_back(curtags);

    fprintf(out, "%s", s.c_str());
    for (t = curtags->begin(); t != curtags->end(); t++) {
      fputc('\t', out);
      fputs(*(t->c_str()) == '_' ? t->c_str()+1 : t->c_str(), out);
    }
    fputc('\n',out);

    /* tmp->fsm_strings(syms, &results, false, want_avm);
    for (ri = results.begin(); ri != results.end(); ri++) {
      fprintf(out, "\t((%s)<%f>)\n", ri->istr.c_str(), ri->weight);
    }
    fputc('\n',out);*/

    // -- verbosity
    if (verbose) {
      ntokens++;
      if (results.empty()) nunknown++;
    }
  }

  // -- cleanup string-sets
  for (i = 0; i < kmax; i++) {
    curtags = stringq.front();
    stringq.pop_front();
    curtags->clear();
    delete curtags;
  }

  return true;
}


/*--------------------------------------------------------------------------
 *--------------------------------------------------------------------------*/
