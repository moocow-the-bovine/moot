/* -*- Mode: C++ -*- */

/*============================================================================
 * File: mootNgramsCompiler.cc
 * Author:  Bryan Jurish <moocow@ling.uni-potsdam.de>
 *
 * Description:
 *    Compiler for TnT parameter files for DWDS PoS tagger
 *============================================================================*/

#include <stdio.h>

#include <FSM.h>

#include "mootNgrams.h"
#include "mootNgramsLexer.h"
#include "mootNgramsParser.h"
#include "mootNgramsCompiler.h"

moot_BEGIN_NAMESPACE

/*----------------------------------------------------------------
 * Constructor / Destructors
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------
 * Accessors
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------
 * Public Methods: PARSING
 *----------------------------------------------------------------*/
int mootNgramsCompiler::yylex()
{
  yylloc.first_line=theLexer.theLine;
  yylloc.first_column=theLexer.theColumn;
  int token=theLexer.yylex(&yylval,&yylloc);
  yylloc.last_line=theLexer.theLine;
  yylloc.last_column=theLexer.theColumn;
  yylloc.text=(char *)theLexer.yytext;
  return token;
}


mootNgrams *mootNgramsCompiler::parse_ngrams()
{
  // sanity check
  if (!ngrams) {
    yyerror("Cannot compile to a NULL ngrams object!");
    return NULL;
  }

  // cleanup any stale ngrams
  prevngram.clear();
  curngram.clear();

  // actual parsing
  if (yyparse() != 0) return NULL;
  return ngrams;
}


/*----------------------------------------------------------------
 * Public Methods: Errors & Warnings
 *----------------------------------------------------------------*/

void mootNgramsCompiler::yyerror(const char *msg) {
    fprintf(stderr,"%s: error:%s%s at line %d, column %d, near token '%s': %s\n",
	    (objname ? objname : "mootNgramsCompiler"),
	    (srcname ? " in file " : ""),
	    (srcname ? srcname : ""),
	    yylloc.first_line, yylloc.first_column, yylloc.text, msg);
}
void mootNgramsCompiler::yywarn(const char *msg) {
    fprintf(stderr,"%s: warning:%s%s at line %d, column %d, near token '%s': %s\n",
	  (objname ? objname : "mootNgramsCompiler"),
	  (srcname ? " in file " : ""),
	  (srcname ? srcname : ""),
	  yylloc.first_line, yylloc.first_column, yylloc.text, msg);
}

moot_END_NAMESPACE
