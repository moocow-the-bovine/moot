/* -*- Mode: C++ -*- */

/*============================================================================
 * File: mootLexfreqsCompiler.cc
 * Author:  Bryan Jurish <moocow@ling.uni-potsdam.de>
 *
 * Description:
 *    Compiler for TnT parameter files for DWDS PoS tagger
 *============================================================================*/

#include <stdio.h>

//#include <FSM.h>

#include "mootLexfreqs.h"
#include "mootLexfreqsLexer.h"
#include "mootLexfreqsParser.h"
#include "mootLexfreqsCompiler.h"

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
int mootLexfreqsCompiler::yylex()
{
  yylloc.first_line=theLexer.theLine;
  yylloc.first_column=theLexer.theColumn;
  int token=theLexer.yylex(&yylval,&yylloc);
  yylloc.last_line=theLexer.theLine;
  yylloc.last_column=theLexer.theColumn;
  yylloc.text=(char *)theLexer.yytext;
  return token;
}


mootLexfreqs *mootLexfreqsCompiler::parse_lexfreqs()
{
  // sanity check
  if (!lexfreqs) {
    yyerror("Cannot compile to a NULL lexfreqs object!");
    return NULL;
  }
  // actual parsing
  if (yyparse() != 0) return NULL;
  return lexfreqs;
}


/*----------------------------------------------------------------
 * Public Methods: Errors & Warnings
 *----------------------------------------------------------------*/

void mootLexfreqsCompiler::yyerror(const char *msg) {
    fprintf(stderr,"%s: error:%s%s at line %d, column %d, near token '%s': %s\n",
	    (objname ? objname : "mootLexfreqsCompiler"),
	    (srcname ? " in file " : ""),
	    (srcname ? srcname : ""),
	    yylloc.first_line, yylloc.first_column, yylloc.text, msg);
}
void mootLexfreqsCompiler::yywarn(const char *msg) {
    fprintf(stderr,"%s: warning:%s%s at line %d, column %d, near token '%s': %s\n",
	  (objname ? objname : "mootLexfreqsCompiler"),
	  (srcname ? " in file " : ""),
	  (srcname ? srcname : ""),
	  yylloc.first_line, yylloc.first_column, yylloc.text, msg);
}

moot_END_NAMESPACE
