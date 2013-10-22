/* -*- Mode: C++ -*- */

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

#include <mootGenericLexer.h>
#include <mootIO.h>

namespace moot
{
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
          size_t line=0, size_t column=0, mootio::ByteOffset byte=0, size_t init_bufsize=GenericLexer::MGL_DEFAULT_BUFFER_SIZE);

      /** Destructor */
      virtual ~wasteScanner();
          
      //@}

      /*------------------------------------------------------------*/
      /** \name Required Wrapper Methods (from GenericLexer) */
      //@{

      /** All other moot lexers are based on flex. Dummy implementation. */
      virtual void *mgl_yy_create_buffer(int size, FILE *unused=stdin)
      {
        yycarp("abstract method mgl_yy_create_buffer() called!");
        abort();
      };

      /** All other moot lexers are based on flex. Dummy implementation. */
      virtual void mgl_yy_switch_to_buffer(void *buf)
      {
        yycarp("abstract method mgl_yy_switch_to_buffer() called!");
        abort();
      };

      /** All other moot lexers are based on flex. Dummy implementation. */
      virtual void mgl_yy_init_buffer(void *buf, FILE *unused=stdin)
      {
        yycarp("abstract method mgl_yy_init_buffer() called!");
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

} // namespace moot


#endif /* _WASTE_SCANNER_H */
