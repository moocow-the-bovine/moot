/* -*- Mode: C++ -*- */

/*============================================================================
 * File: dwdstParamCompiler.cc
 * Author:  Bryan Jurish <moocow@ling.uni-potsdam.de>
 *
 * Description:
 *    Compiler for TnT parameter files for DWDS PoS tagger
 *============================================================================*/

#include <stdio.h>

#include <FSM.h>

#include "dwdstParamLexer.h"
#include "dwdstParamParser.h"
#include "dwdstParamCompiler.h"

/*----------------------------------------------------------------
 * Constructor / Destructors
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------
 * Accessors
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------
 * Public Methods: PARSING
 *----------------------------------------------------------------*/
int dwdstParamCompiler::yylex()
{
  yylloc.first_line=theLexer.theLine;
  yylloc.first_column=theLexer.theColumn;
  int token=theLexer.yylex(&yylval,&yylloc);
  yylloc.last_line=theLexer.theLine;
  yylloc.last_column=theLexer.theColumn;
  yylloc.text=(char *)theLexer.yytext;
  return token;
}


NGramTable *dwdstParamCompiler::parse_ngrams()
{
  // sanity check
  if (!ngtable) {
    yyerror("Cannot compile to a NULL N-Gram table!");
    return NULL;
  }
  // actual parsing
  if (yyparse() != 0) return NULL;
  return ngtable;
}


/*----------------------------------------------------------------
 * Public Methods: Errors & Warnings
 *----------------------------------------------------------------*/

void dwdstParamCompiler::yyerror(const char *msg) {
    fprintf(stderr,"%s: error:%s%s at line %d, column %d, near token '%s': %s\n",
	    (objname ? objname : "dwdstParamCompiler"),
	    (srcname ? " in file " : ""),
	    (srcname ? srcname : ""),
	    yylloc.first_line, yylloc.first_column, yylloc.text, msg);
}
void dwdstParamCompiler::yywarn(const char *msg) {
    fprintf(stderr,"%s: warning:%s%s at line %d, column %d, near token '%s': %s\n",
	  (objname ? objname : "dwdstParamCompiler"),
	  (srcname ? " in file " : ""),
	  (srcname ? srcname : ""),
	  yylloc.first_line, yylloc.first_column, yylloc.text, msg);
}
