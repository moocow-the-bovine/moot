/* -*- Mode: C++; c-basic-offset: 2; -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2014 by Bryan Jurish <moocow@cpan.org>

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

/*--------------------------------------------------------------------------
 * File: mootTokenIO.h
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moocow's PoS tagger : token I/O
 *--------------------------------------------------------------------------*/

/**
\file mootTokenIO.h
\brief Abstract and native classes for I/O of moot::mootToken objects
*/

#ifndef _moot_TOKEN_IO_H
#define _moot_TOKEN_IO_H

#include <mootTokenLexer.h> //-- includes GenericLexer -> BufferIO -> Utils -> CIO -> IO
#include <mootCxxIO.h>

#include <stdexcept>

/*moot_BEGIN_NAMESPACE*/
namespace moot {

//==========================================================================
// Globals

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
  tiofText      = 0x00000080,  ///< literal token text included
  tiofAnalyzed  = 0x00000100,  ///< input is pre-analyzed (>= "medium rare")
  tiofTagged    = 0x00000200,  ///< input is tagged ("medium" or "well done")
  tiofPruned    = 0x00000400,  ///< pruned output
  tiofLocation  = 0x00000800,  ///< locations appear as first non-tag analysis
  tiofCost      = 0x00001000,  ///< parse/output analysis 'prob' field
  tiofTrace     = 0x00002000,  ///< save full Viterbi trellis trace?
  tiofPredict   = 0x00004000,  ///< include Viterbi trellis predictions in trace?
  tiofFlush     = 0x00008000   ///< autoflush output stream after write (native i/o only)?
};
typedef TokenIOFormatE TokenIOFormat;

/** Format alias for 'Cooked Rare' files. */
static const int tiofRare = tiofText;

/** Format alias for 'Cooked Medium Rare' files. */
static const int tiofMediumRare = tiofText|tiofAnalyzed; //|tiofCost

/** Format alias for 'Cooked Medium' files. */
static const int tiofMedium = tiofText|tiofTagged;

/** Format alias for 'Cooked Well Done' files. */
static const int tiofWellDone = tiofText|tiofAnalyzed|tiofTagged; //|tiofCost

//==========================================================================
// TokenIO

/** \brief Abstract class for token I/O */
class TokenIO {
public:
  //--------------------------------------------------------------------
  ///\name Format String <-> Bitmask Utilities
  //@{
  /**
   * Parse a format string, which should be a comma-separated
   * list of moot::TokenIOFormat flag-names (without the 'tiof' prefix,
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
   * @param filename filename used to guess basic format flags (only used if \a request is NULL or empty)
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

  //--------------------------------------------------------------------
  ///\name Format-Based Reader/Writer Creation
  //@{
  /**
   * Create a new TokenReader object suitable for reading the format \p fmt.
   * @param fmt a bitmask composed of moot::TokenIOFormat flags.
   * @warning Caller is responsible for deleting the object returned.
   */
  static class TokenReader *new_reader(int fmt);

  /**
   * Create a new moot::TokenWriter object suitable for writing the format \p fmt.
   * @param fmt a bitmask composed of moot::TokenIOFormat flags.
   * @warning caller is responsible for delting the object returned.
   */
  static class TokenWriter *new_writer(int fmt);

  /**
   * Create a new TokenReader object for reading from the file \a filename.
   * Wrapper for new_reader(parse_format_request(request,filename,fmt_implied,fmt_default)).from_filename(filename)
   * @param filename filename from which to read, will be opened with TokenReader::from_filename()
   * @param fmt_request format request (overrides filename heuristics)
   * @param fmt_implied implied format flags
   * @param fmt_default default format flags
   * @returns new TokenReader for \a filename, or NULL if something went wrong.
   * @warning Caller is responsible for deleting the object returned.
   */
  static class TokenReader *file_reader(const char *filename, const char *fmt_request=NULL, int fmt_implied=tiofNone, int fmt_default=tiofNone);

