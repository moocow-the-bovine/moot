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

/* File: mootTokenExpatIO.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: XML token I/O for moot
 */

#include <string.h>
#include <errno.h>
#include <mootTokenExpatIO.h>

#ifdef MOOT_EXPAT_ENABLED

moot_BEGIN_NAMESPACE

using namespace std;

/*----------------------------------------------------
 * TokenReaderExpat: Destructor
 */
//TokenReaderExpat::~TokenReaderExpat(void) {};

/*----------------------------------------------------
 * TokenReaderExpat: Reset
 */
void TokenReaderExpat::reset(void)
{
  //-- inherited
  mootExpatParser::reset();

  //-- position information
  stack.clear();
  done = 0;

  //-- construction buffers
  cb_nxtsent.clear();
  cb_nxttok = NULL;
  //cb_nxtanl = NULL;

  //-- output buffers
  cb_fullsents.clear();
  trx_sentbuf.clear();
  tr_token = NULL;
}

/*======================================================================
 * READER METHODS
 *======================================================================*/



/*----------------------------------------------------
 * TokenReaderExpat: close()
 */
void TokenReaderExpat::close(void)
{
  TokenReader::close();
  mootExpatParser::close();
  done = 1;
}


/*----------------------------------------------------
 * TokenReaderExpat: Reader: get_token
 */
mootTokenType TokenReaderExpat::get_token(void)
{
  //-- ensure callback sentbuf has some data
  if (!ensure_cb_fullsents()) return TokTypeEOF;

  if ( tr_token == &(cb_fullsents.front()) ) {
    //-- usual case for token-wise input: pop off first buffered token
    cb_fullsents.pop_front();

    //-- ... and re-check for empty buffer
    if (!ensure_cb_fullsents()) return TokTypeEOF;
  }

  //-- the usual case: return a pointer to the first buffered token
  tr_token = &(cb_fullsents.front());
  return tr_token->toktype();
}

/*----------------------------------------------------
 * TokenReaderExpat: Reader: get_sentence
 */
mootTokenType TokenReaderExpat::get_sentence(void)
{
  //-- disable token-wise input
  tr_token = NULL;

  //-- ensure callback sentbuf has some data
  if (!ensure_cb_fullsents()) return TokTypeEOF;

  //-- fill TokenReader sentence buffer by scanning callback buffer & splicing
  trx_sentbuf.clear();
  mootSentence::iterator si;
  while (trx_sentbuf.empty()) {                                   //-- ignore empty sentences
    for (si = cb_fullsents.begin(); si != cb_fullsents.end() && si->toktype() != TokTypeEOS; si++)
      ;
    trx_sentbuf.splice(trx_sentbuf.begin(), cb_fullsents, cb_fullsents.begin(), si);
    if (si->toktype() == TokTypeEOS) {
      cb_fullsents.pop_front();
      break;
    }
  }
  return TokTypeEOS;
}


/*======================================================================
 * XML UTILITIES
 *======================================================================*/

/*----------------------------------------------------
 * TokenReaderExpat: XML Utilities: ensure_cb_fullsents()
 */
bool TokenReaderExpat::ensure_cb_fullsents(void)
{
  while (!done && cb_fullsents.empty()) {
    int len;
    tr_token = NULL;

    if (!parse_chunk(len,done)) {
      //-- something went wrong: append nxtsent whatever it contains
      done = 1;
      cb_fullsents.splice(cb_fullsents.end(), cb_nxtsent);
      break;
    }
  }
  if (done && !cb_nxtsent.empty()) {
    //-- eof: append nxtsent whatever it contains
    cb_fullsents.splice(cb_fullsents.end(), cb_nxtsent);
  }
  return !cb_fullsents.empty();
}

/*----------------------------------------------------
 * TokenReaderExpat: XML Utilities: save_context_data()
 */
