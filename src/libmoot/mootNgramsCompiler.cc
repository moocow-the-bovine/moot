/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2004 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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
 * File: mootNgramsCompiler.cc
 * Author:  Bryan Jurish <moocow@ling.uni-potsdam.de>
 *
 * Description:
 *    Compiler for TnT parameter files for moot PoS tagger
 *============================================================================*/

#include <stdio.h>

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
