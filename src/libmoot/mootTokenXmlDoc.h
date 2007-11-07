/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2005 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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

/* File: mootTokenXmlDoc.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: libxml2 TokenIO layer
 */

#ifndef MOOT_TOKEN_XML_DOC_IO_H

#include <mootXmlDoc.h>
#ifdef MOOT_LIBXML_ENABLED

#warning "moot libxml2 support is and always has been broken: prefer expat"

#include <mootTypes.h>
#include <mootToken.h>
#include <mootTokenIO.h>

moot_BEGIN_NAMESPACE

using namespace std;

/**
 * \brief Class for Token I/O to and/or from in-memory XML trees using libxml2
 *
 * In order to use this class as a TokenReader, a document will
 * have to be loaded first with one of the load*() methods,
 * or created by hand using the libxml2 API.
 */
class TokenXmlDoc : public mootXmlDoc, public TokenReader, public TokenWriter
{
public:
  /*============================================================
   * TokenXmlDoc: Types
   *============================================================*/
  /** Typedef for mapping arbitrary thingies to document nodes */
  //typedef hash_map<void *,xmlNodePtr> Ptr2NodeMap;

public:
  /*============================================================
   * TokenXmlDoc: Data: General
   *============================================================*/
  /** Name of source for diagnostics */
  std::string srcname;

  /*============================================================
   * TokenXmlDoc: Data: Input
   *============================================================*/

  /*----------------------------------------------------*/
  /** \name Input: Defaults */
  //@{
  /** Default parser options */
  const static int defaultParserOptions
      = (0
	 | XML_PARSE_RECOVER  // recover on errors
	 //| XML_PARSE_NOENT    // substitute entities
	 //| XML_PARSE_DTDLOAD  // load external DTD
	 //| XML_PARSE_DTDVALID // validate with the DTD
	 //| XML_PARSE_NOBLANKS // trim "ignorable" whitespace
	 //| XML_PARSE_NONET    // forbid network access
	 | XML_PARSE_NOCDATA  // merge CDATA as text nodes
	 );
  //@}


  /*----------------------------------------------------*/
  /** \name Input: Search Parameters (XPath) */
  //@{
  /** 
   *  XPath query for 'sentence' nodes.
   * \Evaluated relative to document root.
   * \Matches are mapped 1:1 to mootSentence objects.
   * \Default = "//s"
   */
  mootXPathQuery  xpqSentence;

  /**
   * XPath query for 'token' nodes.
   * \Evaluated relative to each 'sentence' node.
   * \Matches are mapped 1:1 to mootToken objects.
   * \Default = "./w"
   */
  mootXPathQuery  xpqToken;

  /**
   * XPath query for token 'text' nodes.
   * \Evaluated relative to each 'token' node.
   * \Matches are concatenated to form 'text' datum of the
   *          corresponding mootToken object.
   * \Default = "./text/text()"
   * \note No whitespace is trimmed from the result(s) of this query!
   */
  mootXPathQuery  xpqText;

  /**
   * XPath query for 'besttag' nodes.
   * \Evaluated relative to each 'token' node.
   * \Matches are concatenated to form 'besttag' datum of the
   *          corresponding mootToken object.
   * \Default = "./moot.tag/text()"
   */
  mootXPathQuery  xpqBesttag;

  /**
   * XPath query for 'analysis' nodes,
   * \Evaluated relative to each 'token' node.
   * \Matches are mapped 1:1 to mootToken::Analysis objects
   *          in the corresponding mootToken object.
   *          The string form of each match will be
   *          used to populate 'details' datum.
   * \Default = ".//analysis"
   */
  mootXPathQuery  xpqAnalysis;

  /**
   * XPath query for 'postag' nodes.
   * \Evaluated relative to each 'analysis' node.
   * \Matches are concatenated to form 'tag' datum of the
   *          corresponding mootToken::Analysis object.
   * \Default = "./@pos"
   */
  mootXPathQuery  xpqPostag;
  //@}

  /*----------------------------------------------------*/
  /** \name Input: Low-level Data */
  //@{
  mootToken            curtok;   ///< current token under construction
  mootSentence         cursent;  ///< current sentence under construction
  mootToken::Analysis  curanal;  ///< current analysis under construction

  //Ptr2NodeMap          ptr2node; ///< maps sentence elements to their nodes
  //@}

  /*============================================================
   * TokenXmlDoc: Data: Output
   *============================================================*/

  /*----------------------------------------------------*/
  /** \name Output: Parameters (Defaults) */
  //@{
  /**
   * Name of document root for output.
   * Only used when writing to an otherwise empty document.
   * Default="doc".
   */
  std::string    outputRootName;
  //@}

  /*----------------------------------------------------*/
  /** \name Output: Low-level Data */
  //@{
  xmlNodePtr      outputNode;  ///< 'current' node for writing of non-XML tokens
  //@}

public:
  /*============================================================
   * TokenXmlDoc: Methods
   *============================================================*/

  /*----------------------------------------------------*/
  /** \name Constructors and Such */
  //@{

