/*--------------------------------------------------------------------------
 * File: dwdstTagger.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger for DWDS project : the guts
 *--------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
# include <dwdstConfig.h>
#endif

#include <stdio.h>
#include <stdarg.h>

#include <string>
#include <set>
#include <list>

#include <FSMTypes.h>

#include "dwdstTaggerLexer.h"
#include "dwdstTagger.h"

using namespace std;

/*--------------------------------------------------------------------------
 * Constructor / Destructor
 *--------------------------------------------------------------------------*/

/*
 * dwdstTagger::dwdstTagger(FSMSymSpec *mysmys=NULL, FSM *mymorph=NULL)
 */
dwdstTagger::dwdstTagger(FSMSymSpec *mysyms, FSM *mymorph)
{
  // -- public data
  morph = mymorph;
  syms = mysyms;
  eos = "--EOS--";

  // -- public flags
  want_avm = false;
  want_mabbaw_format = false;
  track_statistics = false;
  do_dequote = false;
  verbose = 1;

  // -- private data
  i_made_morph = false;
  i_made_syms = false;

  // -- temporary variables
  tmp = new FSM();
  result = NULL;

  // -- files
  infile = NULL;
  outfile = NULL;
  srcname = NULL;
}

/*
 * FSMSymSpec *dwdstTagger::load_morph_symbols(const char *filename)
 */
FSMSymSpec *dwdstTagger::load_morph_symbols(char *filename)
{
  // -- cleanup old symbols first
  if (syms && i_made_syms) delete syms;

  syms = new FSMSymSpec(filename, &syms_msgs, DWDST_SYM_ATT_COMPAT);
  if (!syms_msgs.empty()) {
    fprintf(stderr,
	    "\ndwdstTagger::load_symbols() Error: could not load symbols from file '%s'\n",
	    filename);
    for (list<string>::iterator e = syms_msgs.begin(); e != syms_msgs.end(); e++) {
      fprintf(stderr,"%s\n",e->c_str());
    }
    syms_msgs.clear(); // -- clear messages
    syms = NULL;       // -- invalidate the tagger object
  }

  i_made_syms = true;
  return syms;
}

/*
 * FSM *dwdstTagger::load_fsm_file(const char *filename, FSM **fsm, bool *i_made_fsm=NULL);
 */
FSM *dwdstTagger::load_fsm_file(char *filename, FSM **fsm, bool *i_made_fsm=NULL)
{
  // -- cleanup old FSM first
  if (*fsm && i_made_fsm && *i_made_fsm) { delete *fsm; }

  *fsm = new FSM(filename);
  if (!**fsm) {
    fprintf(stderr,"\ndwdstTagger::load_fsm_file() Error: load failed for FSM file '%s'.\n", filename);
    *fsm = NULL; // -- invalidate the object
  }
  if (i_made_fsm) { *i_made_fsm = true; }
  return *fsm;
}


/*
 * dwdstTagger::~dwdstTagger()
 */
dwdstTagger::~dwdstTagger() {
  if (tmp) delete tmp;
  if (syms && i_made_syms) delete syms;
  if (morph && i_made_morph) delete morph;

  syms = NULL;
  morph = NULL;
  result = NULL;
  tmp = NULL;
}

/*--------------------------------------------------------------------------
 * mid-Level Tagging Methods
 *--------------------------------------------------------------------------*/

//(inlined)


/*--------------------------------------------------------------------------
 * High-Level Tagging Methods
 *--------------------------------------------------------------------------*/

/*
 * bool dwdstTagger::tag_stream(FILE *in, FILE *out, char *srcname)
 */
