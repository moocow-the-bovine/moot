/*-*- Mode: Bison++ -*-*/
/*----------------------------------------------------------------------
 * Name: dwdstAlphabetParser.yy
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + specification for alphabet-file parser for (K)DWDS PoS-disambiguator
 *   + process with Alain Coetmeur's 'bison++' to produce a C++ parser
 *----------------------------------------------------------------------*/

%name dwdstAlphabetParser

/* -- bison++ flags --- */
// -- force use of location stack : defines global 'yyltype' by default
%define LSP_NEEDED

// -- ... so we define yyltype ourselves ... (see %header section)
%define LTYPE dwdstAlphabetParserLType

// -- debugging (see below)
%define DEBUG 1
%define ERROR_VERBOSE

// -- use pure-function errors
/*%define ERROR_BODY =0*/

// -- use inline error-reporting
%define ERROR_BODY { yycarp("dwdstAlphabetParser: Error: %s", msg); }

// -- use pure-function lexer body
//%define LEX_BODY =0
%define LEX_BODY =0

%header{
#ifdef HAVE_CONFIG_H
# include <dwdstConfig.h>
#endif

# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <math.h>

# include <string>
# include <string.h>

# include <FSMSymSpec.h>

# include "cleanConfig.h"

# include <FSM.h>
# include <FSMRegexCompiler.h>

# include "dwdstTypes.h"

// -- get rid of bumble's macros
#undef YYACCEPT
#undef YYABORT
#undef YYERROR
#undef YYRECOVERING

/** Location-stack element type for bison++ parser. */
typedef struct {
  int timestamp;
  int first_line;
  int first_column;
  int last_line;
  int last_column;
  char *text;
} dwdstAlphabetParserLType;

/**
 * \class dwdstAlphabetParser
 * \brief Bison++ parser for dwdst-pargen alphabeteter files.
 */
%}

%define CLASS dwdstAlphabetParser
%define MEMBERS \
  public: \
   /* -- public instance members go here */ \
   /** pointer to the symspec to use for compilation */ \
   FSMSymSpec *isyms; \
   /** pointer to the symspec to use for output (disambiguation) */ \
   FSMSymSpec *osyms; \
   /** regex compiler to use for translation */ \
   FSMRegexCompiler *recomp; \
   /** a pointer to the class-map we are constructing. */ \
   dwdstSymbolVector2SymbolMap *vec2sym; \
   /** whether to try short- or long-names first */ \
   bool use_short_classnames; \
  protected: \
   /* protected instance members go here */ \
   /** for fsm_symbol_vectors() */ \
   set<FSM::FSMWeightedSymbolVector> vectors; \
   /** for fsm_symbol_vectors() iteration */ \
   set<FSM::FSMWeightedSymbolVector>::iterator vsi; \
   /** for key-compilation */ \
   FSM::FSMSymbolVector v; \
  public: \
   /* public methods */ \
   /** destructor: cause gcc to stop reporting errors */\
   virtual ~dwdstAlphabetParser(void); \
   /** do actual integration of an alphabet-line */ \
   virtual void compile_alphabet_class(FSMSymbolString *shortName,\
                                       FSMSymbolString *longName,\
                                       FSMSymbolString *regex);\
   /* report warnings */ \
   virtual void yywarn(const char *msg) { \
      yycarp("dwdstAlphabetParser: Warning: %s", msg);\
   }; \
   /** report anything */ \
   virtual void yycarp(char *fmt, ...);
   

%define CONSTRUCTOR_INIT : \
  isyms(NULL), \
  osyms(NULL), \
  recomp(new FSMRegexCompiler()), \
  vec2sym(NULL), \
  use_short_classnames(true)


/*------------------------------------------------------------
 * Parser Properties
 *------------------------------------------------------------*/

%union {
  FSMSymbolString *symstr;            ///< for single-symbols
}

%header{
/**
 * \typedef yy_dwdstAlphabetParser_stype
 * \brief Bison++ semantic value typedef for dwdst alphabet-file parser.
 */
%}

