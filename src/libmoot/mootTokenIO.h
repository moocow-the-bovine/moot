/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2004 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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
 * File: mootTokenIO.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moocow's PoS tagger : token I/O
 *--------------------------------------------------------------------------*/

#ifndef _moot_TOKEN_IO_H
#define _moot_TOKEN_IO_H

#include <mootToken.h>
#include <mootTokenLexer.h>

#include <mootIO.h>
#include <mootCIO.h>
#include <mootCxxIO.h>
#include <mootBufferIO.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdexcept>

/*moot_BEGIN_NAMESPACE*/
namespace moot {

/*==========================================================================
 * TokenIO
 *==========================================================================*/
/** Enum for I/O format flags */
enum TokenIOFormatE {
  tiofNone      = 0x00000000,  ///< no format
  tiofUnknown   = 0x00000001,  ///< unknown format
  tiofNull      = 0x00000002,  ///< null i/o, useful for testing
  tiofUser      = 0x00000004,  ///< some user-defined format
  tiofNative    = 0x00000008,  ///< native text format
  tiofXML       = 0x00000010,  ///< XML format
  tiofConserve  = 0x00000020,  ///< Conserve raw XML
  tiofPretty    = 0x00000040,  ///< Pretty-print (XML only)
  tiofText      = 0x00000080,  ///< Pretty-print (XML only)
  tiofAnalyzed  = 0x00000100,  ///< input is pre-analyzed (>= "medium rare")
  tiofTagged    = 0x00000200,  ///< input is tagged ("medium" or "well done")
  tiofPruned    = 0x00000400,  ///< pruned output
};
typedef TokenIOFormatE TokenIOFormat;

/** Format alias for 'Cooked Rare' files. */
static const int tiofRare = tiofText;

/** Format alias for 'Cooked Medium Rare' files. */
static const int tiofMediumRare = tiofText|tiofAnalyzed;

/** Format alias for 'Cooked Medium' files. */
static const int tiofMedium = tiofText|tiofTagged;

/** Format alias for 'Cooked Well Done' files. */
static const int tiofWellDone = tiofText|tiofAnalyzed|tiofTagged;

/** Enum for I/O mode flags */
/*
enum TokenIOModeE {
  tiomNone,     ///< no I/O
  tiomUnknown,  ///< unknown mode (dangerous)
  tiomFilename, ///< named file I/O
  tiomFile,     ///< FILE* I/O
  tiomFd,       ///< file descriptor I/O
  tiomCBuffer,  ///< C memory-buffer I/O
  tiomCString,  ///< NUL-terminated string I/O
  tiomString,   ///< STL string I/O
  tiomUser,     ///< some other user-defined I/O mode
  tiomNModes    ///< number of modes; not really an I/O mode itself
};
typedef TokenIOModeE TokenIOMode;
*/


/** \brief Abstract class for token I/O */
class TokenIO {
public:
  /*--------------------------------------------------------------------*/
  ///\name Format String <-> Bitmask Utilities
  //@{
  /**
   * Parse a format string, which should be a comma-separated
   * list of TokenIOFormat flag-names (without the 'tiof' prefix,
   * matching is case-insensitive).
   * A prefix of '!' indicates negation of the given flag.
   * Returns the corresponding bitmask.
   * \see parse_format_request
   */
  static int parse_format_string(const std::string &fmtString);

  /**
   * Guess basic format flags from a filename based on
   * dot-separated extension substrings.
   * Returns a bitmask suitable for passing to sanitize_format().
   * Recognizes filename extensions as documented in mootfiles(5).
   * \see parse_format_request
   */
  static int guess_filename_format(const char *filename);

  /**
   * Returns true iff no information content is requested by fmt.
   * \see parse_format_request
   */
  static bool is_empty_format(int fmt);

  /** Sanity check for format bitmasks.
   * \see parse_format_request
   */
  static int sanitize_format(int fmt,
			     int fmt_implied=tiofNone,
			     int fmt_default=tiofNone);

  /**
   * Top-level format-instantiation utility.
   *
   * @param request user-requested format string, has highest priority
   * @param filename filename used to guess basic format flags
   * @param fmt_implied implied format bitmask (required information content)
   * @param fmt_default default format bitmask (for empty formats)
   */
  static int parse_format_request(const char *request,
				  const char *filename=NULL,
				  int fmt_implied=tiofNone,
				  int fmt_default=tiofNone);

