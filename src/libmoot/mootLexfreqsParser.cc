#define YY_mootLexfreqsParser_h_included

/*  A Bison++ parser, made from mootLexfreqsParser.yy  */

 /* with Bison++ version bison++ Version 1.21-5, adapted from GNU bison by coetmeur@icdc.fr
  */


#line 1 "/usr/local/share/flex++bison++/bison.cc"
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Bob Corbett and Richard Stallman

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* HEADER SECTION */
#ifndef _MSDOS
#ifdef MSDOS
#define _MSDOS
#endif
#endif
/* turboc */
#ifdef __MSDOS__
#ifndef _MSDOS
#define _MSDOS
#endif
#endif

#ifndef alloca
#if defined( __GNUC__)
#define alloca __builtin_alloca

#elif (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc)  || defined (__sgi)
#include <alloca.h>

#elif defined (_MSDOS)
#include <malloc.h>
#ifndef __TURBOC__
/* MS C runtime lib */
#define alloca _alloca
#endif

#elif defined(_AIX)
#include <malloc.h>
#pragma alloca

#elif defined(__hpux)
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */

#endif /* not _AIX  not MSDOS, or __TURBOC__ or _AIX, not sparc.  */
#endif /* alloca not defined.  */
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
#ifndef __STDC__
#define const
#endif
#endif
#include <stdio.h>
#define YYBISON 1  

/* #line 77 "/usr/local/share/flex++bison++/bison.cc" */
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
#line 92 "mootLexfreqsParser.yy"

#ifdef HAVE_CONFIG_H
# include <mootConfig.h>
#endif
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
   virtual ~mootLexfreqsParser(void) {}; \
   /* report warnings */\
   virtual void yywarn(const char *msg) { \
      yycarp("mootLexfreqsParser: Warning: %s", msg);\
   }; \
   /** report anything */\
   virtual void yycarp(char *fmt, ...);
#define YY_mootLexfreqsParser_CONSTRUCTOR_INIT  : lexfreqs(NULL), alltags(NULL)

#line 127 "mootLexfreqsParser.yy"
typedef union {
  moot::mootTokString *tokstr;         ///< for single tokens or tags (strings)
  moot::CountT           count;         ///< for tag-list counts
} yy_mootLexfreqsParser_stype;
#define YY_mootLexfreqsParser_STYPE yy_mootLexfreqsParser_stype
#line 132 "mootLexfreqsParser.yy"

/**
 * \typedef yy_mootLexfreqsParser_stype
 * \brief Bison++ semantic value typedef for moot-pargen parameter-file parser.
 */

#line 77 "/usr/local/share/flex++bison++/bison.cc"
/* %{ and %header{ and %union, during decl */
#define YY_mootLexfreqsParser_BISON 1
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
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_mootLexfreqsParser_STYPE 
#define YY_mootLexfreqsParser_STYPE YYSTYPE
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_mootLexfreqsParser_DEBUG
#define  YY_mootLexfreqsParser_DEBUG YYDEBUG
#endif
#endif
#ifdef YY_mootLexfreqsParser_STYPE
#ifndef yystype
#define yystype YY_mootLexfreqsParser_STYPE
#endif
#endif
#endif

#ifndef YY_mootLexfreqsParser_PURE

/* #line 112 "/usr/local/share/flex++bison++/bison.cc" */

#line 112 "/usr/local/share/flex++bison++/bison.cc"
/*  YY_mootLexfreqsParser_PURE */
#endif

/* section apres lecture def, avant lecture grammaire S2 */

/* #line 116 "/usr/local/share/flex++bison++/bison.cc" */

#line 116 "/usr/local/share/flex++bison++/bison.cc"
/* prefix */
#ifndef YY_mootLexfreqsParser_DEBUG

/* #line 118 "/usr/local/share/flex++bison++/bison.cc" */

#line 118 "/usr/local/share/flex++bison++/bison.cc"
/* YY_mootLexfreqsParser_DEBUG */
#endif


#ifndef YY_mootLexfreqsParser_LSP_NEEDED

/* #line 123 "/usr/local/share/flex++bison++/bison.cc" */