// -- Type declarations
%token <symstr>  SYMBOL
%type  <symstr>  typeID
%type  <symstr>  shortName longName
%type  <symstr>  regex
%type  <symstr>  alphaline alphalines
%type  <symstr>  tab newline

// -- Operator precedence and associativity
//%left TAB       // -- ngram-construction operator

%start alphalines

/* -------------- rules section -------------- */
%%

alphalines:	/* empty */ { $$ = NULL; }
	|	alphalines alphaline { $$ = NULL; }
	;

alphaline:	newline { $$=NULL; }
	|	typeID tab shortName tab longName tab regex
		{
		    // -- index the parsed alphabet-line
		    if (*($1) == "Class") {
		      compile_alphabet_class($3,$5,$7);
		    }
                    // -- cleanup
                    $1->clear(); delete $1;
                    $3->clear(); delete $3;
                    $5->clear(); delete $5;
                    $7->clear(); delete $7;
		    $$ = NULL;
                }
		;

typeID:        SYMBOL { $$=$1; }
	       ;

shortName:     SYMBOL { $$=$1; }
               ;

longName:	SYMBOL { $$=$1; }
		;

regex:		SYMBOL { $$=$1; }
		;


tab:		'\t' { $$=NULL; }
	|	/* empty */
		{
		    yyerror("expected a TAB");
                    YYABORT;
                }
	;

newline:	'\n' { $$=NULL; }
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
/*void dwdstAlphabetParser::yyerror(const char *msg) {
    fprintf(stderr, "dwdstAlphabetParser Error: %s\n", msg);
}*/

dwdstAlphabetParser::~dwdstAlphabetParser() {
  vectors.clear();
  if (recomp) {
    delete recomp;
    recomp = NULL;
  }
}

void dwdstAlphabetParser::compile_alphabet_class(FSMSymbolString *shortName,
                                                 FSMSymbolString *longName,
                                                 FSMSymbolString *regex)
{
  v.clear();

  recomp->theLexer.theLine = yylloc.last_line-1;
  recomp->theLexer.theColumn = yylloc.last_column - regex->size();
  recomp->parse_from_string(regex->c_str());

  if (recomp->result_fsm && *recomp->result_fsm) {
    recomp->result_fsm->fsm_symbol_vectors(vectors,true);
    for (vsi = vectors.begin(); vsi != vectors.end(); vsi++) {
      v.insert(v.end(), vsi->istr.begin(), vsi->istr.end());
    }
    
    FSMSymbolString *className;
    if (use_short_classnames) {
      className = shortName;
    } else {
      className = longName;
    }
    
    FSMSymbol dest = osyms->symbolname_to_symbol(*className);
    if (dest == FSMNOLABEL) {
      yycarp("%s: %s '%s'",
	     "dwdstAlphabetParser",
	     "could not determine FSMSymbol value for class",
	     className->c_str());
    }
    
    // -- sanity check
    dwdstSymbolVector2SymbolMap::iterator sv2smi = vec2sym->find(v);
    if (sv2smi != vec2sym->end()) {
      yycarp("%s: class '%s' appears to already be defined -- skipping.\n",
	     "dwdstAlphabetParser",
	     className->c_str());
    }
    else {
      // -- insert into the lookup table
      (*vec2sym)[v] = dest;
    }
    delete recomp->result_fsm;
    recomp->result_fsm = NULL;
  } else {
    yycarp("%s: %s '%s'",
	   "dwdstAlphabetParser",
	   "could not parse regular expression '%s'",
	   regex->c_str());
    if (recomp->result_fsm) {
      delete recomp->result_fsm;
      recomp->result_fsm = NULL;
    }
  }
}

/*----------------------------------------------------------------
 * Error Methods
 *----------------------------------------------------------------*/

void dwdstAlphabetParser::yycarp(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, " at line %d, column %d, near '%s'\n",
	  yylloc.last_line, yylloc.last_column, yylloc.text);
}
