/* -*- Mode: C++ -*- */
/*--------------------------------------------------------------------------
 * File: dwdst_trainer.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + Trainer for KDWDS PoS-tagger: shared headers
 *--------------------------------------------------------------------------*/

#ifndef _DWDST_TRAINER_H_
#define _DWDST_TRAINER_H_

//#define DWSTT_DEBUG
//#undef DWSTT_DEBUG
//#define DWST_FSTGEN_DEBUG
//#undef DWSTT_FSTGEN_DEBUG
//#define DWST_PARGEN_DEBUG
//#undef DWSTT_PARGEN_DEBUG

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
 * dwds_tagger_trainer : tagger-trainer class
 *--------------------------------------------------------------------------*/
class dwds_tagger_trainer : public dwds_tagger {
  // -- typedefs
public:
  typedef hash_map<string,float> StringToCountTable;
  typedef deque<set<FSMSymbolString> *> FSMSymbolStringQueue;
private:

  // -- data members
public:
  // -- tagsets
  set<FSMSymbolString>  opentags;  // set of PoS tags for open-classes (used for unknown-FST generation)
  set<FSMSymbolString>  alltags;   // set of all PoS tags
  // -- count parameters
  int kmax;
  StringToCountTable    strings2counts;
  FSMSymbolStringQueue  stringq;

private:
  StringToCountTable::iterator sci;


  // -- methods
public:
  // -- public methods: constructor/destructor
  dwds_tagger_trainer(FSMSymSpec *mysyms=NULL, FSM *mymorph=NULL) {};
  ~dwds_tagger_trainer();

  // -- public methods: unknown FSM generation
  FSM *generate_unknown_fsa();

  // -- public methods: disambiguation FSM generation
  //FSM *generate_disambig_fsa();

  // -- public methods: param-generation: (output-file = DEBUG)
  bool train_from_strings(int argc, char **argv, FILE *out=stdout);
  bool train_from_stream(FILE *in=stdin, FILE *out=stdout);

  // -- public methods: param-generation: parameter-file output
  bool write_param_file(FILE *out=stdout);

  // -- public methods: mid-level: listfile-reading
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

  // --- public methods: low-level : taglist file reading
  set<FSMSymbolString> read_taglist_file(set<FSMSymbolString> &tagset, const char *filename=NULL);
  // --- public methods: low-level : param-file manipulation
  //StringToCountTable *read_param_file(const char *filename);  
};

#endif // _DWST_TRAINER_H_