#line 123 "/usr/local/share/flex++bison++/bison.cc"
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
      /* We used to use `unsigned long' as YY_mootLexfreqsParser_STYPE on MSDOS,
	 but it seems better to be consistent.
	 Most programs should declare their own type anyway.  */

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
#if YY_mootLexfreqsParser_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YY_mootLexfreqsParser_LTYPE
#ifndef YYLTYPE
#define YYLTYPE YY_mootLexfreqsParser_LTYPE
#else
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
#endif
#endif
#ifndef YYSTYPE
#define YYSTYPE YY_mootLexfreqsParser_STYPE
#else
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
#endif
#ifdef YY_mootLexfreqsParser_PURE
#ifndef YYPURE
#define YYPURE YY_mootLexfreqsParser_PURE
#endif
#endif
#ifdef YY_mootLexfreqsParser_DEBUG
#ifndef YYDEBUG
#define YYDEBUG YY_mootLexfreqsParser_DEBUG 
#endif
#endif
#ifndef YY_mootLexfreqsParser_ERROR_VERBOSE
#ifdef YYERROR_VERBOSE
#define YY_mootLexfreqsParser_ERROR_VERBOSE YYERROR_VERBOSE
#endif
#endif
#ifndef YY_mootLexfreqsParser_LSP_NEEDED
#ifdef YYLSP_NEEDED
#define YY_mootLexfreqsParser_LSP_NEEDED YYLSP_NEEDED
#endif
#endif


/* #line 230 "/usr/local/share/flex++bison++/bison.cc" */
#define	TOKEN	258
#define	COUNT	259


#line 230 "/usr/local/share/flex++bison++/bison.cc"
 /* #defines tokens */
#else
/* CLASS */
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
#ifndef YY_mootLexfreqsParser_CONSTRUCTOR_CODE
#define YY_mootLexfreqsParser_CONSTRUCTOR_CODE
#endif
#ifndef YY_mootLexfreqsParser_CONSTRUCTOR_INIT
#define YY_mootLexfreqsParser_CONSTRUCTOR_INIT
#endif

class YY_mootLexfreqsParser_CLASS YY_mootLexfreqsParser_INHERIT
{
public: /* static const int token ... */

/* #line 261 "/usr/local/share/flex++bison++/bison.cc" */
static const int TOKEN;
static const int COUNT;


#line 261 "/usr/local/share/flex++bison++/bison.cc"
 /* decl const */
public:
 int YY_mootLexfreqsParser_PARSE (YY_mootLexfreqsParser_PARSE_PARAM);
 virtual void YY_mootLexfreqsParser_ERROR(char *msg) YY_mootLexfreqsParser_ERROR_BODY;
#ifdef YY_mootLexfreqsParser_PURE
#ifdef YY_mootLexfreqsParser_LSP_NEEDED
 virtual int  YY_mootLexfreqsParser_LEX (YY_mootLexfreqsParser_STYPE *YY_mootLexfreqsParser_LVAL,YY_mootLexfreqsParser_LTYPE *YY_mootLexfreqsParser_LLOC) YY_mootLexfreqsParser_LEX_BODY;
#else
 virtual int  YY_mootLexfreqsParser_LEX (YY_mootLexfreqsParser_STYPE *YY_mootLexfreqsParser_LVAL) YY_mootLexfreqsParser_LEX_BODY;
#endif
#else
 virtual int YY_mootLexfreqsParser_LEX() YY_mootLexfreqsParser_LEX_BODY;
 YY_mootLexfreqsParser_STYPE YY_mootLexfreqsParser_LVAL;
#ifdef YY_mootLexfreqsParser_LSP_NEEDED
 YY_mootLexfreqsParser_LTYPE YY_mootLexfreqsParser_LLOC;
#endif
 int   YY_mootLexfreqsParser_NERRS;
 int    YY_mootLexfreqsParser_CHAR;
#endif
#if YY_mootLexfreqsParser_DEBUG != 0
 int YY_mootLexfreqsParser_DEBUG_FLAG;   /*  nonzero means print parse trace     */
#endif
public:
 YY_mootLexfreqsParser_CLASS(YY_mootLexfreqsParser_CONSTRUCTOR_PARAM);
public:
 YY_mootLexfreqsParser_MEMBERS 
};
/* other declare folow */

