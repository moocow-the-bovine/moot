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
 * File: mootXmlParser.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moocow's PoS tagger : expat wrapper
 *--------------------------------------------------------------------------*/

#ifndef _MOOT_EXPAT_PARSER_H
#define _MOOT_EXPAT_PARSER_H

#include <mootConfig.h>

#ifdef MOOT_EXPAT_ENABLED

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#include <string>

#include <expat.h>

/** Default input buffer length for XML parsers */
#define MOOT_DEFAULT_EXPAT_BUFLEN 8192

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
    mootXmlParser(size_t bufsize=MOOT_DEFAULT_EXPAT_BUFLEN, const char *encoding=NULL);

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
     * Read in and parse the next chunk from current C input stream \c in ,
     * using an internal buffer.  Returns true on success,
     * false on failure.
     *
     * \nbytes: number of bytes read from file will be stored here
     * \is_final: will be 1 if the file ended, 0 otherwise
     *
     * \warning you should have called reset() at least once before
     *          calling this method -- no sanity checks are performed!
     */
    bool parseChunk(int &nbytes, int &is_final);

    /**
     * Parse from your very own string buffer.
     * \warning This method is made to be called incrementally,
     * so the stack is \b NOT cleared before parsing the buffer.
     * If you want to parse a whole buffered document, you
     * should call reset() before calling this method.
     *
     * \buf: buffer to parse
     * \buflen: number of bytes to parse from the buffer
     * \in_name: input-name to use for errors/warnings
     * \is_last_chunk: whether this is the final bit of the document
     */
    bool parseBuffer(const char *buf, size_t buflen, bool is_last_chunk);
    //@}

    /** \name Utilities */
    //@{

    /*----------------------------------------------------
     * mootXmlParser: Printing
     */
    /** Print current parser context (in real input encoding) */
    virtual void printContext(FILE *tofile=NULL);

    /** Get current parser context as a std::string */
    virtual std::string contextString(void);

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

#endif // moot_EXPAT_ENABLED

#endif // MOOT_EXPAT_PARSER_H
