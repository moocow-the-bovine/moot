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
 *
 * \file wasteTrainWriter.h
 * \brief TokenWriter wrapper class for writing WASTE tokenizer 'well-done' training data from pre-tokenized input with leading whitespace
 *
 * \author moocow
 *
 * \date 2013
 * 
 */

#ifndef _WASTE_TRAIN_WRITER_H
#define _WASTE_TRAIN_WRITER_H

#include <wasteLexer.h>
#include <wasteScanner.h>

moot_BEGIN_NAMESPACE

//============================================================================
// wasteTrainer
/**
 * \brief TokenWriter wrapper class for writing WASTE tokenizer 'well-done' training data from pre-tokenized input with leading whitespace
 *
 * \detail Input tokens should contain leading whitespace where appropriate;
 * "\n", "\r", "\t", "\f", "\v", "\ ", and "\\" are C-style escapes.
 * Input comments of the form "$c=TEXT" are also treated as raw text.
 * Token text of the form "RAW $= COOKED" will be bashed to RAW.
 */
class wasteTrainWriter : public TokenWriter {
public:
  //----------------------------------------------------------------------
  /// \name local static methods
  //@{
  /** perform Lingua::TT::TextAlignment (*.rtt) style un-escaping in-place on \a s */
  static void rtt_unescape(std::string &s);
  //@}

public:
  //----------------------------------------------------------------------
  /// \name local data
  //@{
  wasteTokenScanner  wt_scanner;   /**< scanner for token-internalsegmentation */
  wasteLexerReader   wt_lexer;     /**< lexer for classification */
  TokenWriter       *wt_writer;    /**< subordinate writer, sink for "well-done" segments */

  mootSentence       wt_segbuf;    /**< local segment buffer */
  mootToken         *wt_pseg;      /**< last vanilla segment (for 'S' attribute), point into wt_buffer */

  std::string        wt_txtbuf;    /**< token text buffer */
  bool               wt_at_eos;    /**< whether we've seen an EOS and no vanilla token since */
  //@}

public:
  //----------------------------------------------------------------------
  /// \name Constructors etc.
  //@{
  /** Default constructor */
  wasteTrainWriter(int fmt=tiofUnknown, const std::string &myname="wasteTrainer");

  /** Destructor */
  virtual ~wasteTrainWriter
();
  //@}

  //------------------------------------------------------------
  /// \name TokenWriter API: Output Selection
  //@{
  /** Select output to a mootio::mostream pointer; just wraps sink->to_mstream() */
  virtual void to_mstream(mootio::mostream *mostreamp);

  /** Finish output to currently selected sink & perform any required cleanup operations. */
  virtual void close(void);
  //@}

  //------------------------------------------------------------
  /// \name TokenWriter API: Token Stream Access
  //@{
  virtual void put_token(const mootToken &token);

  virtual void put_sentence(const mootSentence &sentence) {
    this->put_tokens(sentence);
    this->put_token( mootToken(TokTypeEOS) );
  };

  virtual void put_raw_buffer(const char *buf, size_t len);
  //@}

  //------------------------------------------------------------
  /// \name local methods
  //@{

  /** Write "well-done" output to subordinate writer */
  void to_writer(TokenWriter *writer);

  /** flush buffer to current output sink if defined */
  void flush_buffer(bool force=false);
  //@}
};


moot_END_NAMESPACE

#endif /* _WASTE_TRAIN_WRITER_H */
