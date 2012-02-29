/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2009 by Bryan Jurish <moocow@cpan.org>

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
 * File: mootExpatParser.h (formerly mootXmlParser.h)
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moocow's PoS tagger : expat wrapper
 *--------------------------------------------------------------------------*/

/**
\file mootExpatParser.h
\brief C++ wrapper class for generic \c expat XML parsers (optional)
*/

#ifndef _MOOT_EXPAT_PARSER_H
#define _MOOT_EXPAT_PARSER_H

#include <mootConfig.h>

#ifdef MOOT_EXPAT_ENABLED

#include <expat.h>

#include <stdexcept>

#include <mootCIO.h>
#include <mootCxxIO.h>
#include <mootBufferIO.h>



/** Default input buffer length for XML parsers */
#define MOOT_DEFAULT_EXPAT_BUFLEN 8192

namespace moot {

/*--------------------------------------------------------------
 * TYPES
 */

  /** \brief C++ Wrapper for expat XML parsers */
  class mootExpatParser {
  public:
    /*----------------------------------------------------
     * mootExpatParser: ContextBuffer
     */

    /**
     * \brief Utility class for expat input contexts.
     *
     * Useful class for shoveling around literal input-contexts,
     * in document-original encoding.
     * Only constructable and valid within a single callback.
     */
    class ContextBuffer : public mootio::micbuffer {
    public:
      /** Default constructor */
      ContextBuffer(const char *buffer, size_t buflen)
	: mootio::micbuffer(buffer, buflen)
      {};

      /** Constructor from an expat parser: can only be called during callback routines */
      ContextBuffer(XML_Parser parser)
	: mootio::micbuffer(NULL,0)
      {
	assert(parser != NULL);
	int cb_offset_i, cb_used_i;
	cb_rdata  = XML_GetInputContext(parser, &cb_offset_i, &cb_used_i);
	cb_rdata += cb_offset_i;
	cb_offset = 0;
	cb_used   = XML_GetCurrentByteCount(parser);
      };
      ~ContextBuffer(void) {};

    }; //-- /class mootExpatParser::ContextBuffer

  public:
    /*----------------------------------------------------
     * mootExpatParser: Data
     */
    /** \name I/O Data */
    //@{
    //-- i/o streams
    mootio::mistream *xp_istream;         ///< Current input stream
    bool              xp_istream_created; ///< whether input stream \p mis was created locally
    //@}

    /** \name Internal Data */
    //@{
    //-- expat stuff
    char              *xml_buf;      ///< Parse buffer for expat parser
    size_t             xml_buflen;   ///< Allocated size of \p xml_buf
    std::string        xml_encoding; ///< Input encoding override (goofy)
    //int                xml_done;     ///< whether we're done parsing yet
    XML_Parser         parser;       ///< The underlying expat parser object
    //@}
 
  public:
    /** \name Constructors etc. */
    //@{
    /*----------------------------------------------------
     * mootExpatParser: Constructor
     */
    /** Default constructor:
     *
     * @param bufsize length of parse buffer for expat
     * @param encoding override document input encoding (broken?)
     */
    mootExpatParser(size_t bufsize=MOOT_DEFAULT_EXPAT_BUFLEN,
		    const std::string &encoding="");

    /** Override document encoding. Implicitly reset()s the parser. */
    virtual void setEncoding(const std::string &encoding="")
    {
      xml_encoding = encoding;
      reset();
    };

    /**
     * Reset parser state: re-initializes the internal xml buffer,
     * as well as the expat parser, its encoding and handlers.
     */
    virtual void reset(void);

    /*----------------------------------------------------
     * mootExpatParser: Destructor
     */
    /** Default destructor */
    virtual ~mootExpatParser(void);
    //@}

    /*----------------------------------------------------*/
    /** \name Input Selection */
    //@{

    /**
     * Select input from a mootio::mistream pointer.
     * This is the basic case.
     * Descendant classes may override this method.
     */
    virtual void from_mstream(mootio::mistream *mistreamp);

    /**
     * Select input from a mootio::mistream reference.
     * This is the basic case.
     * Descendant classes may override this method.
     */
    virtual void from_mstream(mootio::mistream &mistream) { from_mstream(&mistream); };

    /**
     * Select input from a named file.
     * Descendants using named file input may override this method.
     * The filename "-" may be used to specify stdin.
     * Default implementation calls from_mstream().
     */
    virtual void from_filename(const char *filename);

    /**
     * Select input from a C stream.
     * Caller is responsible for opening and closing the stream.
     * Descendants using C stream input may override this method.
     * Default implementation calls from_mstream().
     */
    virtual void from_file(FILE *file);

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
    virtual void from_buffer(const void *buf, size_t len);

