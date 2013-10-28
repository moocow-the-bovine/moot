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
 * wasteLexicon
 */

//----------------------------------------------------------------------
bool wasteLexicon::load(moot::TokenReader *reader)
{
  mootTokenType toktyp;
  while ( (toktyp=reader->get_token()) != TokTypeEOF ) {
    if (toktyp==TokTypeVanilla || toktyp==TokTypeLibXML)
      insert( reader->token()->text() );
  }
  return true;
}

//----------------------------------------------------------------------
bool wasteLexicon::load(mootio::mistream *mis)
{
  TokenReaderNative tr(tiofRare, "wasteLexicon::load::TokenReaderNative");
  tr.from_mstream(mis);
  bool rc = load(&tr);
  return rc;
}

//----------------------------------------------------------------------
bool wasteLexicon::load(const char *filename)
{
  mootio::mifstream mis(filename,"r");
  bool rc = load(&mis);
  return rc;
}

/*============================================================================
 * wasteLexer
 */

//----------------------------------------------------------------------
wasteLexer::wasteLexer(int fmt, const std::string &myname)
  : TokenReader(fmt, myname),
    scanner(NULL),
    wl_state(0x40),
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
            wl_tagset[i][j][k][l][m].resize(7);
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
  if(scanner)
    scanner->from_mstream(mistreamp);
}

//----------------------------------------------------------------------
void wasteLexer::close(void)
{
  scanner->close();
}

//----------------------------------------------------------------------
mootTokenType wasteLexer::next_token(void)
{
  if (!scanner)
    return wl_token.toktype(TokTypeEOF);

  if (!wl_tokbuf.empty())
  {
    wl_token = wl_tokbuf.front();
    wl_tokbuf.pop_front();
    return wl_token.toktype();
  }

  mootTokenType toktyp = scanner->get_token ();
  switch ( toktyp ) {
    case TokTypeVanilla:
      break;
    default:
      wl_token = *(scanner->token ());
      return wl_token.toktype(toktyp);
  }
  return wl_token.toktype(toktyp);
}

