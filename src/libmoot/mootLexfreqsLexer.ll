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
 * Name: mootLexfreqsLexer.ll
 * Author: Bryan Jurish
 * Description:
 *   + Lexer for TnT-style lexical-frequency parameter files
 *   + process with Alain Coetmeur's 'flex++' to produce a C++ lexer
 *----------------------------------------------------------------------*/

/* --- Lexer name --- */
%name mootLexfreqsLexer

%header{

#include "mootTypes.h"
#include "mootLexfreqsParser.h"

/*============================================================================
 * Doxygen docs
 *============================================================================*/
/**
\class mootLexfreqsLexer
\brief Flex++ lexer for (TnT-style) moot lexical frequency parameter files. 

\details Supports comments introduced with '%%'.
*/
%}

%define LEX_PARAM \
  YY_mootLexfreqsParser_STYPE *yylval, YY_mootLexfreqsParser_LTYPE *yylloc


%define CLASS mootLexfreqsLexer
%define MEMBERS \
  public: \
    /* -- public typedefs */\
  public: \
   /* -- positional parameters */\
    /** current line*/\
    int theLine;\
    /** current column*/\
    int theColumn;\
    /** token-buffering */\
    moot::mootTagString tokbuf;\
    /** whether to clear the token-buffer on 'tokbuf_append()' */\
    bool tokbuf_clear;\
  private: \
    /* private local data */ \
    bool use_string; \
    char *stringbuf; \
  public: \
    /** virtual destructor to shut up gcc */\
    virtual ~mootLexfreqsLexer(void) {};\
    /** use stream input */\
    void select_streams(FILE *in=stdin, FILE *out=stdout); \
    /** use string input */\
    void select_string(const char *in, FILE *out=stdout); \
    /** for token-buffering: append yyleng characters of yytext to 'tokbuf' */\
    inline void tokbuf_append(char *text, int leng);

%define CONSTRUCTOR_INIT :\
  theLine(1), \
  theColumn(1), \
  use_string(false), \
  stringbuf(NULL)

%define INPUT_CODE \
  /* yy_input(char *buf, int &result, int max_size) */\
  if (use_string) {\
    size_t len = strlen(stringbuf) > (size_t)max_size ? max_size : strlen(stringbuf);\
    strncpy(buffer,stringbuf,len);\
    stringbuf += len;\
    return result = len;\
  }\
  /* black magic */\
  return result= fread(buffer, 1, max_size, YY_mootLexfreqsLexer_IN);



/*----------------------------------------------------------------------
 * Definitions & other lexer properties
 *----------------------------------------------------------------------*/
newline    [\n\r]
nonnewline [^\n\r]
whitespace [ \t]
textchar   [^ \n\r\t]
textorsp   [^\n\r\t]

/*----------------------------------------------------------------------
 * Rules
 *----------------------------------------------------------------------*/
%%

^(" "+) {
  // -- ignore leading whitespace
  theColumn += yyleng;
}

"%%"({nonnewline}*) {
   // -- ignore comments
   theColumn += yyleng;
}

({whitespace}*)"\t"({whitespace}*) {
  // -- tab: return the current token-buffer
  //theColumn += yyleng+7; // -- interpret tab as 8 spaces
  theColumn += yyleng; // -- interpret tab as 8 spaces
  return '\t';
}

([\-\+]?)([0-9]*)(\.?)([0-9]+) {
  // -- count : return it
  //theLine++; theColumn = 0;
  theColumn += yyleng;
  yylval->count = atof((const char *)yytext);
  return mootLexfreqsParser::COUNT;
}

{textchar}({textorsp}*{textchar})? {
  // -- any other text: append to the token-buffer
  theColumn += yyleng;
  yylval->tokstr = new moot::mootTokString((const char *)yytext);
  return mootLexfreqsParser::TOKEN;
}

{newline} {
  // -- newlines : ignore
  theLine++; theColumn = 0;
  return '\n';
}

<<EOF>> {
  return 0;
}

. {
  // -- huh? -- just ignore it!
  theColumn += yyleng;
  fprintf(stderr,"mootLexfreqsLexer warning: unknown character '%c': ignored.\n", yytext[YY_MORE_ADJ]);
}

%%

// -- co-exist with BumbleBee 'clex.h'
#ifdef REJECT
# undef REJECT
#endif



/*----------------------------------------------------------------------
 * Local Methods for mootLexfreqsLexer
 *----------------------------------------------------------------------*/

/*
 * void mootLexfreqsLexer::select_streams(FILE *in, FILE *out)
 */
void mootLexfreqsLexer::select_streams(FILE *in, FILE *out) {
  yyin = in;
  yyout = out;
  use_string = false;

  // -- black magic from flex(1) manpage
  if (yy_current_buffer != NULL) { yy_delete_buffer(yy_current_buffer); }
  yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
  BEGIN(INITIAL);
}

/*
 * void mootLexfreqsLexer::select_string(const char *in, FILE *out=stdout)
 */
void mootLexfreqsLexer::select_string(const char *in, FILE *out) {
  select_streams(stdin,out);  // flex __really__ wants a real input stream

  // -- string-buffer stuff
  use_string = true;
  stringbuf = (char *)in;
}


/*
 * tokbuf_append(text,leng)
 */
inline void mootLexfreqsLexer::tokbuf_append(char *text, int leng) {
  if (tokbuf_clear) {
    tokbuf = (char *)text;
    tokbuf_clear = false;
  } else {
    tokbuf.append((char *)text,leng);
  }
}
