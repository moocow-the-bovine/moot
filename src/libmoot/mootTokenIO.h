/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2004 by Bryan Jurish <moocow@ling.uni-potsdam.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

/*--------------------------------------------------------------------------
 * File: mootTokenIO.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moocow's PoS tagger : token I/O
 *--------------------------------------------------------------------------*/

#ifndef _moot_TOKEN_IO_H
#define _moot_TOKEN_IO_H

#include "mootToken.h"
#include "mootTokenLexer.h"
#include <stdio.h>

/*moot_BEGIN_NAMESPACE*/
namespace moot {

/*--------------------------------------------------------------------------
 * mootTokenIO
 *--------------------------------------------------------------------------*/

/*------------------------------------------------------------
 * TokenReader
 */
/** Class for native token input */
class TokenReader {
public:
  /*----------------------------------------
   * Types: Reading: Data
   */
  /** The underlying lexer (does the dirty work). */
  mootTokenLexer lexer;

  /** Internal sentence buffer used by get_sentence() */
  mootSentence sentence;

  /*----------------------------------------
   * Types: Reading: Methods
   */

  /** Default constructor
   * @param first_analysis_is_best Whether first analysis should be used to instantiate 'besttag' (default=true)
   * @param ignore_first_analysis Whether to (otherwise) ignore first analysis (default=false)
   */
  TokenReader(bool first_analysis_is_best=true, bool ignore_first_analysis=false)
  {
    lexer.first_analysis_is_best = first_analysis_is_best;
    lexer.ignore_first_analysis = ignore_first_analysis;
  };

  /** Get underlying token buffer */
  mootToken &token(void) { return lexer.mtoken; }

  /** Clear any/all underlying buffers */
  void clear(void)
  {
    token().clear();
    sentence.clear();
    lexer.reset();
  }
	
  /** Select stream input */
  void select_stream(FILE *in=stdin, char *source_name=NULL);

  /** Select string input */
  void select_string(const char *instr, char *source_name=NULL);

  /**
   * Read in next token, storing information in internal token buffer.
   * TODO: make this use the parser!
   */
  inline mootTokenLexer::TokenType get_token(void)
  {
    return (mootTokenLexer::TokenType)lexer.yylex();
  }

  /** read in & return next sentence (empty on EOF) */
  mootSentence &get_sentence(void);
};


/*------------------------------------------------------------
 * TokenWriter
 */
/** Class for native token output */
class TokenWriter {
public:
  /*----------------------------------------
   * Wrting: Data
   */
  /** Whether to output all analyses or just those for the 'best' tag (the default). */
  bool want_best_only;
      
  /** Temporary buffer for stringification of costs */
  char costbuf[32];

public:
  /*----------------------------------------
   * Writing: Methods
   */
  /** Default constructor */
  TokenWriter(bool i_want_best_only=false)
    : want_best_only(i_want_best_only)
  {};
	
  /** Generate canonical string-form of a mootToken (without trailing newline) */
  string token_string(const mootToken &token);

  /** Write string for a token to a C stream, followed by trailing newline */
  void token_put(FILE *out, const mootToken &token);

  /** Write a whole sentence to a C stream, followed by 2 trailing newlines */
  void sentence_put(FILE *out, const mootSentence &sentence);
};

}; /*moot_END_NAMESPACE*/

#endif /* _moot_TOKEN_IO_H */
