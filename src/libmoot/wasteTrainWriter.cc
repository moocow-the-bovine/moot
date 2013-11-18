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

#include "wasteTrainWriter.h"

moot_BEGIN_NAMESPACE

//============================================================================
// wasteTrainWriter

//----------------------------------------------------------------------
void wasteTrainWriter::rtt_unescape(std::string &s)
{
  std::string::iterator si=s.begin(), oi=si;
  for ( ; si != s.end(); ++si,++oi) {
    if ( *si=='\\' && (si+1) < s.end() ) {
      //-- rtt-style backslash escapes
      ++si;
      switch (*si) {
      case 'n': *oi = '\n'; break;
      case 'r': *oi = '\r'; break;
      case 't': *oi = '\t'; break;
      case 'f': *oi = '\f'; break;
      case 'v': *oi = '\v'; break;
      case ' ': *oi = ' '; break;
      case '\\': *oi = '\\'; break;
      default: 
	*oi++ = '\\';
	*oi   = *si;
	break;
      }
      continue;
    }
    else if ( *si==' ' && (si+2) < s.end() && *(si+1)=='$' && *(si+2)=='=' ) {
      //-- rttz-style "RAW $= TOK" : return just 'raw' part
      break;
    }
    //-- usual case: just copy
    *oi = *si;
  }
  s.erase(oi, s.end());
}

//----------------------------------------------------------------------
wasteTrainWriter::wasteTrainWriter(int fmt, const std::string &myname)
  : TokenWriter(fmt,myname),
    wt_writer(NULL),
    wt_pseg(NULL),
    wt_txtbuf("\\n"),
    wt_at_eos(true)
{
  wt_lexer.dehyph_mode(false);
  wt_lexer.from_reader( &wt_scanner );
}

//----------------------------------------------------------------------
wasteTrainWriter::~wasteTrainWriter()
{
  this->close();
}

//----------------------------------------------------------------------
void wasteTrainWriter::to_mstream(mootio::mostream *mostreamp)
{
  if (wt_writer) wt_writer->to_mstream(mostreamp);
}

//----------------------------------------------------------------------
void wasteTrainWriter::close(void)
{
  flush_buffer(true);
  wt_writer = NULL;
  wt_pseg   = NULL;
  wt_txtbuf = "\\n";
  wt_at_eos = true;
  wt_lexer.from_reader( &wt_scanner );
}

//----------------------------------------------------------------------
void wasteTrainWriter::put_token(const mootToken &token)
{
  int toktyp;
  bool at_bow = true;
  switch (token.tok_type) {

  case TokTypeEOS:
    //-- eos: save it
    wt_at_eos = true;
    wt_segbuf.push_back( mootToken("!EOS", TokTypeComment) );
    if (wt_pseg) {
      waste_tag_attr_set(wt_pseg->tok_besttag, wtap_S, true);
      flush_buffer();
    }
    break;

  case TokTypeSB:
  case TokTypeWB:
    wt_txtbuf.append(token.text());
    break;
   
  case TokTypeComment:
    //-- comment: check for rtt-style raw character data
    if (token.text().substr(0,3) == "$c=") {
      wt_txtbuf.append(token.text(), 3, mootTokString::npos);
    }
  default:
    //-- non-vanilla token: just buffer
    wt_segbuf.push_back(token);
    break;

  case TokTypeVanilla:
  case TokTypeLibXML:
    //-- vanilla token: scan into segments
    flush_buffer(false);
    wt_txtbuf.append( token.text() );
    wt_txtbuf.push_back('\0');
    rtt_unescape( wt_txtbuf );
    wt_scanner.from_buffer( wt_txtbuf.data(), wt_txtbuf.size() );
    while ((toktyp=wt_lexer.get_token()) != TokTypeEOF) {
      mootToken *ltok = wt_lexer.token();
      if ( !ltok->analyses().empty() ) {
	ltok->besttag( ltok->analyses().front().tag );
	waste_tag_attr_set(ltok->tok_besttag, wtap_s, wt_at_eos );
	waste_tag_attr_set(ltok->tok_besttag, wtap_S, false );
	waste_tag_attr_set(ltok->tok_besttag, wtap_w, at_bow );
	wt_segbuf.push_back( *ltok );
	wt_pseg = &(wt_segbuf.back());
	wt_at_eos = at_bow = false;
      }
    }
    //-- cleanup
    wt_txtbuf.clear();
    break;
  }
}

//----------------------------------------------------------------------
void wasteTrainWriter::put_raw_buffer(const char *buf, size_t len)
{
  this->put_token( mootToken(std::string(buf,len), (tw_is_comment_block ? TokTypeComment : TokTypeXMLRaw)) );
}

//----------------------------------------------------------------------
void wasteTrainWriter::to_writer(TokenWriter *writer)
{
  this->close();
  wt_writer = writer;
}

//----------------------------------------------------------------------
void wasteTrainWriter::flush_buffer(bool force)
{
  if (wt_writer)
    wt_writer->put_tokens( wt_segbuf );
  if (wt_writer || force) {
    wt_segbuf.clear();
    wt_pseg = NULL;
  }
}

moot_END_NAMESPACE