  /** Convert a format bitmask to a canonical string form. */
  static std::string format_canonical_string(int fmt);
  //@}

  /*--------------------------------------------------------------------*/
  ///\name Format-Based Reader/Writer Creation
  //@{
  /**
   * Create a new TokenReader object suitable for reading
   * the format @fmt.
   * \fmt should be a bitmask composed of TokenIOFormat flags.
   * Caller is responsible for deleting the object returned.
   */
  static class TokenReader *new_reader(int fmt);

  /**
   * Create a new TokenReader object suitable for reading
   * the format @fmt.
   * @fmt should be a bitmask composed of TokenIOFormat flags.
   * Caller is responsible for delting the object returned.
   */
  static class TokenWriter *new_writer(int fmt);
  //@}
};

/*==========================================================================
 * TokenReader
 *==========================================================================*/

/** \brief Abstract class for token input */
class TokenReader : public TokenIO {
public:
  /** Default size of input buffer. */
  static const size_t TR_DEFAULT_BUFSIZE = 256;

public:
  /** Format flags: bitmask of TokenIO::TokenIOFormat flags */
  int tr_format;

  /** Name of TokenReader subtype. */
  std::string tr_name;

  /** Currently selected input stream (may be NULL) */
  mootio::mistream *tr_istream;

  /** Whether we created tr_istream locally */
  bool tr_istream_created;

  /**
   * Pointer to a real internal 'current token' buffer,
   * used as return value for token() method.
   *
   * Descendant implementations are responsible for
   * allocation, population, manipulation, and destruction
   * of the data it points to.
   */
  mootToken *tr_token;

  /**
   * Pointer to a real internal 'current sentence' buffer,
   * used as return value for sentence() method.
   *
   * Descendant implementations are responsible for
   * allocation, population, maniuplation, and destruction
   * of the data it points to.
   */
  mootSentence *tr_sentence;

public:
  /*------------------------------------------------------------
   * TokenReader: Constructors
   */
  /** \name Constructors etc. */
  //@{
  /**
   * Default constructor
   * \fmt: bitmask of TokenIO::TokenIOFormat flags
   * \mode: default input mode
   * \name: name of current input source
   */
  TokenReader(int                fmt  =tiofUnknown,
	      const std::string &name ="TokenReader")
    : tr_format(fmt),
      tr_name(name),
      tr_istream(NULL),
      tr_istream_created(false),
      tr_token(NULL),
      tr_sentence(NULL)
  {};

  /** Default destructor : override in descendant classes */
  virtual ~TokenReader(void)
  {
    TokenReader::close();
  };

  /**
   * Clear TokenReader-relevant construction buffers, if they exist.
   */
  inline void tr_clear(void)
  {
    if (tr_token) tr_token->clear();
    if (tr_sentence) tr_sentence->clear();
  };
  //@}


  /*------------------------------------------------------------
   * TokenReader: Input Selection
   */
  /** \name Input Selection */
  //@{

  /**
   * Select input from a mootio::mistream pointer.
   * This is the basic case.
   * Descendendant classes may want to override this method.
   */
  virtual void from_mstream(mootio::mistream *mistreamp) {
    close();
    tr_istream = mistreamp;
    byte_number(1);
    line_number(1);
    column_number(0);
    tr_istream_created = false;
  };

  /**
   * Select input from a mootio::mistream object, reference version.
   * Default implementation just calls from_mstream(&mis).
   */
  virtual void from_mstream(mootio::mistream &mis) {
    from_mstream(&mis);
  };

  /**
   * Select input from a named file.
   * Descendants using named file input may override this method.
   * The filename "-" may be used to specify stdin.
   * Default implementation calls from_mstream().
   */
  virtual void from_filename(const char *filename)
  {
    from_mstream(new mootio::mifstream(filename,"rb"));
    tr_istream_created = true;
    if (!tr_istream->valid()) {
      carp("open failed for \"%s\": %s", filename, strerror(errno));
      close();
    }
  };

  /**
   * Select input from a C stream.
   * Caller is responsible for opening and closing the stream.
   * Descendants using C stream input may override this method.
   * Default implementation calls from_mstream().
   */
  virtual void from_file(FILE *file)
  {
    from_mstream(new mootio::micstream(file));
    tr_istream_created = true;
  };

