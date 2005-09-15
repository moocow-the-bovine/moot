/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2005 by Bryan Jurish <moocow@ling.uni-potsdam.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/*============================================================================
 * File: mootLexfreqsCompiler.cc
 * Author:  Bryan Jurish <moocow@ling.uni-potsdam.de>
 *
 * Description:
 *    Compiler for TnT parameter files for moot PoS tagger
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
