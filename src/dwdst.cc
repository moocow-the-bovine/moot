/*--------------------------------------------------------------------------
 * File: dwdst.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger for DWDS project : the guts
 *--------------------------------------------------------------------------*/

#include <stdio.h>

#include <string>
#include <set>
#include <list>

#include <FSMTypes.h>

#include "dwdst_lexer.h"
#include "dwdst.h"

using namespace std;

/*--------------------------------------------------------------------------
 * Constructor / Destructor
 *--------------------------------------------------------------------------*/

/*
 * dwds_tagger::dwds_tagger(FSM *mymorph=NULL, FSMSymSpec *mysmys=NULL)
 */
dwds_tagger::dwds_tagger(FSM *mymorph, FSMSymSpec *mysyms)
{
  // -- public data
  morph = mymorph;
  syms = mysyms;

  // -- public flags
  want_avm = false;
  verbose = false;

  // -- private data
  i_made_morph = false;
  i_made_syms = false;

  // -- temporary variables
  result = NULL;
}



/*
 * FSMSymSpec *dwds_tagger::load_symbols(const char *filename)
 */
FSMSymSpec *dwds_tagger::load_symbols(char *filename)
{
  list<string> msglist;

  // -- cleanup old symbols first
  if (syms && i_made_syms) delete syms;

  syms = new FSMSymSpec(filename, &msglist, DWDST_SYM_ATT_COMPAT);
  if (!msglist.empty()) {
    fprintf(stderr,"\ndwdst_tagger::load_symbols() Error: could not load symbols from file '%s'\n", filename);
    for (list<string>::iterator e = msglist.begin(); e != msglist.end(); e++) {
      fprintf(stderr,"%s\n",e->c_str());
    }
    syms = NULL; // -- invalidate the object
  }

  i_made_syms = true;
  return syms;
}

/*
 * dwds_tagger::load_morph(const char *filename)
 */
FSM *dwds_tagger::load_morph(char *filename)
{
  int fsmtype;

  // -- cleanup old FSM first
  if (morph && i_made_morph) {
    morph->fsm_free();
  }

  morph = new FSM(filename);
  fsmtype = morph->fsm_type();
  if (!morph || !morph->representation()
      // HACK!
      || (fsmtype != FSMTypeTransducer &&
	  fsmtype != FSMTypeWeightedTransducer &&
	  fsmtype != FSMTypeSubsequentialTransducer &&
	  fsmtype != FSMTypeAcceptor &&
	  fsmtype != FSMTypeWeightedAcceptor))
    {   
      fprintf(stderr,"\ndwds_tagger::load_morph() Error: load failed for FST file '%s'.\n", filename);
      morph = NULL; // -- invalidate the object
    }
  i_made_morph = true;
  return morph;
}


/*
 * dwds_tagger::~dwds_tagger()
 */
dwds_tagger::~dwds_tagger() {
  if (morph && i_made_morph) {
    morph->fsm_free();
  }
  if (syms && i_made_syms) delete syms;

  morph = NULL;
  syms = NULL;
  result = NULL;
}

/*--------------------------------------------------------------------------
 * Tagging Methods
 *--------------------------------------------------------------------------*/

/*
 * bool dwds_tagger::tag_stream(FILE *in, FILE *out)
 */
bool dwds_tagger::tag_stream(FILE *in, FILE *out)
{
  int tok;

  // -- sanity check
  if (!can_tag()) {
    fprintf(stderr, "dwds_tagger::tag_strings(): cannot run uninitialized tagger!\n");
    return false;
  }

  lexer.step_streams(in,out);
  while ((tok = lexer.yylex()) != DTEOF) {
    switch (tok) {
    case EOS:
      /* do something spiffy */
      break;

    case TOKEN:
    default:
      tmp.fsm_clear();
      results.clear();
      s = (char *)lexer.yytext;
      
      result = morph->fsm_lookup(s,&tmp,true);
      tmp.fsm_strings(syms, &results, false, want_avm);
      
      fprintf(out, "%s: %d Analyse(n)\n", lexer.yytext, results.size());
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
  }
  return true;
}

/*
 * bool dwds_tagger::tag_strings(int argc, char **argv, FILE *out=stdout)
 */
bool dwds_tagger::tag_strings(int argc, char **argv, FILE *out=stdout)
{
  // -- sanity check
  if (!can_tag()) {
    fprintf(stderr, "dwds_tagger::tag_strings(): cannot run uninitialized tagger!\n");
    return false;
  }
  
  // -- ye olde guttes
  for ( ; --argc >= 0; argv++) {
    tmp.fsm_clear();
    results.clear();
    s = *argv;

    result = morph->fsm_lookup(s,&tmp,true);
    tmp.fsm_strings(syms, &results, false, want_avm);

    fprintf(out, "%s: %d Analyse(n)\n", *argv, results.size());
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

  return true;
}