void TokenReaderExpat::save_context_data(const char *text, size_t len,
					 mootTokenType toktype,
					 int info)
{
  if (!info) info = top_node_info();

  std::string *sptr = NULL;
  if (!save_raw_xml && (info & TRX_IsAnalysisD)) {
    //-- analysis-internal XML should be saved even in non-raw mode
    sptr = &(cb_nxttok->tok_analyses.back().details);
  }
  else if (info&TRX_IsTokenD) {
    //-- token-internal raw XML should precede corresponding vanilla token.
    //   : BUT the vanilla token has been push()ed already
    //   : hack it...
    mootSentence::iterator nsi = cb_nxtsent.insert(--cb_nxtsent.end(),
						   mootToken(toktype));
    sptr = &(nsi->tok_text);
  } else {
    //-- easy cases: just push literal to the back of the list 
    cb_nxtsent.push_back(mootToken(toktype));
    sptr = &(cb_nxtsent.back().tok_text);
  }

  //-- actual text-append
  sptr->append(text,len);
}

/*======================================================================
 * XML HANDLERS
 *======================================================================*/

/*----------------------------------------------------
 * TokenReaderExpat: Handlers: xmlDecl
 */
void TokenReaderExpat::XmlDeclHandler(const XML_Char  *version,
				      const XML_Char  *encoding,
				      int             standalone)
{
  save_context();
}

/*----------------------------------------------------
 * TokenReaderExpat: Handlers: startElement
 */
void TokenReaderExpat::StartElementHandler(const char *el, const char **attr)
{
  //-- position tracking
  int info = next_node_info(TRX_IsRoot);
  
  if (body_elt.empty() || body_elt == el) {
    //-- //body
    info |= (TRX_IsBodyE|TRX_IsBodyD);
  }
  if (info & TRX_IsBodyD) {
    //-- //body//
    if (token_elt == el) {
      //-- //body//token
      //   : create a new vanilla token on the input buffer
      info |= (TRX_IsTokenE|TRX_IsTokenD);
      cb_nxtsent.push_back(mootToken(TokTypeVanilla));
      cb_nxttok = &(cb_nxtsent.back());
    }
    else if (info & TRX_IsTokenD) {
      //-- //body//token//
      if (text_elt == el) {
	//-- //body//token//text
	//   : clear token-text
	info |= (TRX_IsTokTextE|TRX_IsTokTextD);
	cb_nxttok->tok_text.clear();
      }
      else if (analysis_elt == el) {
	//-- //body//token//analysis
	//   : add tag-only analysis
	info |= (TRX_IsAnalysisE|TRX_IsAnalysisD);
	for (int i = 0; attr[i]; i += 2) {
	  if (postag_attr == attr[i]) {
	    cb_nxttok->insert(attr[i+1],"");
	    break;
	  }
	}
      }
      else if (besttag_elt == el) {
	//-- //body//token//besttag
	//   : clear 'besttag' text
	cb_nxttok->tok_besttag.clear();
	info |= (TRX_IsBestTagE|TRX_IsBestTagD);
      }
      /*
      else {
	//-- //body//token//
	//info &= ~(TRX_IsTokenE|TRX_IsTokTextE|TRX_IsAnalysisE|TRX_IsBestTagE);
      }
      */
    }
    /*
    else {
      //-- inner-body non-special element
      //-- //body// *
      //info &= ~(TRX_IsTokTextE|TRX_IsAnalysisE|TRX_IsBestTagE|TRX_IsTokenE);
      ;
    }
    */
  }
  /*
  else {
    //-- non-body outer start-elt
    //-- // *
    //info &= ~(TRX_IsBodyE|TRX_IsTokenE);
    ;
  }
  */

  //-- update stack
  stack.push_front(info);

  //-- store raw data
  if (save_raw_xml) save_context(TokTypeXMLRaw,info);
}

/*----------------------------------------------------
 * TokenReaderExpat: Handlers: endElement
 */
