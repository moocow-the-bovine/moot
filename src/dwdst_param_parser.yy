/*-*- Mode: Bison++ -*-*/
/*----------------------------------------------------------------------
 * Name: dwdst_param_parser.yy
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + specification for parameter-file parser for (K)DWDS PoS-tagger
 *   + process with Alain Coetmeur's 'bison++' to produce a C++ parser
 *----------------------------------------------------------------------*/

%name dwdst_param_parser

/* -- bison++ flags --- */
// -- force use of location stack
%define LSP_NEEDED

// -- debugging (see below)
//%define DEBUG 1
//%define ERROR_VERBOSE

// -- use pure-function errors
//%define ERROR_BODY =0

// -- use inline error-reporting
%define ERROR_BODY { fprintf(stderr,"dwdst_param_parser error: %s\n", msg); }

// -- use pure-function lexer body
//%define LEX_BODY =0
%define LEX_BODY =0

%header{
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

# include <stdio.h>
# include <stdlib.h>
# include <math.h>
# include <string.h>

# include <string>
# include <FSMSymSpec.h>
# include <dwdst.h>

// -- get rid of bumble's macros
#undef YYACCEPT
#undef YYABORT
#undef YYERROR
#undef YYRECOVERING

/**
 * \class dwdst_param_parser
 * \brief Bison++ parser for dwdst-pargen parameter files
 */
%}

%define CLASS dwdst_param_parser
%define MEMBERS \
  public:\
   /* public instance members go here */\
   /** \brief a pointer to the parameter-map we're constructing */\
   NGramTable *ngtable;\
  private:\
   /* private instance members go here */\
  public:\
   /* public methods */\
   /** \brief report warnings */\
   virtual void yywarn(const char *msg) {\
      fprintf(stderr,"dwdst_param_parser warning: %s\n", msg);\
   };

%define CONSTRUCTOR_INIT :\
   ngtable(NULL)


/*------------------------------------------------------------
 * Parser Properties
 *------------------------------------------------------------*/
%union {
  NGramVector      *ngram;   // -- for tag-lists
  FSMSymbolString *symstr;   // -- for single-tag regex-strings
  float             count;   // -- for tag-list counts
}
%header{
/**
 * \typedef yy_dwdst_param_parser_stype
 * \brief Bison++ semantic value typedef for dwdst-pargen parameter-file parser
 */
%}

// -- Type declarations
%token <symstr>  REGEX
%token <count>   COUNT
%type  <symstr>  regex
%type  <count>   count    tab newline param params
%type  <ngram>   ngram

// -- Operator precedence and associativity
%left TAB       // -- ngram-construction operator

%start params

/* -------------- rules section -------------- */
%%

params:		/* empty */ { $$ = 0; }
	|	params param { $$ = 0; }
	;

param:		ngram tab count newline
		{
		    // -- single-parameter: add the parsed parameter to our table
		    if (ngtable->find(*$1) != ngtable->end()) {
			ngtable[*$1] = $3;
		    } else {
			ngtable[*$1] += $3;
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
		    $$ = new NGramVector;
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

regex:		REGEX { $$ = $1; }
	|	/* empty */
		{
		    yyerror("expected a regex");
                    YYABORT;
		}
	;

count:		COUNT { $$ = $1; }
	|	/* empty */
		{
		    yyerror("expected a count");
                    YYABORT;
		}
	;

tab:		'\t'
	|	/* empty */
		{
		    yyerror("expected a TAB");
                    YYABORT;
                }
	;

newline:	'\n'
	|	'\0'
	|	newline '\n'
	|	/* empty */
		{
		    yyerror("expected a NEWLINE");
                    YYABORT;
		}
	;

%%
/* -------------- body section -------------- */

/*----------------------------------------------------------------
 * Parsing Methods
 *----------------------------------------------------------------*/
//void FSMRegexParser::yyerror(char *msg) { fprintf(stderr, "FSMRegexParser error: %s\n", msg); }