  /**
   * Select input from a file descriptor.
   * Caller is responsible for opening and closing the stream.
   * Descendants using file descriptor input may override this method.
   * No default implementation.
   */
  virtual void from_fd(int fd)
  {
    close();
    throw domain_error("from_fd(): not implemented");
  };

  /**
   * Select input from a C memory-buffer.
   * Caller is responsible for allocation and de-allocation.
   * Descendants using C memory-buffer input may override this method.
   * Default implementation calls from_mstream().
   */
  virtual void from_buffer(const void *buf, size_t len)
  {
    from_mstream(new mootio::micbuffer(buf,len));
    tr_istream_created = true;
  };

  /**
   * Select input from a NUL-terminated C string.
   * Caller is responsible for allocation and de-allocation.
   * Descendants using C string input may override this method.
   * Default implementation calls from_cbuffer(s,len).
   */
  virtual void from_string(const char *s) {
    from_buffer(s,strlen(s));
  };

  /**
   * Select input from a C++ stream.
   * Caller is responsible for allocation and de-allocation.
   * Descendants using C++ stream input may override this method.
   * Default implementation calls from_mstream().
   */
  virtual void from_cxxstream(std::istream &is)
  {
    from_mstream(new mootio::micxxstream(is));
    tr_istream_created = true;
  };

  /**
   * Finish input from currently selected source & perform any required
   * cleanup operations.
   * This method should always be called before selecting a new input source.
   * The current input stream is only closed if it was created locally.
   *
   * Descendants may override this method.
   */
  virtual void close(void) {
    if (tr_istream_created) {
      tr_istream->close();
      if (tr_istream) delete tr_istream;
    }
    tr_istream_created = false;
    tr_istream = NULL;
  };
  //@}

  /*------------------------------------------------------------
   * TokenReader: Token-Level Access
   */
  /** \name Token-Level Access */
  //@{

  /**
   * Get pointer to the current input token.
   * Returns NULL if no token is available.
   *
   * \warning The contents of the token returned may be overwritten
   *          on the next call to any other TokenReader method.
   */
  inline mootToken *token(void) { return tr_token; };

  /**
   * Get a pointer to the current input sentence.
   * Returns NULL if no sentence is available.
   *
   * \warning The contents of the sentence returned may be overwritten
   *          on the next call to any other TokenReader method.
   */
  inline mootSentence *sentence(void) { return tr_sentence; };

  /**
   * Get the next token from the buffer.
   * On completion, current token (if any) is in *tr_token.
   * Descendants \b must override this method.
   */
  virtual mootTokenType get_token(void) {
    throw domain_error("TokenReader: get_token() not implemented");
  };

  /**
   * Read in next sentence.
   * On completion, current sentence (if any) is in *tr_sentence.
   * Descendants may override this method for sentence-wise input.
   */
  virtual mootTokenType get_sentence(void);
  //@}

  /*------------------------------------------------------------
   * TokenReader: Diagnostics
   */
  /** \name Diagnostics */
  //@{
  /** Set reader subtype name to use for diagnostics.
   * Descendants may override this method. */
  virtual void reader_name(const std::string &myname) { tr_name = myname; };

  /** Get current line number. Descendants may override this method. */
  virtual size_t line_number(void) { return 0; };

  /** Set current line number. Descendants may override this method. */
  virtual size_t line_number(size_t n) { return n; };

  /** Get current column number. Descendants may override this method. */
  virtual size_t column_number(void) { return 0; };

  /** Set current column number. Descendants may override this method. */
  virtual size_t column_number(size_t n) { return n; };

  /** Get current byte number. Descendants may override this method. */
  virtual size_t byte_number(void) { return 0; };

  /** Get current byte number. Descendants may override this method. */
  virtual size_t byte_number(size_t n) { return n; };

  /** Complain, giving verbose information */
  virtual void carp(const char *fmt, ...);
  //@}
};


/*------------------------------------------------------------
 * TokenReaderNative
 */
/**
 * \brief Class for native "cooked" text-format token input.
 */
class TokenReaderNative : public TokenReader {
public:
  /*----------------------------------------
   * Reader: Native: Data
   */
  /** The underlying lexer (does the dirty work). */
  mootTokenLexer lexer;

