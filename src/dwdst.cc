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
  eos = "<<EOS>>";
  wdsep = "=";

  // -- public flags
  want_avm = false;
  want_numeric = false;
  want_features = true;
  want_tnt_format = false;
  verbose = false;

  // -- private data
  i_made_morph = false;
  i_made_syms = false;

  // -- temporary variables
  tmp = new FSM();
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
  if (morph && i_made_morph) morph->fsm_free();

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
  if (tmp) tmp->fsm_free();
  if (morph && i_made_morph) morph->fsm_free();
  if (syms && i_made_syms) delete syms;

  morph = NULL;
  syms = NULL;
  result = NULL;
  tmp = NULL;
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
    fprintf(stderr, "dwds_tagger::tag_stream(): cannot run uninitialized tagger!\n");
    return false;
  }

  lexer.step_streams(in,out);
  while ((tok = lexer.yylex()) != DTEOF) {
    switch (tok) {
    case EOS:
      /* do something spiffy */
      if (want_tnt_format) {
	fputc('\n', out);
      } else {
	fputs(eos.c_str(), out);
	fputs("\n\n", out);
      }
      break;

    case TOKEN:
    default:
      if (verbose) ntokens++;

      tmp->fsm_clear();
      s = (char *)lexer.yytext;
      result = morph->fsm_lookup(s,tmp,true);  // v0.0.2 -- getting leaks here!

      fputs((char *)lexer.yytext, out);

      if (want_features) {
	// -- all features
	if (want_numeric) {
	  nresults.clear();
	  // -- all features, numeric
	  tmp->fsm_bstrings(&nresults, false);
	  if (verbose && nresults.empty()) nunknown++;

	  if (want_tnt_format) {
	    // -- all features, numeric, one tok/line
	    for (nri = nresults.begin(); nri != nresults.end(); nri++) {
	      fputs("\t", out);
	      for (vi = nri->ivec.begin(); vi != nri->ivec.end(); vi++) {
		fprintf(out, " %d", *vi);
	      }
	      if (nri->weight) fprintf(out, "<%f>", nri->weight);
	    }
	    fputc('\n',out);
	  } else { // want_tnt_format
	    // -- all features, numeric, madwds-style
	    fprintf(out, ": %d Analyse(n)\n", nresults.size());
	    for (nri = nresults.begin(); nri != nresults.end(); nri++) {
	      fputs("\t", out);
	      //fputs(nri->istr.c_str(), out);
	      for (vi = nri->ivec.begin(); vi != nri->ivec.end(); vi++) {
		fprintf(out, " %d", *vi);
	      }
	      fprintf(out, (nri->weight ? "\t<%f>\n" : "\n"), nri->weight);
	    }
	    fputc('\n',out);
	  } 
	} else { // want_numeric
	  results.clear();
	  // -- all features, strings
	  tmp->fsm_strings(syms, &results, false, want_avm);
	  if (verbose && results.empty()) nunknown++;
	  if (want_tnt_format) {
	    // -- all features, one tok/line
	    for (ri = results.begin(); ri != results.end(); ri++) {
	      fputs("\t", out);
	      fputs(ri->istr.c_str(), out);
	      if (ri->weight) fprintf(out, "<%f>", ri->weight);
	    }
	    fputc('\n',out);
	  } else { // want_tnt_format
	    // -- all features, madwds-style
	    fprintf(out, ": %d Analyse(n)\n", results.size());
	    for (ri = results.begin(); ri != results.end(); ri++) {
	      fputs("\t", out);
	      fputs(ri->istr.c_str(), out);
	      fprintf(out, (ri->weight ? "\t<%f>\n" : "\n"), ri->weight);
	    }
	    fputc('\n',out);
	  } 
	}
      } else { // want_features
	if (want_numeric) {
	  // -- tags only, numeric
	  ntagresults.clear();
	  get_fsm_tags(tmp, &ntagresults);
	  if (verbose && ntagresults.empty()) nunknown++;
	  
	  if (want_tnt_format) {
	    // -- tags-only, numeric, one tok/line
	    for (ntri = ntagresults.begin(); ntri != ntagresults.end(); ntri++) {
	      fprintf(out, "\t%d", *ntri);
	    }
	    fputc('\n', out);
	  } else { // want_tnt_format
	    // -- tags-only, numeric madwds-style
	    fprintf(out, ": %d Analyse(n)\n", ntagresults.size());
	    for (ntri = ntagresults.begin(); ntri != ntagresults.end(); ntri++) {
	      fprintf(out, "\t%d\n", *ntri);
	    }
	    fputc('\n',out);
	  }
	} else { // want_numeric
	  // -- tags only, strings
	  tagresults.clear();
	  get_fsm_tag_strings(tmp, &tagresults);
	  if (verbose && tagresults.empty()) nunknown++;
	  
	  if (want_tnt_format) {
	    // -- tags-only, one tok/line
	    for (tri = tagresults.begin(); tri != tagresults.end(); tri++) {
	      fputc('\t', out);
	      fputs(*(tri->c_str()) == '_' ? tri->c_str()+1 : tri->c_str(), out);
	    }
	    fputc('\n', out);
	  } else { // want_tnt_format
	    // -- tags-only, madwds native format
	    fprintf(out, ": %d Analyse(n)\n", tagresults.size());
	    for (tri = tagresults.begin(); tri != tagresults.end(); tri++) {
	      fputs("\t[", out);
	      fputs(tri->c_str(), out);
	      fputs("]\n", out);
	    }
	    fputc('\n',out);
	  }
	}
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
    // -- verbosity
    if (verbose) ntokens++;

    tmp->fsm_clear();
    results.clear();
    s = *argv;

    result = morph->fsm_lookup(s,tmp,true);

    fputs(*argv, out);
    if (want_features) {
      // -- all features
      if (want_numeric) {
	nresults.clear();
	// -- all features, numeric
	tmp->fsm_bstrings(&nresults, false);
	if (verbose && nresults.empty()) nunknown++;
	
	if (want_tnt_format) {
	  // -- all features, numeric, one tok/line
	  for (nri = nresults.begin(); nri != nresults.end(); nri++) {
	    fputs("\t", out);
	    for (vi = nri->ivec.begin(); vi != nri->ivec.end(); vi++) {
	      fprintf(out, " %d", *vi);
	    }
	    if (nri->weight) fprintf(out, "<%f>", nri->weight);
	  }
	  fputc('\n',out);
	} else { // want_tnt_format
	  // -- all features, numeric, madwds-style
	  fprintf(out, ": %d Analyse(n)\n", nresults.size());
	  for (nri = nresults.begin(); nri != nresults.end(); nri++) {
	    fputs("\t", out);
	    //fputs(nri->istr.c_str(), out);
	    for (vi = nri->ivec.begin(); vi != nri->ivec.end(); vi++) {
	      fprintf(out, " %d", *vi);
	    }
	    fprintf(out, (nri->weight ? "\t<%f>\n" : "\n"), nri->weight);
	  }
	  fputc('\n',out);
	} 
      } else { // want_numeric
	results.clear();
	// -- all features, strings
	tmp->fsm_strings(syms, &results, false, want_avm);
	if (verbose && results.empty()) nunknown++;
	if (want_tnt_format) {
	  // -- all features, one tok/line
	  for (ri = results.begin(); ri != results.end(); ri++) {
	    fputs("\t", out);
	    fputs(ri->istr.c_str(), out);
	    if (ri->weight) fprintf(out, "<%f>", ri->weight);
	  }
	  fputc('\n',out);
	} else { // want_tnt_format
	  // -- all features, madwds-style
	  fprintf(out, ": %d Analyse(n)\n", results.size());
	  for (ri = results.begin(); ri != results.end(); ri++) {
	    fputs("\t", out);
	    fputs(ri->istr.c_str(), out);
	    fprintf(out, (ri->weight ? "\t<%f>\n" : "\n"), ri->weight);
	  }
	  fputc('\n',out);
	} 
      }
    } else { // want_features
      if (want_numeric) {
	// -- tags only, numeric
	ntagresults.clear();
	get_fsm_tags(tmp, &ntagresults);
	if (verbose && ntagresults.empty()) nunknown++;
	
	if (want_tnt_format) {
	  // -- tags-only, numeric, one tok/line
	  for (ntri = ntagresults.begin(); ntri != ntagresults.end(); ntri++) {
	    fprintf(out, "\t%d", *ntri);
	  }
	  fputc('\n', out);
	} else { // want_tnt_format
	  // -- tags-only, numeric madwds-style
	  fprintf(out, ": %d Analyse(n)\n", ntagresults.size());
	  for (ntri = ntagresults.begin(); ntri != ntagresults.end(); ntri++) {
	    fprintf(out, "\t%d\n", *ntri);
	  }
	  fputc('\n',out);
	}
      } else { // want_numeric
	// -- tags only, strings
	tagresults.clear();
	get_fsm_tag_strings(tmp, &tagresults);
	if (verbose && tagresults.empty()) nunknown++;
	
	if (want_tnt_format) {
	  // -- tags-only, one tok/line
	  for (tri = tagresults.begin(); tri != tagresults.end(); tri++) {
	    fputc('\t', out);
	    fputs(*(tri->c_str()) == '_' ? tri->c_str()+1 : tri->c_str(), out);
	  }
	  fputc('\n', out);
	} else { // want_tnt_format
	  // -- tags-only, madwds native format
	  fprintf(out, ": %d Analyse(n)\n", tagresults.size());
	  for (tri = tagresults.begin(); tri != tagresults.end(); tri++) {
	    fputs("\t[", out);
	    fputs(tri->c_str(), out);
	    fputs("]\n", out);
	  }
	  fputc('\n',out);
	}
      }
    }
  }
  return true;
}


/*--------------------------------------------------------------------------
 * Low-level methods
 *--------------------------------------------------------------------------*/

/*
 * set<FSMSymbol> *dwdst_tagger::get_fsm_tags(FSM *fsa, set<FSMSymbol> *tags=NULL);
 */
set<FSMSymbol> *dwds_tagger::get_fsm_tags(FSM *fsa, set<FSMSymbol> *tags=NULL)
{
  FSMState q0;
  FSMTransitions *arcs;
  FSMTransition  *arc;
  FSMBaseRepresentation *rep = fsa->representation();

  // -- ensure 'tags' is defined
  if (!tags) tags = new set<FSMSymbol>();

  // -- get start-state
  q0 = rep->start_state();
  if (q0 == FSMNOSTATE) return tags;

  // -- get input-labels for all transitions from q0
  //    --> WARNING: if you get an '<epsilon>' in here, you
  //                 DO NOT have all initial symbols!
  arcs = rep->transitions(q0,false);
  while ((arc = arcs->next_transition()) != NULL) {
    tags->insert(arc->ilabel);
  }
  rep->delete_transition_iterator(arcs,false);

  return tags;
}


/*
 * set<FSMSymbolString> *dwdst_tagger::get_fsm_tag_strings(FSM *fsa, set<FSMSymbolString> *tag_strings=NULL);
 */
set<FSMSymbolString> *dwds_tagger::get_fsm_tag_strings(FSM *fsa, set<FSMSymbolString> *tag_strings=NULL)
{
  set<FSMSymbol> *tags = get_fsm_tags(fsa,NULL);

  // -- ensure 'tag_strings' is defined
  if (!tag_strings) tag_strings = new set<FSMSymbolString>();

  for (set<FSMSymbol>::const_iterator t = tags->begin(); t != tags->end(); t++) {
    tag_strings->insert(*(syms->symbol_to_symbolname(*t)));
  }

  delete tags;
  return tag_strings;
}
