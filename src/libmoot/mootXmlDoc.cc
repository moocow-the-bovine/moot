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

/* File: mootXmlDoc.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: C++ wrapper for libxml2 tree-mode XML documents
 */

#include <mootXmlDoc.h>

#ifdef MOOT_LIBXML_ENABLED

namespace moot {
  using namespace std;

  /*----------------------------------------------------
   * Load: Filename
   */
  bool mootXmlDoc::loadFilename(const char *filename_or_url, const char *encoding)
  {
    if (!this->_pre_load_hook()) return false;
    reset();
    srcname = filename_or_url;
    xml_doc = xmlReadFile(filename_or_url, encoding, xml_options);
    if (xml_doc == NULL) {
      carp("readFilename(): failed to parse document `%s'\n",
	   filename_or_url);
      return false;
    }
    return _post_load_hook();
  }

  /*----------------------------------------------------
   * Load: Stream
   */
  bool mootXmlDoc::loadFile(FILE *file,
			    const char *encoding,
			    const char *base_url,
			    const std::string& myname)
  {
    if (!_pre_load_hook()) return false;
    reset();
    if (!myname.empty()) srcname = myname;

    //-- create an input buffer
    xmlParserInputBufferPtr ibuf = xmlParserInputBufferCreateFile(file,
								  xmlParseCharEncoding(encoding));
    if (ibuf == NULL) {
      carp("loadFile(): failed to create input buffer for `%s'\n",
	   srcname.c_str());
      return false;
    }

    //-- read in the document
    xml_doc = xmlReadIO(ibuf->readcallback,
			ibuf->closecallback,
			ibuf->context,
			base_url,
			encoding,
			xml_options);
    if (xml_doc == NULL) {
      carp("loadFile(): failed to parse document `%s'\n",
	   srcname.c_str());
      xmlFreeParserInputBuffer(ibuf);
      return false;
    }

    //--cleanup
    xmlFreeParserInputBuffer(ibuf);

    return _post_load_hook();
  }

  /*----------------------------------------------------
   * Load: Stream
   */
  bool mootXmlDoc::loadBuffer(const char*        buffer, 
			      int                bufsize, 
			      const char*        base_url,
			      const char*        encoding, 
			      const std::string& myname)
  {
    if (!_pre_load_hook()) return false;
    reset();
    if (!myname.empty()) srcname = myname;
    //-- read in the document
    xml_doc = xmlReadMemory(buffer, bufsize, base_url, encoding, xml_options);
    if (xml_doc == NULL) {
      carp("loadBuffer(): failed to parse document `%s'\n",
	   srcname.c_str());
      return false;
    }
    return _post_load_hook();
  };


  /*----------------------------------------------------
   * Save: Filename
   */
  bool mootXmlDoc::saveFilename(const char *filename_or_url,
				const char *encoding,
				int         compressMode)
  {
    if (!_pre_save_hook()) return false;
    if (compressMode >= 0) xmlSetDocCompressMode(document(), compressMode);
    return
      xmlSaveFormatFileEnc(filename_or_url, document(), encoding, xml_format) >= 0
      &&
      _post_save_hook();
  };

  /*----------------------------------------------------
   * Save: File
   */
  bool mootXmlDoc::saveFile(FILE*        file,
			    const char*  encoding)
  {
    if (!_pre_save_hook()) return false;

    if (encoding == NULL && !xml_format) {
      //-- the easy way
      return xmlDocDump(file, document()) >= 0;
    }

    //-- create an encoder
    const char *myEncoding = encoding ? encoding : (const char *)document()->encoding;
    xmlCharEncodingHandlerPtr encoder = xmlFindCharEncodingHandler(myEncoding);
    if (myEncoding && !encoder) {
      carp("saveFile(): failed to create output encoder!\n");
      return false;
    }

    //-- create an output buffer
    xmlOutputBufferPtr obuf = xmlOutputBufferCreateFile(file,encoder);
    if (obuf == NULL) {
      carp("saveFile(): failed to create output buffer!\n");
      return false;
    }
    
    //-- save doc
    bool rc = xmlSaveFormatFileTo(obuf,
				  document(),
				  myEncoding,
				  xml_format) >= 0;

    //-- cleanup
    //xmlOutputBufferClose(obuf);
    //xmlOutputBufferFlush(obuf);
    xmlFree(obuf);

    return rc && _post_save_hook();
  };

  /*----------------------------------------------------
   * Save: Buffer
   */
  bool mootXmlDoc::saveBuffer(xmlChar    **buffer_ptr,
			      int         *buflen,
			      const char  *encoding)
  {
    if (!_pre_save_hook()) return false;
    //-- save doc
    xmlDocDumpFormatMemoryEnc(document(),
			      buffer_ptr,
			      buflen,
			      encoding,
			      xml_format);
    return _post_save_hook();
  };



}; /* namespace moot */

#endif /* MOOT_LIBXML_ENABLED */

