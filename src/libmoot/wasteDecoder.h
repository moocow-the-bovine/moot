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

/**
 * \file wasteDecoder.h
 *
 * \brief simple hash_set<>-based lexicon class for moot::wasteLexer
 *
 * \author moocow
 * 
 * \date 2013
 * 
 */

#ifndef _WASTE_DECODER_H
#define _WASTE_DECODER_H

#include <mootTokenIO.h>
#include <wasteTypes.h>

moot_BEGIN_NAMESPACE

/*============================================================================
 * wasteDecoder
 */
/** \brief   */
class wasteDecoder : public TokenWriter
{
public:
  //------------------------------------------------------------
  /// \name static methods
  //@{
  /** get a boolean tag attribute by relative position \a rpos to end-of-tag string */
  inline static bool tag_attr(const mootTagString &tagstr, size_t rpos, bool mydefault=false)
  {
    return tagstr.size() > rpos ? (tagstr[tagstr.size()-rpos]!='0') : mydefault;
  };

  /** get boolean tag attribute 's' (beginning-of-sentence) */
  inline static bool tag_attr_s(const mootTagString &tagstr)
  { return tag_attr(tagstr,7,false); }

  /** get boolean tag attribute 'S' (wnd-of-sentence) */
  inline static bool tag_attr_S(const mootTagString &tagstr)
  { return tag_attr(tagstr,4,false); }

  /** get boolean tag attribute 'w' (beginning-of-word) */
  inline static bool tag_attr_w(const mootTagString &tagstr)
  { return tag_attr(tagstr,1,true); }

public:
  //------------------------------------------------------------
  /// \name public data
  //@{
  bool		wd_sb;    /** whether an SB has been seen */
  mootSentence  wd_buf;   /** intermediate token buffer */
  mootToken    *wd_tok;   /** current token under construction (NULL for none), pointer into wd_buf */
  TokenWriter  *wd_sink;  /** underlying data sink */
  //@}

public:
  //------------------------------------------------------------
  /// \name Constructors etc.
  //@{
  /** Default constructor */
  wasteDecoder(int fmt=tiofWellDone, const std::string &name="wasteDecoder")
    : TokenWriter(fmt,name),
      wd_sb(false),
      wd_tok(NULL),
      wd_sink(NULL)
  {};

  /** destructor calls close() */
  ~wasteDecoder();
  //@}

  //------------------------------------------------------------
  /// \name TokenWriter API: Output Selection
  //@{

  /** Select output to a mootio::mostream pointer; just wraps sink->to_mstream() */
  virtual void to_mstream(mootio::mostream *mostreamp);

  /* Finish output to currently selected sink & perform any required cleanup operations. */
  virtual void close(void);
  //@}

  //------------------------------------------------------------
  /// \name TokenWriter API: Token Stream Access
  //@{
  virtual void put_token(const mootToken &token) {
    _put_token(token);
  };
  virtual void put_tokens(const mootSentence &tokens) {
    _put_tokens(tokens);
  };
  virtual void put_sentence(const mootSentence &sentence) {
    _put_sentence(sentence);
  };
  virtual void put_raw_buffer(const char *buf, size_t len) {
    _put_raw_buffer(buf,len);
  };
  //@}

  //------------------------------------------------------------
  /// \name wasteDecoder: local methods
  //@{
  /** Select output to subordinate TokenWriter */
  void to_writer(TokenWriter *sink);

  void flush_buffer(bool force=false);

  void _put_token(const mootToken &token);
  void _put_tokens(const mootSentence &tokens);
  void _put_sentence(const mootSentence &sentence);
  void _put_raw_buffer(const char *buf, size_t len); //-- not supported
  //@}

};

moot_END_NAMESPACE

#endif /* _WASTE_DECODER_H */