//----------------------------------------------------------------------
mootTokenType wasteLexer::get_token(void)
{
  wl_token.clear();

  // if scanner has not been set
  if (!scanner)
    return wl_token.toktype(TokTypeEOF);

  // TODO: dehyphenation
  // skip whitespace
  // BOD serves as whitespace
  bool skip = true;
  while ( skip )
  {
    // grab next token from scanner
    mootTokenType toktyp = next_token();

    // ??? handle empty token text ???

    //-- token type
    switch ( toktyp ) {
      case TokTypeVanilla:
        break;
      default:
        // wl_token = *(scanner->token ()); -> moved to next_token
        return wl_token.toktype(toktyp);
    }

    // classify current token and handle it appropriately
    std::string real_toktext = scanner->token()->text();
    wasteLexerTypeE lexertyp = waste_lexertype(real_toktext);
    switch ( lexertyp ) {
      //
      // whitespace
      //
      case wLexerTypeSpace:
      case wLexerTypeNewline:
        wl_state |= ls_blanked;
        continue;

      //
      // punctuation
      //
      case wLexerTypeEOS:
        set_token(wl_token, real_toktext, eos, non, uk, 0, (wl_state&ls_blanked) ? kn : uk);
        break;
      case wLexerTypeDot:
        set_token(wl_token, real_toktext, dot, non, uk, 0, (wl_state&ls_blanked) ? kn : uk);
        break;
      case wLexerTypeComma:
        set_token(wl_token, real_toktext, comma, non, uk, 0, (wl_state&ls_blanked) ? kn : uk);
        break;
      case wLexerTypePercent:
      case wLexerTypeMonetary:
      case wLexerTypePunct:
        set_token(wl_token, real_toktext, sc, non, uk, 0, (wl_state&ls_blanked) ? kn : uk);
        break;
      case wLexerTypePlus:
        set_token(wl_token, real_toktext, plus, non, uk, 0, (wl_state&ls_blanked) ? kn : uk);
        break;
      case wLexerTypeColon:
        set_token(wl_token, real_toktext, colon, non, uk, 0, (wl_state&ls_blanked) ? kn : uk);
        break;
      case wLexerTypeSemicolon:
        set_token(wl_token, real_toktext, scolon, non, uk, 0, (wl_state&ls_blanked) ? kn : uk);
        break;
      case wLexerTypeApostrophe:
        set_token(wl_token, real_toktext, apos, non, uk, 0, (wl_state&ls_blanked) ? kn : uk);
        break;
      case wLexerTypeQuote:
        set_token(wl_token, real_toktext, quote, non, uk, 0, (wl_state&ls_blanked) ? kn : uk);
        break;
      case wLexerTypeLBR:
        set_token(wl_token, real_toktext, lbr, non, uk, 0, (wl_state&ls_blanked) ? kn : uk);
        break;
      case wLexerTypeRBR:
        set_token(wl_token, real_toktext, rbr, non, uk, 0, (wl_state&ls_blanked) ? kn : uk);
        break;
      case wLexerTypeSlash:
        set_token(wl_token, real_toktext, slash, non, uk, 0, (wl_state&ls_blanked) ? kn : uk);
        break;
      case wLexerTypeHyph:
        set_token(wl_token, real_toktext, hyphen, non, uk, 0, (wl_state&ls_blanked) ? kn : uk);
        break;

      //
      // alpha
      //
      case wLexerTypeAlphaLower:
        set_token(wl_token, real_toktext,
                 (wl_stopwords.lookup(real_toktext)) ? stop : alpha,
                  lo,
                 (wl_abbrevs.lookup(real_toktext))   ? kn   : uk,
                  real_toktext.length(),
                 (wl_state&ls_blanked)               ? kn   : uk);
        break;
      case wLexerTypeAlphaUpper:
        set_token(wl_token, real_toktext,
                 (wl_stopwords.lookup(real_toktext)) ? stop : alpha,
                  up,
                 (wl_abbrevs.lookup(real_toktext))   ? kn   : uk,
                  real_toktext.length(),
                 (wl_state&ls_blanked)               ? kn   : uk);
        break;
      case wLexerTypeAlphaCaps:
        set_token(wl_token, real_toktext,
                 (wl_stopwords.lookup(real_toktext)) ? stop : alpha,
                  cap,
                 (wl_abbrevs.lookup(real_toktext))   ? kn   : uk,
                  real_toktext.length(),
                 (wl_state&ls_blanked)               ? kn   : uk);
        break;

      //
      // numbers 
      //
      case wLexerTypeNumber:
        set_token(wl_token, real_toktext, num, non, uk, real_toktext.length(), (wl_state&ls_blanked) ? kn : uk);
        break;
      case wLexerTypeRoman:
        set_token(wl_token, real_toktext,
                 (wl_stopwords.lookup(real_toktext)) ? stop : rom,
                 (wl_stopwords.lookup(real_toktext)) ? cap  : non,
                  uk, real_toktext.length(),+
                 (wl_state&ls_blanked) ? kn : uk);
        break;

      //
      // default rule
      //
      default:
        set_token(wl_token, real_toktext, other, non,
                  wl_abbrevs.lookup(real_toktext) ? kn : uk,
                  real_toktext.length(),
                 (wl_state&ls_blanked) ? kn : uk);
    }
    // unset blanked state
    wl_state &= ~(ls_blanked);
    skip = false;
  }

  //-- token location
  wl_token.location( scanner->token()->location () );

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
void wasteLexer::set_token(mootToken &token, const std::string &tok_text, cls wl_cls, cas wl_cas, binary wl_abbr, size_t length, binary wl_blanked)
{
  // length classification
  len wl_length;
  switch ( length )
  {
    case 0:
      wl_length = le_null;
      break;
    case 1:
      wl_length = le_one;
      break;
    case 2:
    case 3:
      wl_length = le_three;
      break;
    case 4:
    case 5:
      wl_length = le_five;
      break;
    default:
      wl_length = longer;
  }

  // special token text
  token.tok_text = wl_tagset[wl_cls][wl_cas][wl_abbr][wl_length][wl_blanked][0];
  if ( wl_cls == stop )
  {
    token.tok_text += ':';
    token.tok_text += utf8ToLower( tok_text );
  }

  // analyses + real token text
  for ( int i = 1; i <= 6; ++i ) {
    token.insert( wl_tagset[wl_cls][wl_cas][wl_abbr][wl_length][wl_blanked][i],
                  std::string("[") + wl_tagset[wl_cls][wl_cas][wl_abbr][wl_length][wl_blanked][i] + " " + tok_text + "]" );
  }
}

moot_END_NAMESPACE
