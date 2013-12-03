/* -*- Mode: C++; coding: utf-8; c-basic-offset: 2; -*- */

/*
   libmoot : moot part-of-speech tagging library
   Copyright (C) 2013 by Bryan Jurish <moocow@cpan.org>

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

#include "wasteAnnotator.h"

namespace moot {

/*==========================================================================
 * wasteAnnotator
 */

//----------------------------------------------------------------------
  wasteAnnotator::wasteAnnotator()
  {
  }

//----------------------------------------------------------------------
  wasteAnnotator::~wasteAnnotator()
  {
  }

/*==========================================================================
 * wasteAnnotatorWriter
 */
  //============================================================================
  // Constructors etc.

  //----------------------------------------------------------------------
  wasteAnnotatorWriter::~wasteAnnotatorWriter()
  {
    this->close();
  }

  //============================================================================
  // TokenWriter API: Output Selection

  //----------------------------------------------------------------------
  void wasteAnnotatorWriter::to_mstream(mootio::mostream *mostreamp)
  {
    if (waw_sink) waw_sink->to_mstream(mostreamp);
  }

  //----------------------------------------------------------------------
  void wasteAnnotatorWriter::close(void)
  {
    waw_sink = NULL;
  }

  //============================================================================
  // wasteAnnotatorWriter: local methods

  //----------------------------------------------------------------------
  void wasteAnnotatorWriter::to_writer(TokenWriter *sink)
  {
    this->close();
    waw_sink = sink;
  }

  //----------------------------------------------------------------------
  void wasteAnnotatorWriter::_put_token(const mootToken &token)
  {
    if (!waw_sink) return;
    mootToken alter_token = token;
    waw_annotator.annotate_token(alter_token);
    waw_sink->put_token(alter_token);
  } 

  //----------------------------------------------------------------------
  void wasteAnnotatorWriter::_put_tokens(const mootSentence &tokens)
  {
    for (mootSentence::const_iterator si=tokens.begin(); si!=tokens.end(); ++si)
      _put_token(*si);
  }

  //----------------------------------------------------------------------
  void wasteAnnotatorWriter::_put_sentence(const mootSentence &sentence)
  {
    _put_tokens(sentence);
    if (sentence.back().toktype() != TokTypeEOS)
      _put_token( mootToken(TokTypeEOS) );
  }

  //----------------------------------------------------------------------
  void wasteAnnotatorWriter::_put_raw_buffer(const char *buf, size_t len)
  {
    //-- not supported
  }

} // namespace moot
