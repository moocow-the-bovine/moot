/*-*- Mode: Bison++ -*-*/
/*----------------------------------------------------------------------
 * Name: dwdstParamParser.yy
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + specification for parameter-file parser for (K)DWDS PoS-tagger
 *   + process with Alain Coetmeur's 'bison++' to produce a C++ parser
 *----------------------------------------------------------------------*/

%name dwdstParamParser

/* -- bison++ flags --- */
/* -- force use of location stack : defines global 'yyltype' by default */
%define LSP_NEEDED

// -- ... so we define yyltype ourselves ... (see %header section)
%define LTYPE dwdstParamParserLType

// -- debugging (see below)
%define DEBUG 1
%define ERROR_VERBOSE

// -- use pure-function errors
/* %define ERROR_BODY =0 */

// -- use inline error-reporting
%define ERROR_BODY { yycarp("dwdstParamParser: Error: %s", msg); }

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

#include <FSMSymSpec.h>
#include <FSMTypes.h>

#include "dwdstTypes.h"

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
} dwdstParamParserLType;

/**
 * \class dwdstParamParser
 * \brief Bison++ parser for dwdst-pargen parameter files.
 */
%}

%{
#ifdef HAVE_CONFIG_H
# include <dwdstConfig.h>
#endif
%}

%define CLASS dwdstParamParser
%define MEMBERS \
  public: \
   /* -- public instance members go here */ \
   /** a pointer to the parameter map we are constructing. */ \
   NGramTable *ngtable; \
   /** to keep track of all possible tags we've parsed. */ \
   set<FSMSymbolString> *alltags; \
  private: \
   /* private instance members go here */ \
  public: \
   /* public methods */ \
   /* report warnings */\
   virtual void yywarn(const char *msg) { \
      yycarp("dwdstParamParser: Warning: %s", msg);\
   }; \
   /** report anything */\
   virtual void yycarp(char *fmt, ...);
   

%define CONSTRUCTOR_INIT : ngtable(NULL), alltags(NULL)



/*------------------------------------------------------------
 * Parser Properties
 *------------------------------------------------------------*/

%union {
  NGramVector      *ngram;            ///< for tag-lists
  FSMSymbolString *symstr;            ///< for single-tag regex-strings
  float             count;            ///< for tag-list counts
}

%header{
/**
 * \typedef yy_dwdstParamParser_stype
 * \brief Bison++ semantic value typedef for dwdst-pargen parameter-file parser.
 */
%}

// -- Type declarations
%token <symstr>  REGEX
%token <count>   COUNT
%type  <symstr>  regex
%type  <count>   count    tab newline param params
%type  <ngram>   ngram

// -- Operator precedence and associativity
//%left TAB       // -- ngram-construction operator

%start params

/* -------------- rules section -------------- */
%%

params:		/* empty */ { $$ = 0; }
	|	params param { $$ = 0; }
	|	newline params { $$ = 0; }
	;

param:		ngram tab count newline
		{
		    // -- single-parameter: add the parsed parameter to our table
		    if (ngtable->find(*$1) != ngtable->end()) {
			(*ngtable)[*$1] = $3;
		    } else {
			(*ngtable)[*$1] += $3;
		    }
                    // -- and delete any components
                    $1->clear();
                    delete $1;
		    $$ = 0;
		}
	;

ngram:		regex
		{
		    // -- single regex: make a new vector
		    $$ = new NGramVector();
                    $$->clear();
                    $$->push_back(*$1);
                    delete $1;
		}
	|	ngram tab regex
		{
		    // -- tab-separated regexes: add to the 'current' ngram
                    $1->push_back(*$3);
                    delete $3;
		    $$ = $1;
		}
	;

regex:		REGEX
		{
		    alltags->insert(*$1);
		    $$ = $1;
		}
/*	|	// empty
		{
		    yyerror("expected a regex");
                    YYABORT;
		}
*/
	;

count:		COUNT { $$ = $1; }
	|	/* empty */
		{
		    yyerror("expected a count.");
                    YYABORT;
		}
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

void dwdstParamParser::yycarp(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, " at line %d, column %d, near '%s'\n",
	  yylloc.last_line, yylloc.last_column, yylloc.text);
}