void TokenReaderExpat::EndElementHandler(const char *el)
{
  int info = top_node_info();
  mootTokenType typ = TokTypeXMLRaw;

  if (eos_elt == el) {
    //-- EOS
    typ = TokTypeEOS;
  }
  else if (info & TRX_IsTokenE) {
    //-- EOT
    //       : HACK : we want end-element pushed AFTER the vanilla token
    info &= ~TRX_IsTokenD;
  }
  /*
  else if (info & TRX_IsAnalysisE) {
    //-- EOA : append to details
    save_context(TokTypeVanilla, info);
    return;
  }
  */
  /*
  else if (besttag_elt == el) {
    //-- BEST : ignore
    return;
  }
  */

  save_context(typ, info);
  if (!stack.empty()) stack.pop_front();

  if (typ == TokTypeEOS) {
    //-- splice in new full sentence if we've got one
    cb_fullsents.splice(cb_fullsents.end(), cb_nxtsent);
  }
}

/*----------------------------------------------------
 * TokenReaderExpat: Handlers: CharacterData
 */
void TokenReaderExpat::CharacterDataHandler(const XML_Char *s, int len)
{
  int info = top_node_info();

  if (info & TRX_IsTokTextE) {
    //-- token//text
    //   : use UTF-8
    cb_nxttok->textAppend(s, len);
    /*
    //   : use document encoding
    ContextBuffer ctb(parser);
    cb_nxttok->textAppend(ctb.buf, ctb.len);
    */
  }
  else if (info & TRX_IsAnalysisE) {
    //-- token//analysis/TEXT : append to details
    if (!save_raw_xml) {
      cb_nxttok->tok_analyses.back().details.append(s,len);
    }
  }
  else if (info & TRX_IsBestTagE) {
    //-- token//besttag
    cb_nxttok->tok_besttag.append(s, len);  //-- remember best tag, although it will get overwritten
  }

  if (save_raw_xml) save_context(TokTypeXMLRaw, info);
}

/*----------------------------------------------------
 * TokenReaderExpat: Handlers: Comment
 */
void TokenReaderExpat::CommentHandler(const XML_Char *s)
{
  if (save_raw_xml) save_context();
  else {
    //-- avoid embedded newlines
    list<string> cmtlines = moot_strtok(s,"\r\n");
    for (list<string>::const_iterator cli = cmtlines.begin();
	 cli != cmtlines.end();
	 cli++)
      {
	save_context_data(cli->data(),cli->size(),TokTypeComment);
      }
  }
}

/*----------------------------------------------------
 * TokenReaderExpat: Handlers: Default
 */
void TokenReaderExpat::DefaultHandler(const XML_Char *s, int len)
{
  int info = top_node_info();

  if (info & TRX_IsTokTextE) {
    //-- token//text
    cb_nxttok->textAppend(s, len);
  }
  else if (info & TRX_IsAnalysisE) {
    //-- token//analysis/*
    if (!save_raw_xml) {
      cb_nxttok->tok_analyses.back().details.append(s,len);
    }
  };

  if (save_raw_xml) save_context();
}

/*----------------------------------------------------
 * TokenReaderExpat: errors
 */
void TokenReaderExpat::carp(char *fmt, ...)
{
  fprintf(stderr, "moot::TokenReaderExpat: ");
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}


moot_END_NAMESPACE

#endif // moot_EXPAT_ENABLED

/*======================================================================
 * WRITER
 *======================================================================*/

moot_BEGIN_NAMESPACE

/*--------------------------------------------------------------------------
 * TokenWriterExpat: Constructors etc.
 *--------------------------------------------------------------------------*/
