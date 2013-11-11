/* -*- Mode: C++; coding: utf-8; c-basic-offset: 2; -*- */
/*
   libmoot : moot part-of-speech tagging library
   Copyright (C) 2013 by Bryan Jurish <moocow@cpan.org> and Kay-Michael Würzner

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

#include "wasteLexer.h"
#include "wasteCase.h"
#include <sstream>

moot_BEGIN_NAMESPACE

/*============================================================================
 * wasteLexer
 */

//----------------------------------------------------------------------
wasteLexer::wasteLexer(int fmt, const std::string &myname)
  : TokenReader(fmt, myname),
    scanner(NULL),
    wl_state(ls_init),
    wl_sb_fw(true),
    wl_current_tok(NULL),
    wl_dehyph_mode(false)
{
  // wasteLexer has local contents
  tr_token = &wl_token;
  tr_sentence = &wl_sentence;

  // build the tagset
  const char* cls_strings[n_cls] = {"stop","rom","alpha","num","$.","$,","$:","$;","$?","$(","$)","$-","$+","$/","$\"","$\'","$~","other"};
  const char* cas_strings[n_cas] = {"*","lo","up","cap"};
  const char* abbr_strings[n_binary] = {"0","1"};
  const char* len_strings[n_len] = {"0","1","3","5","N"};
  const char* blanks_strings[n_binary] = {"0","1"};

  wl_tagset.resize(n_cls);
  for (int i = 0; i < n_cls; ++i)
  {
    wl_tagset[i].resize(n_cas);
    for (int j = 0; j < n_cas; ++j)
    {
      wl_tagset[i][j].resize(n_binary);
      for (int k = 0; k < n_binary; ++k)
      {
        wl_tagset[i][j][k].resize(n_len);
        for (int l = 0; l < n_len; ++l)
        {
          wl_tagset[i][j][k][l].resize(n_binary);
          for (int m = 0; m < n_binary; ++m)
          {
            wl_tagset[i][j][k][l][m].resize(n_hidden);
            std::ostringstream tag;
            tag << cls_strings[i]
              << ',' << cas_strings[j]
              << ",a" << abbr_strings[k]
              << ",l" << len_strings[l]
              << ",b" << blanks_strings[m];
            wl_tagset[i][j][k][l][m][0] = tag.str ();
            wl_tagset[i][j][k][l][m][1] = tag.str () + ",s0,S0,w0";
            wl_tagset[i][j][k][l][m][2] = tag.str () + ",s0,S1,w0";
            wl_tagset[i][j][k][l][m][3] = tag.str () + ",s0,S0,w1";
            wl_tagset[i][j][k][l][m][4] = tag.str () + ",s0,S1,w1";
            wl_tagset[i][j][k][l][m][5] = tag.str () + ",s1,S0,w1";
            wl_tagset[i][j][k][l][m][6] = tag.str () + ",s1,S1,w1";
          }
        }
      }
    }
  }
}

//----------------------------------------------------------------------
wasteLexer::~wasteLexer()
{
}

//----------------------------------------------------------------------
void wasteLexer::from_reader(TokenReader *reader)
{
  scanner = reader;
}

//----------------------------------------------------------------------
void wasteLexer::from_mstream(mootio::mistream *mistreamp)
{
  if (scanner)
    scanner->from_mstream(mistreamp);
}

//----------------------------------------------------------------------
void wasteLexer::close(void)
{
  //if (scanner) scanner->close(); //-- moo: dangerous: this is probably NOT a good idea (the user sets scanner, so he/she is responsible!)
  scanner = NULL;
}

