/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2007 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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
 * File: mootTokenExpatIO.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moocow's PoS tagger : token I/O : XML: Expat
 *--------------------------------------------------------------------------*/

#ifndef _MOOT_TOKEN_EXPAT_IO_H
#define _MOOT_TOKEN_EXPAT_IO_H

#include <mootConfig.h>

#ifdef MOOT_EXPAT_ENABLED

#include <assert.h>

#include <mootTypes.h>
#include <mootToken.h>
#include <mootTokenIO.h>
#include <mootExpatParser.h>
#include <mootRecode.h>

#ifdef __GNUC__
# include <ext/slist>
#else
# include <slist>
#endif

#include <list>

/*#define MOOT_DEBUG_EXPAT*/

moot_BEGIN_NAMESPACE

using namespace std;

/*======================================================================
 * mootTokenExpatIO
 *======================================================================*/

/*--------------------------------------------------------------------------
 * TokenReaderExpat
 */
/** \brief Experimental XML reader class using expat. */
class TokenReaderExpat : public TokenReader, public mootExpatParser {
public:
  /*----------------------------------------------------
   * TokenReaderExpat: Types
   */
  /**
   * Enum for parser node-information stack elements:
   * use these constants to create bitmasks.
   */
  typedef enum {
    TRX_Default     = 0x00000000,  ///< nothing special about this node
    TRX_IsOuter     = 0x00000001,  ///< extra-document data
    TRX_IsRoot      = 0x00000002,  ///< document root node
    TRX_IsBodyE     = 0x00000004,  ///< body node
    TRX_IsBodyD     = 0x00000008,  ///< (indirect) daughter of a body node
    TRX_IsTokenE    = 0x00000010,  ///< token node
    TRX_IsTokenD    = 0x00000020,  ///< (indirect) daughter of a token node
    TRX_IsTokTextE  = 0x00000040,  ///< token-text node
    TRX_IsTokTextD  = 0x00000080,  ///< (indirect) daughter of a token-text node
    TRX_IsAnalysisE = 0x00000100,  ///< token-analysis node
    TRX_IsAnalysisD = 0x00000200,  ///< (indirect) daughter of an analysis node
    TRX_IsBestTagE  = 0x00000400,  ///< 'best tag' node
    TRX_IsBestTagD  = 0x00000800,  ///< (indirect) daughter of a 'best tag' node
    TRX_All         = 0xffffffff   ///< all possible flags (useful for masking)
  } xmlNodeFlags;

  /** Default node-inheritance flags */
  const static int defaultNodeInheritanceMask
   = TRX_IsBodyD|TRX_IsTokenD|TRX_IsTokTextD|TRX_IsAnalysisD|TRX_IsBestTagD;

  /** Node information stack: each element is a bitmask of xmlNodeFlags */
  //typedef list<int> NodeInfoStack;
  typedef slist<int> NodeInfoStack;

public:
  /*----------------------------------------------------
   * TokenReaderExpat: Data
   */
  //----------------------------
  /* I/O Behavior */

  //----------------------------
  /** \name Search Parameters */
  //@{
  bool        save_raw_xml;  ///< whether to store raw XML along with 'normal' tokens (default=false)

  std::string body_elt;      ///< Name of 'body' element. Default="" (tokenize everything)
  std::string eos_elt;       ///< Name of 'eos' (end-)element (sentence boundary). Default="eos"
  std::string token_elt;     ///< Name of 'token' element. Default="token"
  std::string text_elt;      ///< Name of 'text' element (descendant of 'token'). Default="text"
  std::string analysis_elt;  ///< Name of 'analysis' element (descendant of 'token'). Default="analysis"
  std::string postag_attr;   ///< Name of 'pos tag' attribute (of 'analysis' elt) Default="pos".
  std::string besttag_elt;   ///< Name of 'best tag' element (descendant of 'token'). Default="moot.tag"
  //@}

  //----------------------------
  /** \name Internal Data */
  //@{
  //-- position tracking
  NodeInfoStack         stack;      ///< Node-information stack
  int                   done;       ///< true iff we've parsed doc to eof

  //-- construction buffers
  mootSentence          cb_nxtsent; ///< Sentence construction buffer for expat callbacks
  mootToken            *cb_nxttok;  ///< Construction buffer for tokens (points into nxtsent)

  //-- output buffers
  mootSentence         cb_fullsents; ///< LONG buffer of fully parsed sentences (for expat callbacks)
  mootSentence         trx_sentbuf;  ///< current output sentence buffer (for TokenReader interface)
  //@}
 
 public:
  /*----------------------------------------------------
   * TokenReaderExpat: Constructor
   */
  /** \name Constructors and Such */
  //@{

