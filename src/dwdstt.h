/* -*- Mode: C++ -*- */
/*--------------------------------------------------------------------------
 * File: dwdstt.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + trainer for PoS tagger for DWDS project : shared headers
 *--------------------------------------------------------------------------*/

#ifndef _DWDSTT_H_
#define _DWDSTT_H_

#ifdef HAVE_CONFIG_H
# include "nopackage.h"
# include <config.h>
#endif

#include <hash_map>
#include <deque>
#include <set>

#include "dwdst.h"
#include "postag_lexer.h"

using namespace std;

/*--------------------------------------------------------------------------
 * dwds_tagger : tagger-trainer class
 *--------------------------------------------------------------------------*/
class dwds_tagger_trainer : public dwds_tagger {
  // -- typedefs
public:
  typedef hash_map<string,float> StringToCountTable;
  typedef deque<set<FSMSymbolString> *> StringQueue;
private:

  // -- data members
public:
  int kmax;
  StringToCountTable   strings2counts;
  StringQueue          stringq;
  FSM                  *ufsa;
  set<FSMSymbolString> tagset;
  
private:

  // -- methods
public:
  // -- public methods: constructor/destructor
  dwds_tagger_trainer(FSM *mymorph=NULL, FSMSymSpec *mysyms=NULL) : kmax(2), ufsa(NULL) {};
  ~dwds_tagger_trainer();

  // -- public methods: unknown FSM generation
  set<FSMSymbolString> get_pos_tags(const char *filename=NULL);
  FSM *generate_unknown_fsa();

  // -- public methods: training
  bool train_from_strings(int argc, char **argv, FILE *out=stdout);
  bool train_from_stream(FILE *in=stdin, FILE *out=stdout);

};

#endif // _DWDSTT_H_