  /**
   * Create a new TokenWriter object for writing to the file \a filename.
   * Wrapper for new_writer(parse_format_request(request,filename,fmt_implied,fmt_default)).to_filename(filename)
   * @param filename file to be written, will be opened with TokenWriter::to_filename()
   * @param fmt_request format request (overrides filename heuristics)
   * @param fmt_implied implied format flags
   * @param fmt_default default format flags
   * @returns new TokenWriter for \a filename, or NULL if something went wrong.
   * @warning Caller is responsible for deleting the object returned.
   */
  static class TokenWriter *file_writer(const char *filename, const char *fmt_request=NULL, int fmt_implied=tiofNone, int fmt_default=tiofNone);
  //@}

  //--------------------------------------------------------------------
  ///\name Token-Stream Pipeline Utilities
  //@{
  /**
   * Pipes tokens from \a reader to \a writer using reader->get_token() and writer->put_token().
   * \returns number of tokens copied.
   */
  static size_t pipe_tokens(class TokenReader *reader, class TokenWriter *writer);

  /**
   * Pipes sentences from \a reader to \a writer using reader->get_sentence() and writer->put_sentence()
   * \returns number of sentences copied.
   */
  static size_t pipe_sentences(class TokenReader *reader, class TokenWriter *writer);
  //@}
};

//==========================================================================
// TokenReader

/** \brief Abstract class for token input */
class TokenReader : public TokenIO {
public:
  /** Default size of input buffer. */
  static const size_t TR_DEFAULT_BUFSIZE = 256;

public:
  /** Format flags: bitmask of moot::TokenIOFormat flags */
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