  /** Default construction buffer for get_sentence() */
  mootSentence   trn_sentence;

public:
  /*----------------------------------------
   * Reading: Native: Methods: Constructors
   */
  /** \name Constructors etc. */
  //@{
  /** Default constructor
   * \fmt: bitmask of TokenIOFormat flags.
   * \name: name of input source, for diagnostics.
   */
  TokenReaderNative(int                fmt  =tiofWellDone,
		    const std::string &name ="TokenReaderNative")
    : TokenReader(fmt,name)
  {
    tr_format |= tiofNative;
    input_is_tagged(tr_format&tiofTagged);

    tr_sentence = &trn_sentence;
    tr_token    = &lexer.mtoken_default;

    lexer.to_file(stderr);
  };

  /** Default destructor */
  virtual ~TokenReaderNative(void)
  {
    close();
  };
  //@}

  /*----------------------------------------
   * Reader: Native: Methods: Input Selection
   */
  ///\name Input Selection
  //@{
  /** Select input from a mootio::mstream object. */
  virtual void from_mstream(mootio::mistream *mis);
  //@}


  /*----------------------------------------
   * Reader: Native: Methods: Input
   */
  /** \name Overrides */
  //@{
  virtual mootTokenType get_token(void);
  virtual mootTokenType get_sentence(void);
  //@}


  /*----------------------------------------
   * Reader: Native: Methods: Diagnostics
   */
  /** \name Diagnostics */
  //@{

  /** Get current line number. */
  virtual size_t line_number(void) { return lexer.theLine; };

  /** Set current line number. */
  virtual size_t line_number(size_t n) { return lexer.theLine = n; };

  /** Get current column number. */
  virtual size_t column_number(void) { return lexer.theColumn; };

  /** Set current column number. */
  virtual size_t column_number(size_t n) { return lexer.theColumn = n; };
  //@}


  /*----------------------------------------
   * Reader: Native: Methods: New methods
   */
  /** \name New Methods */
  //@{
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
      tr_format |= tiofTagged;
      lexer.first_analysis_is_best = true;
      lexer.ignore_first_analysis = true;
    } else {
      tr_format &= ~tiofTagged;
      lexer.first_analysis_is_best = false;
      lexer.ignore_first_analysis = false;
    }
    return is_tagged;
  };
  //@}
};


/*==========================================================================
 * TokenWriter
 *==========================================================================*/

/*------------------------------------------------------------
 * TokenWriter
 */
/** \brief Abstract class for token output */
class TokenWriter : public TokenIO {
public:
  /** Format flags: bitmask of TokenIO::TokenIOFormat flags */
  int tw_format;

  /** Name of TokenWriter subtype for diagnostics */
  std::string tw_name;

  /** Pointer to underlying mootio::mostream used for output */
  mootio::mostream *tw_ostream;

  /** Whether we created @tw_ostream ourselves */
  bool tw_ostream_created;

  /** Whether we're in a comment-block */
  bool tw_is_comment_block;

public:
  /*----------------------------------------
   * Writer: Methods
   */
  /** \name Constructors etc. */
  //@{
  /**
   * Default constructor
   * @param fmt output format: should be a bitmask of TokenIO::TokenIOFormat flags 
   * @param name name of TokenWriter subtype for diagnostics
   */
  TokenWriter(int fmt=tiofWellDone,
	      const std::string &name="TokenWriter")
    : tw_format(fmt),
      tw_name(name),
      tw_ostream(NULL),
      tw_ostream_created(false)
  {};

  /** Default destructor */
  virtual ~TokenWriter(void)
  {
    //close();
  };
  //@}

  /*------------------------------------------------------------
   * Writer: Methods: Output Selection
   */
  /** \name Output Selection */
  //@{

  /**
   * Select output to a mootio::mostream object, pointer version.
   * This is the basic case.
   * Descendendant classes may override this method.
   */
  virtual void to_mstream(mootio::mostream *mostreamp) {
    close();
    tw_ostream = mostreamp;
    if (!(tw_format&tiofNull) && (!tw_ostream || !tw_ostream->valid())) {
      carp("Warning: selecting output to invalid stream");
    }
    tw_ostream_created = false;
  };

