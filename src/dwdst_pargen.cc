/*--------------------------------------------------------------------------
 * File: dwdst_pargen.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger-trainer for DWDS project : the guts
 *--------------------------------------------------------------------------*/

#include <stdio.h>
#include <errno.h>

#include "dwdst_pargen.h"

using namespace std;

/*--------------------------------------------------------------------------
 * Constructor / Destructor
 *--------------------------------------------------------------------------*/

/*
 * dwds_param_generator::~dwds_param_generator()
 */
dwds_param_generator::~dwds_param_generator()
{
    strings2counts.clear();
    stringq.clear();
}


/*--------------------------------------------------------------------------
 * Training Methods
 *--------------------------------------------------------------------------*/

/*
 * bool dwds_param_generator::train_from_stream(FILE *in, FILE *out)
 */
bool dwds_param_generator::train_from_stream(FILE *in, FILE *out)
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
    fprintf(stderr, "dwds_param_generator::train_from_stream(): cannot run uninitialized trainer!\n");
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
 * bool dwds_param_generator::train_from_strings(int argc, char **argv, FILE *out=stdout)
 */
bool dwds_param_generator::train_from_strings(int argc, char **argv, FILE *out=stdout)
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
    fprintf(stderr, "dwds_param_generator::train_from_strings(): cannot run uninitialized trainer!\n");
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
 * Parameter File Output
 *--------------------------------------------------------------------------*/

/*
 * bool dwds_param_generator::write_param_file(FILE *out=stdout);
 */
bool dwds_param_generator::write_param_file(FILE *out=stdout)
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
