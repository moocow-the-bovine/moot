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
#include <stdarg.h>

/*moot_BEGIN_NAMESPACE*/
namespace moot {

/*==========================================================================
 * TokenReader
 *==========================================================================*/

/** Abstract class for token I/O */
class TokenReader {
public:
  /** Default constructor */
  TokenReader(void) {};

  /** Default destructor */
  virtual ~TokenReader(void) { clear(); };

  /**
   * Get current internal token buffer.
   * The contents of this buffer may be overwritten on the next
   * call to any other TokenReader method -- copy it
   * if you need persistence.
   */
  virtual mootToken &token(void) =0;

  /**
   * Get current internal sentence buffer.
   * The contents of this buffer may be overwritten on the next
   * call to any other TokenReader method -- copy it
   * if you need persistence.
   */
  virtual mootSentence &sentence(void) =0;


  /** Clear any/all underlying buffers */
  virtual void clear(void) {};

  /**
   * Read in next token, storing information in internal token buffer.
   */
  //inline
  virtual mootTokFlavor get_token(void) =0;

  /**
   * Read in next sentence, storing tokens in an
   * internal buffer.  Returns TF_EOF on EOF,
   * otherwise should always return TF_EOS.
   */
  virtual mootTokFlavor get_sentence(void) =0;

  /** Complain. */
  virtual void carp(const char *fmt, ...);
};


/*------------------------------------------------------------
 * TokenReaderCooked
 */
/** Abstract class for native "cooked" text-format token input */
class TokenReaderCooked : public TokenReader {
public:
  /*----------------------------------------
   * Reader: Cooked: Types
   */
  /*
  typedef enum {
    CFT_Unknown,     //< unknown input format, treated as CFT_Rare
    CFT_Rare,        //< -tagged, -analyzed
    CFT_MediumRare,  //< -taggged, +analyzed
    CFT_Medium,      //< +tagged, -analyzed
    CFT_WellDone,    //< +tagged, +analyzed
    CFT_NTypes       //< Not really a format type
  } CookedFormatType;
  */

public:
  /*----------------------------------------
   * Reader: Cooked: Data
   */
  /** The underlying lexer (does the dirty work). */
  mootTokenLexer lexer;

  /** Internal sentence buffer used by get_sentence() */
  mootSentence msentence;

public:
  /*----------------------------------------
   * Reading: Cooked: Methods: Overrides
   */

  /** Default constructor
   *
   * \input_is_tagged:
   * Whether input is tagged with intitial 'best' tags.
   */
  TokenReaderCooked(bool is_tagged=false)
  {
    input_is_tagged(is_tagged);
  };

  /** Default destructor */
  virtual ~TokenReaderCooked(void) {};

  /** Get underlying token buffer */
  virtual mootToken &token(void) { return lexer.mtoken; }

  /** Get underlying sentence buffer. */
  virtual mootSentence &sentence(void) { return msentence; }

  /** Clear any/all underlying buffers */
  virtual void clear(void)
  {
    lexer.mtoken.clear();
    msentence.clear();
    lexer.reset();
  }

  /** Read in next token, storing information in internal token buffer. */
  virtual mootTokenLexer::TokenType get_token(void)
  {
    return (mootTokenLexer::TokenType)lexer.yylex();
  }

  /** Read in next sentence, buffering tokens.  Returns TF_EOF on EOF. */
  virtual mootTokFlavor get_sentence(void);

  /** Complain, giving position information for current input source. */
  virtual void carp(const char *fmt, ...);


  /*----------------------------------------
   * Reader: Cooked: Methods: New methods
   */
  /**
   * Get value of the 'tagged' flag : whether we
   * think the input has been tagged with initial
   * best-tags.
   */
  inline bool input_is_tagged(void)
  {
    return lexer.first_analysis_is_best;
  };

