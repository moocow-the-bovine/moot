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

/* File: mootTokenXmlDoc.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: libxml2 TokenIO layer
 */

#include <mootTokenXmlDoc.h>
#ifdef MOOT_LIBXML_ENABLED

#warning "\
mootTokenXmlDoc:\
 libxml2 XML support is HIGHLY EXPERIMENTAL and likely BROKEN.\
 Use at your own risk.\
"

moot_BEGIN_NAMESPACE
using namespace std;


/*======================================================================
 * TokenReader Overrides
 *======================================================================*/

/*----------------------------------------------------------------------
 * TokenXmlDoc: get_token()
 */
mootTokenType TokenXmlDoc::get_token(void)
{
  xmlNode *nod;     // iterator node
  xmlChar *content; // temporary

  //-- clear current token
  curtok.clear();
  curtok.user_data = NULL;
  curtok.toktype(TokTypeXML);

  //-- get current sentence node
  xmlNodePtr senti = xpqSentence.cur();
  if (!senti) return TokTypeEOF;

  //-- get current token node (if any)
  xmlNodePtr toki = xpqToken.cur();
  if (!toki) {
    //-- EOS : increment sentence & token queries
    evalQuery(xpqToken, xpqSentence.next());;
    xpqToken.first();
    return TokTypeEOS;
  }

  //-- 'normal' token: user_data
  curtok.user_data = toki;

  //-- 'normal' token: text
  if (evalQuery(xpqText, toki)) {
    for (nod = xpqText.first(); nod; nod = xpqText.next()) {
      content = xmlNodeGetContent(nod);
      if (content) {
	curtok.textAppend((const char *)content);
	xmlFree(content);
      }
    }
  }

  //-- 'normal' token: besttag
  if (evalQuery(xpqBesttag, toki)) {
    for (nod = xpqBesttag.first(); nod; nod = xpqBesttag.next()) {
      content = xmlNodeGetContent(nod);
      if (content) {
	curtok.besttagAppend((const char *)content);
	xmlFree(content);
      }
    }
  }

  //-- 'normal' token: analyses
  xmlNode *anali;
  if (evalQuery(xpqAnalysis, toki)) {
    for (anali = xpqAnalysis.first(); anali; anali = xpqAnalysis.next()) {
      curanal.clear();

      //-- token: analysis: details
      content = xmlNodeGetContent(anali);
      if (content) {
	curanal.details = (const char *)content;
	xmlFree(content);
      }

      //-- token: analysis: tag
      if (evalQuery(xpqPostag, anali)) {
	for (nod = xpqPostag.first(); nod; nod = xpqPostag.next()) {
	  content = xmlNodeGetContent(nod);
	  if (content) {
	    curanal.tag.append((const char *)content);
	    xmlFree(content);
	  }
	}
      }
 
      //-- token: analysis: cost (TODO)

      //-- token: analysis: insert
      if (!curanal.empty()) curtok.insert(curanal);
    }
  } //-- end token: analysis

  //-- increment token iterator
  xpqToken.next();

  return TokTypeXML;
}

/*----------------------------------------------------------------------
 * TokenXmlDoc: get_sentence()
 */
mootTokenType TokenXmlDoc::get_sentence(void)
{
  int typ;
  cursent.clear();
  for (typ = get_token(); typ != TokTypeEOS && typ != TokTypeEOF; typ = get_token())
    {
      cursent.push_back(curtok);
    }
  return (mootTokenType)typ;
}

/*----------------------------------------------------------------------
 * TokenXmlDoc: carp()
 */
void TokenXmlDoc::carp(const char *fmt, ...)
{
  fprintf(stderr, "moot::TokenXmlDoc: ");
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}

/*----------------------------------------------------------------------
 * TokenXmlDoc: evalQuery(query)
 */
bool TokenXmlDoc::evalQuery(mootXPathQuery &query, xmlNodePtr xml_ctx)
{
  //-- set context
  if (!query.eval(document(), (xml_ctx ? xml_ctx : root()), true, false)) {
    carp("could not evaluate XPath query \"%s\"\n", query.expr.c_str());
    return false;
  }
  return true;
}


/*----------------------------------------------------------------------
 * TokenXmlDoc: _post_load_hook()
 */
