#ifndef YY_mootNgramsParser_h_included
#define YY_mootNgramsParser_h_included

#line 1 "/usr/local/share/flex++bison++/bison.h"
/* before anything */
#ifdef c_plusplus
#ifndef __cplusplus
#define __cplusplus
#endif
#endif
#ifdef __cplusplus
#ifndef YY_USE_CLASS
#define YY_USE_CLASS
#endif
#else
#endif
#include <stdio.h>

/* #line 14 "/usr/local/share/flex++bison++/bison.h" */
#define YY_mootNgramsParser_LSP_NEEDED 
#define YY_mootNgramsParser_LTYPE  mootNgramsParserLType
#define YY_mootNgramsParser_DEBUG  1
#define YY_mootNgramsParser_ERROR_VERBOSE 
#define YY_mootNgramsParser_LEX_BODY  =0
#line 54 "mootNgramsParser.yy"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#include <string>
#include <string.h>

#include "mootTypes.h"
#include "mootNgrams.h"

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
} mootNgramsParserLType;

/**
 * \class mootNgramsParser
 * \brief Bison++ parser for (TnT-style) moot ngram-parameter files.
 */
#define YY_mootNgramsParser_CLASS  mootNgramsParser
#define YY_mootNgramsParser_MEMBERS  \
  public: \
   /* -- public instance members go here */ \
   /** a pointer to the ngram-parameter object to hold the data we're parsing */ \
   moot::mootNgrams              *ngrams; \
   /** to keep track of all possible tags we've parsed (optional). */ \
   std::set<moot::mootTagString> *alltags; \
   /** previous  ngram parsed (for brief file format */ \
   moot::mootNgrams::Ngram  prevngram; \
   /** current ngram being parsed */ \
   moot::mootNgrams::Ngram  curngram; \
  private: \
   /* private instance members go here */ \
  public: \
   /* public methods */ \
   /** virtual destructor to shut up gcc */\
   virtual ~mootNgramsParser(void) {};\
   /* report warnings */\
   virtual void yywarn(const char *msg) { \
      yycarp("mootNgramsParser: Warning: %s", msg);\
   }; \
   /** report anything */\
   virtual void yycarp(char *fmt, ...);   
#define YY_mootNgramsParser_CONSTRUCTOR_INIT  : \
   ngrams(NULL), \
   alltags(NULL)

#line 129 "mootNgramsParser.yy"
typedef union {
  moot::mootNgrams::Ngram         *ngram;            ///< for tag-lists
  moot::mootTagString            *tagstr;            ///< for single tags (strings)
  moot::mootNgrams::NgramCount     count;            ///< for tag-list counts
} yy_mootNgramsParser_stype;
#define YY_mootNgramsParser_STYPE yy_mootNgramsParser_stype
#line 135 "mootNgramsParser.yy"

/**
 * \typedef yy_mootNgramsParser_stype
 * \brief Bison++ semantic value typedef for moot N-Gram parameter file parser.
 */

#line 14 "/usr/local/share/flex++bison++/bison.h"
 /* %{ and %header{ and %union, during decl */
#ifndef YY_mootNgramsParser_COMPATIBILITY
#ifndef YY_USE_CLASS
#define  YY_mootNgramsParser_COMPATIBILITY 1
#else
#define  YY_mootNgramsParser_COMPATIBILITY 0
#endif
#endif

#if YY_mootNgramsParser_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YYLTYPE
#ifndef YY_mootNgramsParser_LTYPE
#define YY_mootNgramsParser_LTYPE YYLTYPE
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
/* use %define LTYPE */
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_mootNgramsParser_STYPE 
#define YY_mootNgramsParser_STYPE YYSTYPE
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
/* use %define STYPE */
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_mootNgramsParser_DEBUG
#define  YY_mootNgramsParser_DEBUG YYDEBUG
/* WARNING obsolete !!! user defined YYDEBUG not reported into generated header */
/* use %define DEBUG */
#endif
#endif
#ifdef YY_mootNgramsParser_STYPE
#ifndef yystype
#define yystype YY_mootNgramsParser_STYPE
#endif
#endif
#endif

#ifndef YY_mootNgramsParser_PURE

/* #line 54 "/usr/local/share/flex++bison++/bison.h" */

#line 54 "/usr/local/share/flex++bison++/bison.h"
/* YY_mootNgramsParser_PURE */
#endif

/* #line 56 "/usr/local/share/flex++bison++/bison.h" */

#line 56 "/usr/local/share/flex++bison++/bison.h"
/* prefix */
#ifndef YY_mootNgramsParser_DEBUG

/* #line 58 "/usr/local/share/flex++bison++/bison.h" */

#line 58 "/usr/local/share/flex++bison++/bison.h"
/* YY_mootNgramsParser_DEBUG */
#endif
#ifndef YY_mootNgramsParser_LSP_NEEDED

/* #line 61 "/usr/local/share/flex++bison++/bison.h" */

#line 61 "/usr/local/share/flex++bison++/bison.h"
 /* YY_mootNgramsParser_LSP_NEEDED*/