//----------------------------------------------------------------------
void wasteLexer::buffer_token(void)
{
  wl_state &= ~(ls_flush);
  wasteLexerToken local_token;
  scanner->get_token();
  local_token.wlt_token = *(scanner->token ());
  wasteLexerType lextype;
  switch (local_token.wlt_token.tok_type) {
    case TokTypeVanilla:
    case TokTypeLibXML:
      //-- categorize scanned token text
      lextype = waste_lexertype(local_token.wlt_token.tok_text);
      switch (lextype)
      {
        case wLexerTypeNewline:
          wl_state |= ls_nl;
        case wLexerTypeSpace:
          // -- store blanked feature for upcoming token
          wl_state |= ls_blanked;
          return;

        case wLexerTypeHyph:
          wl_state |= ls_hyph;

        default:
          // -- set token features according to lexer state and reset
          local_token.wlt_type = lextype;
          local_token.wlt_blanked = (wl_state & ls_blanked) ? true : false;
          local_token.s = (wl_state & ls_sb_fw) ? true : false;
          local_token.w = (wl_state & ls_wb_fw) ? true : false;
          wl_state &= ~(ls_blanked);
          wl_state &= ~(ls_sb_fw);
          wl_state &= ~(ls_wb_fw);

          // -- store token
          wl_tokbuf.push_back(local_token);
          wl_current_tok = &(wl_tokbuf.back());
          return;
      }
      break;
    case TokTypeSB:
      // -- set bos and bow for upcoming token
      wl_state |= ls_sb_fw;
      wl_state |= ls_wb_fw;

      // -- set eos for last alnum token
      if(wl_current_tok)
      {
        wl_current_tok->S = true;
        wl_current_tok = NULL;
      }

      // -- output possible
      wl_state |= ls_flush;
      // breaks hyphenation
      break;
    case TokTypeWB:
      wl_state |= ls_wb_fw;
      wl_state |= ls_flush;
      // breaks hyphenation
      break;
    case TokTypeEOS:
      // breaks hyphenation
      break;
    case TokTypeEOF:
      // breaks hyphenation
      wl_state |= ls_flush;
      break;
    case TokTypeComment:
      // does not break hyphenation
    default:
      break;
  }
  wl_tokbuf.push_back(local_token);
}

//----------------------------------------------------------------------
mootTokenType wasteLexer::get_token(void)
{
  wl_token.clear();

  //-- if scanner has not been set return EOF
  if (!scanner)
    return wl_token.toktype(TokTypeEOF);

  //-- fill internal token buffer, calls scanner::get_token
  while (wl_tokbuf.empty() || ((wl_state & ls_flush) == 0))
  {
    buffer_token ();
  }

  wl_token = wl_tokbuf.front().wlt_token;
  switch (wl_token.tok_type)
  {
    case TokTypeVanilla:
    case TokTypeLibXML:
      set_token(wl_token, wl_tokbuf.front());
      break;
    default:
      break;
  }
  wl_tokbuf.pop_front();
  return wl_token.toktype();
}

//----------------------------------------------------------------------
mootTokenType wasteLexer::get_sentence(void)
{
  wl_sentence.clear();
  mootTokenType toktyp;
  for (toktyp=get_token(); toktyp != TokTypeEOF; toktyp=get_token()) {
    wl_sentence.push_back( wl_token );
  }
  return toktyp;
}

