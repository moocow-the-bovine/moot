#define YY_mootClassfreqsParser_h_included

/*  A Bison++ parser, made from mootClassfreqsParser.yy  */

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
#line 92 "mootClassfreqsParser.yy"

#ifdef HAVE_CONFIG_H
# include <mootConfig.h>
#endif
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
   virtual void yycarp(char *fmt, ...);
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

#line 77 "/usr/local/share/flex++bison++/bison.cc"
/* %{ and %header{ and %union, during decl */
#define YY_mootClassfreqsParser_BISON 1
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
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_mootClassfreqsParser_STYPE 
#define YY_mootClassfreqsParser_STYPE YYSTYPE
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_mootClassfreqsParser_DEBUG
#define  YY_mootClassfreqsParser_DEBUG YYDEBUG
#endif
#endif
#ifdef YY_mootClassfreqsParser_STYPE
#ifndef yystype
#define yystype YY_mootClassfreqsParser_STYPE
#endif
#endif
#endif

#ifndef YY_mootClassfreqsParser_PURE

/* #line 112 "/usr/local/share/flex++bison++/bison.cc" */

#line 112 "/usr/local/share/flex++bison++/bison.cc"
/*  YY_mootClassfreqsParser_PURE */
#endif

/* section apres lecture def, avant lecture grammaire S2 */

/* #line 116 "/usr/local/share/flex++bison++/bison.cc" */

#line 116 "/usr/local/share/flex++bison++/bison.cc"
/* prefix */
#ifndef YY_mootClassfreqsParser_DEBUG

/* #line 118 "/usr/local/share/flex++bison++/bison.cc" */

#line 118 "/usr/local/share/flex++bison++/bison.cc"
/* YY_mootClassfreqsParser_DEBUG */
#endif


#ifndef YY_mootClassfreqsParser_LSP_NEEDED

/* #line 123 "/usr/local/share/flex++bison++/bison.cc" */

#line 123 "/usr/local/share/flex++bison++/bison.cc"
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
      /* We used to use `unsigned long' as YY_mootClassfreqsParser_STYPE on MSDOS,
	 but it seems better to be consistent.
	 Most programs should declare their own type anyway.  */

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
#if YY_mootClassfreqsParser_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YY_mootClassfreqsParser_LTYPE
#ifndef YYLTYPE
#define YYLTYPE YY_mootClassfreqsParser_LTYPE
#else
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
#endif
#endif
#ifndef YYSTYPE
#define YYSTYPE YY_mootClassfreqsParser_STYPE
#else
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
#endif
#ifdef YY_mootClassfreqsParser_PURE
#ifndef YYPURE
#define YYPURE YY_mootClassfreqsParser_PURE
#endif
#endif
#ifdef YY_mootClassfreqsParser_DEBUG
#ifndef YYDEBUG
#define YYDEBUG YY_mootClassfreqsParser_DEBUG 
#endif
#endif
#ifndef YY_mootClassfreqsParser_ERROR_VERBOSE
#ifdef YYERROR_VERBOSE
#define YY_mootClassfreqsParser_ERROR_VERBOSE YYERROR_VERBOSE
#endif
#endif
#ifndef YY_mootClassfreqsParser_LSP_NEEDED
#ifdef YYLSP_NEEDED
#define YY_mootClassfreqsParser_LSP_NEEDED YYLSP_NEEDED
#endif
#endif


/* #line 230 "/usr/local/share/flex++bison++/bison.cc" */
#define	TAG	258
#define	COUNT	259


#line 230 "/usr/local/share/flex++bison++/bison.cc"
 /* #defines tokens */
#else
/* CLASS */
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
#ifndef YY_mootClassfreqsParser_CONSTRUCTOR_CODE
#define YY_mootClassfreqsParser_CONSTRUCTOR_CODE
#endif
#ifndef YY_mootClassfreqsParser_CONSTRUCTOR_INIT
#define YY_mootClassfreqsParser_CONSTRUCTOR_INIT
#endif

