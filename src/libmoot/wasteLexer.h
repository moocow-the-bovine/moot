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
 * \file wasteLexer.h
 *
 * \brief Declarations of the mid level lexer for the waste tokenizer.
 *
 *
 *
 * \author kmw
 *
 * \date 2013
 * 
 */

#ifndef _WASTE_LEXER_H
#define _WASTE_LEXER_H

#include <mootTokenIO.h>
#include <wasteTypes.h>
#include <wasteLexicon.h>

#include <vector>
#include <list>

namespace moot
{

  /*============================================================================
   * wasteLexerToken
   */
  class wasteLexerToken : public mootToken
  {
    public:
      /*------------------------------------------------------------*/
      /** \name extra data */
      //@{
      wasteLexerType  wlt_type;    /**< fine grained token type, as returned by waste_lexertype */
      bool            wlt_blanked; /**< token has ws to its left, maybe as 'packed bitfield':1; */
      bool            s;
      bool            S;
      bool            w;
      //@}

      /** \name Constructors etc. */
      //@{
      /** Default constructor */
      wasteLexerToken(wasteLexerType type=wLexerTypeOther, bool blanked=true, bool bos=true, bool eos=false, bool bow=true)
        : wlt_type(type),
          wlt_blanked(blanked),
          s(bos),
          S(eos),
          w(bow)
    {}
      //@}
  };

  /*============================================================================
   * wasteLexer
   */
  /** \brief Mid-level TokenReader scanner stage
   *  \detail performs (optional) hyphenation normalization and text classification
   */
  class wasteLexer : public TokenReader
  {
    public:
      //--------------------------------------------------------------------
      ///\name Embedded Types
      //@{

      /** Multi-dimensional vector for constant access on feature bundles */
      typedef std::vector<std::vector<std::vector<std::vector<std::vector<std::vector<std::string> > > > > > wasteTagset;

      /** List of mootToken for buffering while dehyphenating */
      typedef std::list<wasteLexerToken> wasteTokenBuffer;
      
      /** bitmask flags for possible lexer states (mainly used for dehyphenation) */
      enum wasteLexer_state
      {
        ls_flush   = 0x0001,
        ls_hyph    = 0x0002,
        ls_head    = 0x0004,
        ls_tail    = 0x0008,
        ls_nl      = 0x0010,
        ls_sb_fw   = 0x0020,
        ls_wb_fw   = 0x0040,
        ls_blanked = 0x0080,
      };
      static const int ls_init = (ls_wb_fw | ls_sb_fw | ls_blanked); /**< initial state of the lexer*/
      //@}

      //--------------------------------------------------------------------
      ///\name token classification feature indices
      //@{

      /** typographical classes */
      enum cls
      {
        stop     = 0,
        rom      = 1,
        alpha    = 2,
        num      = 3,
        dot      = 4,
        comma    = 5,
        colon    = 6,
        scolon   = 7,
        eos      = 8,
        lbr      = 9,
        rbr      = 10,
        hyphen   = 11,
        plus     = 12,
        slash    = 13,
        quote    = 14,
        apos     = 15,
        sc       = 16,
        other    = 17,
        n_cls    = 18
      };

      /** leter-case */
      enum cas
      {
        non      = 0,
        lo       = 1,
        up       = 2,
        cap      = 3,
        n_cas    = 4
      };

      /** binary feature values */
      enum binary
      {
        uk       = 0,
        kn       = 1,
        n_binary = 2
      };

      /** text length */
      enum len
      {
        le_null  = 0,
        le_one   = 1,
        le_three = 2,
        le_five  = 3,
        longer   = 4,
        n_len    = 5
      };

      /** hidden features: s[01],S[01],w[01] (except s1,*,w0) */
      static const int n_hidden = 7;

      //@}

