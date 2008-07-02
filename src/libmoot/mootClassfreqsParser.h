#ifndef YY_mootClassfreqsParser_h_included
#define YY_mootClassfreqsParser_h_included

#line 1 "./bison.h"
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

/* #line 14 "./bison.h" */
#define YY_mootClassfreqsParser_LSP_NEEDED 
#define YY_mootClassfreqsParser_LTYPE  mootClassfreqsParserLType
#define YY_mootClassfreqsParser_DEBUG  1
#define YY_mootClassfreqsParser_ERROR_VERBOSE 
#define YY_mootClassfreqsParser_LEX_BODY  =0
#line 55 "mootClassfreqsParser.yy"

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
#define YY_mootClassfreqsParser_CLASS  mootClassfreqsParser
#define YY_mootClassfreqsParser_MEMBERS  \
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
#define YY_mootClassfreqsParser_CONSTRUCTOR_INIT  : cfreqs(NULL)

#line 128 "mootClassfreqsParser.yy"
typedef union {
  moot::mootTagString  *tagstr;         ///< for single tags (strings)
  moot::CountT           count;         ///< for tag-list counts
} yy_mootClassfreqsParser_stype;
#define YY_mootClassfreqsParser_STYPE yy_mootClassfreqsParser_stype
#line 133 "mootClassfreqsParser.yy"

/**
 * \typedef yy_mootClassfreqsParser_stype
 * \brief Bison++ semantic value typedef for moot-pargen parameter-file parser.
 */

#line 14 "./bison.h"
 /* %{ and %header{ and %union, during decl */
#ifndef YY_mootClassfreqsParser_COMPATIBILITY
#ifndef YY_USE_CLASS
#define  YY_mootClassfreqsParser_COMPATIBILITY 1
#else
#define  YY_mootClassfreqsParser_COMPATIBILITY 0
#endif
#endif

#if YY_mootClassfreqsParser_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YYLTYPE
#ifndef YY_mootClassfreqsParser_LTYPE
#define YY_mootClassfreqsParser_LTYPE YYLTYPE
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
/* use %define LTYPE */
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_mootClassfreqsParser_STYPE 
#define YY_mootClassfreqsParser_STYPE YYSTYPE
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
/* use %define STYPE */
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_mootClassfreqsParser_DEBUG
#define  YY_mootClassfreqsParser_DEBUG YYDEBUG
/* WARNING obsolete !!! user defined YYDEBUG not reported into generated header */
/* use %define DEBUG */
#endif
#endif
#ifdef YY_mootClassfreqsParser_STYPE
#ifndef yystype
#define yystype YY_mootClassfreqsParser_STYPE
#endif
#endif
#endif

#ifndef YY_mootClassfreqsParser_PURE

/* #line 54 "./bison.h" */

#line 54 "./bison.h"
/* YY_mootClassfreqsParser_PURE */
#endif

/* #line 56 "./bison.h" */

#line 56 "./bison.h"
/* prefix */
#ifndef YY_mootClassfreqsParser_DEBUG

/* #line 58 "./bison.h" */

#line 58 "./bison.h"
/* YY_mootClassfreqsParser_DEBUG */
#endif
#ifndef YY_mootClassfreqsParser_LSP_NEEDED

/* #line 61 "./bison.h" */

#line 61 "./bison.h"
 /* YY_mootClassfreqsParser_LSP_NEEDED*/
#endif
/* DEFAULT LTYPE*/
#ifdef YY_mootClassfreqsParser_LSP_NEEDED
#ifndef YY_mootClassfreqsParser_LTYPE
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

#define YY_mootClassfreqsParser_LTYPE yyltype
#endif
#endif
/* DEFAULT STYPE*/
#ifndef YY_mootClassfreqsParser_STYPE
#define YY_mootClassfreqsParser_STYPE int
#endif
/* DEFAULT MISCELANEOUS */
#ifndef YY_mootClassfreqsParser_PARSE
#define YY_mootClassfreqsParser_PARSE yyparse
#endif
#ifndef YY_mootClassfreqsParser_LEX
#define YY_mootClassfreqsParser_LEX yylex
#endif
#ifndef YY_mootClassfreqsParser_LVAL
#define YY_mootClassfreqsParser_LVAL yylval
#endif
#ifndef YY_mootClassfreqsParser_LLOC
#define YY_mootClassfreqsParser_LLOC yylloc
#endif
#ifndef YY_mootClassfreqsParser_CHAR
#define YY_mootClassfreqsParser_CHAR yychar
#endif
#ifndef YY_mootClassfreqsParser_NERRS
#define YY_mootClassfreqsParser_NERRS yynerrs
#endif
#ifndef YY_mootClassfreqsParser_DEBUG_FLAG
#define YY_mootClassfreqsParser_DEBUG_FLAG yydebug
#endif
#ifndef YY_mootClassfreqsParser_ERROR
#define YY_mootClassfreqsParser_ERROR yyerror
#endif