#endif
/* DEFAULT LTYPE*/
#ifdef YY_mootNgramsParser_LSP_NEEDED
#ifndef YY_mootNgramsParser_LTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YY_mootNgramsParser_LTYPE yyltype
#endif
#endif
/* DEFAULT STYPE*/
#ifndef YY_mootNgramsParser_STYPE
#define YY_mootNgramsParser_STYPE int
#endif
/* DEFAULT MISCELANEOUS */
#ifndef YY_mootNgramsParser_PARSE
#define YY_mootNgramsParser_PARSE yyparse
#endif
#ifndef YY_mootNgramsParser_LEX
#define YY_mootNgramsParser_LEX yylex
#endif
#ifndef YY_mootNgramsParser_LVAL
#define YY_mootNgramsParser_LVAL yylval
#endif
#ifndef YY_mootNgramsParser_LLOC
#define YY_mootNgramsParser_LLOC yylloc
#endif
#ifndef YY_mootNgramsParser_CHAR
#define YY_mootNgramsParser_CHAR yychar
#endif
#ifndef YY_mootNgramsParser_NERRS
#define YY_mootNgramsParser_NERRS yynerrs
#endif
#ifndef YY_mootNgramsParser_DEBUG_FLAG
#define YY_mootNgramsParser_DEBUG_FLAG yydebug
#endif
#ifndef YY_mootNgramsParser_ERROR
#define YY_mootNgramsParser_ERROR yyerror
#endif

#ifndef YY_mootNgramsParser_PARSE_PARAM
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
#define YY_mootNgramsParser_PARSE_PARAM
#ifndef YY_mootNgramsParser_PARSE_PARAM_DEF
#define YY_mootNgramsParser_PARSE_PARAM_DEF
#endif
#endif
#endif
#endif
#ifndef YY_mootNgramsParser_PARSE_PARAM
#define YY_mootNgramsParser_PARSE_PARAM void
#endif
#endif

/* TOKEN C */
#ifndef YY_USE_CLASS

#ifndef YY_mootNgramsParser_PURE
extern YY_mootNgramsParser_STYPE YY_mootNgramsParser_LVAL;
#endif


/* #line 134 "/usr/local/share/flex++bison++/bison.h" */
#define	TAG	258
#define	COUNT	259
#define	TAB	260
#define	NEWLINE	261


#line 134 "/usr/local/share/flex++bison++/bison.h"
 /* #defines token */
/* after #define tokens, before const tokens S5*/
#else
#ifndef YY_mootNgramsParser_CLASS
#define YY_mootNgramsParser_CLASS mootNgramsParser
#endif

#ifndef YY_mootNgramsParser_INHERIT
#define YY_mootNgramsParser_INHERIT
#endif
#ifndef YY_mootNgramsParser_MEMBERS
#define YY_mootNgramsParser_MEMBERS 
#endif
#ifndef YY_mootNgramsParser_LEX_BODY
#define YY_mootNgramsParser_LEX_BODY  
#endif
#ifndef YY_mootNgramsParser_ERROR_BODY
#define YY_mootNgramsParser_ERROR_BODY  
#endif
#ifndef YY_mootNgramsParser_CONSTRUCTOR_PARAM
#define YY_mootNgramsParser_CONSTRUCTOR_PARAM
#endif

class YY_mootNgramsParser_CLASS YY_mootNgramsParser_INHERIT
{
public: /* static const int token ... */

/* #line 160 "/usr/local/share/flex++bison++/bison.h" */
static const int TAG;
static const int COUNT;
static const int TAB;
static const int NEWLINE;


#line 160 "/usr/local/share/flex++bison++/bison.h"
 /* decl const */
public:
 int YY_mootNgramsParser_PARSE(YY_mootNgramsParser_PARSE_PARAM);
 virtual void YY_mootNgramsParser_ERROR(char *) YY_mootNgramsParser_ERROR_BODY;
#ifdef YY_mootNgramsParser_PURE
#ifdef YY_mootNgramsParser_LSP_NEEDED
 virtual int  YY_mootNgramsParser_LEX(YY_mootNgramsParser_STYPE *YY_mootNgramsParser_LVAL,YY_mootNgramsParser_LTYPE *YY_mootNgramsParser_LLOC) YY_mootNgramsParser_LEX_BODY;
#else
 virtual int  YY_mootNgramsParser_LEX(YY_mootNgramsParser_STYPE *YY_mootNgramsParser_LVAL) YY_mootNgramsParser_LEX_BODY;
#endif
#else
 virtual int YY_mootNgramsParser_LEX() YY_mootNgramsParser_LEX_BODY;
 YY_mootNgramsParser_STYPE YY_mootNgramsParser_LVAL;
#ifdef YY_mootNgramsParser_LSP_NEEDED
 YY_mootNgramsParser_LTYPE YY_mootNgramsParser_LLOC;
#endif
 int YY_mootNgramsParser_NERRS;
 int YY_mootNgramsParser_CHAR;
#endif
#if YY_mootNgramsParser_DEBUG != 0
public:
 int YY_mootNgramsParser_DEBUG_FLAG;	/*  nonzero means print parse trace	*/
#endif
public:
 YY_mootNgramsParser_CLASS(YY_mootNgramsParser_CONSTRUCTOR_PARAM);
public:
 YY_mootNgramsParser_MEMBERS 
};
/* other declare folow */
#endif


#if YY_mootNgramsParser_COMPATIBILITY != 0
/* backward compatibility */
#ifndef YYSTYPE
#define YYSTYPE YY_mootNgramsParser_STYPE
#endif

#ifndef YYLTYPE
#define YYLTYPE YY_mootNgramsParser_LTYPE
#endif
#ifndef YYDEBUG
#ifdef YY_mootNgramsParser_DEBUG 
#define YYDEBUG YY_mootNgramsParser_DEBUG
#endif
#endif

#endif
/* END */

/* #line 209 "/usr/local/share/flex++bison++/bison.h" */
#endif