  /**
   * Set value of the 'tagged' flag: whether we
   * think the input has been tagged with initial
   * best-tags.
   */
  inline bool input_is_tagged(bool is_tagged)
  {
    if (is_tagged) {
      lexer.first_analysis_is_best = true;
      lexer.ignore_first_analysis = true;
    }
    return is_tagged;
  };
};


/*------------------------------------------------------------
 * TokenReaderCookedFile
 */
/** Class for native "cooked" text-format token input from a C stream */
class TokenReaderCookedFile : public TokenReaderCooked {
public:
  /*----------------------------------------
   * Reading: Cooked: File: Methods: Overrides
   */
  /**
   * Default constructor
   *
   * \input_is_tagged: Whether input is tagged with intitial 'best' tags.
   * \infile: C stream from which to read (default=stdin)
   * \filename: name for input stream to use for diagnostics
   */
  TokenReaderCookedFile(bool is_tagged=false,
			FILE *infile=stdin,
			const string &filename="")
  {
    input_is_tagged(is_tagged);
    select_stream(infile, filename);
  };

  /** Default destructor */
  virtual ~TokenReaderCookedFile(void) {};

  /*----------------------------------------
   * Reader: Cooked: File: Methods: New
   */
  /** Select a (new) input stream.
   * \infile: C stream from which to read (default=stdin)
   * \filename: name for input stream to use for diagnostics
   */
  virtual void select_stream(FILE *infile, const std::string &filename="")
  {
    lexer.select_streams(infile,stderr);
    lexer.srcname   = filename;
    lexer.theLine   = 1;
    lexer.theColumn = 0;
  };
};


/*------------------------------------------------------------
 * TokenReaderCookedString
 */
/** Class for native "cooked" text-format token input from a C string */
class TokenReaderCookedString : public TokenReaderCooked {
public:
  /*----------------------------------------
   * Reader: Cooked: String: Methods: Overrides
   */

  /**
   * Default constructor
   * \input_is_tagged: Whether input is tagged with intitial 'best' tags.
   * \s: C string from which to read (default=empty)
   * \srcname: name for input string to use for diagnostics
   *
   * \warning: the underlying lexer will try and read \b directly
   * from the string \c s (no copying is performed) so keep
   * it around as long as you need to read tokens from it.
   */
  TokenReaderCookedString(bool is_tagged=false,
			  const char *s="",
			  const string &srcname="(string)")
  {
    input_is_tagged(is_tagged);
    select_string(s, srcname);
  };

  /** Default destructor */
  virtual ~TokenReaderCookedString(void) {};

  /*----------------------------------------
   * Reader: Cooked: String: Methods: New
   */
  /**
   * Select a new input string.
   * \s: C string from which to read (default=empty)
   * \srcname: name for input string to use for diagnostics
   *
   * \warning: the underlying lexer will try and read \b directly
   * from the string \c s (no copying is performed) so keep
   * it around as long as you need to read tokens from it.
   */
  virtual void select_string(const char *s="",
			     const string &srcname="")
  {
    if (!srcname.empty()) lexer.srcname = srcname;
    lexer.select_string(s,stderr);
    lexer.theLine   = 1;
    lexer.theColumn = 0;
  };
};



/*==========================================================================
 * TokenWriter
 *==========================================================================*/

/*------------------------------------------------------------
 * TokenWriter
 */
/** Abstract class for token output */
class TokenWriter {
public:
  /*----------------------------------------
   * Writing: Methods
   */
  /** Default constructor */
  TokenWriter(void) {};

  /** Default destructor */
  virtual ~TokenWriter(void) {};
	
  /** Write a single token */
  virtual void put_token(const mootToken &token) =0;

  /** Write a whole sentence */
  virtual void put_sentence(const mootSentence &sentence) =0;
};

/*------------------------------------------------------------
 * TokenWriterCooked
 */
/** Abstract class for native "cooked" text-format token output */
class TokenWriterCooked : public TokenWriter {
public:
  /*----------------------------------------
   * Writer: Data
   */
  /** Whether to output all analyses or just those for the 'best' tag (the default). */
  bool want_best_only;

public:
  /*----------------------------------------
   * Writer: Cooked: Methods: Overrides
   */
  /** Default constructor */
  TokenWriterCooked(bool i_want_best_only=false)
    : want_best_only(i_want_best_only)
  {};

  /** Default destructor */
  virtual ~TokenWriterCooked(void) {};