  /**
   * Select output to a mootio::mistream object, reference version.
   * Default implementation just calls to_mstream(&mos).
   */
  virtual void to_mstream(mootio::mostream &mos) {
    to_mstream(&mos);
  };

  /**
   * Select output to a named file.
   *  Descendants using named file output may override this method.
   * The filename "-" may be used to specify stdout.
   */
  virtual void to_filename(const char *filename)
  {
    to_mstream(new mootio::mofstream(filename,"wb"));
    tw_ostream_created = true;
    if (!tw_ostream->valid()) {
      carp("open failed for \"%s\": %s", filename, strerror(errno));
      close();
    }
  };

  /**
   * Select output to a C stream.
   * Caller is responsible for opening and closing the stream.
   * Descendants using C stream output may override this method.
   * Default implementation calls to_fd().
   */
  virtual void to_file(FILE *file)
  {
    to_mstream(new mootio::mocstream(file));
    tw_ostream_created = true;
  };

  /**
   * Select output to a file descriptor.
   * Caller is responsible for opening and closing the stream.
   * Descendants using file descriptor may override this method.
   * No default implementation.
   */
  virtual void to_fd(int fd)
  {
    close();
    throw domain_error("to_fd(): not implemented.");
  };

  /**
   * Select output to a C++ stream.
   * Caller is responsible for allocation and de-allocation.
   * Descendants using C++ stream input may override this method.
   * Default implementation calls from_mstream().
   */
  virtual void to_cxxstream(std::ostream &os)
  {
    to_mstream(new mootio::mocxxstream(os));
    tw_ostream_created = true;
  };

  /**
   * Finish output to currently selected sink & perform any required
   * cleanup operations.
   * This method should always be called before selecting a new output sink.
   * The current output stream is only closed if it was created locally.
   *
   * Descendants may override this method.
   */
  virtual void close(void) {
    if (tw_is_comment_block) put_comment_block_end();
    if (tw_ostream && tw_ostream_created) {
      tw_ostream->close();
      delete tw_ostream;
    }
    tw_ostream_created = false;	
    tw_ostream = NULL;
  };
  //@}


  /*----------------------------------------*/
  /** \name Output Methods: Token-level */
  //@{
  /**
   * Write a single token to the currently selected output sink.
   * Descendants \b must override this method.
   */
  virtual void put_token(const mootToken &token) {
    throw domain_error("TokenWriter: put_token() not implemented");
  };

  /**
   * Write a single sentence to the currently selected output sink.
   * Descendants may override this method.
   * Default implementation just calls putToken() for every element of sentence.
   */
  virtual void put_sentence(const mootSentence &sentence)
  {
    for (mootSentence::const_iterator si = sentence.begin(); si != sentence.end(); si++)
      put_token(*si);
  };
  //@}

  /*----------------------------------------*/
  /** \name Output Methods: Comments */
  //@{
  /**
   * Begin a comment block.
   * This may effect behavior of subsequent put_raw_*() calls.
   * Descendants using comments should override this method.
   */
  virtual void put_comment_block_begin(void) {
    tw_is_comment_block = true;
  };

  /**
   * End a comment block.
   * This may effect behavior of subsequent put_raw() calls.
   * Descendants using comments should override this method.
   */
  virtual void put_comment_block_end(void) {
    tw_is_comment_block = false;
  };

  /**
   * Write a single comment to the currently selected output sink
   * Descendants may override this method.
   */
  virtual void put_comment_buffer(const char *buf, size_t len) {
    put_comment_block_begin();
    put_raw_buffer(buf,len);
    put_comment_block_end();
  };

  /**
   * Write a single comment to the currently selected output sink
   * Descendants may override this method.
   */
  virtual void put_comment(const char *s) {
    put_comment_buffer(s,strlen(s));
  };

  /**
   * Write a single comment to the currently selected output sink
   * Descendants may override this method.
   */
  virtual void put_comment_buffer(const std::string &s) {
    put_comment_buffer(s.data(),s.size());
  };

  /**
   * Write a comment to the currently selected output sink, printf() style.
   * Descendants may override this method.
   */
  virtual void printf_comment(const char *fmt, ...);
  //@}

