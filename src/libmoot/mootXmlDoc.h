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
 * File: mootXmlDoc.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: C++ wrapper for libxml2 tree-mode XML documents
 *--------------------------------------------------------------------------*/

#ifndef _MOOT_XML_DOC_H
#define _MOOT_XML_DOC_H

#include <mootConfig.h>

#ifdef MOOT_LIBXML_ENABLED

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/xmlIO.h>

#include <string>

namespace moot {

/*----------------------------------------------------
 * mootXPathQuery
 */
/** Class to represent XPath queries & their results */
class mootXPathQuery
{
public:
  /*------------------------------------------
   * XPathQuery: Data
   */
  std::string           expr; ///< the XPath query expression itself
  xmlXPathCompExprPtr   cmp;  ///< compiled XPath query expression
  xmlXPathContextPtr    ctx;  ///< XPath query context
  xmlXPathObjectPtr     obj;  ///< XPath query results
  //xmlNodePtr           *cur;  ///< Current node (for iteration)
  int                   idx;  ///< index of current node (for iteration)

public:
  /*------------------------------------------
   * XPathQuery: Methods: Constructors
   */
  /**\name Constructors etc. */
  //@{
  /**
   * Default constructor
   * \xpathExpr: the XPath expression
   * \do_compile: whether to pre-compile the expression (default=false)
   */
  mootXPathQuery(const std::string &xpathExpr="", bool do_compile=false)
    : expr(xpathExpr),
      cmp(NULL),
      ctx(NULL),
      obj(NULL),
      idx(0)
  {
    if (do_compile) compile();
  };

  /**
   * Extended constructor 
   * \xpathExpr: the XPath expression
   * \xml_doc: document to query
   * \ctx_node: current context node (may be NULL)
   * \do_eval: whether to pre-evaluate the expression (default=false).
   * \do_sort: whether to sort result nodes in document order -- only (default=false).
   */
  mootXPathQuery(const std::string &xpathExpr,
	     xmlDocPtr          xml_doc,
	     xmlNodePtr         ctx_node=NULL,
	     bool               do_eval=false,
	     bool               do_sort=false)
    : expr(xpathExpr),
      cmp(NULL),
      ctx(NULL),
      obj(NULL),
      idx(0)
  {
    compile();
    context(xml_doc,ctx_node);
    if (do_eval) eval(do_sort);
  };

  /** Copy constructor: only 'expr' is really copied! */
  mootXPathQuery(const mootXPathQuery &q2)
    : expr(q2.expr),
      cmp(NULL),
      ctx(NULL),
      obj(NULL),
      idx(0)
  {};

  /** Destructor */
  inline ~mootXPathQuery(void) { clear(); };

  /** Clear the query (except for 'expr' string), freeing resources */
  inline void clear(void)
  {
    //expr.clear();
    if (cmp) { xmlXPathFreeCompExpr(cmp); cmp = NULL; }
    if (ctx) { xmlXPathFreeContext(ctx); ctx = NULL; }
    if (obj) { xmlXPathFreeObject(obj); obj = NULL; }
    idx = 0;
  };
  //@}

  /*------------------------------------------*/
  /** \name Compilation and Evaluation */
  //@{
  /** (Re-)compile the query.  Returns NULL on error. */
  inline xmlXPathCompExprPtr compile(void)
  {
    clear();
    cmp = xmlXPathCompile(BAD_CAST expr.c_str());
    return cmp;
  };

  /** (re-)initialize evaluation context.  Returns NULL on error */
  inline xmlXPathContextPtr context(xmlDocPtr xml_doc, xmlNodePtr ctx_node=NULL)
  {
    idx = 0;
    if (ctx) { xmlXPathFreeContext(ctx); ctx = NULL; }
    if (obj) { xmlXPathFreeObject(obj); obj = NULL; }
    if (!cmp) return NULL;
    ctx = xmlXPathNewContext(xml_doc);
    if (ctx && ctx_node) ctx->node = ctx_node;
    return ctx;
  };

  /**
   * Evaluate compiled and initialized expression.
   * Returns NULL on error.
   * \do_sort: whether to sort result nodes in document order (default=false).
   */
  inline xmlXPathObjectPtr eval(bool do_sort=false)
  {
    idx = 0;
    if (obj) { xmlXPathFreeObject(obj); obj = NULL; }
    if (!cmp || !ctx) return NULL;
    obj = xmlXPathCompiledEval(cmp,ctx);
    if (do_sort && obj && obj->nodesetval) xmlXPathNodeSetSort(obj->nodesetval);
    return obj;
  };

