/* -*- Mode: C++ -*- */
/*--------------------------------------------------------------------------
 * File: dwdst.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger for DWDS project : shared headers
 *--------------------------------------------------------------------------*/

#ifndef _DWDST_H_
#define _DWDST_H_

#ifdef HAVE_CONFIG_H
# include "nopackage.h"
# include <config.h>
#endif

#include <stdio.h>

#include <FSMTypes.h>
#include <FSMSymSpec.h>
#include <FSM.h>

#include "dwdst_lexer.h"

#define DWDST_SYM_ATT_COMPAT true

/*--------------------------------------------------------------------------
 * dwds_tagger : tagger class
 *--------------------------------------------------------------------------*/
class dwds_tagger {
public:
  // -- public data
  FSM *morph;
  FSMSymSpec *syms;

  dwdst_lexer lexer;

  string eos;
  string wdsep;

  // -- flags
  bool want_avm;
  bool want_features;
  bool want_tnt_format;
  bool verbose;

  // -- verbose data
  unsigned int ntokens;
  unsigned int nunknown;

private:
  // -- private data
  // -- flags
  bool i_made_morph;
  bool i_made_syms;

protected:
  // -- temporary variables
  FSM *result, *tmp;
  FSMSymbolString s;
  set<FSMStringWeight> results;
  set<FSMStringWeight>::iterator ri;
  set<FSMSymbolString> tagresults;
  set<FSMSymbolString>::iterator tri;

public:
  // -- public methods: constructor/destructor
  dwds_tagger(FSM *mymorph=NULL, FSMSymSpec *mysyms=NULL);
  ~dwds_tagger();

  // -- public methods: loading
  FSMSymSpec *load_symbols(char *syms_file);
  FSM *load_morph(char *morph_file);
  
  // -- public methods: tagging
  bool tag_strings(int argc, char **argv, FILE *out=stdout);
  bool tag_stream(FILE *in=stdin, FILE *out=stdout);

  // -- public methods: sanity checks
  bool can_tag(void) const { return morph && syms; }

  // -- public methods: low-level
  set<FSMSymbol> *get_fsm_tags(FSM *fsa, set<FSMSymbol> *tags=NULL);
  set<FSMSymbolString> *get_fsm_tag_strings(FSM *fsa, set<FSMSymbolString> *tag_strings=NULL);

};

#endif // _DWDST_H_



