/*-*- Mode: Bison++ -*-*/
/*----------------------------------------------------------------------
 * Name: dwdstNgramsParser.yy
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + specification for ngram-parameter-file parser for (K)DWDS PoS-tagger
 *   + process with Alain Coetmeur's 'bison++' to produce a C++ parser
 *----------------------------------------------------------------------*/

%name dwdstNgramsParser

/* -- bison++ flags --- */
/* -- force use of location stack : defines global 'yyltype' by default */
%define LSP_NEEDED

// -- ... so we define yyltype ourselves ... (see %header section)
%define LTYPE dwdstNgramsParserLType

// -- debugging (see below)
%define DEBUG 1
%define ERROR_VERBOSE

// -- use pure-function errors
/* %define ERROR_BODY =0 */

// -- use inline error-reporting
%define ERROR_BODY { yycarp("dwdstNgramsParser: Error: %s", msg); }

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

#include "dwdstTypes.h"
#include "dwdstNgrams.h"

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
} dwdstNgramsParserLType;

/**
 * \class dwdstNgramsParser
 * \brief Bison++ parser for dwdst ngram-parameter files.
 */
%}

%{
#ifdef HAVE_CONFIG_H
# include <dwdstConfig.h>
#endif
%}

%define CLASS dwdstNgramsParser
%define MEMBERS \
  public: \
   /* -- public instance members go here */ \
   /** a pointer to the ngram-parameter object to hold the data we're parsing */ \
   dwdstNgrams              *ngrams; \
   /** to keep track of all possible tags we've parsed (optional). */ \
   set<dwdstTagString>      *alltags; \
   /** previous  ngram parsed (for brief file format */ \
   dwdstNgrams::NgramString  prevngram; \
   /** current ngram being parsed */ \
   dwdstNgrams::NgramString  curngram; \
  private: \
   /* private instance members go here */ \
  public: \
   /* public methods */ \
   /** virtual destructor to shut up gcc */\
   virtual ~dwdstNgramsParser(void) {};\
   /* report warnings */\
   virtual void yywarn(const char *msg) { \
      yycarp("dwdstNgramsParser: Warning: %s", msg);\
   }; \
   /** report anything */\
   virtual void yycarp(char *fmt, ...);
   

%define CONSTRUCTOR_INIT : \
   ngrams(NULL), \
   alltags(NULL)


/*------------------------------------------------------------
 * Parser Properties
 *------------------------------------------------------------*/

%union {
  dwdstNgrams::NgramString   *ngram;            ///< for tag-lists
  dwdstTagString            *tagstr;            ///< for single tags (strings)
  dwdstNgrams::NgramCount     count;            ///< for tag-list counts
}

%header{
/**
 * \typedef yy_dwdstNgramsParser_stype
 * \brief Bison++ semantic value typedef for dwdst N-Gram parameter file parser.
 */
%}

// -- Type declarations
%token <tagstr>  TAG
%token <count>   COUNT
%type  <tagstr>  tag
%type  <ngram>   ngram ngtag
%type  <count>   count        tab newline param params

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
		  ngrams->add_count(curngram, $3);
                  // -- remember current ngram
                  prevngram.swap(curngram);
                  //prevngram = curngram;
		  // -- ... and then clear it
		  curngram.clear();
		  $$ = 0;
		}
	;

ngram:		ngtag
		{
		    $$ = $1;
		}
	|	ngram tab ngtag
		{
		    // -- tab-separated tags: add to the current ngram (implicit in ngtag rule)
		    $$ = $3;
		}
	;


ngtag:		/* empty */
		{
		    // -- empty: add corresponding tag from previous ngram to current ngram
		    if (prevngram.empty()) {
			yyerror("no corresponding tag in previous n-gram.");
		    }
                    //dwdstTagString s = prevngram.front();
		    curngram.push_back(prevngram.front());
                    prevngram.pop_front();
                    $$ = &curngram;
		}
	|	tag
		{
		    // -- single tag: add it to the current ngram
		    curngram.push_back(*$1);
                    delete $1;
                    // -- pop previous ngram for purity
                    if (!prevngram.empty()) prevngram.pop_front();
                    $$ = &curngram;                  
		}
        ;

tag:		TAG
		{
		    if (alltags) alltags->insert(*$1);
		    $$ = $1;
		}
/*	|	// empty
		{
		    yyerror("expected a tag");
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

void dwdstNgramsParser::yycarp(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, " at line %d, column %d, near '%s'\n",
	  yylloc.last_line, yylloc.last_column, yylloc.text);
}
