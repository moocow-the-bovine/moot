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

#include "mootXmlParser.h"

#ifdef MOOT_XML_ENABLED

using namespace moot;
using namespace std;

/*----------------------------------------------------
 * mootXmlParser: Constructor
 */
mootXmlParser::mootXmlParser(size_t bufsize, const char *encoding)
{
  //-- set constants
  in = NULL;
  out = NULL;
  srcname = NULL;
  lastc = '\0';
  xml_buflen = bufsize;
  xml_buf = NULL;
  parser = NULL;

  reset(encoding);
}

void mootXmlParser::reset(const char *encoding)
{
  //-- allocate/reset parser
  if (parser) {
    if (!XML_ParserReset(parser, encoding)) {
      carp("Error: XML_ParserReset() failed!\n");
      parser = NULL;
    }
  }
  else {
    parser = XML_ParserCreate(encoding);
    if (!parser) {
      carp("Error: XML_ParserCreate() failed!\n");
      parser = NULL;
    }
  }

  //-- allocate/reset parse buffer
  if (!xml_buf) {
    xml_buf = (char *)malloc(xml_buflen*sizeof(char));
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
			  (XML_XmlDeclHandler)_xp_XmlDeclHandler);
    XML_SetElementHandler(parser,
			  (XML_StartElementHandler)_xp_StartElementHandler,
			  (XML_EndElementHandler)_xp_EndElementHandler);
    XML_SetCharacterDataHandler(parser,
				(XML_CharacterDataHandler)_xp_CharacterDataHandler);
    XML_SetDefaultHandler(parser,
			  (XML_DefaultHandler)_xp_DefaultHandler);
  }
}


/*----------------------------------------------------
 * mootXmlParser: Parsing: Files
 */
bool mootXmlParser::parseFile(const char *infilename, const char *outfilename)
{
  if (strcmp(infilename, "-") == 0) in = stdin;
  else in = fopen(infilename,"r");
  if (!in) {
    carp("open failed for input file `%s': %s\n", infilename, strerror(errno));
    return false;
  }

  if (outfilename) {
    if (strcmp(outfilename, "-") == 0) out = stdout;
    else out = fopen(outfilename,"w");
    if (!out) {
      carp("open failed for output file `%s': %s\n", outfilename, strerror(errno));
      return false;
    }
  }
  else out = NULL;
    
  bool rc = parseStream(in, out, infilename);
    
  if (in != stdin) fclose(in);
  if (out && out != stdout) fclose(out);
    
  return rc;
}


/*----------------------------------------------------
 * mootXmlParser: Parsing: Streams
 */
bool mootXmlParser::parseStream(FILE *infile, FILE *outfile, const char *in_name)
{
  setSrcName(in_name);
  in = infile;
  out = outfile;
    
  //-- sanity check(s)
  if (!in) {
    carp("Error: cannot parse from NULL stream!\n");
    return false;
  }
  else if (!xml_buf) {
    carp("Error: cannot parse to NULL buffer!\n");
    return false;
  }
  else if (!parser) {
    carp("Error: cannot parse with NULL parser!\n");
    return false;
  }

  //-- guts: read to and parse from internal buffer
  reset();
  int done, len;
  do {
    //-- fill 'er up
    len = fread(xml_buf, 1, xml_buflen, infile);
    if (ferror(in)) {
      xpcarp("parseStream(): Read error");
      return false;
    }

    //-- check for eof
    done = feof(in);

    if (!XML_Parse(parser, xml_buf, len, done)) {
      xpcarp("parseStream(): Parse error");
      return false;
    }
  } while (!done);

  //-- maybe add final newline
  if (out && lastc != '\n' && lastc != '\r') fputc('\n', out);

  return true;
}

/*----------------------------------------------------
 * mootXmlParser: Parsing: String buffers
 */
bool mootXmlParser::parseBuffer(const char *buf, size_t buflen, const char *in_name)
{
  setSrcName(in_name);
    
  //-- sanity check(s)
  if (!buf) {
    carp("Error: cannot parse NULL buffer!\n");
    return false;
  }
  else if (!parser) {
    carp("Error: cannot parse with NULL parser!\n");
    return false;
  }

  reset();
  if (!XML_Parse(parser, buf, buflen, 1)) {
    xpcarp("parseBuffer(): Parse error");
    return false;
  }

  return true;
}

/*----------------------------------------------------
 * mootXmlParser: Printing
 */
void mootXmlParser::printContext(FILE *tofile)
{
  if (!tofile) tofile=out;
  if (!tofile) return;
  if (!parser) {
    carp("Error: cannot print context without a parser!\n");
    return;
  }

  int offset, size, bytecount;
  const char *ctx = XML_GetInputContext(parser, &offset, &size);
  bytecount       = XML_GetCurrentByteCount(parser);

  //-- print (literal buffer content)
  if (!ctx || offset < 0) {
    xpcarp("printContext(): Error: buffer overrun!\n");
    return;
  }
  fwrite(ctx+offset, bytecount, 1, tofile);

  //-- remember last character printed
  if (bytecount != 0) lastc = ctx[offset+bytecount-1];
}

/*----------------------------------------------------
 * mootXmlParser: Error Reporting
 */
void mootXmlParser::carp(char *fmt, ...)
{
  fprintf(stderr, "mootXmlParser: ");
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}

void mootXmlParser::xpcarp(char *fmt, ...)
{
  fprintf(stderr, "mootXmlParser: ");
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, " in `%s' at line %d, column %d: %s\n> Context=\"",
	  (srcname ? srcname : "(unknown)"),
	  XML_GetCurrentLineNumber(parser),
	  XML_GetCurrentColumnNumber(parser),
	  XML_ErrorString(XML_GetErrorCode(parser)));
  printContext(stderr);
  fputs("\"\n", stderr);
}

#endif // moot_XML_ENABLED