/* #line 289 "/usr/local/share/flex++bison++/bison.cc" */
const int YY_mootLexfreqsParser_CLASS::TOKEN=258;
const int YY_mootLexfreqsParser_CLASS::COUNT=259;


#line 289 "/usr/local/share/flex++bison++/bison.cc"
 /* const YY_mootLexfreqsParser_CLASS::token */
/*apres const  */
YY_mootLexfreqsParser_CLASS::YY_mootLexfreqsParser_CLASS(YY_mootLexfreqsParser_CONSTRUCTOR_PARAM) YY_mootLexfreqsParser_CONSTRUCTOR_INIT
{
#if YY_mootLexfreqsParser_DEBUG != 0
YY_mootLexfreqsParser_DEBUG_FLAG=0;
#endif
YY_mootLexfreqsParser_CONSTRUCTOR_CODE;
};
#endif

/* #line 299 "/usr/local/share/flex++bison++/bison.cc" */


#define	YYFINAL		24
#define	YYFLAG		-32768
#define	YYNTBASE	7

#define YYTRANSLATE(x) ((unsigned)(x) <= 259 ? yytranslate[x] : 16)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     5,     6,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4
};

#if YY_mootLexfreqsParser_DEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     4,     7,    10,    12,    16,    22,    24,    26,
    28,    30,    31,    33,    35,    36,    38,    41
};

static const short yyrhs[] = {    -1,
     7,     8,     0,    15,     7,     0,     9,    15,     0,    10,
     0,    10,    14,    13,     0,     9,    14,    11,    14,    12,
     0,     3,     0,     4,     0,    10,     0,     4,     0,     0,
    12,     0,     5,     0,     0,     6,     0,    15,     6,     0,
     0
};

#endif

#if YY_mootLexfreqsParser_DEBUG != 0
static const short yyrline[] = { 0,
   153,   154,   155,   158,   165,   166,   167,   175,   179,   185,
   192,   193,   200,   203,   204,   211,   212,   213
};

static const char * const yytname[] = {   "$","error","$illegal.","TOKEN","COUNT",
"'\\t'","'\\n'","entries","entry","entryBody","token","tag","count","total",
"tab","newline",""
};
#endif

static const short yyr1[] = {     0,
     7,     7,     7,     8,     9,     9,     9,    10,    10,    11,
    12,    12,    13,    14,    14,    15,    15,    15
};

static const short yyr2[] = {     0,
     0,     2,     2,     2,     1,     3,     5,     1,     1,     1,
     1,     0,     1,     1,     0,     1,     2,     0
};

static const short yydefact[] = {     1,
    16,     0,     1,     8,     9,     2,    15,     5,    16,     3,
    14,     0,     4,    12,    10,    15,    17,    11,    13,     6,
    12,     7,     0,     0
};

static const short yydefgoto[] = {     2,
     6,     7,     8,    16,    19,    20,    12,     3
};

static const short yypact[] = {    -2,
-32768,     9,     4,-32768,-32768,-32768,     1,     0,-32768,    -1,
-32768,    -1,     5,    10,-32768,     0,-32768,-32768,-32768,-32768,
    10,-32768,    15,-32768
};

static const short yypgoto[] = {    13,
-32768,-32768,     6,-32768,    -4,-32768,    -8,    12
};


#define	YYLAST		19


static const short yytable[] = {    14,
   -18,     4,     5,     1,    11,    11,     1,    21,    23,     9,
    17,     4,     5,    18,    24,    10,    22,    15,    13
};

static const short yycheck[] = {     8,
     0,     3,     4,     6,     5,     5,     6,    16,     0,     6,
     6,     3,     4,     4,     0,     3,    21,    12,     7
};

#line 299 "/usr/local/share/flex++bison++/bison.cc"
 /* fattrs + tables */

/* parser code folow  */


