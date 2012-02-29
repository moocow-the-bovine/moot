/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2009 by Bryan Jurish <moocow@cpan.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/*--------------------------------------------------------------------------
 * File: mootGenericLexer.h
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moocow's PoS tagger : generic lexer routines
 *--------------------------------------------------------------------------*/

/**
\file mootGenericLexer.h
\brief common utilities for \c flex++ lexers
*/

#ifndef _MOOT_GENERIC_LEXER_H
#define _MOOT_GENERIC_LEXER_H

#include <mootBufferIO.h> //-- (indirectly) includes mootCIO.h

namespace moot {
  using namespace std;

/**
 * \brief Abstract base class for Flex++ lexers.
 *
 * Descendant lexer specifications should include the following:

\verbatim

%header{
#include <mootGenericLexer.h>
using namespace moot;
%}
%define CLASS myLexer

%define INHERIT \
  : public GenericLexer

%define INPUT_CODE \
  return moot::GenericLexer::yyinput(buffer,result,max_size);

%define MEMBERS \
  / * ... local members ... * / \
  / *----- moot::GenericLexer helpers ----- * / \
  virtual void  *mgl_yy_current_buffer_p(void) \
                 {return (&yy_current_buffer);};\
  virtual void  *mgl_yy_create_buffer(int size, FILE *unused=stdin) \
                 {return (void*)(yy_create_buffer(unused,size));};\
  virtual void   mgl_yy_init_buffer(void *buf, FILE *unused=stdin) \
                 {yy_init_buffer((YY_BUFFER_STATE)buf,unused);};\
  virtual void   mgl_yy_delete_buffer(void *buf) \
                 {yy_delete_buffer((YY_BUFFER_STATE)buf);};\
  virtual void   mgl_yy_switch_to_buffer(void *buf) \
                 {yy_switch_to_buffer((YY_BUFFER_STATE)buf);};\
  virtual void   mgl_begin(int stateno); \
  //...

%define CONSTRUCTOR_INIT : \
  moot::GenericLexer("myLexer") \
  // ...

%%
%{
//-- rules go here ...
%}
%%
void myLexer::mgl_begin(int stateno) {BEGIN(stateno);}

\endverbatim
*/
class GenericLexer {
public:
  /*--------------------------------------------------------------------------
   * mootGenericLexer: Statics
   */
  /*static const int MGL_DEFAULT_BUFFER_SIZE = 8192;*/ //-- old: flex++ now uses 16384
  static const int MGL_DEFAULT_BUFFER_SIZE = 16384;

public:
  /*--------------------------------------------------------------------
   * mootGenericLexer: Data
   */
  /*------------------------------------------------------------*/
  /** \name Stream Parameters */
  //@{
  mootio::mistream        *mglin; /**< input stream wrapper */
  mootio::mostream       *mglout; /**< output stream wrapper */

  bool mgl_in_created;   /**< whether we created mglin */
  bool mgl_out_created;  /**< whether we created mglout */
  //@}

  /*------------------------------------------------------------*/
  /** \name Positional Parameters */
  //@{
  size_t theLine;               /**< Current line number (counting from BOF==0) */
  size_t theColumn;             /**< Current column number */
  mootio::ByteOffset theByte;   /**< Current byte number */
  //@}

  /*------------------------------------------------------------*/
  /** \name Diagnostic Data */
  //@{
  std::string  lexname; /**< symbolic name of lexer (or program) */
  //@}


  /*------------------------------------------------------------*/
  /** \name Low-level data */
  //@{
  std::string tokbuf;               /**< used for token-buffering */
  bool        tokbuf_clear;         /**< whether to clear tokbuf on next \c tokbuf_append() */
  //@}

public:
  /*--------------------------------------------------------------------
   * mootGenericLexer: Methods
   */

  /*------------------------------------------------------------*/
  /** \name Constructors etc. */
  //@{
  /** Default constructor */
  GenericLexer(const std::string &myname="moot::GenericLexer",
	       size_t line=0, size_t column=0, mootio::ByteOffset byte=0)
    : mglin(NULL),
      mglout(NULL),
      mgl_in_created(false),
      mgl_out_created(false),
      theLine(line),
      theColumn(column),
      theByte(byte),
      lexname(myname),
      tokbuf_clear(false)
  {};

