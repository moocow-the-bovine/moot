/*-*- Mode: Bison++ -*-*/

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

/*----------------------------------------------------------------------
 * Name: mootLexfreqsParser.yy
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + specification for lexical frequency parameter-file parser moot tagger
 *   + process with Alain Coetmeur's 'bison++' to produce a C++ parser
 *----------------------------------------------------------------------*/

%name mootLexfreqsParser

/* -- bison++ flags --- */
/* -- force use of location stack : defines global 'yyltype' by default */
%define LSP_NEEDED

// -- ... so we define yyltype ourselves ... (see %header section)
%define LTYPE mootLexfreqsParserLType

// -- debugging (see below)
%define DEBUG 1
%define ERROR_VERBOSE

// -- use pure-function errors
/* %define ERROR_BODY =0 */

// -- use inline error-reporting : gotta do this below, 'cause msg ain't declared
/* %define ERROR_BODY { yycarp("mootLexfreqsParser: Error: %s", msg); } */

// -- use pure-function lexer body
//%define LEX_BODY =0
%define LEX_BODY =0



%header{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#include <string>
#include <string.h>

#include "mootTypes.h"
#include "mootLexfreqs.h"

// -- get rid of bumble's macros
#undef YYACCEPT
#undef YYABORT
#undef YYERROR
#undef YYRECOVERING

/* ?
 *  Location-stack element type for bison++ parser.
 */
typedef struct {
  int timestamp;
  int first_line;
  int first_column;
  int last_line;
  int last_column;
  char *text;
} mootLexfreqsParserLType;


/**
 * \class mootLexfreqsParser
 * \brief Bison++ parser for (TnT-style) moot lexical frequency parameter files.
 */
%}

%{
#ifdef HAVE_CONFIG_H
# include <mootConfig.h>
#endif
%}

%define CLASS mootLexfreqsParser
%define MEMBERS \
  public: \
   /* -- public instance members go here */ \
   /** a pointer to the lexfreq-parameter object to hold the data we're parsing */ \
   moot::mootLexfreqs        *lexfreqs; \
   /* to keep track of all possible tags we've parsed (optional). */ \
   std::set<moot::mootTagString>  *alltags; \
  private: \
   /* private instance members go here */ \
  public: \
   /* public methods */ \
   /* report warnings */\
   virtual void yywarn(const char *msg) { \
      yycarp("mootLexfreqsParser: Warning: %s", msg);\
   }; \
   /** report anything */\
   virtual void yycarp(char *fmt, ...);
   

%define CONSTRUCTOR_INIT : lexfreqs(NULL), alltags(NULL)



/*------------------------------------------------------------
 * Parser Properties
 *------------------------------------------------------------*/

%union {
  moot::mootTokString *tokstr;         ///< for single tokens or tags (strings)
  moot::CountT           count;         ///< for tag-list counts
}

%header{
/**
 * \typedef yy_mootLexfreqsParser_stype
 * \brief Bison++ semantic value typedef for moot-pargen parameter-file parser.
 */
%}

// -- Type declarations
%token <tokstr>   TOKEN
%token <count>    COUNT
%type  <tokstr>   token tag entryBody
%type  <count>    count total          tab newline entry entries

// -- Operator precedence and associativity
//%left TAB       // -- lexfreq-construction operator

%start entries

/* -------------- rules section -------------- */
%%

entries:	/* empty */ { $$ = 0; }
	|	entries entry { $$ = 0; }
	|	newline entries { $$ = 0; }
	;

entry:		entryBody newline
		{
		  delete $1;
                  $$ = 0;
		}
	;

entryBody:	token { $$ = $1; }
	|	token   tab   total { $$ = $1; /* total is optional (not really) */ }
	|	entryBody   tab   tag   tab   count
		{
		  lexfreqs->add_count(*$1, *$3, $5);
                  delete $3;
                  $$ = $1;
		}
	;

token:		TOKEN
		{
		  $$ = $1;
		}
	|	COUNT
		{
		  $$ = new moot::mootTokString(reinterpret_cast<const char *>(yylloc.text));
		}
	;

tag:		token
		{
		  if (alltags) alltags->insert(*$1);
		  $$ = $1;
		}
	;

count:		COUNT { $$ = $1; }
	|	/* empty */
		{
		    yyerror("expected a COUNT.");
                    YYABORT;
		}
	;

total:		count { $$ = $1; }
	;

tab:		'\t' { $$=0; }
	|	/* empty */
		{
		    yyerror("expected a TAB.");
                    YYABORT;
                }
	;

newline:	'\n' { $$=0; }
	|	newline '\n'
	|	/* empty */
		{
		    yyerror("expected a NEWLINE.");
                    YYABORT;
		}
	;

%%
/* -------------- body section -------------- */

/*----------------------------------------------------------------
 * Error Methods
 *----------------------------------------------------------------*/

void mootLexfreqsParser::yyerror(char *msg)
{
    yycarp("mootLexfreqsParser: Error: %s\n", msg);
}

void mootLexfreqsParser::yycarp(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, " at line %d, column %d, near '%s'\n",
	  yylloc.last_line, yylloc.last_column, yylloc.text);
}