  /** Default constructor:
   * \bufsize: length of parse buffer for expat
   * \encoding: override document encoding (broken?)
   */
  TokenReaderExpat(int                fmt      =tiofXML,
		   size_t             buflen   =MOOT_DEFAULT_EXPAT_BUFLEN,
		   //size_t           buflen   =128, //-- DEBUG
		   const std::string &encoding ="",
		   const std::string &name     ="TokenReaderExpat")
    : TokenReader(fmt,name),
      mootExpatParser(buflen,encoding),
      save_raw_xml(false),
      body_elt(""),
      eos_elt("eos"),
      token_elt("token"),
      text_elt("text"),
      analysis_elt("analysis"),
      postag_attr("pos"),
      besttag_elt("moot.tag"),
      done(1)
  {
    //-- TokenReader pointers
    tr_sentence = &trx_sentbuf;
    tr_token    = NULL;

    save_raw_xml = tr_format & tiofConserve;
  };

  /*----------------------------------------------------
   * TokenReaderExpat: Destructor
   */
  /** Default destructor */
  virtual ~TokenReaderExpat(void) {};

  /*----------------------------------------------------
   * TokenReaderExpat: Reset
   */
  /** Reset parser state */
  virtual void reset(void);
  //@}

  /*----------------------------------------------------*/
  /** \name TokenReader Overrides : Input Selection     */
  //@{

  /**
   * Declare subtype name to use for diagnostics.
   */
  virtual void reader_name(const std::string &myname)
  {
    TokenReader::reader_name(myname);
    //mootExpatParser::setSrcName(myname);
  };

  /** Close currently selected input source. */
  virtual void close(void);

  virtual void from_mstream(mootio::mistream *mistreamp) {
    TokenReader::from_mstream(mistreamp);
    mootExpatParser::from_mstream(tr_istream);
    done = 0;
  };
  virtual void from_mstream(mootio::mistream &mis) {
    TokenReader::from_mstream(mis);
    mootExpatParser::from_mstream(tr_istream);
    done = 0;
  };
  virtual void from_filename(const char *filename) {
    TokenReader::from_filename(filename);
    mootExpatParser::from_mstream(tr_istream);
  };
  virtual void from_file(FILE *infile) {
    TokenReader::from_file(infile);
    mootExpatParser::from_mstream(tr_istream);
  };
  virtual void from_fd(int fd) {
    TokenReader::from_fd(fd);
    mootExpatParser::from_mstream(tr_istream);
  };
  virtual void from_buffer(const void *buf, size_t len) {
    TokenReader::from_buffer(buf,len);
    mootExpatParser::from_mstream(tr_istream);
  };
  virtual void from_cxxstream(std::istream &is) {
    TokenReader::from_cxxstream(is);
    mootExpatParser::from_mstream(tr_istream);
  };
  //@}

  /*----------------------------------------------------*/
  /// \name TokenReader Overrides : Input
  //@{

  /**
   * Get the next token from the buffer.
   * On completion, current token (if any) is in *tr_token.
   */
  virtual mootTokenType get_token(void);

  /**
   * Read in next sentence.
   * On completion, current sentence (if any) is in *tr_sentence.
   */
  virtual mootTokenType get_sentence(void);
  //@}

  /*----------------------------------------------------*/
  /** \name XML Utilities */
  //@{

  /*----------------------------------------------------
   * TokenReaderExpat: XML Utilities
   */
  /**
   * Ensure that there is some data in the callback sentence buffer,
   * possibly parsing another chunk of the document.
   * If more data is read, tr_token is reset to NULL.
   * 
   * Returns false iff no more data is available in cb_fullsents.
   */
  bool ensure_cb_fullsents(void);

  /** Predict node information for the next node by inheritance-masking */
  inline int next_node_info(int emptyStackValue=TRX_IsOuter,
			    int inheritanceMask=defaultNodeInheritanceMask)
  {
    return (stack.empty()
	    ? emptyStackValue
	    : (stack.front() & inheritanceMask));
  };

  /** Get node information for the parent node (top of the stack) */
  inline int top_node_info(int emptyStackValue=TRX_IsOuter)
  {
    return stack.empty() ? emptyStackValue : stack.front();
  };


#ifdef MOOT_DEBUG_EXPAT
  /**
   * Save current parser context as a mootToken to the callback sentence buffer.
   */
  void save_context(mootTokenType toktype=TokTypeXMLRaw, int info=0);

  /**
   * Save a mootToken to the callback sentence buffer
   */
  void save_context_data(const mootio::micbuffer &buf, mootTokenType toktype=TokTypeXMLRaw, int info=0);

#else