  /**
   * Evaluate expression, possibly (re-)compiling and initializing.
   * Returns NULL on error.
   * \xml_doc: document for re-initialization.
   * \ctx_node: current context node for re-initialization.
   * \do_sort: whether to sort result nodes in document order (default=false).
   * \do_compile: whether to re-compile the expression (default=false).
   */
  inline xmlXPathObjectPtr eval(xmlDocPtr xml_doc,
				xmlNodePtr ctx_node=NULL,
				bool do_sort=false,
				bool do_compile=false)
  {
    if (do_compile && !compile()) return NULL;
    if (!context(xml_doc, ctx_node)) return NULL;
    return eval(do_sort);
  };
  //@}

  /*------------------------------------------*/
  /** \name Iteration */
  //@{
  /**
   * Get pointer to the current result set (array of nodes)
   * Returns NULL if no result set is available.
   */
  inline xmlNodeSetPtr nodeset(void)
  {
    return (obj ? obj->nodesetval : NULL);
  };

  /** Get number of nodes in current result set, if any. */
  inline int size(void)
  {
    return (obj && obj->nodesetval ? obj->nodesetval->nodeNr : 0);
  };

  /**
   * Get first result node.
   * Returns NULL if no result nodes are available.
   */
  inline xmlNodePtr first(void)
  {
    idx = 0;
    return cur();
  };

  /**
   * Get 'current' result node.
   * Returns NULL if no current node is available.
   */
  inline xmlNodePtr cur(void)
  {
    return (obj && obj->nodesetval && idx < obj->nodesetval->nodeNr
	    ? obj->nodesetval->nodeTab[idx]
	    : NULL);
  };

  /**
   * Increment 'current' result node index.
   * Returns the new 'current' node, or NULL if none is available.
   */
  inline xmlNodePtr next(void)
  {
    return (obj && obj->nodesetval && ++idx < obj->nodesetval->nodeNr
	    ? obj->nodesetval->nodeTab[idx]
	    : NULL);
  };
  //@}

}; //-- end class mootmootXPathQuery


/*--------------------------------------------------------------
 * mootXmlDoc
 */
/** \brief C++ wrapper for libxml2 tree-mode XML documents */
class mootXmlDoc {
public:
  /*----------------------------------------------------
   * mootXmlDoc: Data
   */
  /** \name Diagnostic Data */
  //@{
  std::string srcname;         ///< name of document source
  //@}

  /** \name Low-level data */
  //@{
  //-- libxml2 stuff
  xmlDocPtr  xml_doc;      ///< underlying document tree
  int        xml_options;  ///< parser flags: combination of xmlParserOption
  bool       xml_format;   ///< whether to pretty-print by default (default=false)
  //@}
 
 public:
  /** \name Constructors / Destructor / Reset */
  //@{
  /*----------------------------------------------------
   * mootXmlDoc: Constructor
   */
  /** Default constructor */
  mootXmlDoc(const std::string &myname         = "(unknown)",
	    int                parse_options  = 0,
	    bool               do_format      = false)
    : srcname(myname),
      xml_doc(NULL),
      xml_options(parse_options),
      xml_format(do_format)
  {
    //-- libxml2 sanity check
    LIBXML_TEST_VERSION ;
  };

  /*----------------------------------------------------
   * mootXmlDoc: Destructor
   */
  /** Default destructor */
  virtual ~mootXmlDoc(void)
  {
    reset();
    srcname.clear();
  };

  /*----------------------------------------------------
   * mootXmlDoc: Reset
   */
  /** reset parser state (frees document) */
  virtual void reset(void)
  {
    if (xml_doc) xmlFreeDoc(xml_doc);
    xml_doc = NULL;
  };
  //@}

  /*----------------------------------------------------*/
  /** \name Input */
  //@{
  /**
   * Parse from a named file or URL. You can use "-" as an alias for stdin.
   * Returns true on success.  Calls reset();
   */
  virtual bool loadFilename(const char *filename_or_url, const char *encoding=NULL);

