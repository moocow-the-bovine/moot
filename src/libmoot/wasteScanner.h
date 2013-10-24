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
 * \file wasteScanner.h
 *
 * \brief Declarations of the low level scanner for the waste tokenizer.
 *
 *
 *
 * \author kmw
 *
 * \date 2013
 * 
 */

#ifndef _WASTE_SCANNER_H
#define _WASTE_SCANNER_H

#include <mootTokenIO.h> //-- includes TokenLexer -> GenericLexer -> BufferIO -> Utils -> CIO -> IO

namespace moot
{

  /*============================================================================
   * wasteScanner
   */
  /** \brief Low-level class for raw text scanning
   * \todo fix column counting
   */
  class wasteScanner : public GenericLexer
  {
    public:
      /*--------------------------------------------------------------------
       * mootWasteScanner: Data
       */
      /*------------------------------------------------------------*/
      /** \name Low-level data */
      //@{
      char*       yybuffer;             /**< used for token-buffering */
      char*       yycursor;             /**< current position in buffer */
      char*       yylimit;              /**< end of buffer */
      char*       yytoken;              /**< beginning of current token */
      char*       yymarker;             /**< last matching position */
      size_t      buffer_size;          /**< current length of yybuffer */
      //@}

      /*--------------------------------------------------------------------
       * mootWasteScanner: Methods
       */
      /*------------------------------------------------------------*/
      /** \name Constructors etc. */
      //@{
      /** Default constructor */
      wasteScanner(const std::string &myname="moot::wasteScanner",
		   size_t line=0,
		   size_t column=0,
		   mootio::ByteOffset byte=0,
		   size_t init_bufsize=GenericLexer::MGL_DEFAULT_BUFFER_SIZE);

      /** Destructor */
      virtual ~wasteScanner();
      //@}

      /*------------------------------------------------------------*/
      /** \name Required Wrapper Methods (from GenericLexer) */
      //@{

      /** All other moot lexers are based on flex. Dummy implementation. */
      virtual void *mgl_yy_create_buffer(int size, FILE *unused=stdin)
      {
        yycarp("API method mgl_yy_create_buffer() not implemented in wasteScanner!");
        abort();
      };

      /** All other moot lexers are based on flex. Dummy implementation. */
      virtual void mgl_yy_switch_to_buffer(void *buf)
      {
        yycarp("API method mgl_yy_switch_to_buffer() not implemented in wasteScanner!");
        abort();
      };

      /** All other moot lexers are based on flex. Dummy implementation. */
      virtual void mgl_yy_init_buffer(void *buf, FILE *unused=stdin)
      {
        yycarp("API method mgl_yy_init_buffer() not implemented in wasteScanner!");
        abort();
      };
      //@}

      /*------------------------------------------------------------*/
      /** \name mootWasteScanner's real funcionalities */
      //@{
      
      /** yylex () (from wasteScannerScan.re.skel) */
      int yylex ();

      /** Returns contents of the current segment. */
      inline std::string yytext ()
      {
	return std::string ( yytoken, yytoken + yyleng () );
      }

      /** Returns length of the current segment. */
      inline int yyleng ()
      {
        return ( yycursor - yytoken );
      }

      /** Grabs new characters from input stream, called by yylex. */
      bool fill ( size_t n );

      /** Resets scanner to initial state */
      virtual void reset ();

      /** Switches input source */
      virtual void from_mstream (mootio::mistream *in);
      //@}
  };

  /*============================================================================
   * wasteTokenScanner
   */
  /** \brief Raw text scanner class returning mootToken; wraps wasteScanner */
  class wasteTokenScanner : public TokenReader {
  public:
    /*----------------------------------------
     * TokenScanner: Data
     */
    /** underlying scanner object */
    wasteScanner scanner;

    /** token buffer for get_token() */
    mootToken wts_token;

    /** sentence/document buffer for get_sentence() */
    mootSentence wts_sentence;

  public:
    //------------------------------------------------------------
    /** \name Constructors etc. */
    //@{
    /**
     * Default constructor
     * @param fmt bitmask of moot::TokenIOFormat flags.
     *  \li set moot::tiofTagged to assign token tags to wasteScannerTypeNames[]
     *  \li set moot::tiofAnalyzed to assign single analysis-strings to wasteScannerTypeNames[]
     *  \li set moot::tiofCost to assign analysis cost to wasteScannerTypeE value
     * @param name name of current input source
     */
    wasteTokenScanner(int                fmt  =tiofCost,
		      const std::string &name ="wasteTokenScanner");

    /** Default destructor */
    virtual ~wasteTokenScanner(void);
    //@}

    /*------------------------------------------------------------
     * TokenReader: Input Selection
     */
    /** \name Input Selection */
    //@{
    
    /** Select input from a mootio::mistream pointer. */
    virtual void from_mstream(mootio::mistream *mistreamp);

    /**
     * Finish input from currently selected source & perform any required
     * cleanup operations.
     */
    virtual void close(void);
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
    virtual size_t line_number(void) { return scanner.theLine; };

    /** Set current line number. Descendants may override this method. */
    virtual size_t line_number(size_t n) { return scanner.theLine = n; };

    /** Get current column number. Descendants may override this method. */
    virtual size_t column_number(void) { return scanner.theColumn; };

    /** Set current column number. Descendants may override this method. */
    virtual size_t column_number(size_t n) { return scanner.theColumn = n; };

    /** Get current byte number. Descendants may override this method. */
    virtual mootio::ByteOffset byte_number(void) { return scanner.theByte; };

    /** Get current byte number. Descendants may override this method. */
    virtual mootio::ByteOffset byte_number(size_t n) { return scanner.theByte = n; };
    //@}
  };


} // namespace moot


#endif /* _WASTE_SCANNER_H */
