/*-*- Mode: Bison++ -*-*/

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2008 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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
 * Name: mootClassfreqsParser.yy
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + specification for lexical-class frequency parameter-file parser for moot PoS-tagger
 *   + process with Alain Coetmeur's 'bison++' to produce a C++ parser
 *----------------------------------------------------------------------*/

%name mootClassfreqsParser

/* -- bison++ flags --- */
/* -- force use of location stack : defines global 'yyltype' by default */
%define LSP_NEEDED

// -- ... so we define yyltype ourselves ... (see %header section)
%define LTYPE mootClassfreqsParserLType

// -- debugging (see below)
%define DEBUG 1
%define ERROR_VERBOSE

// -- use pure-function errors
/* %define ERROR_BODY =0 */

// -- use inline error-reporting : gotta do this below, 'cause msg ain't declared
/* %define ERROR_BODY { yycarp("mootClassfreqsParser: Error: %s", msg); } */

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
#include "mootClassfreqs.h"

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
} mootClassfreqsParserLType;


/**
 * \class mootClassfreqsParser
 * \brief Bison++ parser for (TnT-style) moot lexical frequency parameter files.
 */
%}

%{
#ifdef HAVE_CONFIG_H
# include <mootConfig.h>
#endif
%}

%define CLASS mootClassfreqsParser
%define MEMBERS \
  public: \
   /* -- public instance members go here */ \
   /** a pointer to the parameter object to hold the data we're parsing */ \
   moot::mootClassfreqs            *cfreqs; \
  protected: \
   /* protected instance members go here */ \
   /** current lexical class begin built  */ \
   moot::mootClassfreqs::LexClass   lclass; \
  public: \
   /* public methods */ \
   /** virtual destructor to shut up gcc */\
   virtual ~mootClassfreqsParser(void) {};\
   /* report warnings */\
   virtual void yywarn(const char *msg) { \
      yycarp("mootClassfreqsParser: Warning: %s", msg);\
   }; \
   /** report anything */\
   virtual void yycarp(const char *fmt, ...);
   

%define CONSTRUCTOR_INIT : cfreqs(NULL)



/*------------------------------------------------------------
 * Parser Properties
 *------------------------------------------------------------*/

%union {
  moot::mootTagString  *tagstr;         ///< for single tags (strings)
  moot::CountT           count;         ///< for tag-list counts
}

%header{
/**
 * \typedef yy_mootClassfreqsParser_stype
 * \brief Bison++ semantic value typedef for moot-pargen parameter-file parser.
 */
%}

// -- Type declarations
%token <tagstr>   TAG
%token <count>    COUNT
%type  <tagstr>   tag
%type  <count>    count total entryBody lclass tab newline entry entries

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
                  lclass.clear();
                  $$ = 0;
		}
	;

entryBody:	lclass { $$ = $1; }
	|	lclass   tab   total { $$ = $1; /* total is optional (not really) */ }
	|	entryBody   tab   tag   tab   count
		{
		  cfreqs->add_count(lclass, *$3, $5);
		  delete $3;
                  $$ = $1;
		}
	;

lclass:		/* empty */ { $$ = 0; }
	|	lclass tag
		{
		  lclass.insert(*$2);
                  delete $2;
                  $$ = 0;
		}
	|	lclass COUNT
		{
		  lclass.insert(reinterpret_cast<const char *>(yylloc.text));
		  $$ = 0;
		}
	;

tag:		TAG
		{
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

void mootClassfreqsParser::yyerror(const char *msg)
{
    yycarp("mootClassfreqsParser: Error: %s\n", msg);
}

void mootClassfreqsParser::yycarp(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, " at line %d, column %d, near '%s'\n",
	  yylloc.last_line, yylloc.last_column, yylloc.text);
}
