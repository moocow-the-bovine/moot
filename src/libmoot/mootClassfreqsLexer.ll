/*-*- Mode: Flex++ -*-*/

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2017 by Bryan Jurish <moocow@cpan.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
/*----------------------------------------------------------------------
 * Name: mootClassfreqsLexer.ll
 * Author: Bryan Jurish
 * Description:
 *   + Lexer for lexical-class frequency parameter files
 *   + process with Alain Coetmeur's 'flex++' to produce a C++ lexer
 *----------------------------------------------------------------------*/

/* --- Lexer name --- */
%name mootClassfreqsLexer

%header{

/*============================================================================
 * Doxygen docs
 *============================================================================*/
/**
\class mootClassfreqsLexer
\brief \c flex++ lexer for (TnT-style) moot lexical-class-frequency parameter files
\details Supports comments introduced with '%%'.

\file mootClassfreqsLexer.h
\brief \c flex++ lexical-class-frequency parameter file lexer: autogenerated header

\file mootClassfreqsLexer.ll
\brief \c flex++ lexical-class-frequency parameter file lexer: sources
*/

#include <mootClassfreqsParser.h>
#include <mootGenericLexer.h>

using namespace moot;
%}

%define LEX_PARAM \
  YY_mootClassfreqsParser_STYPE *yylval, YY_mootClassfreqsParser_LTYPE *yylloc

%define CLASS mootClassfreqsLexer

%define INHERIT \
  : public GenericLexer

%define INPUT_CODE \
  return moot::GenericLexer::yyinput(buffer,result,max_size);

%define MEMBERS \
  public: \
    /** virtual destructor to shut up gcc */\
    virtual ~mootClassfreqsLexer(void) {};\
  /*----- BEGIN moot::GenericLexer helpers -----*/ \
  virtual void  *mgl_yy_current_buffer_p(void) \
                 {return reinterpret_cast<void*>(&yy_current_buffer);};\
  virtual void  *mgl_yy_create_buffer(int size, FILE *unused=stdin) \
                 {return reinterpret_cast<void*>(yy_create_buffer(unused,size));};\
  virtual void   mgl_yy_init_buffer(void *buf, FILE *unused=stdin) \
                 {yy_init_buffer(reinterpret_cast<YY_BUFFER_STATE>(buf),unused);};\
  virtual void   mgl_yy_delete_buffer(void *buf) \
                 {yy_delete_buffer(reinterpret_cast<YY_BUFFER_STATE>(buf));};\
  virtual void   mgl_yy_switch_to_buffer(void *buf) \
                 {yy_switch_to_buffer(reinterpret_cast<YY_BUFFER_STATE>(buf));};\
  virtual void   mgl_begin(int stateno); \
  /*----- END moot::GenericLexer helpers -----*/

%define CONSTRUCTOR_INIT :\
  GenericLexer("mootClassfreqsLexer")


/*----------------------------------------------------------------------
 * Definitions & other lexer properties
 *----------------------------------------------------------------------*/
newline    [\n\r]
nonnewline [^\n\r]
space      [ ]
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
  theColumn += (static_cast<int>(theColumn/8)+1)*8 + (yyleng ? yyleng-1 : 0);
  return '\t';
}

[-+]?[0-9]*[.]?[0-9]+([eE][-+]?[0-9]+)? {
  // -- count (float) : return it
  //theLine++; theColumn = 0;
  theColumn += yyleng;
  yylval->count = strtod(reinterpret_cast<const char *>(yytext),NULL);
  return mootClassfreqsParser::COUNT;
}

{textchar}+ {
  // -- text characters: just return
  theColumn += yyleng;
  yylval->tagstr = new moot::mootTagString(reinterpret_cast<const char *>(yytext));
  return mootClassfreqsParser::TAG;
}

{newline} {
  // -- newlines : return 'em
  theLine++; theColumn = 0;
  return '\n';
}

{space} {
  //-- ignore spaces
  theColumn += yyleng;
}

<<EOF>> {
  return 0;
}

. {
  // -- huh? -- just ignore it!
  theColumn += yyleng;
  fprintf(stderr,"mootClassfreqsLexer warning: unrecognized character '%c' (ignored) at line %zd col %zd, near `%s'.\n",
          yytext[YY_MORE_ADJ], theLine, theColumn, yytext);
}

%%

// -- co-exist with BumbleBee 'clex.h'
#ifdef REJECT
# undef REJECT
#endif


/*----------------------------------------------------------------------
 * Local Methods for mootClassfreqsLexer
 *----------------------------------------------------------------------*/
void mootClassfreqsLexer::mgl_begin(int stateno) {BEGIN(stateno);}
