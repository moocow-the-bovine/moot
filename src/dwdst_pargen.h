/* -*- Mode: C++ -*- */
/*--------------------------------------------------------------------------
 * File: dwdst_pargen.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + Parameter-file generator for KDWDS PoS-tagger : shared headers
 *--------------------------------------------------------------------------*/

#ifndef _DWDST_PARGEN_H_
#define _DWDST_PARGEN_H_

//#define DWDST_PARGEN_DEBUG
//#undef DWDST_PARGEN_DEBUG

#ifdef HAVE_CONFIG_H
# include "nopackage.h"
# include <config.h>
#endif

#include <hash_map>
#include <deque>
#include <set>

#include "dwdst_fstgen.h"

using namespace std;

/*--------------------------------------------------------------------------
 * dwds_tagger : tagger-trainer class
 *--------------------------------------------------------------------------*/
class dwds_param_generator : public dwds_fst_generator {
  // -- typedefs
public:
  typedef hash_map<string,float> StringToCountTable;
  typedef deque<set<FSMSymbolString> *> FSMSymbolStringQueue;
private:

  // -- data members
public:
  int kmax;
  StringToCountTable    strings2counts;
  FSMSymbolStringQueue  stringq;
    //set<FSMSymbolString>  tagset;

private:
  StringToCountTable::iterator sci;

  // -- methods
public:
  // -- public methods: constructor/destructor
  dwds_param_generator(FSM *mymorph=NULL, FSMSymSpec *mysyms=NULL) : kmax(2) {};
  ~dwds_param_generator();

  // -- public methods: param-generation: (output-file = DEBUG)
  bool train_from_strings(int argc, char **argv, FILE *out=stdout);
  bool train_from_stream(FILE *in=stdin, FILE *out=stdout);

  // -- public methods: training: parameter-file output
  bool write_param_file(FILE *out=stdout);
};

#endif // _DWDST_PARGEN_H_



