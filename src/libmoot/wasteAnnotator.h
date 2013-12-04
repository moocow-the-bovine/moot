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
 * \file wasteAnnotator.h
 *
 * \date 2013
 * 
 */

#ifndef _WASTE_ANNOTATOR_H
#define _WASTE_ANNOTATOR_H

#include <mootTokenIO.h> //-- includes TokenLexer -> GenericLexer -> BufferIO -> Utils -> CIO -> IO

namespace moot
{

  /*============================================================================
   * wasteAnnotator
   */
  /** \brief Sentence-functional annotations for tokens usually not covered by
   *         standard morphological analysis.
   *
   */
  class wasteAnnotator
  {
    public:
      /*--------------------------------------------------------------------
       * mootWasteAnnotator: Methods
       */
      /*------------------------------------------------------------*/
      /** \name Constructors etc. */
      //@{
      /** Default constructor */
      wasteAnnotator();

      /** Destructor */
      virtual ~wasteAnnotator();
      //@}

      /*------------------------------------------------------------*/

      /** \name Functionalities */
      //@{

      /** Adds analyses to token. */
      void annotate_token(mootToken& token);

      //@}
  };

  /*============================================================================
   * wasteAnnotatorWriter
   */
  /** \brief TokenWriter wrapper for wasteAnnotator */
  class wasteAnnotatorWriter : public TokenWriter
  {

  public:
    //------------------------------------------------------------
    /// \name public data
    //@{
    wasteAnnotator   waw_annotator; /** wrapped annotator */
    TokenWriter     *waw_sink;      /** underlying data sink */
    //@}

  public:
    //------------------------------------------------------------
    /// \name Constructors etc.
    //@{
    /** Default constructor */
    wasteAnnotatorWriter(int fmt=tiofMediumRare, const std::string &name="wasteAnnotatorWriter")
      : TokenWriter(fmt,name)
    {};

    /** destructor calls close() */
    ~wasteAnnotatorWriter();
    //@}

    //------------------------------------------------------------
    /// \name TokenWriter API: Output Selection
    //@{
    /** Select output to a mootio::mostream pointer; just wraps sink->to_mstream() */
    virtual void to_mstream(mootio::mostream *mostreamp);

    /** Finish output to currently selected sink & perform any required cleanup operations.
     *  wasteAnnotatorWriter override force-fluses buffer and unsets sink.
     */
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
    /// \name wasteAnnotatorWriter: local methods
    //@{
    /** Select output to subordinate TokenWriter */
    void to_writer(TokenWriter *sink);

    void _put_token(const mootToken &token);
    void _put_tokens(const mootSentence &tokens);
    void _put_sentence(const mootSentence &sentence);
    void _put_raw_buffer(const char *buf, size_t len); //-- passed through to sink
    //@}

  };
} // namespace moot

#endif /* _WASTE_ANNOTATOR_H */
