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
 * Name: mootNgramsLexer.ll
 * Author: Bryan Jurish
 * Description:
 *   + Lexer for TnT-style n-gram frequency parameter files
 *   + process with Alain Coetmeur's 'flex++' to produce a C++ lexer
 *----------------------------------------------------------------------*/

/* --- Lexer name --- */
%name mootNgramsLexer

%header{

/*============================================================================
 * Doxygen docs
 *============================================================================*/
/**
 * \class mootNgramsLexer
 * \brief Flex++ lexer for (TnT-style) moot n-gram parameter files. 
 *
 * Supports comments introduced with '%%'.
 */

#include <mootTypes.h>
#include <mootNgramsParser.h>
#include <mootGenericLexer.h>

using namespace moot;
%}

%define LEX_PARAM \
  YY_mootNgramsParser_STYPE *yylval, YY_mootNgramsParser_LTYPE *yylloc

%define CLASS mootNgramsLexer

%define INHERIT \
  : public GenericLexer

%define INPUT_CODE \
  return moot::GenericLexer::yyinput(buffer,result,max_size);

%define MEMBERS \
  public: \
    /** virtual destructor to shut up gcc */\
    virtual ~mootNgramsLexer(void) {}; \
  /*----- moot::GenericLexer helpers -----*/ \
  virtual void **mgl_yy_current_buffer_p(void) \
                 {return (void**)(&yy_current_buffer);};\
  virtual void  *mgl_yy_create_buffer(int size, FILE *unused=stdin) \
                 {return (void*)(yy_create_buffer(unused,size));};\
  virtual void   mgl_yy_init_buffer(void *buf, FILE *unused=stdin) \
                 {yy_init_buffer((YY_BUFFER_STATE)buf,unused);};\
  virtual void   mgl_yy_delete_buffer(void *buf) \
                 {yy_delete_buffer((YY_BUFFER_STATE)buf);};\
  virtual void   mgl_yy_switch_to_buffer(void *buf) \
                 {yy_switch_to_buffer((YY_BUFFER_STATE)buf);};\
  virtual void   mgl_begin(int stateno);


%define CONSTRUCTOR_INIT :\
  GenericLexer("mootNgramsLexer")


/*----------------------------------------------------------------------
 * Definitions & other lexer properties
 *----------------------------------------------------------------------*/
newline    [\n\r]
nonnewline [^\n\r]
whitespace [ ]
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
  return mootNgramsParser::COUNT;
}

{textchar}({textorsp}*{textchar})? {
  // -- any other text: append to the token-buffer
  theColumn += yyleng;
  yylval->tagstr = new moot::mootTagString((const char *)yytext);
  return mootNgramsParser::TAG;
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
  fprintf(stderr,"mootNgramsLexer warning: unknown character '%c': ignored.\n", yytext[YY_MORE_ADJ]);
}

%%

// -- co-exist with BumbleBee 'clex.h'
#ifdef REJECT
# undef REJECT
#endif



/*----------------------------------------------------------------------
 * Local Methods for mootNgramsLexer
 *----------------------------------------------------------------------*/

void mootNgramsLexer::mgl_begin(int stateno) {BEGIN(stateno);}
