/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2007 by Bryan Jurish <moocow@cpan.org>

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

#include "mootExpatParser.h"
#include <string>

#ifdef MOOT_EXPAT_ENABLED

using namespace std;
using namespace moot;
using namespace mootio;


/*======================================================================
 * class mootExpatParser
 *======================================================================*/

/*----------------------------------------------------
 * mootExpatParser: Constructor
 */
mootExpatParser::mootExpatParser(size_t bufsize, const std::string &encoding)
  : xp_istream(NULL),
    xp_istream_created(false),
    xml_buf(NULL),
    xml_buflen(bufsize),
    xml_encoding(encoding),
    //xml_done(1),
    parser(NULL)
{
  reset();
}

/*----------------------------------------------------
 * mootExpatParser: destructor
 */
mootExpatParser::~mootExpatParser(void)
{
  this->close();
  if (xml_buf) {
    free(xml_buf);
    xml_buf = NULL;
    xml_buflen = 0;
  }
  if (parser) {
    XML_ParserFree(parser);
    parser = NULL;
  }
}

/*----------------------------------------------------
 * mootExpatParser: reset
 */
void mootExpatParser::reset(void)
{
  //-- get encoding
  const char *encoding = xml_encoding.empty() ? NULL : xml_encoding.c_str();

  //-- reset 'done' flag
  //xml_done = 0;

  //-- allocate/reset parser
  if (parser) {
    if (!XML_ParserReset(parser, encoding)) {
      carp("Error: XML_ParserReset(p,%s) failed!\n",
	   encoding ? encoding : "(default-encoding)");
      //XML_ParserFree(parser);
      //parser = NULL;
    }
  }
  else {
    parser = XML_ParserCreate(encoding);
    if (!parser) {
      carp("Error: XML_ParserCreate(%s) failed!\n",
	   encoding ? encoding : "(default-encoding)");
    }
  }

  //-- allocate/reset parse buffer
  if (!xml_buf) {
    xml_buf = reinterpret_cast<char *>(malloc(xml_buflen*sizeof(char)));
    //xml_buf = (char *)XML_GetBuffer(parser, xml_buflen*sizeof(char));
    if (!xml_buf) {
      carp("Error: could not allocate parser buffer!\n");
      xml_buf = NULL;
      xml_buflen = 0;
    }
  }

  //-- setup parser: user-data
  if (parser) {
    XML_SetUserData(parser, this);

    //-- setup parser: set handlers
    XML_SetXmlDeclHandler(parser,
			  reinterpret_cast<XML_XmlDeclHandler>(_xp_XmlDeclHandler));
    XML_SetElementHandler(parser,
			  reinterpret_cast<XML_StartElementHandler>(_xp_StartElementHandler),
			  reinterpret_cast<XML_EndElementHandler>(_xp_EndElementHandler));
    XML_SetCharacterDataHandler(parser,
				reinterpret_cast<XML_CharacterDataHandler>(_xp_CharacterDataHandler));
    XML_SetCommentHandler(parser,
			  reinterpret_cast<XML_CommentHandler>(_xp_CommentHandler));
    XML_SetDefaultHandler(parser,
			  reinterpret_cast<XML_DefaultHandler>(_xp_DefaultHandler));
  }
}

/*----------------------------------------------------
 * mootExpatParser: Input Selection: from_mstream()
 */
void mootExpatParser::from_mstream(mootio::mistream *mistreamp, bool autoclose)
{
  if (autoclose) this->close();
  xp_istream = mistreamp;
  xp_istream_created = false;
};

/*----------------------------------------------------
 * mootExpatParser: Input Selection: from_filename()
 */
void mootExpatParser::from_filename(const char *filename) {
  from_mstream(new mootio::mifstream(filename,"rb"));
  xp_istream_created = true;
  if (!xp_istream->valid()) {
    carp("open failed for \"%s\": %s", filename, strerror(errno));
    this->close();
  }
};

/*----------------------------------------------------
 * mootExpatParser: Input Selection: from_file()
 */
void mootExpatParser::from_file(FILE *file)
{
  from_mstream(new mootio::micstream(file));
  xp_istream_created = true;
};

/*----------------------------------------------------
 * mootExpatParser: Input Selection: from_fd()
 */
//nyi

/*----------------------------------------------------
 * mootExpatParser: Input Selection: from_buffer()
 */
