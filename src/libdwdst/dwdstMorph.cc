/*--------------------------------------------------------------------------
 * File: dwdstMorph.cc
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

#include "dwdstMorph.h"

using namespace std;

/*--------------------------------------------------------------------------
 * Constructor / Destructor
 *--------------------------------------------------------------------------*/

/*
 * dwdstMorph::~dwdstMorph()
 */
dwdstMorph::~dwdstMorph() {
  if (tmp) delete tmp;
  if (syms && i_made_syms) delete syms;
  if (mfst && i_made_mfst) delete mfst;

  syms = NULL;
  mfst = NULL;
  result = NULL;
  tmp = NULL;
}

/*--------------------------------------------------------------------------
 * Initialization
 *--------------------------------------------------------------------------*/

/*
 * FSMSymSpec *dwdstMorph::load_morph_symbols(const char *filename)
 */
FSMSymSpec *dwdstMorph::load_morph_symbols(const char *filename)
{
  // -- cleanup old symbols first
  if (filename) syms_filename = (const char *)filename;
  if (syms && i_made_syms) delete syms;

  //-- argh: FSMSymSpec don't take a 'const char *'
  char *syms_filename_cp = (char *)malloc(syms_filename.size()+1);
  strcpy(syms_filename_cp, syms_filename.c_str());
  syms = new FSMSymSpec(syms_filename_cp, &syms_msgs, DWDST_SYM_ATT_COMPAT);

  if (!syms_msgs.empty()) {
    carp("dwdstMorph::load_symbols() Error: could not load symbols from file '%s'\n",
	 syms_filename.c_str());
    for (list<string>::iterator e = syms_msgs.begin(); e != syms_msgs.end(); e++) {
      carp("%s\n",e->c_str());
    }
    syms_msgs.clear(); // -- clear messages
    syms = NULL;       // -- invalidate the tagger object
  }

  i_made_syms = true;

  //-- use symspec in FST(s) if not already in use
  if (mfst) mfst->fsm_use_symbol_spec(syms);
  if (xfst) xfst->fsm_use_symbol_spec(syms);

  //-- cleanup
  free(syms_filename_cp);

  return syms;
}

/*
 * FSM *dwdstMorph::load_fsm_file(const char *filename, FSM **fsm, bool *i_made_fsm=NULL);
 */
FSM *dwdstMorph::load_fsm_file(const char *filename, FSM **fsm, bool *i_made_fsm=NULL)
{
  // -- cleanup old FSM first
  if (*fsm && i_made_fsm && *i_made_fsm) { delete *fsm; }

  *fsm = new FSM(filename);
  if (!**fsm) {
    carp("dwdstMorph::load_fsm_file() Error: load failed for FSM file '%s'.\n", filename);
    *fsm = NULL; // -- invalidate the object
  }
  if (i_made_fsm) { *i_made_fsm = true; }
  return *fsm;
}


/*--------------------------------------------------------------------------
 * Tag-extraction (should be inlined eventually)
 *--------------------------------------------------------------------------*/

//void dwdstMorph::extract_tags(FSM &fsm, MorphAnalysisSet &as)

/*--------------------------------------------------------------------------
 * Top-level tagging methods
 *--------------------------------------------------------------------------*/

bool dwdstMorph::tag_stream(FILE *in=stdin, FILE *out, char *srcname)
{
  dwdstTaggerLexer lexer;

  // -- sanity check
  if (!can_tag()) {
    carp("dwdstMorph::tag_stream(): cannot run uninitialized tagger!\n");
    return false;
  }

  // -- lexer prep
  lexer.step_streams(in,out);
  lexer.theLine = 1;
  lexer.theColumn = 0;

  int tok;
  while ((tok = lexer.yylex()) != dwdstTaggerLexer::DTEOF) {
    switch (tok) {
    case dwdstTaggerLexer::EOS:
      /* do something spiffy */
      tag_print_eos(out);
      break;

    case dwdstTaggerLexer::TAG:
    case dwdstTaggerLexer::EOT:
      /* -- ignore TAGs in the input file */
      break;

    case dwdstTaggerLexer::TOKEN:
    default:
      tag_token((char *)lexer.yytext);
      print_token_analyses(out, (char *)lexer.yytext);
    }
  }

  return true;
}