TokenWriterExpat::TokenWriterExpat(int                fmt
				   , bool             got_raw_xml
				   , const std::string &encoding
				   )
  : TokenWriter(fmt),
    use_raw_xml(got_raw_xml),
    root_elt("doc"),
    eos_elt("eos"),
    token_elt("token"),
    text_elt("text"),
    analysis_elt("analysis"),
    postag_attr("pos"),
    besttag_elt("moot.tag"),
    lastc(' ')
{
  //-- TokenWriter flags
  if (! tw_format&tiofXML ) tw_format |= tiofXML;

  //-- encoding
  setEncoding(encoding);

  //-- XML conservation
  if (tw_format & tiofConserve) use_raw_xml = true;
}

/*--------------------------------------------------------------------------
 * TokenWriterExpat: Overrides
 *--------------------------------------------------------------------------*/

/*--------------------------------------------------------------
 * TokenWriterExpat: Output Selection: to_mstream()
 */
void TokenWriterExpat::to_mstream(mootio::mostream *os)
{
  TokenWriter::to_mstream(os);
  if (!os || !os->valid() || (tw_format&tiofNull)) return;
  if (!use_raw_xml) {
    os->puts("<?xml version=\"1.0\"");
    if (!twx_encoding.empty()) {
      //-- you asked for it...
      os->printf(" encoding=\"%s\"", twx_encoding.c_str());
    }
    os->printf("?>%s<%s>",
	       (tw_format&tiofPretty ? "\n" : ""),
	       root_elt.c_str());
  }
  lastc = ' ';
}

/*--------------------------------------------------------------------------
 * TokeWriterExpat: Output Selection: close()
 */
void TokenWriterExpat::close(void)
{
  if (!(tw_format&tiofNull) && tw_ostream && tw_ostream->valid()) {

    //-- ensure that we don't close in a comment block
    put_comment_block_end(); 

    //-- print close-tag for document root (gen mode only)
    if (!use_raw_xml) {
      tw_ostream->printf("%s</%s>\n",
			 (tw_format&tiofPretty ? "\n" : ""),
			 root_elt.c_str());
    } else {
      tw_ostream->putc('\n'); //-- always add terminating newline in raw mode
    }
  }

  //-- inherited close ops
  TokenWriter::close();
}


/*--------------------------------------------------------------------------
 * TokenWriterExpat: Utilities
 *--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
 * TokenWriterExpat: Utilities: put_token_file_raw()
 */
void TokenWriterExpat::_put_token_raw(const mootToken &token, mootio::mostream *os)
{
  if (!os || (tw_format&tiofNull) || !os->valid()) return;

  switch(token.toktype()) {
  case TokTypeComment:
    //-- ignore: we should already have raw XML for these
    break;

  case TokTypeVanilla:
  case TokTypeLibXML:
    //-- raw mode: just print besttag
    os->printf("%s<%s>",
	       (tw_format&tiofPretty ? "  " : ""),
	       besttag_elt.c_str());
    twx_recoder.string2mstream(token.besttag(), os);
    os->printf("</%s>%s",
	       besttag_elt.c_str(),
	       (tw_format&tiofPretty ? "\n    " : ""));
    break;
    
  case TokTypeXMLRaw:
    //-- raw XML: pass it on through...
    os->write(token.text().data(), token.text().size());
    break;
      
  case TokTypeEOS:
    os->printf("%s<%s/>",
	       (tw_format&tiofPretty ? "\n  " : ""),
	       eos_elt.c_str());
    break;

  default:
    //-- ignore
    break;
  }
}

/*--------------------------------------------------------------------------
 * TokenWriterExpat: Utilities: put_token_gen()
 */