  /** Default constructor */
  TokenXmlDoc(int fmt=tiofWellDone)
    :
    //-- general
    srcname(""),
    //-- input search parameters
    xpqSentence("//s",true),
    xpqToken("./w",true),
    xpqText("./text/text()",true),
    xpqBesttag("./moot.tag/text()",true),
    xpqAnalysis(".//analysis",true),
    xpqPostag("./@pos",true),
    //
    //-- low-level input data
    //(empty)
    //
    //-- default output parameters
    outputRootName("doc"),
    outputNode(NULL)
  {
    //-- reader/writer format
    if (! (fmt&tiofXML) ) fmt |= tiofXML;
    tw_format = tr_format = fmt;

    //-- TokenReader pointers
    tr_token = &curtok;
    tr_sentence = &cursent;

    //-- xml options
    xml_options = defaultParserOptions;
    if (fmt & tiofPretty) {
      xml_options |= XML_PARSE_NOBLANKS;
      xml_format = true;
    }
  };

  /** Default destructor */
  virtual ~TokenXmlDoc(void)
  {
    //-- clear TokenReader pointers
    tr_token = NULL;
    tr_sentence = NULL;
  };

  /*----------------------------------------------------
   * mootTokenXmlDocIO: Reset
   */
  /** Reset state: frees document, but does NOT recompile XPath queries */
  virtual void reset(void)
  {
    mootXmlDoc::reset();
    //ptr2node.clear();
  };
  //@}


  /*----------------------------------------------------*/
  /** \name TokenReader Overrides */
  //@{
  /** Clear TokenReader-relevant buffers */
    /*
  virtual void tr_clear(void)
  {
    cursent.clear();
    curtok.clear(); 
    curtok.toktype(TokTypeXML);
    curanal.clear();
  };
    */

  /**
   * Set source name
   */
  virtual void sourceName(const std::string &myname) { srcname = myname; };

  /**
   * Select input from a C stream.  Caller is responsible
   * for opening and closing the stream.
   */
  virtual void fromFile(FILE *file)
  {
    TokenReader::fromFile(file); //-- set tr_format, tr_source
    loadFile(file,NULL,NULL,srcname);
  };

  /**
   * Select input from a C string.
   * Caller is responsible for allocation and de-allocation.
   */
  virtual void fromString(const char *s)
  {
    TokenReader::fromString(s); //-- set tr_format, tr_source
    loadBuffer(s, strlen(s), NULL, NULL, srcname);
  };

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

  /** Complain. */
  virtual void carp(const char *fmt, ...);
  //@}


  /*----------------------------------------------------*/
  /** \name TokenWriter Overrides */
  //@{

  /**
   * Select output to an STL string
   * Caller is responsible for allocation and de-allocation.
   *
   * No output is written as a result of this method:
   * call saveBuffer() for that.
   */
  virtual void toString(std::string &s)
  {
    TokenWriter::toString(s);
  };

  /**
   * Select output to a C stream
   * Caller is responsible for opening and closing the stream.
   *
   * No output is written as a result of this method:
   * call saveFile() for that.
   */
  virtual void toFile(FILE *file)
  {
    tw_format |= tiofFile;
    tw_format &= ~tiofString;
    tw_sink = file;
  };

  /**
   * Write a single token to the document.
   *
   * If \c token is of type TokTypeXML, then its
   * 'user_data' is assumed to be an XMLNodePtr
   * to the token node, and that node is altered
   * directly by modifying the result of 'besttagQuery'
   * Otherwise, token information is
   * written in a default hierarchy under the
   * 'current' output node (outputNode : default=root())
   *
   * Implicitly creates a new document if none is already
   * loaded.
   */
  virtual void put_token(const mootToken &token);

  /**
   * Write a whole sentence to the document.
   * Same caveats as for put_token().
   */
  virtual void put_sentence(const mootSentence &sentence);

  /** put_token() guts for local tokens */
  void put_token_local(const mootToken &token);

  /** put_token() guts for nonlocal tokens */
  void put_token_nonlocal(const mootToken &token);
  //@}


  /*----------------------------------------------------*/
  /** \name XML Hooks and Utilities */
  //@{
  /**
   * Post-load hook evaluates our XPath queries, positioning
   * their pseudo-iterators to the first relevant nodes
   * in the document.
   *
   * If you need to parse a document built in some other
   * fashion, you will need to
   * call this->xpqSentence.eval() and this->xpqToken.eval()
   * yourself.
   */
  virtual bool _post_load_hook(void);

  /** Evaluate a query with error-reporing */
  bool evalQuery(mootXPathQuery &query, xmlNodePtr xml_ctx=NULL);

  /**
   * Check whether we think \c token belongs to our document.
   * Returns true iff \c token is of type
   * TokTypeXML and its (user_data) points to a node of the
   * current document.
   */
  inline bool is_local_token(const mootToken &token) const
  {
    return (token.toktype() == TokTypeXML
	    &&
	    ((const xmlNodePtr)(token.user_data))->doc == xml_doc);
  };

  /**
   * Check whether we think \c sentence belongs to our document.
   * Returns true iff is_local_token() is true for every element
   * of \c sentencel
   * It's ugly and slow, but it should be pretty safe.
   */
  inline bool is_local_sentence(const mootSentence &sentence) const
  {
    for (mootSentence::const_iterator si = sentence.begin();
	 si != sentence.end();
	 si++)
      {
	if (!is_local_token(*si)) return false;
      }
    return true;
  };
  //@}

};

moot_END_NAMESPACE

#endif /* MOOT_LIBXML_ENABLED */

#endif /* MOOT_TOKEN_XML_DOC_IO_H */