class YY_mootClassfreqsParser_CLASS YY_mootClassfreqsParser_INHERIT
{
public: /* static const int token ... */

/* #line 261 "/usr/local/share/flex++bison++/bison.cc" */
static const int TAG;
static const int COUNT;


#line 261 "/usr/local/share/flex++bison++/bison.cc"
 /* decl const */
public:
 int YY_mootClassfreqsParser_PARSE (YY_mootClassfreqsParser_PARSE_PARAM);
 virtual void YY_mootClassfreqsParser_ERROR(char *msg) YY_mootClassfreqsParser_ERROR_BODY;
#ifdef YY_mootClassfreqsParser_PURE
#ifdef YY_mootClassfreqsParser_LSP_NEEDED
 virtual int  YY_mootClassfreqsParser_LEX (YY_mootClassfreqsParser_STYPE *YY_mootClassfreqsParser_LVAL,YY_mootClassfreqsParser_LTYPE *YY_mootClassfreqsParser_LLOC) YY_mootClassfreqsParser_LEX_BODY;
#else
 virtual int  YY_mootClassfreqsParser_LEX (YY_mootClassfreqsParser_STYPE *YY_mootClassfreqsParser_LVAL) YY_mootClassfreqsParser_LEX_BODY;
#endif
#else
 virtual int YY_mootClassfreqsParser_LEX() YY_mootClassfreqsParser_LEX_BODY;
 YY_mootClassfreqsParser_STYPE YY_mootClassfreqsParser_LVAL;
#ifdef YY_mootClassfreqsParser_LSP_NEEDED
 YY_mootClassfreqsParser_LTYPE YY_mootClassfreqsParser_LLOC;
#endif
 int   YY_mootClassfreqsParser_NERRS;
 int    YY_mootClassfreqsParser_CHAR;
#endif
#if YY_mootClassfreqsParser_DEBUG != 0
 int YY_mootClassfreqsParser_DEBUG_FLAG;   /*  nonzero means print parse trace     */
#endif
public:
 YY_mootClassfreqsParser_CLASS(YY_mootClassfreqsParser_CONSTRUCTOR_PARAM);
public:
 YY_mootClassfreqsParser_MEMBERS 
};
/* other declare folow */

/* #line 289 "/usr/local/share/flex++bison++/bison.cc" */
const int YY_mootClassfreqsParser_CLASS::TAG=258;
const int YY_mootClassfreqsParser_CLASS::COUNT=259;


#line 289 "/usr/local/share/flex++bison++/bison.cc"
 /* const YY_mootClassfreqsParser_CLASS::token */
/*apres const  */
YY_mootClassfreqsParser_CLASS::YY_mootClassfreqsParser_CLASS(YY_mootClassfreqsParser_CONSTRUCTOR_PARAM) YY_mootClassfreqsParser_CONSTRUCTOR_INIT
{
#if YY_mootClassfreqsParser_DEBUG != 0
YY_mootClassfreqsParser_DEBUG_FLAG=0;
#endif
YY_mootClassfreqsParser_CONSTRUCTOR_CODE;
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

#if YY_mootClassfreqsParser_DEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     4,     7,    10,    12,    16,    22,    23,    26,
    29,    31,    33,    34,    36,    38,    39,    41,    44
};

static const short yyrhs[] = {    -1,
     7,     8,     0,    15,     7,     0,     9,    15,     0,    10,
     0,    10,    14,    13,     0,     9,    14,    11,    14,    12,
     0,     0,    10,    11,     0,    10,     4,     0,     3,     0,
     4,     0,     0,    12,     0,     5,     0,     0,     6,     0,
    15,     6,     0,     0
};

#endif

#if YY_mootClassfreqsParser_DEBUG != 0
static const short yyrline[] = { 0,
   154,   155,   156,   159,   166,   167,   168,   176,   177,   183,
   190,   196,   197,   204,   207,   208,   215,   216,   217
};

static const char * const yytname[] = {   "$","error","$illegal.","TAG","COUNT",
"'\\t'","'\\n'","entries","entry","entryBody","lclass","tag","count","total",
"tab","newline",""
};
#endif

static const short yyr1[] = {     0,
     7,     7,     7,     8,     9,     9,     9,    10,    10,    10,
    11,    12,    12,    13,    14,    14,    15,    15,    15
};

static const short yyr2[] = {     0,
     0,     2,     2,     2,     1,     3,     5,     0,     2,     2,
     1,     1,     0,     1,     1,     0,     1,     2,     0
};

static const short yydefact[] = {     1,
    17,     8,     1,     2,    19,     5,    17,     3,    15,     0,
     4,    11,    10,     9,    13,    16,    18,    12,    14,     6,
    13,     7,     0,     0
};