bool TokenXmlDoc::_post_load_hook(void)
{
  return
    evalQuery      ( xpqSentence ,               root()  )
      && evalQuery ( xpqToken    , xpqSentence . first() )
    /*
    && evalQuery ( xpqText     , xpqToken    . first() )
    && evalQuery ( xpqBesttag  , xpqToken    . first() )
    && evalQuery ( xpqAnalysis , xpqToken    . first() )
    && evalQuery ( xpqPostag   , xpqAnalysis . first() )
    */
    ;
}


/*======================================================================
 * TokenWriter Overrides
 *======================================================================*/

/*----------------------------------------------------------------------
 * TokenXmlDoc: is_internal_sentence(const mootSentence &sent)
 */


/*----------------------------------------------------------------------
 * TokenXmlDoc: put_token()
 */
void TokenXmlDoc::put_token(const mootToken &token)
{
  if (is_local_token(token))
    put_token_local(token);
  else
    put_token_nonlocal(token);
}

void TokenXmlDoc::put_token_local(const mootToken &token)
{
  xmlNodePtr toki = ((xmlNodePtr)(token.user_data));
  xmlNodePtr nod;

  //-- local token : re-execute the 'besttag' query
  if (evalQuery(xpqBesttag, toki) && (nod = xpqBesttag.first()) != NULL) {
    for (nod = xpqBesttag.next(); nod; nod = xpqBesttag.next()) {
      //-- delete all but the first 'besttag' node
      xmlUnlinkNode(nod);
      xmlFreeNode(nod);
    }
    //-- set content of first old 'besttag' node
    setContent(xpqBesttag.first(), token.besttag().c_str());
  }
  else if (!token.besttag().empty()) {
    //-- -besttag:old, +besttag:new
    if (toki->type == XML_ELEMENT_NODE) {
      //-- token is an element : add a new child node
      nod = addNewNode(toki, "moot.tag");
      setContent(nod, token.besttag().c_str());
      /*
      //-- token is an element : add property
      setProp(((xmlNodePtr)(token.user_data)),
              BAD_CAST "moot.tag",
	      token.besttag.c_str());
      */
    }
    else {
      //-- -besttag:old +besttag:new : token is non-element
      //   : append text content
      carp("token node is not an element!");
      addContent(toki, "[moot.tag=\"");
      addContent(toki, token.besttag().c_str());
      addContent(toki, "\"]");
    }
  }
  return;
}

void TokenXmlDoc::put_token_nonlocal(const mootToken &token)
{
  xmlNodePtr nod;

  //-- non-local token: append under current output node
  if (!root()) addNewNode(NULL, outputRootName.c_str());
  if (!outputNode) outputNode = root();

  if (token.toktype() == TokTypeComment) {
    addComment(outputNode, token.text().c_str());
    return;
  }

  //-- "normal" node

  //-- build token node
  xmlNodePtr toknod = addNewNode(outputNode, "w");

  //-- token: text
  nod = addNewNode(toknod, "text");
  setContent(nod, token.text().c_str());

  //-- token: besttag
  nod = addNewNode(toknod, "moot.tag");
  setContent(nod, token.besttag().c_str());
  
  //-- token: analyses
  for (mootToken::AnalysisSet::const_iterator asi = token.analyses().begin();
       asi != token.analyses().end();
       asi++) 
    {
      nod = addNewNode(toknod, "analysis");
      setProp(nod, "pos", asi->tag.c_str());
      setContent(nod, asi->details.c_str());
    }
}

/*----------------------------------------------------------------------
 * TokenXmlDoc: put_sentence()
 */
void TokenXmlDoc::put_sentence(const mootSentence &sentence)
{
  mootSentence::const_iterator si;
  if (is_local_sentence(sentence)) {
    for (si = sentence.begin(); si != sentence.end(); si++) put_token_local(*si);
    return;
  }

  //-- non-local sentence : ensure document
  if (!root()) addNewNode(NULL, outputRootName.c_str());
  if (!outputNode) outputNode = root();
 
  //-- add sentence node
  xmlNodePtr snod = addNewNode(outputNode, "s");
  outputNode = snod;
  for (si = sentence.begin(); si != sentence.end(); si++) put_token_nonlocal(*si);
  outputNode = snod->parent;
};


moot_END_NAMESPACE

#endif /* MOOT_LIBXML_ENABLED */
