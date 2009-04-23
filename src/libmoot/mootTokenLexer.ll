/*-*- Mode: Flex++ -*-*/

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2009 by Bryan Jurish <moocow@ling.uni-potsdam.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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
 *     - analysis format(s):
 *        ...(COST?)... "[" TAG ["]"|" "] ...(COST?)...
 *     - COST format(s):
 *       "<" COST_FLOAT ">"
 *
 *   + process with Coetmeur's flex++ to produce 'mootTokenLexer.cc'
 *----------------------------------------------------------------------*/

/* --- Lexer name --- */
%name mootTokenLexer

%header{
/*============================================================================
 * Doxygen docs
 *============================================================================*/
/**
 * \class mootTokenLexer
 * \brief Flex++ lexer for moot PoS tagger native text input.
 *
 * Assumes pre-tokenized input:
 * one token per line,  blank lines = EOS, raw text only (no markup!).
 * Format: 1 tok/line, comments introduced with '%%'.  Supports
 * multiple tags/tok.
 *
 * token-line format (TAB-separated):
 *
 * TOKEN_TEXT  ANALYSIS_1 ... ANALYSIS_N
 *
 * Analysis format(s):
 *
 * ...(COST?)... "[" TAG ["]"|" "] ...(COST?)...
 *
 * COST format(s):
 *
 * "<" COST_FLOAT ">"
 *
 */

#include <mootToken.h>
#include <mootGenericLexer.h>

using namespace moot;
%}

/*%define FLEX_DEBUG*/

%define CLASS mootTokenLexer

%define INHERIT \
  : public moot::GenericLexer

%define INPUT_CODE \
  return moot::GenericLexer::yyinput(buffer,result,max_size);

%define MEMBERS \
  public: \
  /* -- public typedefs */\
  typedef moot::mootTokenType TokenType; \
  /* extra token types */ \
  static const int LexTypeText = moot::NTokTypes+1;    /* literal token text */ \
  static const int LexTypeTag = moot::NTokTypes+2;     /* analysis tag */ \
  static const int LexTypeDetails = moot::NTokTypes+3; /* analysis details */ \
  static const int LexTypeEOA = moot::NTokTypes+4;     /* end-of-analysis (separator) */ \
  static const int LexTypeEOT = moot::NTokTypes+5;     /* end-of-token */ \
  static const int LexTypeIgnore = moot::NTokTypes+6;  /* ignored data (unused) */ \
  \
  public: \
   /** last token type */ \
   int lasttyp; \
   \
   /* -- pre-allocated construction buffers */ \
   /* current token (default) */ \
   moot::mootToken mtoken_default; \
   /* current token (real) */ \
   moot::mootToken *mtoken; \
   \
   /** current analysis (real) */ \
   moot::mootToken::Analysis *manalysis;\
   \
   /** whether to ignore comments (default=false) */ \
   bool ignore_comments; \
   /** whether first analysis parsed should be considered 'best' (default=true) */ \
   bool first_analysis_is_best; \
   /** whether we're parsing a 'best' analysis */\
   bool current_analysis_is_best; \
   /** whether to (otherwise) ignore first analysis (default=false) */ \
   bool ignore_first_analysis; \
   /** whether to ignore current analysis */\
   bool ignore_current_analysis; \
   /** whether first non-tag analysis should be considered 'location' (default=false) */ \
   bool first_nontag_is_location; \
   /** whether we're parsing a 'location' analysis */\
   bool current_analysis_is_location; \
   \
  public: \
    /* -- local methods */ \
    /** virtual destructor to shut up gcc */\
    virtual ~mootTokenLexer(void) {};\
    /** reset to initial state */ \
    virtual void reset(void); \
    /** actions to perform on end-of-analysis */ \
    inline void on_EOA(void) \
    { \
      /*-- EOA: add & clear current analysis, if any */ \
      /*-- add & clear current analysis, if any */ \
      if (lasttyp != LexTypeEOA) { \
        /*-- set default tag */\
        if (manalysis->tag.empty()) { \
          manalysis->tag.swap(manalysis->details); \
        }  \
        /* set best tag if applicable */\
        if (current_analysis_is_best) { \
          mtoken->besttag(manalysis->tag); \
          current_analysis_is_best = false; \
        } \
        if (ignore_current_analysis) { \
          ignore_current_analysis=false; \
          mtoken->tok_analyses.pop_back(); \
        } \
      } \
    }; \
  /*-- moot::GenericLexer helpers */ \
  virtual void  *mgl_yy_current_buffer_p(void) \
                 {return reinterpret_cast<void*>(&yy_current_buffer);}; \
  virtual void  *mgl_yy_create_buffer(int size, FILE *unused=stdin) \
                 {return reinterpret_cast<void*>(yy_create_buffer(unused,size));};\
  virtual void   mgl_yy_init_buffer(void *buf, FILE *unused=stdin) \
                 {yy_init_buffer(reinterpret_cast<YY_BUFFER_STATE>(buf),unused);};\
  virtual void   mgl_yy_delete_buffer(void *buf) \
                 {yy_delete_buffer(reinterpret_cast<YY_BUFFER_STATE>(buf));};\
  virtual void   mgl_yy_switch_to_buffer(void *buf) \
                 {yy_switch_to_buffer(reinterpret_cast<YY_BUFFER_STATE>(buf));};\
  virtual void   mgl_begin(int stateno);