  /**
   * Read in document from a C stream.
   * Returns true on success.  Calls reset().
   * \file: input stream (you'll have to open and close it yourself)
   * \encoding: encoding of the document if known
   * \base_url: base URL of the document (?)
   * \myname: name to use for diagnostics
   */
  virtual bool loadFile(FILE *             file,
			const char *       encoding =NULL,
			const char *       base_url =NULL,
			const std::string& myname   ="");

  /**
   * Parse from your very own in-memoty buffer.
   * \buffer: buffer to parse
   * \bufsize: number of bytes to parse from the buffer
   * \encoding: encoding of the document if known
   * \base_url: base URL of the document (?)
   * \myname: name to use for diagnostics
   */
  virtual bool loadBuffer(const char*        buffer, 
			  int                bufsize, 
			  const char*        base_url   = NULL,
			  const char*        encoding   = NULL, 
			  const std::string& myname     ="");

  /** Override this run some code before every load*() operation. */
  virtual bool _pre_load_hook(void) { return true; };

  /** Override this run some code after every load*() operation. */
  virtual bool _post_load_hook(void) { return true; };
  //@}



  /*----------------------------------------------------*/
  /** \name Output */
  //@{
  /**
   * Save document to a named file. You can use "-" as an alias for stdout
   * Returns true on success.
   * \filename_or_url: name of output file
   * \encoding: name of output encoding or NULL
   * \compressMode: zlib compression level (default: document() value)
   */
  virtual bool saveFilename(const char *filename_or_url,
			    const char *encoding=NULL,
			    int         compressMode=-1);

  /**
   * Save document to a C stream (you will need to open and close it yourself).
   * Returns true on success.
   * \file: output stream
   * \encoding: name of output encoding or NULL
   */
  virtual bool saveFile(FILE *file, const char *encoding=NULL);

  /**
   * Save document to an in-memory buffer.
   * Returns true on success.
   * \buffer_ptr: address of the output buffer.  You will need to free this with xmlFree().
   * \buflen: pointer to the currently allocated length in bytes of the output buffer
   * \encoding: name of output encoding or NULL
   * \prettyprint: whether to pretty-print the output
   */
  virtual bool saveBuffer(xmlChar    **buffer_ptr,
			  int         *buflen,
			  const char  *encoding=NULL);

  /** Override this run some code before every load*() operation. */
  virtual bool _pre_save_hook(void) { return true; };

  /** Override this run some code after every load*() operation. */
  virtual bool _post_save_hook(void) { return true; };
  //@}

  /*----------------------------------------------------*/
  /** \name Document Tweaking */
  //@{
  /**
   * Get pointer to the root element.
   * Returns NULL if no document is loaded or if the document is empty.
   */
  inline xmlNodePtr root(void)
  {
    return xml_doc ? xmlDocGetRootElement(xml_doc) : NULL;
  };

  /** Ensure that we have a document to mess with. 
   *  Returns a pointer to the document, or NULL if
   *  none was present or could be allocated.
   */
  inline xmlDocPtr document(void)
  {
    return xml_doc ? xml_doc : (xml_doc = xmlNewDoc(BAD_CAST "1.0")); // (version)
  };

  /**
   * Create and add a new child node under parent.
   * Returns a pointer to the new node.
   * \parent: parent node; use NULL for the document root
   * \name: name of the new child in UTF-8
   */
  inline xmlNodePtr addNewNode(xmlNodePtr parent, const char *name)
  {
    xmlNodePtr node = xmlNewNode(NULL, BAD_CAST name); // (ns,name)
    assert(node != NULL);

    if (parent == NULL) {
      //-- no parent: try root node
      parent = root();
      if (parent == NULL) {
	//-- still no parent: add a new root node
	xmlDocSetRootElement(document(), node);
	return node;
      }
    }
    return xmlAddChild(parent,node);
  };

  /**
   * Append some text data to parent.
   * Returns a pointer to the new text node.
   * \text: text to add, in UTF-8
   */
  static inline xmlNodePtr addNewText(xmlNodePtr parent, const char *text)
  {
    assert(parent != NULL);
    xmlNodePtr node = xmlNewText(BAD_CAST text); // (content)
    assert(node != NULL);
    return xmlAddChild(parent, node);
  };

