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
 * dwds_tagger::dwds_tagger(FSMSymSpec *mysmys=NULL, FSM *mymorph=NULL)
 */
dwds_tagger::dwds_tagger(FSMSymSpec *mysyms, FSM *mymorph)
{
  // -- public data
  morph = mymorph;
  syms = mysyms;
  eos = "--EOS--";
  wdsep = "\t";
  ufsa = NULL;
  dfsa = NULL;

  // -- public flags
  want_avm = false;
  //want_binary = false;
  want_features = true;
  want_tnt_format = false;
  verbose = false;

  // -- private data
  i_made_morph = false;
  i_made_syms = false;
  i_made_ufsa = false;
  i_made_dfsa = false;

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
 * FSM *dwds_tagger::load_fsm_file(const char *filename, FSM **fsm, bool *i_made_fsm=NULL);
 */
FSM *dwds_tagger::load_fsm_file(char *filename, FSM **fsm, bool *i_made_fsm=NULL)
{
  // -- cleanup old FSM first
  if (*fsm && i_made_fsm && *i_made_fsm) { delete *fsm; }

  *fsm = new FSM(filename);
  if (!**fsm) {
    fprintf(stderr,"\ndwds_tagger::load_fsm_file() Error: load failed for FST file '%s'.\n", filename);
    *fsm = NULL; // -- invalidate the object
  }
  if (i_made_fsm) { *i_made_fsm = true; }
  return *fsm;
}


/*
 * dwds_tagger::~dwds_tagger()
 */
dwds_tagger::~dwds_tagger() {
  if (tmp) delete tmp;
  if (syms && i_made_syms) delete syms;
  if (morph && i_made_morph) delete morph;
  if (ufsa && i_made_ufsa) delete ufsa;
  if (dfsa && i_made_dfsa) delete dfsa;

  syms = NULL;
  morph = NULL;
  ufsa = NULL;
  dfsa = NULL;
  result = NULL;
  tmp = NULL;
}

/*--------------------------------------------------------------------------
 * Tagging Macros
 *--------------------------------------------------------------------------*/

/*
 * inline void dwds_tagger::tag_token(void)
 *  + tags the input-token in this->token
 *  + ouputs to this->outfile
 *  + formats according to this->want_* data members
 */
inline void dwds_tagger::tag_token(void)
{
  tmp->fsm_clear();
  s = (char *)token;
  result = morph->fsm_lookup(s,tmp,true);
  
  fputs(token, outfile);
  if (want_features) {
    /*-- all features */
    results.clear();
    
    /*--all features, strings */
    tmp->fsm_strings(*syms, results, false, want_avm);
    if (verbose && results.empty()) nunknown++;
    if (want_tnt_format) {
      /*-- all features, one tok/line */
      for (ri = results.begin(); ri != results.end(); ri++) {
	fputs("\t", outfile);
	fputs(ri->istr.c_str(), outfile);
	if (ri->weight) fprintf(outfile, "<%f>", ri->weight);
      }
      fputc('\n',outfile);
    } else { /*-- want_tnt_format */
      /*-- all features, madwds-style */
      fprintf(outfile, ": %d Analyse(n)\n", results.size());
      for (ri = results.begin(); ri != results.end(); ri++) {
	fputs("\t", outfile);
	fputs(ri->istr.c_str(), outfile);
	fprintf(outfile, (ri->weight ? "\t<%f>\n" : "\n"), ri->weight);
      }
      fputc('\n',outfile);
    }
  } else { /*-- want_features */
    /*-- tags only, strings */
    tagresults.clear();
    get_fsm_tag_strings(tmp, &tagresults);
    if (verbose && tagresults.empty()) nunknown++;
    
    if (want_tnt_format) {
      /*-- tags-only, one tok/line */
      for (tri = tagresults.begin(); tri != tagresults.end(); tri++) {
	fputc('\t', outfile);
	fputs(want_avm && *(tri->c_str()) == '_'
	      ? tri->c_str()+1
	      : tri->c_str(),
	      outfile);
      }
      fputc('\n', outfile);
    } else { /*-- want_tnt_format */
      /*-- tags-only, madwds native format */
      fprintf(outfile, ": %d Analyse(n)\n", tagresults.size());
      for (tri = tagresults.begin(); tri != tagresults.end(); tri++) {
	  /*
	    fputs("\t[", outfile);
	    fputs(want_avm && *(tri->c_str()) == '_'
	       ? tri->c_str()+1
   	       : tri->c_str(),
	       outfile);
	    fputs("]\n", outfile);
	  */
	fputc('\t',outfile);
	fputs(tri->c_str(), outfile);
	fputc('\n', outfile);
      }
      fputc('\n',outfile);
    }
  }
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

  // -- files
  infile = in;
  outfile = out;

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
      token = (char *)lexer.yytext;
      tag_token();
    }
  }

  // -- cleanup
  infile = NULL;
  outfile = NULL;

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

  // -- files
  infile = NULL;
  outfile = out;
  
  // -- ye olde guttes
  for ( ; --argc >= 0; argv++) {
    // -- verbosity
    if (verbose) ntokens++;

    token = *argv;
    tag_token();
  }

  // -- cleanup
  infile = NULL;
  outfile = NULL;

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
  FSMRepresentation *rep = fsa->fsm_representation();

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
  get_fsm_tags(fsa,&fsm_tags_tmp);

  // -- ensure 'tag_strings' is defined
  if (!tag_strings) tag_strings = new set<FSMSymbolString>();

  for (set<FSMSymbol>::const_iterator t = fsm_tags_tmp.begin(); t != fsm_tags_tmp.end(); t++) {
      FSMSymbolString tagstr = *(syms->symbol_to_symbolname(*t));
      if (want_avm && tagstr[0] == '_') { tagstr.replace(0,1,"["); }
      else { tagstr.insert(0,"["); }
      tagstr.append("]");
      tag_strings->insert(tagstr);
  }

  fsm_tags_tmp.clear();
  return tag_strings;
}