void TokenWriterExpat::_put_token_gen(const mootToken &token, mootio::mostream *os)
{
  if (!os || (tw_format&tiofNull) || !os->valid()) return;

  mootTokString::const_iterator ti;

  switch(token.toktype()) {

    case TokTypeComment:
    //-- generate XML comments
    if (tw_is_comment_block) {
      //-- embedded comment: handle gracefully
      twx_recoder.string2mstream(token.text(), os);
    } else {
      //-- standalone comment
      if (tw_format&tiofPretty) os->putc('\n');
      os->puts("<!--");
      twx_recoder.string2mstream(token.text(), os);
      os->puts("-->");
    }
    break;

  case TokTypeVanilla:
  case TokTypeLibXML:
    //-- gen mode: token start-element
    if (tw_format&tiofPretty) os->puts("\n  ");
    os->printf("<%s>", token_elt.c_str());

    //-- gen mode: token text
    if (tw_format & tiofText) {
      //-- the easy way:
      /*
      fprintf(out, "%s<%s>%s</%s>",
	      (tw_format&tiofPretty ? "\n    " : ""),
	      text_elt.c_str(),
	      token.text().c_str(),
	      text_elt.c_str());
      */
      //-- the slightly harder, noticeably slower (but slightly more correct) way:
      os->printf("%s<%s>", (tw_format&tiofPretty ? "\n    " : ""), text_elt.c_str());
      twx_recoder.string2mstream(token.text(), os);
      os->printf("</%s>", text_elt.c_str());
    }

    //-- gen mode: analyses
    if (tw_format & tiofAnalyzed) {
      for (mootToken::Analyses::const_iterator ai = token.analyses().begin();
	   ai != token.analyses().end();
	   ai++)
	{
	  if ((tw_format & tiofPruned) && ai->tag != token.besttag()) continue;

	  os->printf("%s<%s %s=\"%s\"",
		     (tw_format&tiofPretty ? "\n    " : ""),
		     analysis_elt.c_str(),
		     postag_attr.c_str(),
		     ai->tag.c_str());

	  if (ai->prob != 0)       os->printf(" %s=\"%g\"", "prob", ai->prob);

	  if (ai->details.empty()) os->puts("/>");
	  else {
	    os->putc('>');
	    twx_recoder.string2mstream(ai->details.c_str(), os);
	    os->printf("</%s>", analysis_elt.c_str());
	  }
	}
    }

    //-- gen mode: besttag
    if (tw_format & tiofTagged) {
      os->printf("%s<%s>%s</%s>",
		 (tw_format&tiofPretty ? "\n    " : ""),
		 besttag_elt.c_str(),
		 token.besttag().c_str(),
		 besttag_elt.c_str());
    }

    //-- gen mode: end-token
    os->printf("%s</%s>",
	       (tw_format&tiofPretty ? "\n  " : ""),
	       token_elt.c_str());

    break;
    
  case TokTypeXMLRaw:
    //-- gen mode: ignore raw XML
    break;
      
  case TokTypeEOS:
    os->printf("%s<%s/>",
	       (tw_format&tiofPretty ? "\n  " : ""),
	       eos_elt.c_str());
    break;

  default:
    //-- ignore
    break;
  }
}

/*--------------------------------------------------------------------------
 * TokenWriterExpat: Utilities: put_sentence()
 */
//(inlined)


/*--------------------------------------------------------------------------
 * TokenWriterExpat: Utilities: Comments: block_begin()
 */
void TokenWriterExpat::_put_comment_block_begin(mootio::mostream *os)
{
  if (!tw_is_comment_block) {
    if (tw_format&tiofPretty) _put_raw_buffer("\n",1,os);
    _put_raw_buffer("<!--",4,os);
  }
  tw_is_comment_block = true;
}

/*--------------------------------------------------------------------------
 * TokenWriterExpat: Utilities: Comments: block_end()
 */
void TokenWriterExpat::_put_comment_block_end(mootio::mostream *os)
{
  if (tw_is_comment_block) {
    _put_raw_buffer("-->",3,os);
  }
  tw_is_comment_block = false;
}

/*--------------------------------------------------------------------------
 * TokenWriterExpat: Utilities: Raw: cbuffer()
 */
void TokenWriterExpat::_put_raw_buffer(const char *buf, size_t len, mootio::mostream *os)
{
  if (!os) os = tw_ostream;
  if (!os || (tw_format&tiofNull) || !os->valid()) return;

  os->write(buf, len);
}


moot_END_NAMESPACE
