/*-*- Mode: Bison++ -*-*/
/*----------------------------------------------------------------------
 * Name: dummyParser.yy
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + specification for parameter-file parser for (K)DWDS PoS-tagger
 *   + process with Alain Coetmeur's 'bison++' to produce a C++ parser
 *----------------------------------------------------------------------*/

%name dummyParser

/* -- bison++ flags --- */
// -- force use of location stack
%define LSP_NEEDED

// -- debugging (see below)
//%define DEBUG 1
//%define ERROR_VERBOSE

// -- use pure-function errors
%define ERROR_BODY =0

// -- use inline error-reporting
//%define ERROR_BODY { fprintf(stderr,"dummyParser error: %s\n", msg); }

// -- use pure-function lexer body
//%define LEX_BODY =0
%define LEX_BODY =0

%header{
#ifdef HAVE_CONFIG_H
# include <mootConfig.h>
#endif

# include <stdio.h>
# include <stdlib.h>
# include <math.h>

# include <string.h>
# include <string>

# include <FSMSymSpec.h>
# include "mootTypes.h"

// -- get rid of bumble's macros
#undef YYACCEPT
#undef YYABORT
#undef YYERROR
#undef YYRECOVERING

/**
 * \class dummyParser
 * \brief Bison++ parser for moot-pargen parameter files
 */
%}

%define CLASS dummyParser
%define MEMBERS \
  public: \
   /* -- public instance members go here */ \
   /** \brief A pointer to the parameter map we are constructing. */ \
   NGramTable *ngtable; \
  private: \
   /* -- private instance members go here */ \
  public: \
   /* -- public methods */ \
   /** \brief report warnings */ \
   virtual void yywarn(const char *msg) { \
      fprintf(stderr,"mootParamParser warning: %s\n", msg); \
   };


//%define CONSTRUCTOR_INIT : ngtable(NULL)



/*------------------------------------------------------------
 * Parser Properties
 *------------------------------------------------------------*/

%union {
  /*NGramVector      *ngram;*/            // -- for tag-lists
  /*FSMSymbolString *symstr;*/            // -- for single-tag regex-strings
  char *symstr;
  float             count;            // -- for tag-list counts
}

%header{
/**
 * \typedef yy_dummyParser_stype
 * \brief Bison++ semantic value typedef for dummy parameter-file parser.
 */
%}

// -- Type declarations
%token <symstr>  STRING
%token <count>   COUNT
%type  <symstr>  all

// -- Operator precedence and associativity
//%left TAB       // -- ngram-construction operator

%start all

/* -------------- rules section -------------- */
%%

all:		 STRING { $$=$1; }
	|	 COUNT { $$=$1; }
	;

%%
/* -------------- body section -------------- */

/*----------------------------------------------------------------
 * Parsing Methods
 *----------------------------------------------------------------*/
void dummyParser::yyerror(char *msg) {
  fprintf(stderr, "dummyParser Error: %s\n", msg);
}
