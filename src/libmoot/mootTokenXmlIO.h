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
 * File: mootTokenXmlIO.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moocow's PoS tagger : token I/O : XML: Expat
 *--------------------------------------------------------------------------*/

#ifndef _MOOT_TOKEN_EXPAT_IO_H
#define _MOOT_TOKEN_EXPAT_IO_H

#include <mootConfig.h>

#ifdef MOOT_EXPAT_ENABLED

#include <mootTypes.h>
#include <mootToken.h>
#include <mootTokenIO.h>
#include <mootXmlParser.h>

moot_BEGIN_NAMESPACE

/*======================================================================
 * mootTokenXmlIO
 *======================================================================*/

/*--------------------------------------------------------------------------
 * mootTokenXmlReader
 */
/** C++ Wrapper for expat XML parsers */
class TokenReaderXml : public TokenReader, public mootXmlParser {
public:
  /*----------------------------------------------------
   * TokenReaderXml: Types
   */
  /**
   * Enum for parser node-information stack elements:
   * use these constants to create bitmasks.
   */
  typedef enum {
    TRX_Default     = 0x00000000,  ///< nothing special about this node
    TRX_IsOuter     = 0x00000001,  ///< extra-document data
    TRX_IsRoot      = 0x00000002,  ///< document root node
    TRX_IsSentenceE = 0x00000004,  ///< sentence node
    TRX_IsSentenceD = 0x00000008,  ///< (indirect) daughter of a sentence node
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
  const int defaultNodeInheritanceMask
   = TRX_IsSentenceD|TRX_IsTokenD|TRX_IsTokTextD|TRX_IsAnalysisD|TRX_IsBestTagD;

  /** Node information stack: each element is a bitmask of xmlNodeFlags */
  typedef std::list<int> NodeInfoStack;

  /* Type for structured input buffer */
  //typedef std::list<mootSentence> mootSentenceList;

public:
  /*----------------------------------------------------
   * TokenReaderXml: Data
   */
  //----------------------------
  /* I/O Behavior */

  //----------------------------
  /** \name Search Parameters */
  //@{
  std::string sentence_elt;  ///< Name of 'sentence' element. Default="sentence"
  std::string token_elt;     ///< Name of 'token' element (descendant of 'sentence'). Default="token"
  std::string text_elt;      ///< Name of 'text' element (descendant of 'token'). Default="text"
  std::string analysis_elt;  ///< Name of 'token' element (descendant of 'token'). Default="analysis"
  std::string postag_attr;   ///< Name of 'pos tag' attribute (of 'analysis') Default="pos".
  std::string besttag_elt;   ///< Name of 'best tag' element (descendant of 'token'). Default="besttag"
  //@}

  //----------------------------
  /** \name Internal Data */
  //@{
  //-- position tracking
  NodeInfoStack        stack;    ///< Node-information stack
  int                  done;     ///< true iff we've parsed doc to eof

  //-- construction buffers
  mootSentence         nxtsent;  ///< Sentence currently under construction 
  mootToken            nxttok;   ///< Token currently under construction
  mootToken::Analysis  nxtanal;  ///< analysis currently under construction

  //-- output buffers
  mootSentence         isentbuf;   ///< LONG buffer of fully parsed sentences
  mootToken           *otokptr;    ///< pointer to current output token 
  mootSentence         osentbuf;   ///< current output sentence buffer
  //@}
 
 public:
  /** \name Constructors and Such */
  //@{
  /*----------------------------------------------------
   * TokenReaderXml: Constructor
   */
  /** Default constructor:
   * \bufsize: length of parse buffer for expat
   * \encoding: override document encoding (broken?)
   */
  TokenReaderXml(size_t bufsize=MOOT_DEFAULT_EXPAT_BUFLEN, const char *encoding=NULL)
    : //churn_all(true),
      done(1),
      sentence_elt("sentence"),
      token_elt("token"),
      text_elt("text"),
      analysis_elt("analysis"),
      postag_attr("pos"),
      besttag_elt("besttag")
  {};

  /*----------------------------------------------------
   * TokenReaderXml: Destructor
   */
  /** Default destructor */
  virtual ~TokenReaderXml(void);

  /*----------------------------------------------------
   * TokenReaderXml: Reset
   */
  /** Reset parser state */
  virtual reset(void);
  //@}

  /*----------------------------------------------------
   * TokenReaderXml: Reader methods
   */
  /** \name Reader Methods */
  //@{
  /**
   * Get contents of the 'current' token buffer.
   * The contents of this buffer may be overwritten on the next
   * call to any other TokenReader method -- copy it
   * if you need persistence.
   */
  virtual mootToken &token(void) { return isentbuf.front(); }

  /**
   * Get current internal sentence buffer.
   * The contents of this buffer may be overwritten on the next
   * call to any other TokenReader method -- copy it
   * if you need persistence.
   */
  virtual mootSentence &sentence(void) { return osentbuf; }

  /** Clear buffers for token() and sentence() methods (unused) */
  virtual void clear(void) {};

  /**
   * Read in next token, storing information in internal token buffer.
   */
  virtual mootTokenType get_token(void);

  /**
   * Read in next sentence, storing tokens in an
   * internal buffer.  Returns TF_EOF on EOF,
   * otherwise should always return TF_EOS.
   */
  virtual mootTokenType get_sentence(void);
  //@}

  /*----------------------------------------------------
  /** \name XML Utilities */
  //@{

  /*----------------------------------------------------
   * TokenReaderXml: XML Utilities
   */
  /** Try and fill structured sentence buffer by parsing another
   *  chunk of the document.  Returns as for parseChunk.
   */
  inline bool _parseMore(void)
  {
    int len;
    while (!done && isentbuf.empty()) {
      if (!parseChunk(len,done)) {
	//-- something went wrong: add nxtsent whatever it contains
	isentbuf.splice(isentbuf.end(), nxtsent);
	return false;
      }
    }
    if (done && !nxtsent.empty()) {
      //-- eof: add nxtsent whatever it contains
      isentbuf.splice(isentbuf.end(), nxtsent);
    }
    return true;
  };

  /** Get inherited node information for the next node */
  inline int nextNodeInfo(int emptyStackValue=TRX_IsOuter,
			  int inheritanceMask=defaultNodeInheritanceMask)
  {
    return (stack.empty()
	    ? emptyStackValue
	    : (stack.front() & inheritanceMask));
  };

  /** Get node information for the parent node (top of the stack) */
  inline int topNodeInfo(int emptyStackValue=TRX_IsOuter)
  {
    return stack.empty() ? emptyStackValue : stack.front();
  };

  /**
   * Save current parser context as a mootToken to the input sentence buffer.
   */
  virtual void saveContext(mootTokenType toktype=TokTypeLiteral);
  //@}
  
  /*----------------------------------------------------
   * TokenReaderXml: Expat Handlers
   */
  /** \name expat handlers */
  //@{
  virtual void XmlDeclHandler(const XML_Char  *version,
			      const XML_Char  *encoding,
			      int             standalone);
  virtual void StartElementHandler(const char *el, const char **attr);
  virtual void EndElementHandler(const char *el);
  virtual void CharacterDataHandler(const XML_Char *s, int len);
  virtual void DefaultHandler(const XML_Char *s, int len);
  //@}

  /*----------------------------------------------------
  /** \name Error reporting */
  //@{
  /** complain */
  virtual void carp(char *fmt, ...);
  //@}
};

moot_END_NAMESPACE

#endif // moot_EXPAT_ENABLED

#endif // MOOT_EXPAT_TOKEN_IO_H
