#ifndef YY_mootLexfreqsParser_h_included
#define YY_mootLexfreqsParser_h_included

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
#define YY_mootLexfreqsParser_LSP_NEEDED 
#define YY_mootLexfreqsParser_LTYPE  mootLexfreqsParserLType
#define YY_mootLexfreqsParser_DEBUG  1
#define YY_mootLexfreqsParser_ERROR_VERBOSE 
#define YY_mootLexfreqsParser_LEX_BODY  =0
#line 55 "mootLexfreqsParser.yy"

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
#define YY_mootLexfreqsParser_CLASS  mootLexfreqsParser
#define YY_mootLexfreqsParser_MEMBERS  \
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
#define YY_mootLexfreqsParser_CONSTRUCTOR_INIT  : lexfreqs(NULL), alltags(NULL)

#line 126 "mootLexfreqsParser.yy"
typedef union {
  moot::mootTokString *tokstr;         ///< for single tokens or tags (strings)
  moot::CountT           count;         ///< for tag-list counts
} yy_mootLexfreqsParser_stype;
#define YY_mootLexfreqsParser_STYPE yy_mootLexfreqsParser_stype
#line 131 "mootLexfreqsParser.yy"

/**
 * \typedef yy_mootLexfreqsParser_stype
 * \brief Bison++ semantic value typedef for moot-pargen parameter-file parser.
 */

#line 14 "/usr/local/share/flex++bison++/bison.h"
 /* %{ and %header{ and %union, during decl */
#ifndef YY_mootLexfreqsParser_COMPATIBILITY
#ifndef YY_USE_CLASS
#define  YY_mootLexfreqsParser_COMPATIBILITY 1
#else
#define  YY_mootLexfreqsParser_COMPATIBILITY 0
#endif
#endif

#if YY_mootLexfreqsParser_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YYLTYPE
#ifndef YY_mootLexfreqsParser_LTYPE
#define YY_mootLexfreqsParser_LTYPE YYLTYPE
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
/* use %define LTYPE */
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_mootLexfreqsParser_STYPE 
#define YY_mootLexfreqsParser_STYPE YYSTYPE
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
/* use %define STYPE */
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_mootLexfreqsParser_DEBUG
#define  YY_mootLexfreqsParser_DEBUG YYDEBUG
/* WARNING obsolete !!! user defined YYDEBUG not reported into generated header */
/* use %define DEBUG */
#endif
#endif
#ifdef YY_mootLexfreqsParser_STYPE
#ifndef yystype
#define yystype YY_mootLexfreqsParser_STYPE
#endif
#endif
#endif

#ifndef YY_mootLexfreqsParser_PURE

/* #line 54 "/usr/local/share/flex++bison++/bison.h" */

#line 54 "/usr/local/share/flex++bison++/bison.h"
/* YY_mootLexfreqsParser_PURE */
#endif

/* #line 56 "/usr/local/share/flex++bison++/bison.h" */

#line 56 "/usr/local/share/flex++bison++/bison.h"
/* prefix */
#ifndef YY_mootLexfreqsParser_DEBUG

/* #line 58 "/usr/local/share/flex++bison++/bison.h" */

#line 58 "/usr/local/share/flex++bison++/bison.h"
/* YY_mootLexfreqsParser_DEBUG */
#endif
#ifndef YY_mootLexfreqsParser_LSP_NEEDED

/* #line 61 "/usr/local/share/flex++bison++/bison.h" */

#line 61 "/usr/local/share/flex++bison++/bison.h"
 /* YY_mootLexfreqsParser_LSP_NEEDED*/
#endif
/* DEFAULT LTYPE*/
#ifdef YY_mootLexfreqsParser_LSP_NEEDED
#ifndef YY_mootLexfreqsParser_LTYPE
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

#define YY_mootLexfreqsParser_LTYPE yyltype
#endif
#endif
/* DEFAULT STYPE*/
#ifndef YY_mootLexfreqsParser_STYPE
#define YY_mootLexfreqsParser_STYPE int
#endif
/* DEFAULT MISCELANEOUS */
#ifndef YY_mootLexfreqsParser_PARSE
#define YY_mootLexfreqsParser_PARSE yyparse
#endif
#ifndef YY_mootLexfreqsParser_LEX
#define YY_mootLexfreqsParser_LEX yylex
#endif
#ifndef YY_mootLexfreqsParser_LVAL
#define YY_mootLexfreqsParser_LVAL yylval
#endif
#ifndef YY_mootLexfreqsParser_LLOC
#define YY_mootLexfreqsParser_LLOC yylloc
#endif
#ifndef YY_mootLexfreqsParser_CHAR
#define YY_mootLexfreqsParser_CHAR yychar
#endif
#ifndef YY_mootLexfreqsParser_NERRS
#define YY_mootLexfreqsParser_NERRS yynerrs
#endif
#ifndef YY_mootLexfreqsParser_DEBUG_FLAG
#define YY_mootLexfreqsParser_DEBUG_FLAG yydebug
#endif
#ifndef YY_mootLexfreqsParser_ERROR
#define YY_mootLexfreqsParser_ERROR yyerror
#endif