%define CONSTRUCTOR_INIT :\
  GenericLexer("mootTokenLexer"), \
  yyin(NULL), \
  lasttyp(moot::TokTypeEOS), \
  manalysis(NULL), \
  ignore_comments(false), \
  first_analysis_is_best(true), \
  current_analysis_is_best(false), \
  ignore_first_analysis(false), \
  ignore_current_analysis(false), \


%define CONSTRUCTOR_CODE \
  mtoken = &mtoken_default;


/*----------------------------------------------------------------------
 * Start States
 *----------------------------------------------------------------------*/
%x TAG
%x DETAILS
%x SEPARATORS
%s TOKEN

/*----------------------------------------------------------------------
 * Definitions
 *----------------------------------------------------------------------*/
space      [ \r]
wordchar   [^ \t\n\r]
tab        [\t]
eoachar    [\t\n\r]
eotchar    [\n\r]
newline    [\n]
tokchar    [^\t\n\r]
/*detchar    [^ \t\n\r\<\>\[]*/
detchar    [^ \t\n\r\[]
tagchar    [^ \t\n\r\]]
anlchar    [^ \t\n\r]
/*bestchar   [\/]*/

/*----------------------------------------------------------------------
 * Rules
 *----------------------------------------------------------------------*/
%%
%{
  BEGIN(TOKEN);
  using namespace std;
  using namespace moot;
%}

%{
/*--------------------------------------------------------------------
 * TOKEN
 */
%}

<TOKEN>^([ \t]*){newline} {
  //-- EOS: blank line: maybe return eos (ignore empty sentences)
  theLine++; theColumn=0; theByte += yyleng;
  if (mtoken->tok_type != TokTypeEOS) {
    mtoken->tok_type=TokTypeEOS;
    mtoken->tok_text="\n";
    return TokTypeEOS;
  }
}

<TOKEN>^"%%"([^\r\n]*){newline} {
  //-- COMMENT: return comments as special tokens
  theLine++;
  theColumn = 0;
  theByte += yyleng;
  lasttyp = TokTypeComment;
  if (!ignore_comments) {
    mtoken->clear();
    mtoken->toktype(TokTypeComment);
    mtoken->textAppend(reinterpret_cast<const char *>(yytext)+2, yyleng-3);
    return TokTypeComment;
  }
}

<TOKEN>^({tokchar}*){wordchar} {
  //-- TOKEN-TEXT: keep only internal whitespace
  theColumn += yyleng;
  theByte += yyleng;
  mtoken->clear();
  mtoken->toktype(TokTypeVanilla);
  mtoken->text(reinterpret_cast<const char *>(yytext), yyleng);
  lasttyp = LexTypeText;
}

<TOKEN>{newline} {
  //-- TOKEN: end-of-token
  theLine++;
  theColumn = 0;
  theByte += yyleng;
  mtoken->toktype(TokTypeVanilla);
  lasttyp = TokTypeVanilla;
  return TokTypeVanilla;
}

<<EOF>> {
  //-- EOF: should only happen in TOKEN mode
  mtoken->tok_text="";
  switch (lasttyp) {
   case LexTypeText:
   case LexTypeTag:
   case LexTypeDetails:
   case LexTypeEOA:
     on_EOA();
     lasttyp = TokTypeVanilla;
     break;

   case TokTypeUnknown:
   case TokTypeVanilla:
   case TokTypeComment:
   case TokTypeUser:
     lasttyp = TokTypeEOS;
     break;

   case TokTypeEOS:
     lasttyp = TokTypeEOF;
     break;

   case TokTypeEOF:
     break;

   default:
     lasttyp = TokTypeEOS;
     break;
  }
  mtoken->toktype(static_cast<mootTokenType>(lasttyp));
  return lasttyp;
}