static const short yydefgoto[] = {     2,
     4,     5,     6,    14,    19,    20,    10,     3
};

static const short yypact[] = {    -4,
-32768,     8,     3,-32768,    -2,     2,-32768,-32768,-32768,     9,
     5,-32768,-32768,-32768,    10,    11,-32768,-32768,-32768,-32768,
    10,-32768,    13,-32768
};

static const short yypgoto[] = {    12,
-32768,-32768,-32768,     7,    -3,-32768,    -6,    14
};


#define	YYLAST		19


static const short yytable[] = {    15,
   -16,     1,     9,     1,    12,    13,     9,    23,     7,    21,
    17,    12,    24,    18,     8,     9,    16,    22,    11
};

static const short yycheck[] = {     6,
     3,     6,     5,     6,     3,     4,     5,     0,     6,    16,
     6,     3,     0,     4,     3,     5,    10,    21,     5
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
#define yyclearin       (YY_mootClassfreqsParser_CHAR = YYEMPTY)
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
  if (YY_mootClassfreqsParser_CHAR == YYEMPTY && yylen == 1)                               \
    { YY_mootClassfreqsParser_CHAR = (token), YY_mootClassfreqsParser_LVAL = (value);                 \
      yychar1 = YYTRANSLATE (YY_mootClassfreqsParser_CHAR);                                \
      YYPOPSTACK;                                               \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    { YY_mootClassfreqsParser_ERROR ("syntax error: cannot back up"); YYERROR; }   \
while (0)

#define YYTERROR        1
#define YYERRCODE       256

#ifndef YY_mootClassfreqsParser_PURE
/* UNPURE */
#define YYLEX           YY_mootClassfreqsParser_LEX()
#ifndef YY_USE_CLASS
/* If nonreentrant, and not class , generate the variables here */
int     YY_mootClassfreqsParser_CHAR;                      /*  the lookahead symbol        */
YY_mootClassfreqsParser_STYPE      YY_mootClassfreqsParser_LVAL;              /*  the semantic value of the */
				/*  lookahead symbol    */
int YY_mootClassfreqsParser_NERRS;                 /*  number of parse errors so far */
#ifdef YY_mootClassfreqsParser_LSP_NEEDED
YY_mootClassfreqsParser_LTYPE YY_mootClassfreqsParser_LLOC;   /*  location data for the lookahead     */
			/*  symbol                              */
#endif
#endif


#else
/* PURE */
#ifdef YY_mootClassfreqsParser_LSP_NEEDED
#define YYLEX           YY_mootClassfreqsParser_LEX(&YY_mootClassfreqsParser_LVAL, &YY_mootClassfreqsParser_LLOC)
#else
#define YYLEX           YY_mootClassfreqsParser_LEX(&YY_mootClassfreqsParser_LVAL)
#endif
#endif
#ifndef YY_USE_CLASS
#if YY_mootClassfreqsParser_DEBUG != 0
int YY_mootClassfreqsParser_DEBUG_FLAG;                    /*  nonzero means print parse trace     */
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
 YY_mootClassfreqsParser_CLASS::
#endif
     YY_mootClassfreqsParser_PARSE(YY_mootClassfreqsParser_PARSE_PARAM)
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
/* parameter definition without protypes */
YY_mootClassfreqsParser_PARSE_PARAM_DEF
#endif
#endif
#endif
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YY_mootClassfreqsParser_STYPE *yyvsp;
  int yyerrstatus;      /*  number of tokens to shift before error messages enabled */
  int yychar1=0;          /*  lookahead token as an internal (translated) token number */

  short yyssa[YYINITDEPTH];     /*  the state stack                     */
  YY_mootClassfreqsParser_STYPE yyvsa[YYINITDEPTH];        /*  the semantic value stack            */

  short *yyss = yyssa;          /*  refer to the stacks thru separate pointers */
  YY_mootClassfreqsParser_STYPE *yyvs = yyvsa;     /*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YY_mootClassfreqsParser_LSP_NEEDED
  YY_mootClassfreqsParser_LTYPE yylsa[YYINITDEPTH];        /*  the location stack                  */
  YY_mootClassfreqsParser_LTYPE *yyls = yylsa;
  YY_mootClassfreqsParser_LTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YY_mootClassfreqsParser_PURE
  int YY_mootClassfreqsParser_CHAR;
  YY_mootClassfreqsParser_STYPE YY_mootClassfreqsParser_LVAL;
  int YY_mootClassfreqsParser_NERRS;
#ifdef YY_mootClassfreqsParser_LSP_NEEDED
  YY_mootClassfreqsParser_LTYPE YY_mootClassfreqsParser_LLOC;
#endif
#endif

  YY_mootClassfreqsParser_STYPE yyval;             /*  the variable used to return         */
				/*  semantic values from the action     */
				/*  routines                            */

  int yylen;

#if YY_mootClassfreqsParser_DEBUG != 0
  if (YY_mootClassfreqsParser_DEBUG_FLAG)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  YY_mootClassfreqsParser_NERRS = 0;
  YY_mootClassfreqsParser_CHAR = YYEMPTY;          /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YY_mootClassfreqsParser_LSP_NEEDED
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
      YY_mootClassfreqsParser_STYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YY_mootClassfreqsParser_LSP_NEEDED
      YY_mootClassfreqsParser_LTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YY_mootClassfreqsParser_LSP_NEEDED
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
#ifdef YY_mootClassfreqsParser_LSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  YY_mootClassfreqsParser_ERROR("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyvs = (YY_mootClassfreqsParser_STYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#ifdef YY_mootClassfreqsParser_LSP_NEEDED
      yyls = (YY_mootClassfreqsParser_LTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YY_mootClassfreqsParser_LSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YY_mootClassfreqsParser_DEBUG != 0
      if (YY_mootClassfreqsParser_DEBUG_FLAG)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YY_mootClassfreqsParser_DEBUG != 0
  if (YY_mootClassfreqsParser_DEBUG_FLAG)
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

  if (YY_mootClassfreqsParser_CHAR == YYEMPTY)
    {
#if YY_mootClassfreqsParser_DEBUG != 0
      if (YY_mootClassfreqsParser_DEBUG_FLAG)
	fprintf(stderr, "Reading a token: ");
#endif
      YY_mootClassfreqsParser_CHAR = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (YY_mootClassfreqsParser_CHAR <= 0)           /* This means end of input. */
    {
      yychar1 = 0;
      YY_mootClassfreqsParser_CHAR = YYEOF;                /* Don't call YYLEX any more */

#if YY_mootClassfreqsParser_DEBUG != 0
      if (YY_mootClassfreqsParser_DEBUG_FLAG)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(YY_mootClassfreqsParser_CHAR);

#if YY_mootClassfreqsParser_DEBUG != 0
      if (YY_mootClassfreqsParser_DEBUG_FLAG)
	{
	  fprintf (stderr, "Next token is %d (%s", YY_mootClassfreqsParser_CHAR, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, YY_mootClassfreqsParser_CHAR, YY_mootClassfreqsParser_LVAL);
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

#if YY_mootClassfreqsParser_DEBUG != 0
  if (YY_mootClassfreqsParser_DEBUG_FLAG)
    fprintf(stderr, "Shifting token %d (%s), ", YY_mootClassfreqsParser_CHAR, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (YY_mootClassfreqsParser_CHAR != YYEOF)
    YY_mootClassfreqsParser_CHAR = YYEMPTY;

  *++yyvsp = YY_mootClassfreqsParser_LVAL;
#ifdef YY_mootClassfreqsParser_LSP_NEEDED
  *++yylsp = YY_mootClassfreqsParser_LLOC;
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

#if YY_mootClassfreqsParser_DEBUG != 0
  if (YY_mootClassfreqsParser_DEBUG_FLAG)
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
#line 154 "mootClassfreqsParser.yy"
{ yyval.count = 0; ;
    break;}
case 2:
#line 155 "mootClassfreqsParser.yy"
{ yyval.count = 0; ;
    break;}
case 3:
#line 156 "mootClassfreqsParser.yy"
{ yyval.count = 0; ;
    break;}
case 4:
#line 160 "mootClassfreqsParser.yy"
{
                  lclass.clear();
                  yyval.count = 0;
		;
    break;}
case 5:
#line 166 "mootClassfreqsParser.yy"
{ yyval.count = yyvsp[0].count; ;
    break;}
case 6:
#line 167 "mootClassfreqsParser.yy"
{ yyval.count = yyvsp[-2].count; /* total is optional (not really) */ ;
    break;}
case 7:
#line 169 "mootClassfreqsParser.yy"
{
		  cfreqs->add_count(lclass, *yyvsp[-2].tagstr, yyvsp[0].count);
		  delete yyvsp[-2].tagstr;
                  yyval.count = yyvsp[-4].count;
		;
    break;}
case 8:
#line 176 "mootClassfreqsParser.yy"
{ yyval.count = 0; ;
    break;}
case 9:
#line 178 "mootClassfreqsParser.yy"
{
		  lclass.insert(*yyvsp[0].tagstr);
                  delete yyvsp[0].tagstr;
                  yyval.count = 0;
		;
    break;}
case 10:
#line 184 "mootClassfreqsParser.yy"
{
		  lclass.insert((const char *)yylloc.text);
		  yyval.count = 0;
		;
    break;}
case 11:
#line 191 "mootClassfreqsParser.yy"
{
		  yyval.tagstr = yyvsp[0].tagstr;
		;
    break;}
case 12:
#line 196 "mootClassfreqsParser.yy"
{ yyval.count = yyvsp[0].count; ;
    break;}
case 13:
#line 198 "mootClassfreqsParser.yy"
{
		    yyerror("expected a COUNT.");
                    YYABORT;
		;
    break;}
case 14:
#line 204 "mootClassfreqsParser.yy"
{ yyval.count = yyvsp[0].count; ;
    break;}
case 15:
#line 207 "mootClassfreqsParser.yy"
{ yyval.count=0; ;
    break;}
case 16:
#line 209 "mootClassfreqsParser.yy"
{
		    yyerror("expected a TAB.");
                    YYABORT;
                ;
    break;}
case 17:
#line 215 "mootClassfreqsParser.yy"
{ yyval.count=0; ;
    break;}
case 19:
#line 218 "mootClassfreqsParser.yy"
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
#ifdef YY_mootClassfreqsParser_LSP_NEEDED
  yylsp -= yylen;
#endif

#if YY_mootClassfreqsParser_DEBUG != 0
  if (YY_mootClassfreqsParser_DEBUG_FLAG)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YY_mootClassfreqsParser_LSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = YY_mootClassfreqsParser_LLOC.first_line;
      yylsp->first_column = YY_mootClassfreqsParser_LLOC.first_column;
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
      ++YY_mootClassfreqsParser_NERRS;

#ifdef YY_mootClassfreqsParser_ERROR_VERBOSE
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
	      YY_mootClassfreqsParser_ERROR(msg);
	      free(msg);
	    }
	  else
	    YY_mootClassfreqsParser_ERROR ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YY_mootClassfreqsParser_ERROR_VERBOSE */
	YY_mootClassfreqsParser_ERROR("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (YY_mootClassfreqsParser_CHAR == YYEOF)
	YYABORT;

#if YY_mootClassfreqsParser_DEBUG != 0
      if (YY_mootClassfreqsParser_DEBUG_FLAG)
	fprintf(stderr, "Discarding token %d (%s).\n", YY_mootClassfreqsParser_CHAR, yytname[yychar1]);
#endif

      YY_mootClassfreqsParser_CHAR = YYEMPTY;
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
#ifdef YY_mootClassfreqsParser_LSP_NEEDED
  yylsp--;
#endif

#if YY_mootClassfreqsParser_DEBUG != 0
  if (YY_mootClassfreqsParser_DEBUG_FLAG)
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

#if YY_mootClassfreqsParser_DEBUG != 0
  if (YY_mootClassfreqsParser_DEBUG_FLAG)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = YY_mootClassfreqsParser_LVAL;
#ifdef YY_mootClassfreqsParser_LSP_NEEDED
  *++yylsp = YY_mootClassfreqsParser_LLOC;
#endif

  yystate = yyn;
  goto yynewstate;
}

/* END */

/* #line 909 "/usr/local/share/flex++bison++/bison.cc" */
#line 224 "mootClassfreqsParser.yy"

/* -------------- body section -------------- */

/*----------------------------------------------------------------
 * Error Methods
 *----------------------------------------------------------------*/

void mootClassfreqsParser::yyerror(char *msg)
{
    yycarp("mootClassfreqsParser: Error: %s\n", msg);
}

void mootClassfreqsParser::yycarp(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, " at line %d, column %d, near '%s'\n",
	  yylloc.last_line, yylloc.last_column, yylloc.text);
}
