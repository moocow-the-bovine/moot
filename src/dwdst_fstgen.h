/* -*- Mode: C++ -*- */
/*--------------------------------------------------------------------------
 * File: dwdst_fstgen.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + FST auto-generator for KDWDS PoS-tagger: shared headers
 *--------------------------------------------------------------------------*/

#ifndef _DWDST_FSTGEN_H_
#define _DWDST_FSTGEN_H_

//#define DWST_FSTGEN_DEBUG
//#undef DWST_FSTGEN_DEBUG

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
 * dwds_fst_generator : tagger-trainer class
 *--------------------------------------------------------------------------*/
class dwds_fst_generator : public dwds_tagger {
  // -- typedefs
public:
  typedef hash_map<string,float> StringToCountTable;
  typedef deque<set<FSMSymbolString> *> FSMSymbolStringQueue;
private:

  // -- data members
public:
  set<FSMSymbolString>  opentags;  // set of PoS tags for open-classes (used for unknown-FST generation)
  set<FSMSymbolString>  alltags;   // set of all PoS tags

private:

  // -- methods
public:
  // -- public methods: constructor/destructor
  dwds_fst_generator(FSMSymSpec *mysyms=NULL, FSM *mymorph=NULL) {};
  ~dwds_fst_generator();

  // -- public methods: unknown FSM generation
  FSM *generate_unknown_fsa();

  // -- public methods: listfile-reading
  set<FSMSymbolString> get_all_pos_tags(const char *filename=NULL)
  {
    return read_taglist_file(alltags, filename); 
  };
  set<FSMSymbolString> get_open_class_tags(const char *filename=NULL)
  {
    read_taglist_file(opentags, filename);
    alltags.insert(opentags.begin(),opentags.end());
    return opentags;
  };

  // -- public methods: low-level
  set<FSMSymbolString> read_taglist_file(set<FSMSymbolString> &tagset, const char *filename=NULL);
};

#endif // _DWST_FSTGEN_H_