bool dwdstTagger::tag_stream(FILE *in, FILE *out, char *infilename)
{
  int tok;

  // -- sanity check
  if (!can_tag()) {
    fprintf(stderr, "dwdstTagger::tag_stream(): cannot run uninitialized tagger!\n");
    return false;
  }

  // -- files
  infile = in;
  outfile = out;
  srcname = infilename;

  // -- lexer prep
  lexer.step_streams(in,out);
  lexer.theLine = 1;
  lexer.theColumn = 0;

  while ((tok = lexer.yylex()) != dwdstTaggerLexer::DTEOF) {
    switch (tok) {
    case dwdstTaggerLexer::EOS:
      /* do something spiffy */
      tag_eos();
      break;

    case dwdstTaggerLexer::TAG:
    case dwdstTaggerLexer::EOT:
      break;

    case dwdstTaggerLexer::TOKEN:
    default:
      if (track_statistics) ntokens++;

      curtok = (char *)lexer.yytext;
      tag_token();
      if (verbose > 0) print_token_analyses();
    }
  }

  // -- cleanup
  infile = NULL;
  outfile = NULL;
  srcname = NULL;

  return true;
}

/*
 * bool dwdstTagger::tag_strings(int argc, char **argv, FILE *out, char *infilename)
 */
bool dwdstTagger::tag_strings(int argc, char **argv, FILE *out, char *infilename)
{
  // -- sanity check
  if (!can_tag()) {
    fprintf(stderr, "dwdstTagger::tag_strings(): cannot run uninitialized tagger!\n");
    return false;
  }

  // -- files
  infile = NULL;
  outfile = out;
  srcname = infilename;
  
  // -- ye olde guttes
  for ( ; --argc >= 0; argv++) {
    // -- verbosity
    if (track_statistics) ntokens++;

    curtok = *argv;
    tag_token();
    if (verbose > 0) print_token_analyses();
  }
  tag_eos();

  // -- cleanup
  infile = NULL;
  outfile = NULL;

  return true;
}


/*--------------------------------------------------------------------------
 * Low-level methods
 *--------------------------------------------------------------------------*/

/*
 * set<FSMSymbol> *dwdstTagger::get_fsm_tags(FSM *fsa, set<FSMSymbol> *tags=NULL);
 * -- OBSOLETE HACK
 */
/*
set<FSMSymbol> *dwdstTagger::get_fsm_tags(FSM *fsa, set<FSMSymbol> *tags=NULL)
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
*/

/*
 * set<FSMSymbolString> *dwdstTagger::get_fsm_tag_strings(FSM *fsa, set<FSMSymbolString> *tag_strings=NULL);
 * -- OBSOLETE HACK
 */
/*
set<FSMSymbolString> *
dwdstTagger::get_fsm_tag_strings(FSM *fsa, set<FSMSymbolString> *tag_strings=NULL)
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
*/

/*--------------------------------------------------------------------------
 * Low-level tagging utilities: output
 *--------------------------------------------------------------------------*/

//(inlined)


void dwdstTagger::symbol_vector_to_string_dq(const vector<FSMSymbol> &vec, FSMSymbolString &str)
{
  str.clear();
  for (vector<FSMSymbol>::const_iterator vi = vec.begin(); vi != vec.end(); ) {
    if (*vi == EPSILON || *vi == FSMNOLABEL) {
      vi++;
      continue;
    }
    register const FSMSymbolString *isym = syms->symbol_to_symbolname(*vi);
    if (isym == NULL) {
      if (verbose > 2) {
	carp("dwdstTagger::symbol_vector_to_string_sq(): Error: undefined symbol '%d' -- ignored", *vi);
      }
      vi++;
      continue;
    }
    else { //-- isym==NULL
      // -- it's a kosher symbol
      if ((*isym)[0] == '_') {
	// -- it's a category (hack!)
	str.append(*isym, 1, isym->length());
	const vector<FSMSymbolString>* feat = syms->features_of_category(*isym);
	vi++;
	for (vector<FSMSymbolString>::const_iterator fi = feat->begin();
	     fi != feat->end() && vi != vec.end();
	     vi++, fi++)
	  {
	    const set<FSMSymbol> *fsubtypes = syms->subtypes_of(*fi);
	    if (fsubtypes->find(*vi) == fsubtypes->end()) {
	      // -- not a valid value for this feature -- assume the category is done
	      break;
	    }
	    isym = syms->symbol_to_symbolname(*vi);
	    str.push_back('.');
	    str.append(*isym);
	  }
	// -- done with this category
	str.push_back('.');  //-- just in case
	continue; // -- we've already incremented vi enough
      }
      else { //-- isym[0]=='_' : non-category
	if (isym->length() > 1) {
	  str.append(*isym);
	  str.push_back('.');
	} else {
	  str.push_back((*isym)[0]);
	}
      }
    }
    vi++;
  }
  //-- delete trailing '.'
  if (str[str.size()-1] == '.') str.pop_back();
};




