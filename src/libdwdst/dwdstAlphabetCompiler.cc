/* -*- Mode: C++ -*- */

/*============================================================================
 * File: dwdstAlphabetCompiler.cc
 * Author:  Bryan Jurish <moocow@ling.uni-potsdam.de>
 *
 * Description:
 *    Compiler for TnT alphabet files for DWDS PoS disambiguator
 *============================================================================*/

#include <stdio.h>

#include <FSM.h>

#include "dwdstAlphabetLexer.h"
#include "dwdstAlphabetParser.h"
#include "dwdstAlphabetCompiler.h"

/*----------------------------------------------------------------
 * Globals / defines
 *----------------------------------------------------------------*/
//#define DWDST_ALPHA_DEBUG

/*----------------------------------------------------------------
 * Constructor / Destructors
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------
 * Accessors
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------
 * Public Methods: PARSING
 *----------------------------------------------------------------*/
int dwdstAlphabetCompiler::yylex()
{
  yylloc.first_line=theLexer.theLine;
  yylloc.first_column=theLexer.theColumn;
  int token=theLexer.yylex(&yylval,&yylloc);
  yylloc.last_line=theLexer.theLine;
  yylloc.last_column=theLexer.theColumn;
  yylloc.text=(char *)theLexer.yytext;
  return token;
}

/*
 * parse a whole alphabet...
 */
dwdstSymbolVector2SymbolMap *dwdstAlphabetCompiler::parse_alphabet()
{
  // sanity check(s)
  if (!isyms) {
    yyerror("Cannot compile alphabet without an input SymSpec!");
    return NULL;
  }
  if (!osyms) {
    yyerror("Cannot compile alphabet without an output SymSpec!");
    return NULL;
  }
  if (!vec2sym) {
    vec2sym = new dwdstSymbolVector2SymbolMap();
  }
  
  recomp->use_symbol_spec(isyms);
  
  // actual parsing
  if (yyparse() != 0) return NULL;
  return vec2sym;
}

/*
 * parse a single class-entry
 */
void dwdstAlphabetCompiler::compile_alphabet_class(FSMSymbolString *shortName,
                                                 FSMSymbolString *longName,
                                                 FSMSymbolString *regex)
{
  v.clear();

  /* DEBUG
#ifdef DWDST_ALPHA_DEBUG
     printf("first=(line=%d,col=%d), last=(line=%d,col=%d), shortName=%s\n",
     yylloc.first_line, yylloc.first_column,
     yylloc.last_line, yylloc.last_column,
     $3->c_str());
#endif
  */

  /* DEBUG */
#ifdef DWDST_ALPHA_DEBUG
  yycarp("%s: compiling short=%s, long=%s, cursize=%d",
	 "<debug>",
	 shortName->c_str(),
	 longName->c_str(),
	 vec2sym->size());
  dump_symbol_vector("pre", longName->c_str(), -1, v);
#endif

  recomp->theLexer.theLine = yylloc.last_line-1;
  recomp->theLexer.theColumn = yylloc.last_column - regex->size();
  recomp->parse_from_string(regex->c_str());

  if (recomp->result_fsm && *recomp->result_fsm) {
    recomp->result_fsm->fsm_symbol_vectors(vectors,true);
    for (vsi = vectors.begin(); vsi != vectors.end(); vsi++) {
      v.insert(v.end(), vsi->istr.begin(), vsi->istr.end());
    }
    
    FSMSymbolString *className = use_short_classnames ? shortName : longName;
    
    FSMSymbol dest = osyms->symbolname_to_symbol(*className);
    if (dest == FSMNOLABEL) {
      yycarp("%s: %s '%s'",
	     (objname ? objname : "dwdstAlphabetCompiler"),
	     "could not determine FSMSymbol value for class",
	     className->c_str());
    }
    
    // -- sanity check
    dwdstSymbolVector2SymbolMap::iterator sv2smi = vec2sym->find(v);
    if (sv2smi != vec2sym->end()) {
      yycarp("%s: class '%s' appears to already be defined -- skipping.\n",
	     (objname ? objname : "dwdstAlphabetCompiler"),
	     className->c_str());
#    ifdef DWDST_ALPHA_DEBUG
      dump_symbol_vector("new", longName->c_str(), dest, v);
      dump_symbol_vector("old", "?", sv2smi->second, sv2smi->first);
#    endif
    }
    else {
      // -- insert into the lookup table
      (*vec2sym)[v] = dest;
    }
    vectors.clear();
    recomp->result_fsm->fsm_clear();
    delete recomp->result_fsm;
    recomp->result_fsm = NULL;
  } else {
    yycarp("%s: %s '%s'",
	   (objname ? objname : "dwdstAlphabetCompiler"),
	   "could not parse regular expression '%s'",
	   regex->c_str());
    if (recomp->result_fsm) {
      delete recomp->result_fsm;
      recomp->result_fsm = NULL;
    }
  }
}

/*----------------------------------------------------------------
 * Public Methods: Debugging
 *----------------------------------------------------------------*/
void dwdstAlphabetCompiler::dump_symbol_vector(const char *label,
					       const char *name,
					       const FSMSymbol class_symbol,
					       const FSM::FSMSymbolVector v)
{
  fprintf(stderr, "<dump(%s)>: name=%s\n", label, name);
  fprintf(stderr, "<dump(%s)>: symbol=%d\n", label, class_symbol);
  fprintf(stderr, "<dump(%s)>: vector =", label);
  for (FSM::FSMSymbolVector::const_iterator vi = v.begin(); vi != v.end(); vi++)
    {
      fprintf(stderr, " %d", *vi);
    }
  fprintf(stderr, "\n");
}

/*----------------------------------------------------------------
 * Public Methods: Errors & Warnings
 *----------------------------------------------------------------*/

void dwdstAlphabetCompiler::yyerror(const char *msg) {
  yycarp("%s: Error: %s", (objname ? objname : "dwdstAlphabetCompiler"), msg);
}

void dwdstAlphabetCompiler::yywarn(const char *msg) {
   yycarp("%s: Warning: %s", (objname ? objname : "dwdstAlphabetCompiler"), msg);
}

void dwdstAlphabetCompiler::yycarp(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, " %s%s at line %d, column %d, near '%s'\n",
	  (srcname ? "in file " : ""),
	  (srcname ? srcname : ""),
	  yylloc.last_line, yylloc.last_column, yylloc.text);
}