bool dwdstMorph::tag_strings(int argc, char **argv, FILE *out, char *srcname)
{
  // -- sanity check
  if (!can_tag()) {
    fprintf(stderr, "dwdstMorph::tag_strings(): cannot run uninitialized tagger!\n");
    return false;
  }

  // -- ye olde guttes
  for ( ; --argc >= 0; argv++) {
    //-- verbosity
    tag_token(*argv);
    print_token_analyses(out, *argv);
  }
  tag_print_eos(out);

  return true;
}


/*--------------------------------------------------------------------------
 * mid-Level Tagging Methods
 *--------------------------------------------------------------------------*/

//(inlined)


/*--------------------------------------------------------------------------
 * public methods: tagging utilities: string-generation
 *--------------------------------------------------------------------------*/

//(inlined)

/*--------------------------------------------------------------------------
 * Mid-level tagging utilities: output
 *--------------------------------------------------------------------------*/

//(inlined)

/*--------------------------------------------------------------------------
 * Mid-level tagging utilities: conversions
 *--------------------------------------------------------------------------*/

void dwdstMorph::symbol_vector_to_string_dq(const vector<FSMSymbol> &vec, FSMSymbolString &str)
const
{
  bool last_was_char = true;
  str.clear();

  //-- Special case for first symbol --
  vector<FSMSymbol>::const_iterator vi;
  for (vi = vec.begin(); vi != vec.end(); vi++) {
    register const FSMSymbolString *isym = syms->symbol_to_symbolname(*vi);
    if (isym == NULL) {
      if (verbose >= vlWarnings) {
	carp("dwdstMorph::symbol_vector_to_string_dq(): Error: undefined symbol '%d' -- ignored\n",
	     *vi);
      }
      continue;
    }
    else { //-- isym==NULL
      if (isym->size() > 1) {
	if ((*isym)[0] == '_') {
	  //str.append(*isym, 1, isym->size());
	  str.append(isym->begin()+1, isym->end());
	} else {
	  str.append(*isym);
	}
	//str.push_back('.');
	last_was_char = false;
      } else {
	str.push_back((*isym)[0]);
	last_was_char = true;
      }
    }
    break;
  }


  //-- Normal case --
  for (vi++; vi != vec.end(); vi++) {
    if (*vi == EPSILON || *vi == FSMNOLABEL) continue;
    register const FSMSymbolString *isym = syms->symbol_to_symbolname(*vi);
    if (isym == NULL) {
      if (verbose >= vlWarnings) {
	carp("dwdstMorph::symbol_vector_to_string_dq(): Error: undefined symbol '%d' -- ignored\n",
	     *vi);
      }
      continue;
    }
    else { //-- isym==NULL
      // -- it's a kosher symbol
      if (isym->size() > 1) {
	str.push_back('.');
	if ((*isym)[0] == '_') {
	  //str.append(*isym, 1, isym->size());
	  str.append(isym->begin()+1, isym->end());
	} else {
	  str.append(*isym);
	}
	last_was_char = false;
      } else {
	if (!last_was_char) str.push_back('.');
	str.push_back((*isym)[0]);
	last_was_char = true;
      }
    }
  }
};



/*--------------------------------------------------------------------------
 * Debugging Methods
 *--------------------------------------------------------------------------*/

// (removed)

/*--------------------------------------------------------------------------
 * Error reporting
 *--------------------------------------------------------------------------*/

void dwdstMorph::carp(char *fmt, ...) const
{
  if (verbose >= vlErrors) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    /*
      fprintf(stderr, " %s%s at line %d, column %d, near '%s'\n",
      (srcname ? "in file " : ""),
      (srcname ? srcname : ""),
      lexer.theLine, lexer.theColumn, lexer.yytext);
    */
  }
}
