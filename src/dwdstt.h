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

#include "dwdst.h"

/*--------------------------------------------------------------------------
 * dwds_tagger : tagger-trainer class
 *--------------------------------------------------------------------------*/
class dwds_tagger_trainer : public dwds_tagger {
  // -- typedefs
public:
  typedef hash_map<string,int> StringToCountTable;
  typedef deque<set<FSMStringWeight> > StringWeightQueue;
private:

  // -- data members
public:
  StringToCountTable strings2counts;
  StringWeightQueue  stringq;
  int kmax;
  
private:

  // -- methods
public:
  // -- public methods: constructor/destructor
  dwds_tagger_trainer(FSM *mymorph=NULL, FSMSymSpec *mysyms=NULL);
  ~dwds_tagger_trainer();

  // -- public methods: training
  bool train_from_strings(int argc, char **argv, FILE *out=stdout);
  bool train_from_stream(FILE *in=stdin, FILE *out=stdout);
};

#endif // _DWDSTT_H_