void mootExpatParser::from_buffer(const void *buf, size_t len)
{
  from_mstream(new mootio::micbuffer(buf,len));
  xp_istream_created = true;
}

/*----------------------------------------------------
 * mootExpatParser: Input Selection: from_cxxstream
 */
void mootExpatParser::from_cxxstream(std::istream &is)
{
  from_mstream(new mootio::micxxstream(is));
  xp_istream_created = true;
};


/*----------------------------------------------------
 * mootExpatParser: Input Selection: close
 */
void mootExpatParser::close(void) {
  if (xp_istream && xp_istream_created) {
    xp_istream->close();
    delete xp_istream;
  }
  xp_istream_created = false;	
  xp_istream = NULL;
  //xml_done = 1;
};


/*----------------------------------------------------
 * mootExpatParser: Parsing: sanity check
 */
bool mootExpatParser::parse_check(void)
{
  //-- sanity check(s)
  if (!xp_istream || !xp_istream->valid()) {
    carp("Error: cannot parse from invalid stream!\n");
    return false;
  }
  else if (!xml_buf) {
    carp("Error: cannot parse to a NULL buffer!\n");
    return false;
  }
  else if (!parser) {
    carp("Error: cannot parse from a NULL parser!\n");
    return false;
  }

  return true;
};

/*----------------------------------------------------
 * mootExpatParser: Parsing: Filenames
 */
bool mootExpatParser::parse_filename(const std::string &filename)
{
  from_filename(filename.c_str());
  return parse_check() && parse_all();
}

/*----------------------------------------------------
 * mootExpatParser: Parsing: C FILE*s
 */
bool mootExpatParser::parse_file(FILE *infile, const std::string &in_name)
{
  from_file(infile);
  return parse_check() && parse_all();
}

/*----------------------------------------------------
 * mootExpatParser: Parsing: buffers
 */
bool mootExpatParser::parse_buffer(const char *buf, size_t buflen)
{
  from_buffer(buf,buflen);
  return parse_check() && parse_all();
}

/*----------------------------------------------------
 * mootExpatParser: Parsing: all
 */
bool mootExpatParser::parse_all(mootio::mistream *in)
{
  //-- guts: read to and parse from internal buffer
  reset();
  int len  = 0;
  int done = 0;
  do {
    if (!parse_chunk(len,done,in)) return false;
  } while (!done);

  in->close();
  return true;
}

/*----------------------------------------------------
 * mootExpatParser: Parsing: Chunks
 */
bool mootExpatParser::parse_chunk(int &nbytes, int &is_final, mootio::mistream *in)
{
  //-- minimal sanity check
  if (!in) in = xp_istream;
  if (!in) return false;
  
  //-- fill 'er up
  nbytes = in->read(xml_buf, xml_buflen);
  if (!in->valid()) {
    xpcarp("parse_chunk(): Read error");
    return false;
  }

  //-- check for eof
  is_final = in->eof();

  //-- ye olde expatte guttes
  if (!XML_Parse(parser, xml_buf, nbytes, is_final)) {
    xpcarp("parse_chunk(): Parse error");
    return false;
  }

  return true;
}

/*----------------------------------------------------
 * mootExpatParser: Context
 */
void mootExpatParser::context_dump(FILE *tofile)
{
  if (!tofile) {
    carp("Error: cannot print parser context to NULL file!\n");
    return;
  }
  if (!parser) {
    carp("Error: cannot print context without a parser!\n");
    return;
  }
  ContextBuffer ctb(parser);
  fwrite(ctb.data(), ctb.size(), 1, tofile);
}

/*----------------------------------------------------
 * mootExpatParser: Error Reporting
 */
void mootExpatParser::carp(const char *fmt, ...)
{
  fprintf(stderr, "mootExpatParser: ");
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}

void mootExpatParser::xpcarp(const char *fmt, ...)
{
  fprintf(stderr, "mootExpatParser: ");
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, " in `%s' at line %lu, column %lu: %s\n",
	  (xp_istream && !xp_istream->name.empty()
	   ? xp_istream->name.c_str() : "(unknown)"),
	  static_cast<long unsigned int>(XML_GetCurrentLineNumber(parser)),
	  static_cast<long unsigned int>(XML_GetCurrentColumnNumber(parser)),
	  XML_ErrorString(XML_GetErrorCode(parser)));
}

#endif // MOOT_EXPAT_ENABLED
