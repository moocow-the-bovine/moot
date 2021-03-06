/*-*- Mode: Flex++ -*-*/

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2009 by Bryan Jurish <moocow@cpan.org>

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
 * Name: mootPPLexer.ll
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + preprocessor for the moot tagger
 *   + process with Coetmeur's flex++ to produce 'moot_lexer.cc'
 *----------------------------------------------------------------------*/

/* --- Lexer name --- */
%name mootPPLexer

%header{
/*============================================================================
 * Doxygen docs
 *============================================================================*/
/**
\class mootPPLexer
\brief \c flex++ lexer for 'mootpp' rudimentary raw-text preprocessor
\details
 Does rudimentary end-of-sentence and abbreviation recognition,
 and filters out (most) XML markup.

\file mootPPLexer.h
\brief \c flex++ raw-text lexer: autogenerated header

\file mootPPLexer.ll
\brief \c flex++ raw-text lexer: sources
*/

#include <mootGenericLexer.h>
using namespace moot;
%}

%define CLASS mootPPLexer

%define INHERIT \
  : public GenericLexer

%define INPUT_CODE \
  return moot::GenericLexer::yyinput(buffer,result,max_size);

%define MEMBERS \
  public: \
  /* -- public typedefs */\
  /** typedef for token-types */\
  typedef enum { \
    PPEOF, \
    UNKNOWN, \
    EOS, \
    XML_START_TAG, \
    XML_END_TAG, \
    XML_ENTITY, \
    WORD, \
    INTEGER, \
    FLOAT, \
    PUNCT, \
  } TokenType; \
  \
  public: \
    /* -- public local data */ \
    /** enable verbose reporting (track ntokens)?  */\
    bool verbose; \
    /** number of tokens processed (for verbose mode) */\
    unsigned int ntokens; \
    /** output sentence separator */ \
    std::string output_sentence_separator; \
    /** output token separator */ \
    std::string output_token_separator; \
    \
    /** make g++ happy */\
    virtual ~mootPPLexer(void) {}; \
    \
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

%define CONSTRUCTOR_INIT : \
  GenericLexer("mootPPLexer"), \
  ntokens(0), \
  output_sentence_separator("\n\n"), \
  output_token_separator("\n")

/*----------------------------------------------------------------------
 * Definitions
 *----------------------------------------------------------------------*/
digit	       [0-9]
latin1_uc      [A-Z?-?]
latin1_lc      [a-z?-?]
latin1_punct   [!-\/:-\@\[-\`\{-\~?-?]
latin1_ws      [ ?\t\n\r]

space          [ ?\t]
hyphen	       "-"
eos_punct      [\.\!\?]

latin1_letter  [A-Z?-?a-z?-?]
latin1_word    ([A-Z?-?a-z?-?]+)

/*----------------------------------------------------------------------
 * Rules
 *----------------------------------------------------------------------*/
%%

%{
/*----------------------------------------------------------------------
 * XML markup removal 
 */
//int xml_c_tmp;
%}

\<(\/?)(eos)\> {
  add_columns(yyleng);
  return EOS;
}
\<([\?\!]?){latin1_letter}+[^\>]*\> {
  add_columns(yyleng);
  return XML_START_TAG;
}
\<\/{latin1_letter}+([0-9\.\_\-]*)\> {
  add_columns(yyleng);
  return XML_END_TAG;
}

"&#"[0-9]+";" {
  add_columns(yyleng);
  //-- character entity: translate (weird!)
  /*
  theColumn += yyleng;
  sscanf(yytext+2, "%d", &xml_c_tmp);
  yytext[0] = (unsigned char)xml_c_tmp;
  yytext[1] = '\0';
  */
  return XML_ENTITY;
}
"&dash;" { add_columns(yyleng); return XML_ENTITY; }
"&quot;" { add_columns(yyleng); return XML_ENTITY; }
"&lt;"   { add_columns(yyleng); return XML_ENTITY; }
"&gt;"   { add_columns(yyleng); return XML_ENTITY; }

[\+\-]?([0-9]+)						{ add_columns(yyleng); return INTEGER; }
[\+\-]?[0-9]*[\.,]([0-9]+)				{ add_columns(yyleng); return FLOAT; }

{latin1_word}-{latin1_word}-{latin1_word}		{ add_columns(yyleng); return WORD; }
{latin1_word}-{latin1_word}				{ add_columns(yyleng); return WORD; }
{latin1_word}						{ add_columns(yyleng); return WORD; }

{eos_punct}/({space}+)					{ add_columns(1); return EOS; }
{eos_punct}/([\n\r])                                    { add_columns(1); return EOS; }
{latin1_punct}+                                         { add_columns(yyleng); return PUNCT; }

{space}+						{ add_columns(yyleng); /* do nothing */ }
\r                                                      { add_columns(1); /* do nothing; */ }
\n							{ add_lines(1); /* do nothing; */ }
.							{ add_columns(1); return UNKNOWN; }

<<EOF>>                                                 { return PPEOF; }

%%
/*----------------------------------------------------------------------
 * moot::GenericLexer requirements
 */
void mootPPLexer::mgl_begin(int stateno) { BEGIN(stateno); }