  /*----------------------------------------
   * Writer: Cooked: Methods: New
   */
  //(none)
};


/*------------------------------------------------------------
 * TokenWriterCookedString
 */
/** Class for native "cooked" text-format token output to strings */
class TokenWriterCookedString : public TokenWriterCooked {
public:
  /*----------------------------------------
   * Writer: Cooked: String: Data
   */
  /** Pointer to current output buffer (use buffer() instead) */
  std::string *dst;

  /** Default buffer for output string (use buffer() instead) */
  std::string dst_default;

  /** Temporary buffer for stringification of costs */
  char costbuf[32];
public:
  /*----------------------------------------
   * Writer: Cooked: String: Methods: Overrides
   */
  /**
   * Default constructor
   * \i_want_best_only: whether to output only 'best' tags
   */
  TokenWriterCookedString(bool i_want_best_only=false)
  {
    want_best_only = i_want_best_only;
  };

  /** Default destructor */
  virtual ~TokenWriterCookedString(void) {};
	
  /** Write string for a token to current string buffer (buffer is cleared) */
  virtual void put_token(const mootToken &token)
  {
    if (!dst) dst = &dst_default;
    dst->clear();
    token2string(token, *dst);
  }

  /** Write a whole sentence to current string buffer (buffer is cleared) */
  virtual void put_sentence(const mootSentence &sentence)
  {
    if (!dst) dst = &dst_default;
    dst->clear();
    sentence2string(sentence, *dst);
  }

  /*----------------------------------------
   * Writer: Cooked: String: Methods: New
   */
  /**
   * Append canonical string-form of a mootToken (without trailing newline)
   * to string \c s .  Returns a reference to \c s .
   */
  std::string &token2string(const mootToken &token, std::string &s);

  /** Clear internal buffer and stringify token into it */
  inline std::string &token2string(const mootToken &token)
  {
    buffer().clear();
    return token2string(token,buffer());
  };

  /**
   * Append canonical string-form of a mootSentence (without trailing empty line)
   * to string \c s .  Returns reference to \c s .
   */
  std::string &sentence2string(const mootSentence &sentence, std::string &s);

  /** Clear internal buffer and stringify sentence into it */
  inline std::string &sentence2string(const mootSentence &sentence)
  {
    buffer().clear();
    return sentence2string(sentence,buffer());
  };

  /** Select a new output string buffer for \c put_* methods */
  inline std::string &buffer(std::string &buf)
  { 
    dst = &buf;
    return *dst;
  };

  /** Get currently selected output string buffer */
  inline std::string &buffer(void)
  {
    if (!dst) dst = &dst_default;
    return *dst;
  };
};


/*------------------------------------------------------------
 * TokenWriterCookedFile
 */
/** Class for native "cooked" text-format output to a C stream */
class TokenWriterCookedFile : public TokenWriterCooked {
public:
  /*----------------------------------------
   * Writer: Cooked: File: Data
   */
  /** Current output file */
  FILE *out;

public:
  /*----------------------------------------
   * Writer: Cooked: File: Methods: Overrides
   */
  /**
   * Default constructor
   * \i_want_best_only: whether to output only 'best' tags
   * \outfile: C stream to which to write data.
   */
  TokenWriterCookedFile(bool i_want_best_only=false, FILE *outfile=stdout)
    : out(outfile)
  {
    want_best_only = i_want_best_only;
  };

  /** Default destructor */
  virtual ~TokenWriterCookedFile(void) {};
	
  /** Write string for a token to current string buffer (buffer is cleared) */
  virtual void put_token(const mootToken &token);

  /** Write a whole sentence to current string buffer (buffer is cleared) */
  virtual void put_sentence(const mootSentence &sentence);

  /*----------------------------------------
   * Writing: Cooked: File: Methods: New
   */
  /** Get current output file */
  inline FILE *output_file(void) { return out; };

  /** Set current output file */
  inline FILE *output_file(FILE *outfile) { out=outfile; return out; };
};


}; /*moot_END_NAMESPACE*/

#endif /* _moot_TOKEN_IO_H */