#ifndef YY_mootClassfreqsParser_PARSE_PARAM
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
#define YY_mootClassfreqsParser_PARSE_PARAM
#ifndef YY_mootClassfreqsParser_PARSE_PARAM_DEF
#define YY_mootClassfreqsParser_PARSE_PARAM_DEF
#endif
#endif
#endif
#endif
#ifndef YY_mootClassfreqsParser_PARSE_PARAM
#define YY_mootClassfreqsParser_PARSE_PARAM void
#endif
#endif

/* TOKEN C */
#ifndef YY_USE_CLASS

#ifndef YY_mootClassfreqsParser_PURE
extern YY_mootClassfreqsParser_STYPE YY_mootClassfreqsParser_LVAL;
#endif


/* #line 134 "./bison.h" */
#define	TAG	258
#define	COUNT	259


#line 134 "./bison.h"
 /* #defines token */
/* after #define tokens, before const tokens S5*/
#else
#ifndef YY_mootClassfreqsParser_CLASS
#define YY_mootClassfreqsParser_CLASS mootClassfreqsParser
#endif

#ifndef YY_mootClassfreqsParser_INHERIT
#define YY_mootClassfreqsParser_INHERIT
#endif
#ifndef YY_mootClassfreqsParser_MEMBERS
#define YY_mootClassfreqsParser_MEMBERS 
#endif
#ifndef YY_mootClassfreqsParser_LEX_BODY
#define YY_mootClassfreqsParser_LEX_BODY  
#endif
#ifndef YY_mootClassfreqsParser_ERROR_BODY
#define YY_mootClassfreqsParser_ERROR_BODY  
#endif
#ifndef YY_mootClassfreqsParser_CONSTRUCTOR_PARAM
#define YY_mootClassfreqsParser_CONSTRUCTOR_PARAM
#endif

class YY_mootClassfreqsParser_CLASS YY_mootClassfreqsParser_INHERIT
{
public: /* static const int token ... */

/* #line 160 "./bison.h" */
static const int TAG;
static const int COUNT;


#line 160 "./bison.h"
 /* decl const */
public:
 int YY_mootClassfreqsParser_PARSE(YY_mootClassfreqsParser_PARSE_PARAM);
 virtual void YY_mootClassfreqsParser_ERROR(const char *) YY_mootClassfreqsParser_ERROR_BODY;
#ifdef YY_mootClassfreqsParser_PURE
#ifdef YY_mootClassfreqsParser_LSP_NEEDED
 virtual int  YY_mootClassfreqsParser_LEX(YY_mootClassfreqsParser_STYPE *YY_mootClassfreqsParser_LVAL,YY_mootClassfreqsParser_LTYPE *YY_mootClassfreqsParser_LLOC) YY_mootClassfreqsParser_LEX_BODY;
#else
 virtual int  YY_mootClassfreqsParser_LEX(YY_mootClassfreqsParser_STYPE *YY_mootClassfreqsParser_LVAL) YY_mootClassfreqsParser_LEX_BODY;
#endif
#else
 virtual int YY_mootClassfreqsParser_LEX() YY_mootClassfreqsParser_LEX_BODY;
 YY_mootClassfreqsParser_STYPE YY_mootClassfreqsParser_LVAL;
#ifdef YY_mootClassfreqsParser_LSP_NEEDED
 YY_mootClassfreqsParser_LTYPE YY_mootClassfreqsParser_LLOC;
#endif
 int YY_mootClassfreqsParser_NERRS;
 int YY_mootClassfreqsParser_CHAR;
#endif
#if YY_mootClassfreqsParser_DEBUG != 0
public:
 int YY_mootClassfreqsParser_DEBUG_FLAG;	/*  nonzero means print parse trace	*/
#endif
public:
 YY_mootClassfreqsParser_CLASS(YY_mootClassfreqsParser_CONSTRUCTOR_PARAM);
public:
 YY_mootClassfreqsParser_MEMBERS 
};
/* other declare folow */
#endif


#if YY_mootClassfreqsParser_COMPATIBILITY != 0
/* backward compatibility */
#ifndef YYSTYPE
#define YYSTYPE YY_mootClassfreqsParser_STYPE
#endif

#ifndef YYLTYPE
#define YYLTYPE YY_mootClassfreqsParser_LTYPE
#endif
#ifndef YYDEBUG
#ifdef YY_mootClassfreqsParser_DEBUG 
#define YYDEBUG YY_mootClassfreqsParser_DEBUG
#endif
#endif

#endif
/* END */

/* #line 209 "./bison.h" */
#endif
