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
 * File: mootXmlParser.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moocow's PoS tagger : expat wrapper
 *--------------------------------------------------------------------------*/

#ifndef _MOOT_XML_PARSER_H
#define _MOOT_XML_PARSER_H

#include <mootConfig.h>

#ifdef MOOT_XML_ENABLED

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#include <expat.h>

/** Default input buffer length for XML parsers */
#define MOOT_DEFAULT_XML_BUFLEN 8192

namespace moot {

/*--------------------------------------------------------------
 * TYPES
 */

  /** C++ Wrapper for expat XML parsers */
  class mootXmlParser {
  public:
    /*----------------------------------------------------
     * mootXmlParser: Data
     */
    /** \name I/O Data */
    //@{
    //-- i/o streams
    FILE   *in;         /**< Current input file */
    FILE   *out;        /**< Current output file (may be \c NULL) */
    char   *srcname;    /**< Symbolic name of input file, for diagnostics */
    //@}

    /** \name Internal Data */
    //@{
    //-- expat stuff
    char       *xml_buf;    /**< Parse buffer for expat parser */
    size_t      xml_buflen; /**< Allocated size of \c xml_buf */
    XML_Parser  parser;     /**< The underlying expat parser object */
    char        lastc;      /**< Last character parsed: we might add a pre-eof newline */
    //@}
 
  public:
    /** \name Constructors / Destructors */
    //@{
    /*----------------------------------------------------
     * mootXmlParser: Constructor
     */
    /** Default constructor:
     *
     * \bufsize: length of parse buffer for expat
     * \encoding: override document input encoding (broken?)
     */
    mootXmlParser(size_t bufsize=MOOT_DEFAULT_XML_BUFLEN, const char *encoding=NULL);

    /** Reset parser state */
    virtual void reset(const char *encoding=NULL);

    /*----------------------------------------------------
     * mootXmlParser: Destructor
     */
    /** Default destructor */
    virtual ~mootXmlParser(void)
    {
      if (xml_buf) {
	free(xml_buf);
	xml_buf = NULL;
	xml_buflen = 0;
      }
      if (parser) {
	XML_ParserFree(parser);
	parser = NULL;
      }
      if (srcname) free(srcname);
    };
    //@}

    /** \name Parsing */
    //@{

    /** Set source name */
    virtual char *setSrcName(const char *myname=NULL)
    {
      if (srcname) free(srcname);
      if (myname) srcname = strdup(myname);
      return srcname;
    };

    /**
     * Parse from a named file, optionally writing output to a named file.
     * You can use "-" as an alias for stdin and stdout, respectively.
     */
    bool parseFile(const char *infilename, const char *outfilename=NULL);

    /**
     * Parse from a C stream, optionally writing output to another C stream.
     */
    bool parseStream(FILE *infile=stdin,
		     FILE *outfile=NULL,
		     const char *in_name=NULL);

    /**
     * Parse from your very own string buffer.
     * \warning This method should not be called incrementally,
     * since the stack is cleared before parsing the buffer.
     * If you need to do incremental buffer parsing, use
     * \c XMLParseBuffer(this->parser, buf, buflen, int is_last_chunk);
     *
     * \buf: buffer to parse
     * \buflen: number of bytes to parse from the buffer
     * \in_name: input-name to use for errors/warnings
     */
    bool parseBuffer(const char *buf, size_t buflen,
		     const char *in_name=NULL);
    //@}

    /** \name Utilities */
    //@{

    /*----------------------------------------------------
     * mootXmlParser: Printing
     */
    /** Print current parser context (in real input encoding) */
    virtual void printContext(FILE *tofile=NULL);

    /*----------------------------------------------------
     * mootXmlParser: Error Reporting
     */
    /** complain */
    virtual void carp(char *fmt, ...);

    /** Complain initiated by expat */
    virtual void xpcarp(char *fmt, ...);
    //@}

    /** \name Handlers */
    //@{
    /*----------------------------------------------------
     * mootXmlParser: Handlers
     */
    /** Handle XML declarations */
    virtual void XmlDeclHandler(const XML_Char  *version,
				const XML_Char  *encoding,
				int             standalone)
    {
      XML_DefaultCurrent(parser);
    };

    /** Handle start elements */
    virtual void StartElementHandler(const char *el, const char **attr)
    {
      XML_DefaultCurrent(parser);
    };

    /** Handle end elements */
    virtual void EndElementHandler(const char *el)
    {
      XML_DefaultCurrent(parser);
    };

    /** Handle character data */
    virtual void CharacterDataHandler(const XML_Char *s, int len)
    {
      XML_DefaultCurrent(parser);
    };

    /** Handle PIs */
    virtual void ProcessingInstructionHandler(const XML_Char *s,
					      const XML_Char *target,
					      const XML_Char *data)
    {
      XML_DefaultCurrent(parser);
    };

    /** Handle comments */
    virtual void CommentHandler(const XML_Char *s)
    {
      XML_DefaultCurrent(parser);
    };


    /** Handle start of CDATA sections */
    virtual void StartCdataSectionHandler(void)
    {
      XML_DefaultCurrent(parser);
    };

    /** Handle end of CDATA sections */
    virtual void EndCdataSectionHandler(void)
    {
      XML_DefaultCurrent(parser);
    };

    /** Handle any other document-internal data (no entity expansion!) */
    virtual void DefaultHandler(const XML_Char *s, int len)
    {};
    //@}

    /** \name Expat Callback Wrappers */
    //@{
    /*----------------------------------------------------
     * mootXmlParser: Expat wrappers (static)
     */
    /* expat callback wrapper */
    static void _xp_XmlDeclHandler(mootXmlParser    *mp,
				   const XML_Char  *version,
				   const XML_Char  *encoding,
				   int             standalone)
    {
      if (mp) mp->XmlDeclHandler(version,encoding,standalone);
    };

    /* expat callback wrapper */
    static void _xp_StartElementHandler(mootXmlParser *mp,
					const char *el,
					const char **attr)
    {
      if (mp) mp->StartElementHandler(el,attr);
    };

    /* expat callback wrapper */
    static void _xp_EndElementHandler(mootXmlParser *mp, const char *el)
    {
      if (mp) mp->EndElementHandler(el);
    };

    /* expat callback wrapper */
    static void _xp_CharacterDataHandler(mootXmlParser *mp,
					 const XML_Char *s,
					 int len)
    {
      if (mp) mp->CharacterDataHandler(s,len);
    };

    /* expat callback wrapper */
    static void _xp_DefaultHandler(mootXmlParser *mp,
				   const XML_Char *s,
				   int len)
    {
      if (mp) mp->DefaultHandler(s,len);
    };
    //@}
  };

}; // moot_END_NAMESPACE

#endif // moot_XML_ENABLED

#endif // MOOT_XML_PARSER_H