/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: dollar marks section change
   the next  is replaced by the list of actions, each action
   as one case of the switch.  */ 

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (YY_mootLexfreqsParser_CHAR = YYEMPTY)
#define YYEMPTY         -2
#define YYEOF           0
#define YYACCEPT        return(0)
#define YYABORT         return(1)
#define YYERROR         goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL          goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do                                                              \
  if (YY_mootLexfreqsParser_CHAR == YYEMPTY && yylen == 1)                               \
    { YY_mootLexfreqsParser_CHAR = (token), YY_mootLexfreqsParser_LVAL = (value);                 \
      yychar1 = YYTRANSLATE (YY_mootLexfreqsParser_CHAR);                                \
      YYPOPSTACK;                                               \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    { YY_mootLexfreqsParser_ERROR ("syntax error: cannot back up"); YYERROR; }   \
while (0)

#define YYTERROR        1
#define YYERRCODE       256

#ifndef YY_mootLexfreqsParser_PURE
/* UNPURE */
#define YYLEX           YY_mootLexfreqsParser_LEX()
#ifndef YY_USE_CLASS
/* If nonreentrant, and not class , generate the variables here */
int     YY_mootLexfreqsParser_CHAR;                      /*  the lookahead symbol        */
YY_mootLexfreqsParser_STYPE      YY_mootLexfreqsParser_LVAL;              /*  the semantic value of the */
				/*  lookahead symbol    */
int YY_mootLexfreqsParser_NERRS;                 /*  number of parse errors so far */
#ifdef YY_mootLexfreqsParser_LSP_NEEDED
YY_mootLexfreqsParser_LTYPE YY_mootLexfreqsParser_LLOC;   /*  location data for the lookahead     */
			/*  symbol                              */
#endif
#endif


#else
/* PURE */
#ifdef YY_mootLexfreqsParser_LSP_NEEDED
#define YYLEX           YY_mootLexfreqsParser_LEX(&YY_mootLexfreqsParser_LVAL, &YY_mootLexfreqsParser_LLOC)
#else
#define YYLEX           YY_mootLexfreqsParser_LEX(&YY_mootLexfreqsParser_LVAL)
#endif
#endif
#ifndef YY_USE_CLASS
#if YY_mootLexfreqsParser_DEBUG != 0
int YY_mootLexfreqsParser_DEBUG_FLAG;                    /*  nonzero means print parse trace     */
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif
#endif



/*  YYINITDEPTH indicates the initial size of the parser's stacks       */

#ifndef YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif


#if __GNUC__ > 1                /* GNU C and GNU C++ define this.  */
#define __yy_bcopy(FROM,TO,COUNT)       __builtin_memcpy(TO,FROM,COUNT)
#else                           /* not GNU C or C++ */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */

#ifdef __cplusplus
static void __yy_bcopy (char *from, char *to, int count)
#else
#ifdef __STDC__
static void __yy_bcopy (char *from, char *to, int count)
#else
static void __yy_bcopy (from, to, count)
     char *from;
     char *to;
     int count;
#endif
#endif
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}
#endif

int
#ifdef YY_USE_CLASS
 YY_mootLexfreqsParser_CLASS::
#endif
     YY_mootLexfreqsParser_PARSE(YY_mootLexfreqsParser_PARSE_PARAM)
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
/* parameter definition without protypes */
YY_mootLexfreqsParser_PARSE_PARAM_DEF
#endif
#endif
#endif
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YY_mootLexfreqsParser_STYPE *yyvsp;
  int yyerrstatus;      /*  number of tokens to shift before error messages enabled */
  int yychar1=0;          /*  lookahead token as an internal (translated) token number */

  short yyssa[YYINITDEPTH];     /*  the state stack                     */
  YY_mootLexfreqsParser_STYPE yyvsa[YYINITDEPTH];        /*  the semantic value stack            */

  short *yyss = yyssa;          /*  refer to the stacks thru separate pointers */
  YY_mootLexfreqsParser_STYPE *yyvs = yyvsa;     /*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YY_mootLexfreqsParser_LSP_NEEDED
  YY_mootLexfreqsParser_LTYPE yylsa[YYINITDEPTH];        /*  the location stack                  */
  YY_mootLexfreqsParser_LTYPE *yyls = yylsa;
  YY_mootLexfreqsParser_LTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YY_mootLexfreqsParser_PURE
  int YY_mootLexfreqsParser_CHAR;
  YY_mootLexfreqsParser_STYPE YY_mootLexfreqsParser_LVAL;
  int YY_mootLexfreqsParser_NERRS;
