/*-*- Mode: Flex++ -*-*/

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2004 by Bryan Jurish <moocow@ling.uni-potsdam.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

/*----------------------------------------------------------------------
 * Name: mootTokenLexer.ll
 * Author: Bryan Jurish
 * Description:
 *   + lexer for moocow's tagger
 *   + assumes pre-tokenized input
 *     - one token per line
 *     - n>=0 (possible) tags per token
 *     - blank lines mark end-of-sentence
 *     - supports line-comments introduced by '%%'
 *     - raw text (no markup!)
 *     - token-line format (TAB-separated)
 *        TOKEN_TEXT  ANALYSIS_1 ... ANALYSIS_N
 *     - analysis format:
 *        TAG[<COST>][:DETAILS[<COST>]]
 *   + process with Coetmeur's flex++ to produce 'mootTokenLexer.cc'
 *----------------------------------------------------------------------*/

/* --- Lexer name --- */
%name mootTokenLexer

%header{

#include <stdarg.h>
#include "mootToken.h"
/*#include "mootTypes.h"*/


/*============================================================================
 * Doxygen docs
 *============================================================================*/
/**
 * \class mootTokenLexer
 * \brief Flex++ lexer for KDWDS tagger.
 *
 * Assumes pre-tokenized input:
 * one token per line,  blank lines = EOS, raw text only (no markup!).
 * Format: 1 tok/line, comments introduced with '%%'.  Supports
 * multiple tags/tok.
 *
 * token-line format (TAB-separated)
 *
 * TOKEN_TEXT "\t" ANALYSIS_1 "\t" ... "\t" ANALYSIS_N "\n"
 *
 * analysis format:
 *
 *  TAG ":" ANALYSIS_DETAILS
 */

%}

/*%define LEX_PARAM \
  YY_mootTokenParser_STYPE *yylval, YY_mootTokenParser_LTYPE *yylloc
*/

%define CLASS mootTokenLexer
%define MEMBERS \
  public: \
    /* -- public typedefs */\
    /** typedef for token-types */\
    typedef enum {\
      /* Output token-types */\
      TLUNKNOWN,   /**< unknown token */\
      TLEOF,       /**< end-of-file */\
      TLEOS,       /**< end-of-sentence */\
      TLTOKEN,     /**< token (+ analyses) */\
      /*-- internal use only */\
      TLTEXT,      /**< token text (internal use only) */\
      TLTAB,       /**< tabs (internal use only) */\
      TLTAG,       /**< analysis tags (internal use only) */\
      TLDETAILS,   /**< analysis details (internal use only) */\
      TLCOST,      /**< analysis costs (internal use only) */\
      TLNEWLINE,   /**< newlines (internal use only) */\
      TLIGNORE,    /**< ignored (internal use only) */\
      TLNTYPES \
    } TokenType;\
    \
  public: \
   /* -- positional parameters */ \
   /** current line*/\
   int theLine;\
   /** current column*/\
   int theColumn;\
   \
   /* -- pre-allocated construction buffers */ \
   /** current token */\
   moot::mootToken mtoken; \
   /** current analysis */ \
   moot::mootToken::Analysis manalysis;\
   /** last token type */ \
   TokenType lasttyp; \
   \
   /* -- token-buffering */\
   /** token-buffer */\
   std::string itokbuf;\
   /** whether to clear the token-buffer on 'itokbuf_append()' */\
   bool itokbuf_clear;\
   \
   /* -- diagnositcs */\
   /** Name of our input source: used for diagnostics & error messages */\
   char *srcname;\
  private: \
    /* private local data */ \
    bool use_string; \
    char *stringbuf; \
  public: \
    /* -- local methods */ \
    /** virtual destructor to shut up gcc */\
    virtual ~mootTokenLexer(void) {}; \
    /** reset to initial state */ \
    void reset(void); \
    /** hack for non-global yywrap() */\
    void select_streams(FILE *in=stdin, FILE *out=stdout); \
    /** use string input */\
    void select_string(const char *in, FILE *out=stdout); \
    /** for token-buffering: append yyleng characters of yytext to 'itokbuf' */\
    inline void itokbuf_append(char *text, int leng); \
    /** for error reporting */ \
    virtual void yycarp(char *fmt, ...);

%define CONSTRUCTOR_INIT :\
  theLine(1), \
  theColumn(0), \
  lasttyp(TLEOS), \
  itokbuf_clear(true), \
  srcname("(unknown)"),\
  use_string(false), \
  stringbuf(NULL)

%define INPUT_CODE \
  /* yy_input(char *buf, int &result, int max_size) */\
  if (use_string) {\
    size_t len = strlen(stringbuf) > (size_t)max_size \
      ? max_size \
      : strlen(stringbuf);\
    strncpy(buffer,stringbuf,len);\
    stringbuf += len;\
    return result = len;\
  }\
  /* black magic */\
  return result= fread(buffer, 1, max_size, YY_mootTokenLexer_IN);

/*----------------------------------------------------------------------
 * Start States
 *----------------------------------------------------------------------*/
%x TAG
%x DETAILS
%s TOKEN

/*----------------------------------------------------------------------
 * Definitions
 *----------------------------------------------------------------------*/
space      [ ]
tab        [\t]
newline    [\n\r]
tagchar    [^\t\n\r\<\>\:\=]
tokchar    [^\t\n\r]
detchar    [^\t\n\r]

/*----------------------------------------------------------------------
 * Rules
 *----------------------------------------------------------------------*/
%%
%{
  BEGIN(TOKEN);
%}