#ifndef YY_mootLexfreqsParser_PARSE_PARAM
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
#define YY_mootLexfreqsParser_PARSE_PARAM
#ifndef YY_mootLexfreqsParser_PARSE_PARAM_DEF
#define YY_mootLexfreqsParser_PARSE_PARAM_DEF
#endif
#endif
#endif
#endif
#ifndef YY_mootLexfreqsParser_PARSE_PARAM
#define YY_mootLexfreqsParser_PARSE_PARAM void
#endif
#endif

/* TOKEN C */
#ifndef YY_USE_CLASS

#ifndef YY_mootLexfreqsParser_PURE
extern YY_mootLexfreqsParser_STYPE YY_mootLexfreqsParser_LVAL;
#endif


/* #line 134 "/usr/local/share/flex++bison++/bison.h" */
#define	TOKEN	258
#define	COUNT	259


#line 134 "/usr/local/share/flex++bison++/bison.h"
 /* #defines token */
/* after #define tokens, before const tokens S5*/
#else
#ifndef YY_mootLexfreqsParser_CLASS
#define YY_mootLexfreqsParser_CLASS mootLexfreqsParser
#endif

#ifndef YY_mootLexfreqsParser_INHERIT
#define YY_mootLexfreqsParser_INHERIT
#endif
#ifndef YY_mootLexfreqsParser_MEMBERS
#define YY_mootLexfreqsParser_MEMBERS 
#endif
#ifndef YY_mootLexfreqsParser_LEX_BODY
#define YY_mootLexfreqsParser_LEX_BODY  
#endif
#ifndef YY_mootLexfreqsParser_ERROR_BODY
#define YY_mootLexfreqsParser_ERROR_BODY  
#endif
#ifndef YY_mootLexfreqsParser_CONSTRUCTOR_PARAM
#define YY_mootLexfreqsParser_CONSTRUCTOR_PARAM
#endif

class YY_mootLexfreqsParser_CLASS YY_mootLexfreqsParser_INHERIT
{
public: /* static const int token ... */

/* #line 160 "/usr/local/share/flex++bison++/bison.h" */
static const int TOKEN;
static const int COUNT;


#line 160 "/usr/local/share/flex++bison++/bison.h"
 /* decl const */
public:
 int YY_mootLexfreqsParser_PARSE(YY_mootLexfreqsParser_PARSE_PARAM);
 virtual void YY_mootLexfreqsParser_ERROR(char *) YY_mootLexfreqsParser_ERROR_BODY;
#ifdef YY_mootLexfreqsParser_PURE
#ifdef YY_mootLexfreqsParser_LSP_NEEDED
 virtual int  YY_mootLexfreqsParser_LEX(YY_mootLexfreqsParser_STYPE *YY_mootLexfreqsParser_LVAL,YY_mootLexfreqsParser_LTYPE *YY_mootLexfreqsParser_LLOC) YY_mootLexfreqsParser_LEX_BODY;
#else
 virtual int  YY_mootLexfreqsParser_LEX(YY_mootLexfreqsParser_STYPE *YY_mootLexfreqsParser_LVAL) YY_mootLexfreqsParser_LEX_BODY;
#endif
#else
 virtual int YY_mootLexfreqsParser_LEX() YY_mootLexfreqsParser_LEX_BODY;
 YY_mootLexfreqsParser_STYPE YY_mootLexfreqsParser_LVAL;
#ifdef YY_mootLexfreqsParser_LSP_NEEDED
 YY_mootLexfreqsParser_LTYPE YY_mootLexfreqsParser_LLOC;
#endif
 int YY_mootLexfreqsParser_NERRS;
 int YY_mootLexfreqsParser_CHAR;
#endif
#if YY_mootLexfreqsParser_DEBUG != 0
public:
 int YY_mootLexfreqsParser_DEBUG_FLAG;	/*  nonzero means print parse trace	*/
#endif
public:
 YY_mootLexfreqsParser_CLASS(YY_mootLexfreqsParser_CONSTRUCTOR_PARAM);
public:
 YY_mootLexfreqsParser_MEMBERS 
};
/* other declare folow */
#endif


#if YY_mootLexfreqsParser_COMPATIBILITY != 0
/* backward compatibility */
#ifndef YYSTYPE
#define YYSTYPE YY_mootLexfreqsParser_STYPE
#endif

#ifndef YYLTYPE
#define YYLTYPE YY_mootLexfreqsParser_LTYPE
#endif
#ifndef YYDEBUG
#ifdef YY_mootLexfreqsParser_DEBUG 
#define YYDEBUG YY_mootLexfreqsParser_DEBUG
#endif
#endif

#endif
/* END */

/* #line 209 "/usr/local/share/flex++bison++/bison.h" */
#endif