      /*--------------------------------------------------------------------
       * wasteLexer: data
       */
      /*------------------------------------------------------------*/
      /** \name Low-level data */
      //@{
      TokenReader      *scanner;          /**< Input source */
      mootToken         wl_token;         /**< Local token */
      mootSentence      wl_sentence;      /**< Local sentence */
      wasteTagset       wl_tagset;        /**< Token feature bundles */
      int               wl_state;         /**< Current state of the lexer */
      bool              wl_sb_fw;         /**< Indicates forward bos feature */
      wasteTokenBuffer  wl_tokbuf;        /**< Buffer for dehyphenation */
      wasteLexerToken  *wl_current_tok;   /** current token under construction (NULL for none), pointer into wl_tokbuf */
      bool              wl_dehyph_mode;   /**< Dehyphenation switch */
      //@}

      /*------------------------------------------------------------*/
      /** \name Lexica */
      //@{
      wasteLexicon      wl_stopwords;     /**< List of stopwords */
      wasteLexicon      wl_abbrevs;       /**< List of abbreviations */
      wasteLexicon      wl_conjunctions;  /**< List of conjunctions (for dehyphenating) */
      //@}

      /*--------------------------------------------------------------------
       * wasteLexer: Methods
       */
      /*------------------------------------------------------------*/
      /** \name Constructors etc. */
      //@{
      /** Default constructor */
      wasteLexer(int fmt=tiofUnknown, const std::string &myname="wasteLexer");

      /** Destructor */
      virtual ~wasteLexer();
      //@}

      /*------------------------------------------------------------
       * TokenReader: Input Selection
       */
      /** \name Input Selection */
      //@{

      /** Select input from a mootio::mistream pointer; just wraps scanner->from_mstream() */
      virtual void from_mstream(mootio::mistream *mistreamp);

      /**
       * Finish input from currently selected source & perform any required cleanup operations.
       * Currently just sets scanner=NULL and user is responsible for closing the scanner.
       */
      virtual void close();
      //@}

      /*------------------------------------------------------------
       * TokenReader: Token-Level Access
       */
      /** \name Token-Level Access */
      //@{
      /**
       * Get the next token from the buffer.
       * On completion, current token (if any) is in *tr_token.
       */
      virtual mootTokenType get_token(void);

      /**
       * Buffer the remaining tokens from the currently selected input
       * stream as a mootSentence.
       * On completion, buffered tokens (if any) are in *tr_sentence.
       */
      virtual mootTokenType get_sentence(void);
      //@}

      /*------------------------------------------------------------
       * TokenReader: Diagnostics
       */
      /** \name Diagnostics */
      //@{

      /** Get current line number. Descendants may override this method. */
      virtual size_t line_number(void) { return scanner ? scanner->line_number() : 0; };

      /** Set current line number. Descendants may override this method. */
      virtual size_t line_number(size_t n) { return scanner ? scanner->line_number(n) : 0; };

      /** Get current column number. Descendants may override this method. */
      virtual size_t column_number(void) { return scanner ? scanner->column_number() : 0; };

      /** Set current column number. Descendants may override this method. */
      virtual size_t column_number(size_t n) { return scanner ? scanner->column_number(n) : 0; };

      /** Get current byte number. Descendants may override this method. */
      virtual mootio::ByteOffset byte_number(void) { return scanner ? scanner->byte_number() : 0; };

      /** Get current byte number. Descendants may override this method. */
      virtual mootio::ByteOffset byte_number(size_t n) { return scanner ? scanner->byte_number(n) : 0; };
      //@}

      /*------------------------------------------------------------*/
      /** \name wasteLexer's own functions */
      //@{
      /** Set token source (usually some wasteScanner) */
      void from_reader(TokenReader *reader);

      /** Set token features (token.tok_analyses) w.r.t. model features **/
      void set_token(mootToken &token, wasteLexerToken &lex_token);

      /** Turn on/off dehyphenation mode **/
      inline void dehyph_mode(bool on)
      {
        wl_dehyph_mode = on;
      };

      /**
       * MOves the next token(s) from internal scanner to internal buffer.
       * If wl_dehyph_mode is true, seeks and removes hyphenations.
       */
      void buffer_token(void);
      //@}
  };

} // namespace moot


#endif /* _WASTE_LEXER_H */