    /**
     * Select input from a NUL-terminated C string.
     * Caller is responsible for allocation and de-allocation.
     * Descendants using C string input may override this method.
     * Default implementation calls from_cbuffer(s,len).
     */
    virtual void from_string(const char *s) { from_buffer(s,strlen(s)); };

    /**
     * Select input from a C++ stream.
     * Caller is responsible for allocation and de-allocation.
     * Descendants using C++ stream input may override this method.
     * Default implementation calls from_mstream().
     */
    virtual void from_cxxstream(std::istream &is);

    /**
     * Finish input from currently selected source & perform any required
     * cleanup operations.  This method should always be called
     * before selecting a new input source.
     *
     * Descendants may override this method.
     */
    virtual void close(void);
    //@}

    /*----------------------------------------------------*/
    /** \name Parsing */
    //@{
    /**
     * Check whether object is in a sane state for parsing,
     * printing warning messages if this is not the case
     */
    virtual bool parse_check(void);

    /** Parse an entire named file */
    bool parse_filename(const std::string &filename);

    /** Parse a C stream until EOF */
    bool parse_file(FILE *infile=stdin, const std::string &in_name="");

    /**
     * Parse from your very own string buffer.
     * \warning This method is not intended to be called incrementally,
     * so the stack is cleared before parsing the buffer.
     * If you want to parse incrementally, use parse_chunk()
     *
     * \param buf buffer to parse
     * \param buflen number of bytes to parse from the buffer
     */
    bool parse_buffer(const char *buf, size_t buflen);

    /** Parse the entirety of the stream \p in, which
     * defaults to the currently selected input stream.
     * Only minimal sanity checks are performed.
     */
    bool parse_all(mootio::mistream *in=NULL);

    /**
     * Read in and parse the next chunk from input source \p in
     * using an internal buffer.  Returns true on success,
     * false on failure.
     *
     * @param nbytes number of bytes read from file will be stored here
     * @param is_final will be 1 if the file ended, 0 otherwise
     * @param in input stream to read from (default is to use current selection)
     *
     * \warning you should have called reset() at least once before
     *          calling this method -- no sanity checks are performed!
     */
    bool parse_chunk(int &nbytes, int &is_final, mootio::mistream *in=NULL);
    //@}

    /*----------------------------------------------------*/
    /** \name Utilities */
    //@{

    /*----------------------------------------------------
     * mootExpatParser: Printing and Context
     */
    /** Print current parser context (in real input encoding) */
    virtual void context_dump(FILE *tofile=NULL);

    /** Get current parser context as a std::string */
    virtual std::string context_string(void)
    {
      return parser ? ContextBuffer(parser).as_string() : std::string("");
    };

    /*----------------------------------------------------
     * mootExpatParser: Error Reporting
     */
    /** complain */
    virtual void carp(const char *fmt, ...);

    /** Complain initiated by expat */
    virtual void xpcarp(const char *fmt, ...);

    /** Get current line number of expat parser */
    inline long int line_number(void)
    { return XML_GetCurrentLineNumber(parser); }

    /** Get current column number of expat parser */
    inline int column_number(void)
    { return XML_GetCurrentColumnNumber(parser); }

    /** Get current byte offset of expat parser */
    inline long byte_offset(void)
    { return XML_GetCurrentByteIndex(parser); }

    /** Get byte-count for current event of expat parser*/
    inline int byte_count(void)
    { return XML_GetCurrentByteCount(parser); }
    //@}

    /** \name Handlers */
    //@{
    /*----------------------------------------------------
     * mootExpatParser: Handlers
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
     * mootExpatParser: Expat wrappers (static)
     */
    /* expat callback wrapper */
    static void _xp_XmlDeclHandler(mootExpatParser    *mp,
				   const XML_Char  *version,
				   const XML_Char  *encoding,
				   int             standalone)
    {
      if (mp) mp->XmlDeclHandler(version,encoding,standalone);
    };

    /* expat callback wrapper */
    static void _xp_StartElementHandler(mootExpatParser *mp,
					const char *el,
					const char **attr)
    {
      if (mp) mp->StartElementHandler(el,attr);
    };

    /* expat callback wrapper */
    static void _xp_EndElementHandler(mootExpatParser *mp, const char *el)
    {
      if (mp) mp->EndElementHandler(el);
    };

    /* expat callback wrapper */
    static void _xp_CharacterDataHandler(mootExpatParser *mp,
					 const XML_Char *s,
					 int len)
    {
      if (mp) mp->CharacterDataHandler(s,len);
    };

    /* expat callback wrapper */
    static void _xp_CommentHandler(mootExpatParser *mp,
				   const XML_Char *s)
    {
      if (mp) mp->CommentHandler(s);
    };

    /* expat callback wrapper */
    static void _xp_DefaultHandler(mootExpatParser *mp,
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
