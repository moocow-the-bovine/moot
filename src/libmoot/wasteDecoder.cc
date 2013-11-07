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

#include "wasteDecoder.h"

moot_BEGIN_NAMESPACE

//============================================================================
// Constructors etc.

//----------------------------------------------------------------------
wasteDecoder::~wasteDecoder()
{
  this->close();
}

//============================================================================
// TokenWriter API: Output Selection

//----------------------------------------------------------------------
void wasteDecoder::to_mstream(mootio::mostream *mostreamp)
{
  if (wd_sink) wd_sink->to_mstream(mostreamp);
}

//----------------------------------------------------------------------
void wasteDecoder::close(void)
{
  flush_buffer(true);
  //if (wd_sink) wd_sink->close(); //-- moo: dangerous: this is probably NOT a good idea (the user sets sink, so he/she is responsible!)
  wd_sink = NULL;
  wd_tok = NULL;
  wd_sb = false;
}

//============================================================================
// wasteDecoder: local methods

//----------------------------------------------------------------------
void wasteDecoder::to_writer(TokenWriter *sink)
{
  this->close();
  wd_sink = sink;
}

//----------------------------------------------------------------------
void wasteDecoder::flush_buffer(bool force)
{
  if (!wd_sink) return;
  for (; !wd_buf.empty() && (force || &(wd_buf.front()) != wd_tok); wd_buf.pop_front()) {
    wd_sink->put_token(wd_buf.front());
  }
} 

//----------------------------------------------------------------------
void wasteDecoder::_put_token(const mootToken &token)
{
  //-- merge or push token
  switch (token.tok_type) {
  case TokTypeVanilla:
  case TokTypeLibXML:
    {
      //-- parse raw text from details
      const mootTagString &detail = token.tok_analyses.empty() ? "" : token.tok_analyses.front().details;
      size_t rawtext_start = detail.rfind(' ');
      mootTagString rawtext(detail, rawtext_start+1, (rawtext_start==detail.npos ? 0 : detail.size()-rawtext_start-2));

      if (wd_tok && !tag_attr_w(token.besttag())) {
	//-- merge tokens
	wd_tok->tok_text += rawtext;
	wd_tok->tok_besttag.push_back(' ');
	wd_tok->tok_besttag += token.besttag();
	wd_tok->tok_location.length = (token.tok_location.offset + token.tok_location.length - wd_tok->tok_location.offset);
	break;
      }
      else if (wd_tok) {
	//-- we have a buffered wd_tok: check for EOS
	if (!wd_sb && tag_attr_S(wd_tok->besttag()) && tag_attr_s(token.besttag()))
	  wd_buf.push_back( mootToken(TokTypeEOS) );

	//-- spit out old wd_tok and update
	wd_tok = NULL;
	flush_buffer();
      }

      //-- no buffered wd_tok: buffer this one
      wd_buf.push_back(token);
      wd_tok = &(wd_buf.back());
      wd_tok->text(rawtext);
      wd_tok->tok_analyses.clear();
      wd_sb  = false;
      break;
    }

  case TokTypeSB:
    //-- sentence break: flush WIP token
    wd_tok = NULL;
    flush_buffer();
    wd_buf.push_back(token);
    if (!wd_sb) {
      wd_buf.push_back( mootToken(TokTypeEOS) );
      wd_sb = true;
    }

  case TokTypeWB:
    //-- word break: flush WIP token
    wd_tok = NULL;

  case TokTypeEOS:
    if (wd_sb) break;
    
  default:
    wd_buf.push_back(token);
    break;
  }
  flush_buffer();
}

//----------------------------------------------------------------------
void wasteDecoder::_put_tokens(const mootSentence &tokens)
{
  for (mootSentence::const_iterator si=tokens.begin(); si!=tokens.end(); ++si)
    _put_token(*si);
}

//----------------------------------------------------------------------
void wasteDecoder::_put_sentence(const mootSentence &sentence)
{
  _put_tokens(sentence);
  if (sentence.back().toktype() != TokTypeEOS)
    _put_token( mootToken(TokTypeEOS) );
}

//----------------------------------------------------------------------
void wasteDecoder::_put_raw_buffer(const char *buf, size_t len)
{
  //-- not supported
}

moot_END_NAMESPACE