  /**
   * User data associated with this reader object.
   * Guaranteed to be unused by internal library routines.
   * Useful e.g. for perl wrappers etc
   */
  void *tr_data;

public:
  /*------------------------------------------------------------
   * TokenReader: Constructors
   */
  /** \name Constructors etc. */
  //@{
  /**
   * Default constructor
   * @param fmt bitmask of moot::TokenIOFormat flags
   * @param name name of current input source
   */
  TokenReader(int                fmt  =tiofUnknown,
	      const std::string &name ="TokenReader")
    : tr_format(fmt),
      tr_name(name),
      tr_istream(NULL),
      tr_istream_created(false),
      tr_token(NULL),
      tr_sentence(NULL),
      tr_data(NULL)
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
  virtual void from_mstream(mootio::mistream *mistreamp)
  {
    this->close();
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
  virtual void from_mstream(mootio::mistream &mis)
  {
    this->from_mstream(&mis);
  };

  /**
   * Select input from a named file.
   * Descendants using named file input may override this method.
   * The filename "-" may be used to specify stdin.
   * Default implementation calls from_mstream().
   */
  virtual void from_filename(const char *filename)
  {
    this->from_mstream(new mootio::mifstream(filename,"rb"));
    tr_istream_created = true;
    if (!tr_istream || !tr_istream->valid()) {
      this->carp("open failed for \"%s\": %s", filename, strerror(errno));
      this->close();
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
    this->from_mstream(new mootio::micstream(file));
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
    this->close();
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
    this->from_mstream(new mootio::micbuffer(buf,len));
    tr_istream_created = true;
  };

  /**
   * Select input from a NUL-terminated C string.
   * Caller is responsible for allocation and de-allocation.
   * Descendants using C string input may override this method.
   * Default implementation calls from_cbuffer(s,len).
   */
  virtual void from_string(const char *s)
  {
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
    this->from_mstream(new mootio::micxxstream(is));
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

  /** Test whether this reader is currently opened.
   *  Default just checks <code>tr_istream && tr_istream->valid()</code>
   */
  virtual bool opened(void)
  {
    return tr_istream!=NULL && tr_istream->valid();
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
  virtual mootio::ByteOffset byte_number(void) { return 0; };

  /** Get current byte number. Descendants may override this method. */
  virtual mootio::ByteOffset byte_number(size_t n) { return n; };

  /** Complain, giving verbose information */
  virtual void carp(const char *fmt, ...);
  //@}
};

//==========================================================================
// TokenReaderNative

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
   * @param fmt bitmask of moot::TokenIOFormat flags.
   * @param name name of input source, for diagnostics.
   */
  TokenReaderNative(int                fmt  =tiofWellDone,
		    const std::string &name ="TokenReaderNative")
    : TokenReader(fmt,name)
  {
    tr_format |= tiofNative;
    input_is_tagged(tr_format&tiofTagged);
    input_has_locations(tr_format&tiofLocation);
    input_has_cost(tr_format&tiofCost);

    tr_sentence = &trn_sentence;
    tr_token    = &lexer.mtoken_default;

    lexer.to_file(stderr);
  };

  /** Default destructor */
  virtual ~TokenReaderNative(void)
  {
    this->close();
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

  /** Get current byte offset. */
  virtual mootio::ByteOffset byte_number(void) { return lexer.theByte; };

  /** Set current byte offset. */
  virtual mootio::ByteOffset byte_number(mootio::ByteOffset n) { return lexer.theByte = n; };
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

  /**
   * Get value of the 'locations' flag : whether we
   * think the input has been marked with 'offset length' pairs.
   */
  inline bool input_has_locations(void)
  {
    return lexer.parse_location;
  };

  /**
   * Set value of the 'locations' flag: whether we
   * think the input has been marked with 'offset length' pairs.
   */
  inline bool input_has_locations(bool has_locs)
  {
    if (has_locs) {
      tr_format |= tiofLocation;
      lexer.parse_location = true;
    } else {
      tr_format &= ~tiofLocation;
      lexer.parse_location = false;
    }
    return has_locs;
  };

  /**
   * Get value of the 'cost' flag : whether we
   * want to parse analysis costs (prob field) from input stream
   */
  inline bool input_has_cost(void)
  {
    return lexer.parse_analysis_cost;
  };

  /**
   * Set value of the 'cost' flag: whether we
   * want to parse analysis costs (prob field) from input stream
   */
  inline bool input_has_cost(bool has_cost)
  {
    if (has_cost) {
      tr_format |= tiofCost;
      lexer.parse_analysis_cost = true;
      lexer.analysis_cost_details = false;
    } else {
      tr_format &= ~tiofCost;
      lexer.parse_analysis_cost = false;
      lexer.analysis_cost_details = true;
    }
    return has_cost;
  };
  //@}
};


//==========================================================================
// TokenWriter

/** \brief Abstract class for token output */
class TokenWriter : public TokenIO {
public:
  /** Format flags: bitmask of moot::TokenIOFormat flags */
  int tw_format;

  /** Name of TokenWriter subtype for diagnostics */
  std::string tw_name;

  /** Pointer to underlying mootio::mostream used for output */
  mootio::mostream *tw_ostream;

  /** Whether we created @p tw_ostream ourselves */
  bool tw_ostream_created;

  /** Whether we're in a comment-block */
  bool tw_is_comment_block;

  /**
   * User data associated with this writer object.
   * Guaranteed to be unused by internal library routines.
   * Useful e.g. for perl wrappers etc
   */
  void *tw_data;

public:
  /*----------------------------------------
   * Writer: Methods
   */
  /** \name Constructors etc. */
  //@{
  /**
   * Default constructor
   * @param fmt output format: should be a bitmask of TokenIOFormatE flags 
   * @param name name of TokenWriter subtype for diagnostics
   */
  TokenWriter(int fmt=tiofWellDone,
	      const std::string &name="TokenWriter")
    : tw_format(fmt),
      tw_name(name),
      tw_ostream(NULL),
      tw_ostream_created(false),
      tw_is_comment_block(false),
      tw_data(NULL)
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
  virtual void to_mstream(mootio::mostream *mostreamp)
  {
    this->close();
    tw_ostream = mostreamp;
    if ( !(tw_format&tiofNull) && (!tw_ostream || !tw_ostream->valid())) {
      this->carp("Warning: selecting output to invalid stream");
    }
    tw_ostream_created = false;
  };

  /**
   * Select output to a mootio::mistream object, reference version.
   * Default implementation just calls to_mstream(&mos).
   */
  virtual void to_mstream(mootio::mostream &mos)
  {
    this->to_mstream(&mos);
  };

  /**
   * Select output to a named file.
   *  Descendants using named file output may override this method.
   * The filename "-" may be used to specify stdout.
   */
  virtual void to_filename(const char *filename)
  {
    this->to_mstream(new mootio::mofstream(filename,"wb"));
    tw_ostream_created = true;
    if (!tw_ostream || !tw_ostream->valid()) {
      this->carp("open failed for \"%s\": %s", filename, strerror(errno));
      this->close();
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
    this->to_mstream(new mootio::mocstream(file));
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
    this->close();
    throw domain_error("to_fd(): not implemented.");
  };

  /**
   * Select output to a C++ stream.
   * Caller is responsible for allocation and de-allocation.
   * Descendants using C++ stream input may override this method.
   * Default implementation calls to_mstream().
   */
  virtual void to_cxxstream(std::ostream &os)
  {
    this->to_mstream(new mootio::mocxxstream(os));
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
    if (tw_is_comment_block) this->put_comment_block_end();
    if (tw_ostream && tw_ostream_created) {
      tw_ostream->close();
      delete tw_ostream;
    }
    tw_ostream_created = false;	
    tw_ostream = NULL;
  };

  /** Test whether this writer is currently opened.
   *  Default just checks <code>tr_istream && tr_istream->valid()</code>
   */
  virtual bool opened(void)
  {
    return tw_ostream!=NULL && tw_ostream->valid();
  };

  /** Flush currently selected output stream (wrapper for tw_ostream->flush()) . */
  virtual bool flush(void)
  {
    return this->opened() && tw_ostream->flush();
  };

  /** Flush stream \a os just in case moot::tiofFlush flag is set; \a os should be valid and non-NULL */
  inline bool autoflush(mootio::mostream *os)
  {
    return (tw_format&tiofFlush)==0 || os->flush();
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
   * Write a single (partial) sentence to the currently selected output sink.
   * Descendants may override this method.
   * Default implementation just calls put_token() for every element of sentence.
   */
  virtual void put_tokens(const mootSentence &tokens)
  {
    for (mootSentence::const_iterator si=tokens.begin(); si!=tokens.end(); si++)
      this->put_token(*si);
  };

  /**
   * Write a single sentence to the currently selected output sink.
   * Descendants may override this method.
   * Default implementation just calls put_sentence().
   */
  virtual void put_sentence(const mootSentence &sentence)
  {
    this->put_tokens(sentence);
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
    this->put_comment_block_begin();
    this->put_raw_buffer(buf,len);
    this->put_comment_block_end();
  };

  /**
   * Write a single comment to the currently selected output sink
   * Descendants may override this method.
   */
  virtual void put_comment(const char *s) {
    this->put_comment_buffer(s,strlen(s));
  };

  /**
   * Write a single comment to the currently selected output sink
   * Descendants may override this method.
   */
  virtual void put_comment_buffer(const std::string &s) {
    this->put_comment_buffer(s.data(),s.size());
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
    this->put_raw_buffer(s,strlen(s));
  };
  /**
   * Write some data to the currently selected output sink
   * Descendants may override this method.
   */
  virtual void put_raw(const std::string &s) {
    this->put_raw_buffer(s.data(),s.size());
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

//==========================================================================
// TokenWriterNative

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
    if (! (tw_format&tiofNative) ) tw_format |= tiofNative;
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
  virtual void put_tokens(const mootSentence &tokens) {
    _put_tokens(tokens,tw_ostream);
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

  /** Write a partial sentence to a mootio::mostream (without eos marker) */
  void _put_tokens(const mootSentence &tokens, mootio::mostream *os);

  /** Write a whole sentence to a mootio::mostream (with eos marker) */
  void _put_sentence(const mootSentence &sentence, mootio::mostream *os);

  /** Write a raw comment to a mootio::mostream */
  void _put_comment(const char *buf, size_t len, mootio::mostream *os);

  /** Write some raw data to a mootio::mostream, respecting \a tw_is_comment_block */
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


//==========================================================================
// TokenBuffer

/**
 * \brief Class for in-memory token buffers using mootSentence
 *
 * \detail Data must be written e.g. with put_token() before it
 *  it is read with get_token() and its ilk, otherwise EOF
 *  will be returned.
 */
class TokenBuffer : public TokenReader, public TokenWriter {
public:
  //------------------------------------------------------------
  // Buffer: typedefs
  typedef mootSentence Buffer;

public:
  //------------------------------------------------------------
  // Buffer: Data
  Buffer tb_buffer;		/**< underlying data buffer */
  mootSentence tb_sentence;	/**< secondary buffer for get_sentence() */

public:
  //------------------------------------------------------------
  /// \name Constructors etc.
  //@{
  /**
   * Default constructor
   * @param fmt bitmask of moot::TokenIOFormat flags
   * @param name symbolic name for the object
   */
  TokenBuffer(int fmt=tiofUnknown, const std::string name="TokenBuffer")
    : TokenReader(fmt,name),
      TokenWriter(fmt,name)
  {
    tr_sentence = &tb_sentence;
  };

  /** destructor override in descendant classes */
  virtual ~TokenBuffer(void);
  //@}

  //------------------------------------------------------------
  /// \name I/O Selection
  //@{
  /** TokenReader method from_mstream() just throws an error */
  virtual void from_mstream(mootio::mistream *mistreamp)
  {
    throw domain_error("from_mstream(): not implemented for class moot::TokenBuffer");
  };

  /** TokenWriter method to_mstream() just throws an error */
  virtual void to_mstream(mootio::mostream *mostreamp)
  {
    throw domain_error("to_mstream(): not implemented for class moot::TokenBuffer");
  };

  /** close() override does nothing */
  virtual void close()
  {};

  /** buffers are always "opened" */
  virtual bool opened()
  { return true; };

  /** Append all remaining tokens from \a reader th buffer */
  virtual void from_reader(TokenReader *reader);

  /** Flush buffer contents to \a writer ; wraps TokenWriter::put_tokens() */
  virtual void to_writer(TokenWriter *writer);

  /** clears internal buffer */
  virtual void clear_buffer();
  //@}

  //------------------------------------------------------------
  /// \name Token-Level Access: Input
  //@{

  /**
   * Get the next token from the buffer.
   * On completion, current token (if any) is in *tr_token.
   */
  virtual mootTokenType get_token(void);

  /**
   * Read in next sentence.
   * On completion, current sentence (if any) is in *tr_sentence.
   * Descendants may override this method for sentence-wise input.
   */
  virtual mootTokenType get_sentence(void);
  //@}

  //------------------------------------------------------------
  /// \name Token-Level Access: Output
  //@{
  /**
   * Write a single token to the currently selected output sink.
   * Local override just appends \a token to \a tb_buffer
   */
  virtual void put_token(const mootToken &token);

  /**
   * Write a single (partial) sentence to the currently selected output sink.
   * Local override just appends \a tokens \a tb_buffer
   */
  virtual void put_tokens(const mootSentence &tokens);

  /**
   * Write a single sentence to the currently selected output sink.
   * Local override appends \a tokens and an EOS-token to \a tb_buffer
   */
  virtual void put_sentence(const mootSentence &tokens);
  //@}

  //------------------------------------------------------------
  /// \name Raw Output
  //@{
  /**
   * Write some data to the currently selected output sink
   * Descendants may override this method.
   */
  virtual void put_raw_buffer(const char *buf, size_t len);
  //@}
};

}; /*moot_END_NAMESPACE*/

#endif /* _moot_TOKEN_IO_H */