#ifdef YY_mootLexfreqsParser_LSP_NEEDED
  YY_mootLexfreqsParser_LTYPE YY_mootLexfreqsParser_LLOC;
#endif
#endif

  YY_mootLexfreqsParser_STYPE yyval;             /*  the variable used to return         */
				/*  semantic values from the action     */
				/*  routines                            */

  int yylen;

#if YY_mootLexfreqsParser_DEBUG != 0
  if (YY_mootLexfreqsParser_DEBUG_FLAG)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  YY_mootLexfreqsParser_NERRS = 0;
  YY_mootLexfreqsParser_CHAR = YYEMPTY;          /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YY_mootLexfreqsParser_LSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YY_mootLexfreqsParser_STYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YY_mootLexfreqsParser_LSP_NEEDED
      YY_mootLexfreqsParser_LTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YY_mootLexfreqsParser_LSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YY_mootLexfreqsParser_LSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  YY_mootLexfreqsParser_ERROR("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyvs = (YY_mootLexfreqsParser_STYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#ifdef YY_mootLexfreqsParser_LSP_NEEDED
      yyls = (YY_mootLexfreqsParser_LTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YY_mootLexfreqsParser_LSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YY_mootLexfreqsParser_DEBUG != 0
      if (YY_mootLexfreqsParser_DEBUG_FLAG)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YY_mootLexfreqsParser_DEBUG != 0
  if (YY_mootLexfreqsParser_DEBUG_FLAG)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (YY_mootLexfreqsParser_CHAR == YYEMPTY)
    {
#if YY_mootLexfreqsParser_DEBUG != 0
      if (YY_mootLexfreqsParser_DEBUG_FLAG)
	fprintf(stderr, "Reading a token: ");
#endif
      YY_mootLexfreqsParser_CHAR = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (YY_mootLexfreqsParser_CHAR <= 0)           /* This means end of input. */
    {
      yychar1 = 0;
      YY_mootLexfreqsParser_CHAR = YYEOF;                /* Don't call YYLEX any more */

#if YY_mootLexfreqsParser_DEBUG != 0
      if (YY_mootLexfreqsParser_DEBUG_FLAG)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(YY_mootLexfreqsParser_CHAR);

#if YY_mootLexfreqsParser_DEBUG != 0
      if (YY_mootLexfreqsParser_DEBUG_FLAG)
	{
	  fprintf (stderr, "Next token is %d (%s", YY_mootLexfreqsParser_CHAR, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, YY_mootLexfreqsParser_CHAR, YY_mootLexfreqsParser_LVAL);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YY_mootLexfreqsParser_DEBUG != 0
  if (YY_mootLexfreqsParser_DEBUG_FLAG)
    fprintf(stderr, "Shifting token %d (%s), ", YY_mootLexfreqsParser_CHAR, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (YY_mootLexfreqsParser_CHAR != YYEOF)
    YY_mootLexfreqsParser_CHAR = YYEMPTY;

  *++yyvsp = YY_mootLexfreqsParser_LVAL;
#ifdef YY_mootLexfreqsParser_LSP_NEEDED
  *++yylsp = YY_mootLexfreqsParser_LLOC;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YY_mootLexfreqsParser_DEBUG != 0
  if (YY_mootLexfreqsParser_DEBUG_FLAG)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


/* #line 712 "/usr/local/share/flex++bison++/bison.cc" */

  switch (yyn) {

case 1:
#line 153 "mootLexfreqsParser.yy"
{ yyval.count = 0; ;
    break;}
case 2:
#line 154 "mootLexfreqsParser.yy"
{ yyval.count = 0; ;
    break;}
case 3:
#line 155 "mootLexfreqsParser.yy"
{ yyval.count = 0; ;
    break;}
case 4:
#line 159 "mootLexfreqsParser.yy"
{
		  delete yyvsp[-1].tokstr;
                  yyval.count = 0;
		;
    break;}
case 5:
#line 165 "mootLexfreqsParser.yy"
{ yyval.tokstr = yyvsp[0].tokstr; ;
    break;}
case 6:
#line 166 "mootLexfreqsParser.yy"
{ yyval.tokstr = yyvsp[-2].tokstr; /* total is optional (not really) */ ;
    break;}
case 7:
#line 168 "mootLexfreqsParser.yy"
{
		  lexfreqs->add_count(*yyvsp[-4].tokstr, *yyvsp[-2].tokstr, yyvsp[0].count);
                  delete yyvsp[-2].tokstr;
                  yyval.tokstr = yyvsp[-4].tokstr;
		;
    break;}
case 8:
#line 176 "mootLexfreqsParser.yy"
{
		  yyval.tokstr = yyvsp[0].tokstr;
		;
    break;}
case 9:
#line 180 "mootLexfreqsParser.yy"
{
		  yyval.tokstr = new moot::mootTokString(reinterpret_cast<const char *>(yylloc.text));
		;
    break;}
case 10:
#line 186 "mootLexfreqsParser.yy"
{
		  if (alltags) alltags->insert(*yyvsp[0].tokstr);
		  yyval.tokstr = yyvsp[0].tokstr;
		;
    break;}
case 11:
#line 192 "mootLexfreqsParser.yy"
{ yyval.count = yyvsp[0].count; ;
    break;}
case 12:
#line 194 "mootLexfreqsParser.yy"
{
		    yyerror("expected a COUNT.");
                    YYABORT;
		;
    break;}
case 13:
#line 200 "mootLexfreqsParser.yy"
{ yyval.count = yyvsp[0].count; ;
    break;}
case 14:
#line 203 "mootLexfreqsParser.yy"
{ yyval.count=0; ;
    break;}
case 15:
#line 205 "mootLexfreqsParser.yy"
{
		    yyerror("expected a TAB.");
                    YYABORT;
                ;
    break;}
case 16:
#line 211 "mootLexfreqsParser.yy"
{ yyval.count=0; ;
    break;}
case 18:
#line 214 "mootLexfreqsParser.yy"
{
		    yyerror("expected a NEWLINE.");
                    YYABORT;
		;
    break;}
}

#line 712 "/usr/local/share/flex++bison++/bison.cc"
   /* the action file gets copied in in place of this dollarsign  */
  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YY_mootLexfreqsParser_LSP_NEEDED
  yylsp -= yylen;
#endif

#if YY_mootLexfreqsParser_DEBUG != 0
  if (YY_mootLexfreqsParser_DEBUG_FLAG)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YY_mootLexfreqsParser_LSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = YY_mootLexfreqsParser_LLOC.first_line;
      yylsp->first_column = YY_mootLexfreqsParser_LLOC.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++YY_mootLexfreqsParser_NERRS;

#ifdef YY_mootLexfreqsParser_ERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      YY_mootLexfreqsParser_ERROR(msg);
	      free(msg);
	    }
	  else
	    YY_mootLexfreqsParser_ERROR ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YY_mootLexfreqsParser_ERROR_VERBOSE */
	YY_mootLexfreqsParser_ERROR("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (YY_mootLexfreqsParser_CHAR == YYEOF)
	YYABORT;

#if YY_mootLexfreqsParser_DEBUG != 0
      if (YY_mootLexfreqsParser_DEBUG_FLAG)
	fprintf(stderr, "Discarding token %d (%s).\n", YY_mootLexfreqsParser_CHAR, yytname[yychar1]);
#endif

      YY_mootLexfreqsParser_CHAR = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;              /* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YY_mootLexfreqsParser_LSP_NEEDED
  yylsp--;
#endif

#if YY_mootLexfreqsParser_DEBUG != 0
  if (YY_mootLexfreqsParser_DEBUG_FLAG)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YY_mootLexfreqsParser_DEBUG != 0
  if (YY_mootLexfreqsParser_DEBUG_FLAG)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = YY_mootLexfreqsParser_LVAL;
#ifdef YY_mootLexfreqsParser_LSP_NEEDED
  *++yylsp = YY_mootLexfreqsParser_LLOC;
#endif

  yystate = yyn;
  goto yynewstate;
}

/* END */

/* #line 909 "/usr/local/share/flex++bison++/bison.cc" */
#line 220 "mootLexfreqsParser.yy"

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
