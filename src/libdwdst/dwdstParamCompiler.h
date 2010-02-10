/* -*- Mode: C++ -*- */

/*============================================================================
 * File: dwdstParamCompiler.h
 * Author:  Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *    Compiler for TnT parameter files for DWDS PoS tagger
 *============================================================================*/

#ifndef _DWDST_PARAM_COMPILER_H
#define _DWDST_PARAM_COMPILER_H

#include <stdio.h>
#include <string.h>

#include <FSM.h>

#include "dwdstParamLexer.h"
#include "dwdstParamParser.h"

/**
 * dwdstParamCompiler compiles dwdst-style parameter files into
 * internal n-gram tables, using a lexer/parser pair generated by
 * Alain Coetmeur's flex++ and bison++ programs.
 *
 * \brief Parameter-file compiler.
 */
class dwdstParamCompiler : public dwdstParamParser {
 public:
  // -- public data
  /** flex++ lexer object */
  dwdstParamLexer theLexer;

  /**
   * objname: name to use for object when reporting errors -- default: "dwdstParamCompiler"
   * \b Warning: no copying is performed on this string; you must alloc&free it yourself!
   */
  char *objname;

  /**
   * srcname: name to use for current file when reporting errors -- default: "(unknown)"
   * \b Warning: no copying is performed on this string; you must alloc&free it yourself!
   */
  char *srcname;

 public:
  // -- public methods: CONSTRUCTORS / DESTRUCTORS
  /** Default constructor */
  dwdstParamCompiler() : objname(NULL), srcname(NULL) {};
  /** Default destructor */
  virtual ~dwdstParamCompiler() {};

  /**
   * set n-gram table to use for compilation
   * \b Warning: no copying is performed on 'myngrams'.
   */
  void set_ngrams(NGramTable *myngrams) { ngtable = myngrams; };

  // -- high-level parsing methods

  /** parse n-gram parameters from a C-stream.  Returns NULL on error. */
  inline NGramTable *parse_from_file(FILE *file, const char *filename=NULL) {
      select_streams(file,stdout);
      return parse_ngrams();
  };

  /** parse one regular expression from a C-string.  Returns NULL on error. */
  inline NGramTable *parse_from_string(const char *string, const char *srcname=NULL) {
    select_string(string,srcname);
    return parse_ngrams();
  };

  // -- low-level public methods: INPUT SELECTION
  /** low-level input selection: input from a C-stream. */
  void select_streams(FILE *in, FILE *out, const char *my_srcname=NULL) {
    theLexer.select_streams(in,out);
    srcname = (char *)my_srcname;
  };

  /**
   * low-level input selection: input from a C-string.
   * \b WARNING: do NOT free the string 'in' until parsing has finished!
   */
  void select_string(const char *in, const char *my_srcname=NULL) {
    theLexer.select_string(in);
    srcname = (char *)my_srcname;
  };

  // -- low-level public methods: PARSING
  virtual int yylex();

  /**
   * low-level parsing method: parse all remaining n-grams
   * from the currently selected input source.
   */
  inline NGramTable *parse_ngrams();

  // -- low-level public methods: ERRORS & WARNINGS
  /** yyerror: report parse errors. */
  virtual void yyerror(const char *msg);

  /** yywarn: report parse warnings. */
  virtual void yywarn(const char *msg);
};


#endif /* _DWDST_PARAM_COMPILER_H */