//----------------------------------------------------------------------
void wasteLexer::set_token(mootToken &token, wasteLexerToken &lex_token)
{
  //-- length classification
  len tok_length;
  switch (lex_token.wlt_token.tok_text.length())
  {
    case 0:
      tok_length = le_null;
      break;
    case 1:
      tok_length = le_one;
      break;
    case 2:
    case 3:
      tok_length = le_three;
      break;
    case 4:
    case 5:
      tok_length = le_five;
      break;
    default:
      tok_length = longer;
  }

  //-- class selection (based on waste_lexertype)
  cls tok_class = other;
  cas tok_case = non;
  binary tok_abbr = uk;
  switch (lex_token.wlt_type) {

      //
      // punctuation
      //
    case wLexerTypeEOS:
      tok_class = eos;
      break;
    case wLexerTypeDot:
      tok_class = dot;
      break;
    case wLexerTypeComma:
      tok_class = comma;
      break;
    case wLexerTypePercent:
    case wLexerTypeMonetary:
    case wLexerTypePunct:
      tok_class = sc;
      break;
    case wLexerTypePlus:
      tok_class = plus;
      break;
    case wLexerTypeColon:
      tok_class = colon;
      break;
    case wLexerTypeSemicolon:
      tok_class = scolon;
      break;
    case wLexerTypeApostrophe:
      tok_class = apos;
      break;
    case wLexerTypeQuote:
      tok_class = quote;
      break;
    case wLexerTypeLBR:
      tok_class = lbr;
      break;
    case wLexerTypeRBR:
      tok_class = rbr;
      break;
    case wLexerTypeSlash:
      tok_class = slash;
      break;
    case wLexerTypeHyph:
      tok_class = hyphen;
      break;

      //
      // alpha
      //
    case wLexerTypeAlphaLower:
      tok_class = (wl_stopwords.lookup(lex_token.wlt_token.tok_text)) ? stop : alpha;
      tok_case = lo;
      tok_abbr = (wl_abbrevs.lookup(lex_token.wlt_token.tok_text)) ? kn : uk;
      break;
    case wLexerTypeAlphaUpper:
      tok_class = (wl_stopwords.lookup(lex_token.wlt_token.tok_text)) ? stop : alpha;
      tok_case = up;
      tok_abbr = (wl_abbrevs.lookup(lex_token.wlt_token.tok_text)) ? kn : uk;
      break;
    case wLexerTypeAlphaCaps:
      tok_class = (wl_stopwords.lookup(lex_token.wlt_token.tok_text)) ? stop : alpha;
      tok_case = cap;
      tok_abbr = (wl_abbrevs.lookup(lex_token.wlt_token.tok_text)) ? kn : uk;
      break;

      //
      // numbers 
      //
    case wLexerTypeNumber:
      tok_class = num;
      break;
    case wLexerTypeRoman:
      tok_class = (wl_stopwords.lookup(lex_token.wlt_token.tok_text)) ? stop : rom;
      tok_case = (wl_stopwords.lookup(lex_token.wlt_token.tok_text)) ? cap : non;
      break;

      //
      // default rule
      //
    default:
      tok_class = other;
      tok_abbr = wl_abbrevs.lookup(lex_token.wlt_token.tok_text) ? kn : uk;
  }

  //-- blanked state
  binary tok_blanked = lex_token.wlt_blanked ? kn : uk;

  //-- set model token text
  token.tok_text = wl_tagset[tok_class][tok_case][tok_abbr][tok_length][tok_blanked][0];
  if (tok_class == stop)
  {
    token.tok_text.push_back(':');
    token.tok_text.append( tok_case==lo ? lex_token.wlt_token.tok_text : utf8ToLower(lex_token.wlt_token.tok_text) );
  }

  //-- set model analyses (includes scanned token text)
  for (int i = 1; i < n_hidden; ++i) {
    switch (i)
    {
      case 1:
        if ( lex_token.s || lex_token.S || lex_token.w )
          continue;
        else
          break;
      case 2:
        if ( lex_token.s || lex_token.w )
          continue;
        else
          break;
      case 3:
        if ( lex_token.s || lex_token.S )
          continue;
        else
          break;
      case 4:
        if ( lex_token.s )
          continue;
        else
          break;
      case 5:
        if ( lex_token.S )
          continue;
        else
          break;
      default:
        break;
    }
    std::string analysis = "[";
    analysis.append(wl_tagset[tok_class][tok_case][tok_abbr][tok_length][tok_blanked][i]);
    analysis.push_back(' ');
    analysis.append(lex_token.wlt_token.tok_text);
    analysis.push_back(']');
    token.insert(wl_tagset[tok_class][tok_case][tok_abbr][tok_length][tok_blanked][i], analysis);
  }
}

moot_END_NAMESPACE
