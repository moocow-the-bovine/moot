/* -*- Mode: C++; coding: utf-8; c-basic-offset: 4; -*- */
/*
   libmoot : moot part-of-speech tagging library
   Copyright (C) 2013-2016 by Bryan Jurish <moocow@cpan.org> and Kay-Michael Würzner

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
  static const int ls_init         = (ls_wb_fw | ls_sb_fw | ls_blanked); /**< initial state of the lexer*/
  static const int ls_head_hyph    = ( ls_head | ls_hyph ); /**< lexer has seen some word followed by a hyphen */
  static const int ls_head_hyph_nl = ( ls_head_hyph | ls_nl ); /**< lexer has seen some word followed by a hyphen and a newline */

  /*============================================================================
   * wasteLexerToken
   */
  class wasteLexerToken
  {
    public:
      /*------------------------------------------------------------*/
      /** \name standard mootToken */
      //@{
      mootToken       wlt_token;   /**< standard data as text, type location etc. */
      //@}
      /** \name extra data */
      //@{
      wasteLexerType  wlt_type;    /**< fine grained token type, as returned by waste_lexertype */
      bool            wlt_blanked; /**< token has ws to its left, maybe as 'packed bitfield':1; */
      bool            s;
      bool            S;
      bool            w;
      //@}

      /*--------------------------------------------------------------------
       * wasteLexer: Methods
       */
      /*------------------------------------------------------------*/
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

      /** \name Functions */
      //@{

      /** Set token's extra data */
      inline void set_wlt_data(wasteLexerType lextype, bool blanked, bool s, bool S, bool w)
      {
        this->wlt_type    = lextype;
        this->wlt_blanked = blanked;
        this->s           = s;
        this->S           = S;
        this->w           = w;
      }
      //@}
  };

  /*============================================================================
   * wasteLexer
   */
  /** \brief Mid-level scanner stage
   *  \detail performs (optional) hyphenation normalization and text classification
   */
  class wasteLexer
  {
    public:
      //--------------------------------------------------------------------
      ///\name Embedded Types
      //@{

      /** Multi-dimensional vector for constant access on feature bundles */
      typedef std::vector<std::vector<std::vector<std::vector<std::vector<std::vector<std::string> > > > > > wasteTagset;

      /** List of wasteLexerToken for buffering while dehyphenating */
      typedef std::list<wasteLexerToken> wasteLexerBuffer;
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

      /** letter-case */
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
      wasteTagset       wl_tagset;        /**< Token feature bundles */
      int               wl_state;         /**< Current state of the lexer */
      wasteLexerBuffer  wl_lexbuf;        /**< Buffer for dehyphenation: list of wasteLexerToken */
      wasteLexerToken  *wl_current_tok;   /**< current token under construction (NULL for none), pointer into wl_lexbuf */
      wasteLexerToken  *wl_head_tok;      /**< head of hyphenation sequence (NULL for none), pointer into wl_lexbuf */
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
      wasteLexer();

      /** Destructor */
      virtual ~wasteLexer();
      //@}

      /*------------------------------------------------------------*/
      /** \name Lexing functions */
      //@{

      /** Length to length attribute conversion */
      inline len length_attr(size_t length) const
      {
        switch (length)
        {
          case 0:
            return le_null;
          case 1:
            return le_one;
          case 2:
          case 3:
            return le_three;
          case 4:
          case 5:
            return le_five;
          default:
            return longer;
        }
      }

      /** Set token features (token.tok_analyses) w.r.t. model features from source lex_token **/
      void set_token(mootToken &token, const wasteLexerToken &lex_token);

      /**
       * copies stok to internal buffer.
       * If wl_dehyph_mode is true, seeks and removes hyphenations.
       */
      void buffer_token(const mootToken& stok);

      void reset(void);
      //@}

      /*------------------------------------------------------------*/
      /** \name low-level utilities */
      //@{
      /**
       * wrapper for wl_lexbuf.pop_front() which may clear wl_current_tok, wl_head_tok
       * \note calling this method may break auto-magic de-hyphenation
       * \note workaround for crashes (segfault, double-free, heap corruption) on kira (g++5, ubuntu-16.04), 2016-11-18 (moocow);
       *       see wasteLexer::get_token() at wasteLexer.cc:477
       */
      inline void lexbuf_pop_front(void)
      {
	  wasteLexerToken *front = &(wl_lexbuf.front());
	  if (wl_current_tok==front) wl_current_tok=NULL; 
	  if (wl_head_tok==front)    wl_head_tok=NULL;
	  wl_lexbuf.pop_front();
      }
      //@}
  };

  /*============================================================================
   * wasteLexerReader
   */
  /** \brief Mid-level scanner stage, wraps moot::wasteLexer in moot::TokenReader API */
  class wasteLexerReader : public TokenReader {
  public:
    /*----------------------------------------
     * wasteLexerReader: Data
     */
    /** underlying lexer object */
    wasteLexer lexer;

    /** data source */
    TokenReader *scanner;

    /** token buffer for get_token() */
    mootToken wlr_token;

    /** sentence/document buffer for get_sentence() */
    mootSentence wlr_sentence;

  public:
    //------------------------------------------------------------
    /** \name Constructors etc. */
    //@{
    /**
     * Default constructor
     * @param fmt bitmask of moot::TokenIOFormat flags.
     *  \li set moot::tiofTagged to assign token tags to wasteLexerTypeNames[]
     *  \li set moot::tiofAnalyzed to assign single analysis-strings to wasteLexerTypeNames[]
     * @param name name of current input source
     */
    wasteLexerReader(int                fmt  =tiofText,
		     const std::string &name ="wasteLexerReader");

    /** Default destructor */
    virtual ~wasteLexerReader(void);
    //@}

    /*------------------------------------------------------------
     * TokenReader: Input Selection
     */
    /** \name Input Selection */
    //@{
    /** Set token source (usually some wasteScanner) */
    void from_reader(TokenReader *reader);

    /** Select input from a mootio::mistream pointer. */
    virtual void from_mstream(mootio::mistream *mistreamp);

    /**
     * Finish input from currently selected source & perform any required cleanup operations.
     * Currently just sets scanner=NULL and user is responsible for closing the scanner.
     */
    virtual void close(void);

    /** Turn on/off dehyphenation mode **/
    inline void dehyph_mode(bool on)
    {
      lexer.wl_dehyph_mode = on;
    };
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
  };


} // namespace moot


#endif /* _WASTE_LEXER_H */
