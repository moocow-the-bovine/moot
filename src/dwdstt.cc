/*--------------------------------------------------------------------------
 * File: dwdstt.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger-trainer for DWDS project : the guts
 *--------------------------------------------------------------------------*/

#include <stdio.h>

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
}

/*--------------------------------------------------------------------------
 * Unknown-Analysis FSA generation
 *--------------------------------------------------------------------------*/

/*
 * FSM *dwds_tagger_trainer::generate_unknown_fsa(set<FSMSymbol> pos_tags);
 */
FSM *dwds_tagger_trainer::generate_unknown_fsa(set<FSMSymbol> pos_tags)
{
  // ???
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
      fputs("<<EOS>>\n\n", out);
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
  string sent;
  string eos = "EOS";

  // -- sanity check
  if (!can_tag()) {
    fprintf(stderr, "dwds_tagger_trainer::tag_strings(): cannot run uninitialized trainer!\n");
    return false;
  }

  // -- initialize string-sets
  for (i = 0; i < kmax; i++) {
    stringq.push_front(*(new set<FSMStringWeight>()));
    stringq.front().insert(stringq.front().begin(), *(new FSMStringWeight(eos)));
  }
 
  // -- ye olde guttes
  for ( ; --argc >= 0; argv++) {
    // -- tag it first
    tmp->fsm_clear();

    // ARGH!

    results.clear();
    s = *argv;
    result = morph->fsm_lookup(s,tmp,true);
    tmp->fsm_strings(syms, &results, false, want_avm);

    for (ri = results.begin(); ri != results.end(); ri++) {
      fprintf(out, "\t((%s)<%f>)\n", ri->istr.c_str(), ri->weight);
    }
    fputc('\n',out);

    // -- verbosity
    if (verbose) {
      ntokens++;
      if (results.empty()) nunknown++;
    }
  }

  // -- cleanup string-sets
  stringq.clear();

  return true;
}