^([ \t]*){newline} {
  /** blank line: maybe return eos (ignore empty sentences) */
  theLine++; theColumn=0;
  if (lasttyp != TLEOS) {
    lasttyp=TLEOS;
    return TLEOS;
  }
}

^([ \t]*)"%%"([^\r\n]*){newline} {
  /* mostly ignore comments */
  theLine++;
  theColumn = 0;
  lasttyp = TLIGNORE;
}

^{tokchar}* {
  /* TOKEN: token text at beginning of string */
  theColumn += yyleng;
  mtoken.clear();
  mtoken.toktext = (const char *)yytext;
  lasttyp = TLTEXT;
  BEGIN(TAG);
}

<TAG>{tab}+ {
  //-- TAB: add & clear current analysis, if any
  theColumn = (((int)theColumn/8)+1)*8;
  if (lasttyp != TLTEXT) {
    mtoken.analyses.insert(manalysis);
    manalysis.clear();
  }
  lasttyp = TLTAB;
}

<DETAILS>{tab}+ {
  //-- TAB: add & clear current analysis, if any
  theColumn = (((int)theColumn/8)+1)*8;
  if (lasttyp != TLTEXT) {
    mtoken.analyses.insert(manalysis);
    manalysis.clear();
  }
  lasttyp = TLTAB;
  BEGIN(TAG);
}

<TAG>"/"({tagchar}*) {
   //-- TAG: best tag
   theColumn += yyleng;
   manalysis.tag.append((const char *)yytext+1);
   mtoken.besttag.append((const char *)yytext+1);
   lasttyp = TLTAG;
}

<TAG>{tagchar}* {
  //--  TAG: set in analysis
  theColumn += yyleng;
  manalysis.tag.append((const char *)yytext);
  lasttyp = TLTAG;
}

<TAG>(" "*)":"(" "*) {
  //-- COLON: switch to detail-mode
  theColumn += yyleng;
  lasttyp = TLDETAILS;
  BEGIN(DETAILS);
}

<TAG>(" "*)/{newline} { BEGIN(DETAILS); }

<DETAILS>(" "*){newline} {
  //-- NEWLINE: add & clear current analysis, if any */
  theLine++; theColumn = 0;
  //-- add & clear current analysis, if any
  if (lasttyp != TLTEXT) {
    mtoken.analyses.insert(manalysis);
    manalysis.clear();
  }
  //-- reset to initial state
  BEGIN(TOKEN);
  //-- return token flag (actual data is in 'mtoken' member)
  lasttyp = TLTOKEN;
  return TLTOKEN;
}

<TAG>"<"[0-9]*(\.?)([0-9]+)">"(" "*) {
  //-- COST: add cost to current analysis
  theColumn += yyleng;
  moot::mootToken::Cost cost;
  sscanf((const char *)yytext+1, "%f", &cost);
  manalysis.cost += cost;
  lasttyp = TLCOST;
}

<DETAILS>{detchar}+ {
  /* analysis strings */
  theColumn += yyleng;
  manalysis.details.append((const char *)yytext);
  lasttyp = TLDETAILS;
}

{space}+ {
  /* mostly ignore spaces */
  theColumn += yyleng;
  lasttyp = TLIGNORE;
}

. {
  theColumn += yyleng;
  yycarp("Unrecognized character '%c'", *yytext);
}

<TAG>. {
  theColumn += yyleng;
  yycarp("Unrecognized character '%c'", *yytext);
}

<DETAILS>. {
  theColumn += yyleng;
  yycarp("Unrecognized character '%c'", *yytext);
}

<<EOF>> {
  switch (lasttyp) {
   case TLNEWLINE:
     lasttyp = TLEOS;
     break;
   case TLEOS:
   case TLEOF:
     lasttyp = TLEOF;
     break;
   default:
     lasttyp = TLEOS;
  }
  return lasttyp;
}

%%


/*----------------------------------------------------------------------
 * Local Methods for mootTokenLexer
 *----------------------------------------------------------------------*/

void mootTokenLexer::reset(void) { BEGIN(TOKEN); }


/*-----------------------------------------------------------------------
 * mootTokenLexer::select_streams(FILE *in, FILE *out)
 *   + hack for non-global yywrap()
 */
void mootTokenLexer::select_streams(FILE *in, FILE *out)
{
  yyin = in;
  yyout = out;
  use_string = false;

  // -- black magic from flex(1) manpage
  if (yy_current_buffer != NULL) { yy_delete_buffer(yy_current_buffer); }
  yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
  BEGIN(TOKEN);
}

/*
 * void mootTokenLexer::select_string(const char *in, FILE *out=stdout)
 */
void mootTokenLexer::select_string(const char *in, FILE *out) {
  select_streams(stdin,out);  // flex __really__ wants a real input stream

  // -- string-buffer stuff
  use_string = true;
  stringbuf = (char *)in;
}

/*
 * itokbuf_append(text,leng)
 */
inline void mootTokenLexer::itokbuf_append(char *text, int leng) {
  if (itokbuf_clear) {
    itokbuf = (char *)text;
    itokbuf_clear = false;
  } else {
    itokbuf.append((char *)text,leng);
  }
}

void mootTokenLexer::yycarp(char *fmt, ...)
{
    fprintf(stderr, "mootTokenLexer: ");
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, " in %s %s at line %d, column %d, near `%s'\n",
            (use_string ? "string" : "file"),
            (srcname ? srcname : "(null)"),
            theLine, theColumn, yytext);
}