  /**
   * Save current parser context as a mootToken to the callback sentence buffer.
   */
  inline void save_context(mootTokenType toktype=TokTypeXMLRaw, int info=0)
  {
    if (!save_raw_xml && toktype == TokTypeXMLRaw) return;
    if (!info) info = top_node_info();
    ContextBuffer ctb(parser);
    save_context_data(ctb, toktype, info);
  };

  /**
   * Save a mootToken to the callback sentence buffer, micbuffer version
   */
  inline void save_context_data(const mootio::micbuffer &buf,
				mootTokenType toktype=TokTypeXMLRaw,
				int info=0)
  {
    save_context_data(buf.cb_rdata + buf.cb_offset,
		      buf.cb_used  - buf.cb_offset,
		      toktype, info);
  };
#endif /* MOOT_DEBUG_EXPAT */

  /**
   * Save a mootToken to the callback sentence buffer, string version
   */
  void save_context_data(const char *text, size_t len,
			 mootTokenType toktype=TokTypeXMLRaw,
			 int info=0);
  //@}
  
  /*----------------------------------------------------
   * TokenReaderExpat: Expat Handlers
   */
  /** \name expat handlers */
  //@{
  virtual void XmlDeclHandler(const XML_Char  *version,
			      const XML_Char  *encoding,
			      int             standalone);
  virtual void StartElementHandler(const char *el, const char **attr);
  virtual void EndElementHandler(const char *el);
  virtual void CharacterDataHandler(const XML_Char *s, int len);
  virtual void CommentHandler(const XML_Char *s);
  virtual void DefaultHandler(const XML_Char *s, int len);
  //@}

  /*----------------------------------------------------*/
  /** \name Error reporting */
  //@{
  /** Get current line number. */
  virtual size_t line_number(void) {
    return parser ? static_cast<size_t>(XML_GetCurrentLineNumber(parser)) : 0;
  };

  /** Set current line number -- not implemented. */
  virtual size_t line_number(size_t n) { return line_number(); };

  /** Get current column number. */
  virtual size_t column_number(void) {
      return parser ? static_cast<size_t>(XML_GetCurrentLineNumber(parser)) : 0;
  };

  /** Set current column number. */
  virtual size_t column_number(size_t n) { return column_number(); };

  /** Get current byte number. */
  virtual mootio::ByteOffset byte_number(void) {
      return parser ? static_cast<mootio::ByteOffset>(XML_GetCurrentByteIndex(parser)) : 0;
  };

  /** Set current byte number. */
  virtual mootio::ByteOffset byte_number(mootio::ByteOffset n) { return byte_number(); };

  /** complain */
  virtual void carp(char *fmt, ...);
  //@}
};

moot_END_NAMESPACE

/*#endif // moot_EXPAT_ENABLED*/


moot_BEGIN_NAMESPACE

/*======================================================================
 * WRITER
 *======================================================================*/

/*--------------------------------------------------------------------------
 * TokenWriterExpat
 */
/**
 * \brief Experimental XML writer class for use with
 *        expat-parsed XML or vanilla input.
 */
class TokenWriterExpat : public TokenWriter {
public:
  /*----------------------------------------------------
   * TokenWriterExpat: Data
   */
  //----------------------------
  /* I/O Behavior */

  //----------------------------
  /** \name Output Parameters */
  //@{

  /**
   * Whether the data to write contains raw XML as generated by TokenReaderExpat
   * with \c save_raw_xml=true.  Useful for lossless XML I/O.
   *
   * \warning
   * If this flag is set, arguments to put_* methods must provide
   * tokens of type TokTypeXMLRaw for all document content except
   * the following:
   *
   * \li sentence boundary markers (TokTypeEOS)
   * \li 'vanilla' tokens (TokTypeVanilla)
   *     In 'use_raw_xml' mode, only the 'besttag' element will be written for
   *     'vanilla' tokens -- in other words, TokTypeXMLRaw tokens are expected
   *     for token start- and end- elements, as well as for text, analyses, etc.,
   *     although no checking is performed for the presence of such elements.
   *
   * If the flag is false (the default), a document will be generated
   * with a default structure (see root_elt, eos_elt, token_elt, etc.),
   * which should be compatible with the default behavior of
   * TokenReaderExpat.
   */
  bool        use_raw_xml;

  std::string root_elt;      ///< Default name of root element. Default="doc"
  std::string eos_elt;       ///< Default name of 'eos' element (sentence boundary). Default="eos"
  std::string token_elt;     ///< Default name of 'token' element. Default="token"
  std::string text_elt;      ///< Default name of 'text' element. Default="text"
  std::string analysis_elt;  ///< Default name of 'analysis' element. Default="analysis"
  std::string postag_attr;   ///< Default name of 'pos' attribute (of 'analysis' elt) Default="pos".