<TOKEN>{space}*/{eoachar} {
  //-- TOKEN: end-of-token
  theColumn += yyleng;
  theByte += yyleng;

  if (first_analysis_is_best) current_analysis_is_best = true;
  if (ignore_first_analysis)  ignore_current_analysis = true;

  lasttyp = LexTypeText;
  BEGIN(SEPARATORS);
}


%{
/*--------------------------------------------------------------------
 * SEPARATORS
 */
%}

<SEPARATORS>{tab}({space}*) {
  //-- SEPARATORS: Separator character(s): increment column nicely
  theColumn = (static_cast<int>(theColumn/8)+1)*8;
  theByte += yyleng;
  lasttyp = LexTypeEOA;
}
<SEPARATORS>""/{wordchar} {
  //-- SEPARATORS: end of separators
  theColumn += yyleng;
  theByte += yyleng;
  BEGIN(DETAILS);
  //-- allocate new analysis
  mtoken->insert(mootToken::Analysis());
  manalysis = &(mtoken->tok_analyses.back());
}
<SEPARATORS>""/{eotchar} {
  //-- SEPARATORS/EOT: reset to initial state
  //theLine++;
  theColumn = 0;
  theByte += yyleng;
  BEGIN(TOKEN);
}


%{
/*--------------------------------------------------------------------
 * DETAILS
 */
%}

<DETAILS>"["_?/{tagchar} {
  //-- DETAILS: looks like a tag
  theColumn += yyleng;
  theByte += yyleng;
  manalysis->details.append(reinterpret_cast<const char *>(yytext), yyleng);
  lasttyp = LexTypeDetails;
  BEGIN(TAG);
}

<DETAILS>{detchar}+ {
  //-- DETAILS: detail text
  theColumn += yyleng;
  theByte += yyleng;
  manalysis->details.append(reinterpret_cast<const char *>(yytext), yyleng);
  lasttyp = LexTypeDetails;
}

<DETAILS>{space}+/{wordchar} {
  //-- DETAILS: internal whitespace: keep it
  theColumn += yyleng;
  theByte += yyleng;
  manalysis->details.append(reinterpret_cast<const char *>(yytext), yyleng);
  lasttyp = LexTypeDetails;
}

<DETAILS>{space}*/{eoachar} {
  //-- DETAILS/EOA: add & clear current analysis, if any
  theByte += yyleng;
  on_EOA();
  BEGIN(SEPARATORS);
}

%{
/*--------------------------------------------------------------------
 * TAG
 */
%}

<TAG>{tagchar}+ {
  //-- TAG: tag text
  theColumn += yyleng;
  theByte += yyleng;
  manalysis->details.append(reinterpret_cast<const char *>(yytext), yyleng);
  if (manalysis->tag.empty()) manalysis->tag.assign(reinterpret_cast<const char *>(yytext), yyleng);
  lasttyp = LexTypeTag;
  BEGIN(DETAILS);
}

%{
/*--------------------------------------------------------------------
 * UNKNOWN
 */
%}

{space}+ {
  /* mostly ignore spaces */
  theColumn += yyleng;
  theByte += yyleng;
  //lasttyp = LexTypeIgnore;
}

. {
  theColumn += yyleng;
  theByte += yyleng;
  yycarp("Unrecognized TOKEN character '%c'", *yytext);
}

<DETAILS>. {
  theColumn += yyleng;
  theByte += yyleng;
  yycarp("Unrecognized DETAIL character '%c'", *yytext);
}

<TAG>. {
  theColumn += yyleng;
  theByte += yyleng;
  yycarp("Unrecognized TAG character '%c'", *yytext);
}

%%

/*----------------------------------------------------------------------
 * mootTokenLexer helpers
 */
void mootTokenLexer::mgl_begin(int stateno) {BEGIN(stateno);}

/*----------------------------------------------------------------------
 * Local Methods for mootTokenLexer
 *----------------------------------------------------------------------*/
void mootTokenLexer::reset(void)
{
  theLine = 0;
  theColumn = 0;
  theByte = 0;
  current_analysis_is_best = false;
  manalysis = NULL;
  mtoken_default.clear();
  lasttyp = moot::TokTypeEOS;
  BEGIN(TOKEN);
}