 /** Destructor */
  virtual ~GenericLexer(void);

  /** Reset lexer state */
  virtual void reset(void);

  /** Clear any selected streams */
  virtual void clear(bool clear_input=true, bool clear_output=true);
  //@}

  /*------------------------------------------------------------*/
  /** \name Required Wrapper Methods */
  //@{
  virtual void *mgl_yy_current_buffer_p(void);
  //{ return (&yy_current_buffer); }

  /**
   * Set current start-state.
   * Descendants must override this method (section 3 only!)
   */
  virtual void mgl_begin(int stateno);
  //{ BEGIN(stateno); };

  /** Flex++ should define this, but it doesn't get through to us */
  virtual void mgl_yy_delete_buffer(void *buf)
  {
    yycarp("abstract method mgl_yy_delete_buffer() called!");
    abort();
  };
  
  /** Flex++ should define this (backwards), but it doesn't get through to us */
  virtual void *mgl_yy_create_buffer(int size, FILE *unused=stdin) =0;

  /** Flex++ should define this, but it doesn't get through to us */
  virtual void mgl_yy_switch_to_buffer(void *buf) =0;

  /** Flex++ should define this, but it doesn't get through to us */
  virtual void mgl_yy_init_buffer(void *buf, FILE *unused=stdin) =0;
  //@}

  /*------------------------------------------------------------*/
  /** \name Input selection */
  //@{
  /** Select mootio::mstream input */
  virtual void from_mstream(mootio::mistream *in=NULL);

  /** Select named file input */
  virtual void from_filename(const std::string &filename);

  /** Select C-stream input */
  virtual void from_file(FILE *in=stdin);
  
  /** Select C-buffer input */
  virtual void from_buffer(const char *buf, size_t len);

  /** Select C-string input */
  inline void from_string(const char *s) {
    from_buffer(s, strlen(s));
  };
  /** Select C++ string input */
  inline void from_string(const std::string &s) {
    from_buffer(s.data(), s.size());
  };

  /** Backwards-compatible alias */
  inline void select_streams(FILE *in, FILE *out=stdout, const char *myname=NULL) {
    from_file(in);
    to_file(out);
    if (myname) lexname = myname;
  };

  /** Backwards-compatible alias */
  inline void select_string(const char *in, FILE *out=stderr, const char *myname=NULL) {
    from_string(in);
    to_file(out);
    if (myname) lexname = myname;
  };
  //@}

  /*------------------------------------------------------------*/
  /** \name Output selection */
  //@{
  /** Select mootio::mstream output */
  virtual void to_mstream(mootio::mostream *out=NULL);

  /** Select named file output */
  virtual void to_filename(const std::string &filename);

  /** Select C-stream output */
  virtual void to_file(FILE *out=stdout);
  //@}


  /*------------------------------------------------------------*/
  /** \name Input Reverse-Override Hacks */
  //@{
  /** yyinput() hack, for use with flex++ INPUT_CODE */
  inline int yyinput(char *buffer, int &result, int max_size)
  {
    return (result = (mglin ? mglin->read(buffer, max_size) : 0));
  };
  //@}

  /*------------------------------------------------------------*/
  /** \name Token Buffering */
  //@{
  /** Add some characters to 'tokbuf' buffer */
  inline void tokbuf_append(const char *text, size_t len)
  {
    if (tokbuf_clear) {
      tokbuf.assign(text,len);
      tokbuf_clear = false;
    } else {
      tokbuf.append(text,len);
    }
  };
  //@}

  /*------------------------------------------------------------*/
  /** \name Location-tracking */
  //@{
  /** Increment both 'theColumn' and 'theByte' by len */
  inline void add_columns(ByteOffset len)
  {
    theColumn += len;
    theByte   += len;
  };

  /** Increment both 'theLine' and 'theByte' by len, set theColumn=0 */
  inline void add_lines(ByteOffset len=1)
  {
    theLine += len;
    theByte += len;
    theColumn = 0;
  };
  //@}


  /*------------------------------------------------------------*/
  /** \name Diagnostics */
  //@{
  /** Complain */
  virtual void yycarp(const char *fmt, ...);
  //@}
}; //-- /class mootGenericLexer

}; //-- /namespace moot

#endif // _MOOT_GENERIC_LEXER_H