  std::string besttag_elt;   ///< Name of 'best tag' element. Default="moot.tag"
  //@}

  //----------------------------
  /** \name Internal Data */
  //@{
  /** Name of destination encoding.  See setEncoding() */
  std::string    twx_encoding;

  /** Recoder object: handles text (re-)coding */
  mootXMLRecoder twx_recoder;

  /** Last character written */
  int lastc;
  //@}
 
 public:
  /*----------------------------------------------------
   * TokenReaderExpat: Constructor
   */
  /** \name Constructors and Such */
  //@{

  /**
   * Default constructor:
   * \fmt: output format, a bitmask of TokenIOFormat flags
   * \got_raw_xml: whether this object is part of a lossless
   *               XML I/O chain: useful but cryptic --
   *               see \c use_raw_xml for details.
   * \encoding: destination encoding.  still somewhat buggy.
   */
  TokenWriterExpat(int                   fmt         =tiofXML
		   , bool                got_raw_xml =false
		   , const std::string  &encoding    =""
		   );

  /*----------------------------------------------------
   * TokenWriterExpat: encoding
   */
  /** Set default output encoding.  Still somewhat buggy */
  inline void setEncoding(const std::string &encoding="")
  {
    twx_encoding = encoding;
    twx_recoder.scan_request("UTF-8", (twx_encoding.empty()
				       ? "XML-standalone"
				       : twx_encoding));
  };


  /*----------------------------------------------------
   * TokenWriterExpat: Destructor
   */
  /** Default destructor */
  virtual ~TokenWriterExpat(void)
  {
    close();
  };

  /*----------------------------------------
   * Writer: Expat: Methods: Output Selection
   */
  /** \name Output Selection */
  //@{
  /** Select output to a mootio::mostream */
  virtual void to_mstream(mootio::mostream *os);

  /** Close currently selected output sink */
  virtual void close(void);
  //@}

  /*----------------------------------------
   * Writer: Expat: Methods: Output
   */
  /** \name Overrides */
  //@{
  /** Write a single token to the current output sink. */
  virtual void put_token(const mootToken &token) {
    _put_token(token,tw_ostream);
  };

  /** Write a whole sentence to the current output sink. */
  virtual void put_sentence(const mootSentence &sentence) {
    _put_sentence(sentence,tw_ostream);
  };

  /** Begin a comment block. */
  virtual void put_comment_block_begin(void) {
    _put_comment_block_begin(tw_ostream);
  };

  /** End a comment block, if one is active. */
  virtual void put_comment_block_end(void) {
    _put_comment_block_end(tw_ostream);
  };

  /** Write some raw data to the current sink.  No recoding is performed. */
  virtual void put_raw_buffer(const char *buf, size_t len) {
    _put_raw_buffer(buf,len,tw_ostream);
  };
  //@}

  /*----------------------------------------
   * Writer: Expat: Methods: Utilities
   */
  /** \name Output Utilities */

  /** Write a single token to a mootio::mostream, raw mode */
  void _put_token_raw(const mootToken &token, mootio::mostream *os);

  /** Write a single token to a mootio::mostream, gen mode */
  void _put_token_gen(const mootToken &token, mootio::mostream *os);

  /** Write a single token to a mootio::mostream */
  inline void _put_token(const mootToken &token, mootio::mostream *os)
  {
    if (use_raw_xml)  _put_token_raw(token,os);
    else              _put_token_gen(token,os);
  };

  /** Write a single sentence to a C stream, obeying use_raw_xml flag */
  inline void _put_sentence(const mootSentence &sentence, mootio::mostream *os)
  {
    if (!os || (tw_format&tiofNone) || !os->valid()) return;
    mootSentence::const_iterator si;
    if (use_raw_xml) {
      for (si = sentence.begin(); si != sentence.end(); si++) _put_token_raw(*si, os);
    } else {
      for (si = sentence.begin(); si != sentence.end(); si++) _put_token_gen(*si, os);
      _put_token_gen(mootToken(TokTypeEOS), os);
    }
  };

  /** Begin a comment block. */
  void _put_comment_block_begin(mootio::mostream *os);

  /** End a comment block, if one is active. */
  void _put_comment_block_end(mootio::mostream *os);

  /** Write some raw data to the current sink.  No recoding is performed. */
  void _put_raw_buffer(const char *buf, size_t len, mootio::mostream *os);
  //@}
};

moot_END_NAMESPACE

#endif // moot_EXPAT_ENABLED

#endif // MOOT_EXPAT_TOKEN_IO_H