/*--------------------------------------------------------------------------
 * Debugging Methods
 *--------------------------------------------------------------------------*/

/** Convert a symbol-vector to a string of numeric ASCII string */
string dwdstTagger::symbol_vector_to_ascii(const FSM::FSMSymbolVector v)
{
  string vs;
  char buf[256];
  for (FSM::FSMSymbolVector::const_iterator vi = v.begin(); vi != v.end(); vi++)
    {
      sprintf(buf,"%d_",*vi);
      vs.append(buf);
    }
  return vs;
}

/** Stringify a token-analysis-set (weighted-vector version) */
string dwdstTagger::analyses_to_string(const set<FSM::FSMWeightedSymbolVector> &analyses)
{
  string s = "{";
  for (set<FSM::FSMWeightedSymbolVector>::const_iterator asi = analyses.begin();
       asi != analyses.end();
       asi++)
    {
      if (asi != analyses.begin()) s += ", ";
      s += symbol_vector_to_ascii(asi->istr);
      if (!asi->ostr.empty()) {
	s += " : ";
	s += symbol_vector_to_ascii(asi->istr);
      }
      if (asi->weight != 0.0) {
	char buf[256];
	sprintf(buf, " <%g>", asi->weight);
	s += buf;
      }
    }
  s += "}";
  return s;
}


/** Stringify a token-analysis-set (weighted-string-version) */
string
dwdstTagger::analyses_to_string(const set<FSM::FSMStringWeight> &analyses)
{
  string s = "{";
  for (set<FSM::FSMStringWeight>::const_iterator asi = analyses.begin();
       asi != analyses.end();
       asi++)
    {
      if (asi != analyses.begin()) s += ", ";
      s += asi->istr;
      if (!asi->ostr.empty() && asi->ostr != "") {
	s += " : ";
	s += asi->ostr;
      }
      if (asi->weight != 0.0) {
	char buf[256];
	sprintf(buf, " <%g>", asi->weight);
	s += buf;
      }
    }
  s += "}";
  return s;
}

/** Stringify a token-analysis-set (numeric-tags version) */
string
dwdstTagger::analyses_to_string(const set<FSMSymbol> &analyses)
{
  string s = "{";
  char buf[256];
  for (set<FSMSymbol>::const_iterator asi = analyses.begin();
       asi != analyses.end();
       asi++)
    {
      if (asi != analyses.begin()) s += ", ";
      sprintf(buf,"%d", *asi);
      s += buf;
    }
  s += "}";
  return s;
}

/** Stringify a token-analysis-set (string-tags version) */
string
dwdstTagger::analyses_to_string(const set<FSMSymbolString> &analyses)
{
  string s = "{";
  for (set<FSMSymbolString>::const_iterator asi = analyses.begin();
       asi != analyses.end();
       asi++)
    {
      if (asi != analyses.begin()) s += ", ";
      s += *asi;
    }
  s += "}";
  return s;
}


/*--------------------------------------------------------------------------
 * Error reporting
 *--------------------------------------------------------------------------*/

void dwdstTagger::check_symspec_messages(void) {
  if (syms->messages && !syms->messages->empty()) {
    if (verbose > 0) {
      for (list<string>::iterator e = syms_msgs.begin(); e != syms_msgs.end(); e++) {
	fprintf(stderr,"%s\n",e->c_str());
      }
    }
    syms->messages->clear();
  }
}

void dwdstTagger::carp(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, " %s%s at line %d, column %d, near '%s'\n",
	  (srcname ? "in file " : ""),
	  (srcname ? srcname : ""),
	  lexer.theLine, lexer.theColumn, lexer.yytext);
}
