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

/* File: mootTokenXmlIO.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: XML token I/O for moot
 */

#include <mootTokenXmlIO.h>

#ifdef MOOT_XML_ENABLED

moot_BEGIN_NAMESPACE

using namespace std;

/*----------------------------------------------------
 * TokenReaderXml: Destructor
 */
TokenReaderXml::~TokenReaderXml(void)
{
  /*
  //-- low-level data
  stack.clear();
  sents.clear();
  nxtsent.clear();
  nxttok.clear();
  nxtanal.clear();

  //-- search parameters
  sentence_elt.clear();
  token_elt.clear();
  text_elt.clear();
  analyis_elt.clear();
  postag_attr.clear();
  besttag_elt.clear();
  */
}

/*----------------------------------------------------
 * TokenReaderXml: Reset
 */
void TokenReaderXml::Reset(void)
{
  //-- inherited
  mootXmlParser::reset();

  //-- position information
  stack.clear();
  done = 0;

  //-- construction buffers
  nxtsent.clear();
  nxttok.clear();
  nxtanal.clear();

  //-- output buffers
  isentbuf.clear();
  osentbuf.clear();
  otokptr = NULL;
}

/*======================================================================
 * READER METHODS
 *======================================================================*/

/*----------------------------------------------------
 * TokenReaderXml: Reader: get_token
 */
virtual mootTokenType TokenXmlReader::get_token(void)
{
  if (!_parseMore() || isentbuf.empty()) {
    otokptr = NULL;
    return TokTypeEOF;
  }
  if (otokptr != &isentbuf.front()) isentbuf.pop_front(); //-- simulate native read (hack)
  otokptr = &isentbuf.front();
  return isentbuf.front().toktype();
}

/*----------------------------------------------------
 * TokenReaderXml: Reader: get_sentence
 */
virtual mootTokenType TokenXmlReader::get_sentence(void)
{
  otokptr = NULL;
  if (!_parseMore() || isentbuf.empty()) return TokTypeEOF;

  //-- fill sentence buffer
  osentbuf.clear();
  mootSentence::const_iterator si;
  while (osentbuf.empty()) {
    for (si = isentbuf.begin(); si != isentbuf.end(); si++) {
      if (si->toktype != TokTypeEOS) {
	si++;
	break;
      }
    }
    osentbuf.splice(osentbuf.begin(), isentbuf.begin(), si);
  }
  return TokTypeEOS;
}


/*======================================================================
 * XML UTILTIES
 *======================================================================*/
void TokenReaderXml::saveContext(mootTokenType toktype)
{
  int offset, size, bytecount;
  const char *ctx = XML_GetInputContext(parser, &offset, &size);
  bytecount       = XML_GetCurrentByteCount(parser);

  //-- print (literal buffer content)
  if (!ctx || offset < 0) {
    xpcarp("saveContext(): Error: buffer overrun!\n");
    return;
  }
  isentbuf.push_back(mootToken(mootTokString(ctx+offset, bytecount), toktype));
};

/*======================================================================
 * XML HANDLERS
 *======================================================================*/

/*----------------------------------------------------
 * TokenReaderXml: Handlers: xmlDecl
 */
void TokenReaderXml::XmlDeclHandler(const XML_Char  *version,
				    const XML_Char  *encoding,
				    int             standalone)
{
  saveContext();
}

/*----------------------------------------------------
 * TokenReaderXml: Handlers: startElement
 */
void StartElementHandler(const char *el, const char **attr)
{
  //-- position tracking
  int info = nextNodeInfo(TRX_IsRoot);
  
  if (sentence_elt == el) {
    //-- //sentence
    info |= (TRX_IsSentenceE|TRX_IsSentenceD);
  }
  else if (info & TRX_IsSentenceD) {
    //-- //sentence//
    if (token_elt == el) {
      info |= (TRX_IsTokenE|TRX_IsTokenD);
    }
    else if (info & TRX_IsTokenD) {
      //-- //sentence//token//
      if (text_elt == el) {
	//-- //sentence//token//text
	info |= (TRX_IsTextE|TRX_IsTextD);
      }
      else if (analysis_elt == el) {
	//-- //sentence//token//analysis
	info |= (TRX_IsAnalysisE|TRX_IsAnalysisD);
	//-- : get pos attribute
	int i;
	for (i = 0; attr[i]; i += 2) {
	  if (postag_attr == attr[i]) {
	    nxtanl.tag = attr[i+1];
	    break;
	  }
	}
      }
      else if (besttag_elt == el) {
	//-- //sentence//token//besttag
	info |= (TRX_IsBestTagE|TRX_IsBesttagD);
      }
      else {
	//-- //sentence//token//
	//info &= ~(TRX_IsTokenE|TRX_IsTextE|TRX_IsAnalysisE|TRX_IsBestTagE);
	;
      }
    }
    else {
      //-- inner-sentence non-special element
      //-- //sentence//*
      //info &= ~(TRX_IsTextE|TRX_IsAnalysisE|TRX_IsBestTagE|TRX_IsTokenE);
      ;
    }
  }
  else {
    //-- non-sentence outer start-elt
    //-- //*
    //info &= ~(TRX_IsSentenceE|TRX_IsTokenE);
    ;
  }

  //-- update stack
  stack.push_front(info);

  //-- store data
  saveContext();
}

/*----------------------------------------------------
 * TokenReaderXml: Handlers: endElement
 */
void TokenReaderXml::EndElementHandler(const char *el)
{
  int info = topNodeInfo();
  mootTokenType typ = TokTypeLiteral;

  if (info & TRX_IsSentenceE) {
    //-- EOS
    typ = TokenTypeEOS;
  }
  else if (info & TRX_IsTokenE) {
    //-- EOT
    nxttok.toktype(TokTypeXML);
    isentbuf.push_back(nxttok); //-- push literal token before we push close-elt!
    nxttok.clear();
  }
  else if (info & TRX_IsAnalysisE) {
    //-- EOA
    nxttok.insert(nxtanl);
    nxtanl.clear(); 
  }
  else if (besttag_elt == el) {
    //-- BEST : ignore
    return;
  }

  if (!stack.empty()) stack.pop_front();
  saveContext(typ);
}

/*----------------------------------------------------
 * TokenReaderXml: Handlers: CharacterData
 */
void TokenReaderXml::CharacterDataHandler(const XML_Char *s, int len)
{
  int info = topNodeInfo();

  if (info & TRX_IsTextE) {
    //-- token//text
    nxttok.textAppend(s, len);
  }
  else if (TRX_IsAnalysisE) {
    //-- token//analysis
    nxtanl.details.append(s, len);
  }
  else if (TRX_IsBestTagE) {
    //-- token//besttag
    nxtanl.besttag.append(s, len);  //-- remember best tag, although it will get overwritten
  }

  saveContext();
}

/*----------------------------------------------------
 * TokenReaderXml: Handlers: Default
 */
void TokenReaderXml::DefaultHandler(const XML_Char *s, int len)
{
  //int info = nextNodeInfo();
  saveContext();
}

/*----------------------------------------------------
 * TokenReaderXml: errors
 */
void TokenReaderXml::carp(char *fmt, ...)
{
  fprintf(stderr, "moot::TokenReaderXml: ");
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}



moot_END_NAMESPACE

#endif // moot_XML_ENABLED