  /**
   * Create and add a new comment node under parent.
   * Returns a pointer to the new node.
   * \parent: parent node; use NULL for a document-level comment
   * \text: text content of the new comment node
   */
  inline xmlNodePtr addComment(xmlNodePtr parent, const char *text)
  {
    if (parent == NULL) {
      //-- no parent: add root comment
      return xmlNewDocComment(document(), BAD_CAST text);
    }
    //-- parent given: create and add new node under parent
    xmlNodePtr cmt = xmlNewComment(BAD_CAST text);
    assert(cmt != NULL);
    return xmlAddChild(parent, cmt);
  };


  /**
   * Append some text content to a node.
   * \text: text to add, in UTF-8
   */
  static inline void addContent(xmlNodePtr node, const char *content)
  {
    assert(node != NULL);
    xmlNodeAddContent(node, BAD_CAST content);
  };

  /**
   * Set text content of a node.
   * \text: text of the node, in UTF-8
   */
  static inline void setContent(xmlNodePtr node, const char *content)
  {
    assert(node != NULL);
    xmlNodeSetContent(node, BAD_CAST content);
  };

  /**
   * Check whether a node has a given attribute.
   * Returns a pointer to the attribute, the attribute declaration, or NULL.
   */
  static inline xmlAttrPtr hasProp(xmlNodePtr node, const char *name)
  {
    assert(node != NULL);
    return xmlHasProp(node, BAD_CAST name);
  };

  /**
   * Get a node's value for a given attribute.
   * Returns the empty string if \c node has no attribute
   * named \c name (also if that attribute's value is the
   * empty string).
   */
  static inline std::string getProp(xmlNodePtr node, const char *name)
  {
    assert(node != NULL);
    xmlChar *v = xmlGetProp(node, BAD_CAST name);
    std::string s="";
    if (v) {
      s = (const char *)v;
      xmlFree(v);
    }
    return s;
  };

  /**
   * Set a node attribute.  @name and @value should be UTF-8 encoded.
   * Returns a pointer to the new attribute.
   */
  static inline xmlAttrPtr setProp(xmlNodePtr node,
				   const char *name,
				   const char *value)
  {
    assert(node != NULL);
    return xmlSetProp(node, BAD_CAST name, BAD_CAST value);
  };

  /**
   * Get text value of an attribute node.
   * Returns the empty string if \c node has no attribute
   * named \c name (also if the attribute's value is the
   * empty string).
   */
  static inline std::string attrValue(xmlAttrPtr attr_node)
  {
    assert(attr_node != NULL && attr_node->type == XML_ATTRIBUTE_NODE);
    return std::string(attr_node->children != NULL && attr_node->children->content != NULL
		       ? (const char *)attr_node->children->content
		       : "");
  };
  //@}

  /*----------------------------------------------------*/
  /** \name XPath Queries */
  //@{
  /**
   * Return a pointer to a new pre-compiled and initialized mootXPathQuery
   * for this document, or NULL if any requested step of query-building
   * failed.  Caller is responsible for deleting the returned pointer.
   *
   * \xpathExpr: the XPath expression to compile and evaluate
   * \ctx_node: the context-node for the expression.  (default=root())
   * \do_eval: whether to evaluate the query (default=false)
   * \do_sort: for do_eval==true, whether to sort result nodes in doc-order (default=false)
   */
  virtual mootXPathQuery *xpath(const std::string &xpathExpr,
 				xmlNodePtr         ctx_node=NULL,
				bool               do_eval=false,
				bool               do_sort=false)
  {
    mootXPathQuery *xpq =  new mootXPathQuery(xpathExpr);
    if (!xpq->compile()) {
      carp("could not compile XPath expression `%s'\n", xpq->expr.c_str());
      delete xpq;
      return NULL;
    }
    else if (!xpq->context(xml_doc, ctx_node ? ctx_node : root())) {
      carp("could not initialize XPath context for `%s'\n", xpq->expr.c_str());
      delete xpq;
      return NULL;
    }
    else if (do_eval && !xpq->eval(do_sort)) {
      carp("could not evaluate XPath query `%s'\n", xpq->expr.c_str());
      delete xpq;
      return NULL;
    }
    return xpq;
  };
  //@}


  /*----------------------------------------------------*/
  /** \name Diagnostics */
  //@{
  /** complain */
  virtual void carp(char *fmt, ...)
  {
    fprintf(stderr, "mootXmlDoc: ");
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
  };
  //@}
};


}; /* moot_END_NAMESPACE */

#endif /* MOOT_LIBXML_ENABLED */

#endif /* MOOT_XML_DOC_H */