  /*----------------------------------------*/
  /** \name Output Methods: Raw */
  //@{
  /**
   * Write some data to the currently selected output sink
   * Descendants may override this method.
   */
  virtual void put_raw_buffer(const char *buf, size_t len)
  {};
  /**
   * Write some data to the currently selected output sink
   * Descendants may override this method.
   */
  virtual void put_raw(const char *s) {
    put_raw_buffer(s,strlen(s));
  };
  /**
   * Write some data to the currently selected output sink
   * Descendants may override this method.
   */
  virtual void put_raw(const std::string &s) {
    put_raw_buffer(s.data(),s.size());
  };

  /**
   * Write some data to the currently selected output sink, printf() style.
   * Descendants may override this method.
   */
  virtual void printf_raw(const char *fmt, ...);
  //@}

  /*----------------------------------------*/
  /** \name Diagnostics */
  //@{
  /** Set writer subtype name to use for diagnostics.
   * Descendants may override this method. */
  virtual void writer_name(const std::string &myname) { tw_name = myname; };

  /** Complain */
  virtual void carp(const char *fmt, ...);
  //@}
};

/*------------------------------------------------------------
 * TokenWriterNative
 */
/**
 * \brief Class for native "cooked" text-format token output.
 */
class TokenWriterNative : public TokenWriter {
public:
  /*----------------------------------------
   * Writer: Native: Data
   */
  /** Temporary buffer for *2string methods */
  mootio::mocbuffer twn_tmpbuf;

public:
  /*----------------------------------------
   * Writer: Native: Methods: construction
   */
  /** \name Constructors etc. */
  //@{
  /** Default constructor */
  TokenWriterNative(int fmt=tiofWellDone,
		    const std::string name="TokenWriterNative")
    : TokenWriter(fmt,name)
  {
    if (! tw_format&tiofNative ) tw_format |= tiofNative;
  };

  /** Default destructor */
  virtual ~TokenWriterNative(void)
  {
    //TokenWriterNative::close();
  };
  //@}

  /*----------------------------------------
   * Writer: Native: Methods: Output Selection
   */
  /** \name Output Selection */
  // @ {

  /*
   * Finish output to currently selected sink & perform any required
   * cleanup operations.
   * Used by named-file interface.
   */
  //virtual void close(void);
  // @ }

  /*----------------------------------------
   * Writer: Native: Methods: Output
   */
  /** \name Overrides */
  //@{
  virtual void put_token(const mootToken &token) {
    _put_token(token,tw_ostream);
  };
  virtual void put_sentence(const mootSentence &sentence) {
    _put_sentence(sentence,tw_ostream);
  };

  virtual void put_raw_buffer(const char *buf, size_t len) {
    _put_raw_buffer(buf,len,tw_ostream);
  };
  //@}

  /*----------------------------------------
   * Writer: Native: Methods: Utilities
   */
  /** \name Output Utilities */
  //@{
  /** Write a single token to a mootio::mostream (with eot marker) */
  void _put_token(const mootToken &token, mootio::mostream *os);

  /** Write a whole sentence to a mootio::mostream (with eos marker) */
  void _put_sentence(const mootSentence &sentence, mootio::mostream *os);

  /** Write a raw comment to a mootio::mostream */
  void _put_comment(const char *buf, size_t len, mootio::mostream *os);

  /** Write some raw data to a mootio::mostream, respecinting @tw_is_comment_block */
  void _put_raw_buffer(const char *buf, size_t len, mootio::mostream *os);

  /** Clear internal buffer and stringify sentence into it,
   * returning result as a new std::string
   */
  inline std::string token2string(const mootToken &token)
  {
    mostream *tw_ostream_old = tw_ostream;
    twn_tmpbuf.clear();
    tw_ostream = &twn_tmpbuf;
    _put_token(token,tw_ostream);
    std::string t2s(twn_tmpbuf.data(), twn_tmpbuf.size());
    tw_ostream = tw_ostream_old;
    return t2s;
  };

  /** Clear internal buffer and stringify sentence into it,
   * returning result as a new std::string
   */
  inline std::string sentence2string(const mootSentence &sentence)
  {
    twn_tmpbuf.clear();
    _put_sentence(sentence,&twn_tmpbuf);
    return std::string(twn_tmpbuf.data(), twn_tmpbuf.size());
  };
  //@}
};

}; /*moot_END_NAMESPACE*/

#endif /* _moot_TOKEN_IO_H */
