/* -*- Mode: C++ -*- */
/*--------------------------------------------------------------------------
 * File: dwdst_trainer.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + Trainer for KDWDS PoS-tagger: shared headers
 *--------------------------------------------------------------------------*/

#ifndef _DWDST_TRAINER_H_
#define _DWDST_TRAINER_H_

//#define DWDST_DEBUG
//#undef DWDST_DEBUG
//#define DWDST_FSTGEN_DEBUG
//#undef DWDST_FSTGEN_DEBUG
//#define DWDST_PARGEN_DEBUG
//#undef DWDST_PARGEN_DEBUG

#ifdef HAVE_CONFIG_H
# include "nopackage.h"
# include <config.h>
#endif

#include <hash_map>
#include <deque>
#include <set>

#include "dwdst.h"

using namespace std;

/*--------------------------------------------------------------------------
 * dwds_tagger_trainer : tagger-trainer class
 *--------------------------------------------------------------------------*/
class dwds_tagger_trainer : public dwds_tagger {
public:
  // -- typedefs (see dwdst.h)
private:
  // -- private data
public:
  // -- public data
  // -- tagsets
  set<FSMSymbolString>  opentags;  // set of PoS tags for open-classes (used for unknown-FST generation)
  set<FSMSymbolString>  alltags;   // set of all PoS tags (to be used for disambig-FST generation)
  // -- count parameters
  int kmax;
  NGramTable            ngtable;
  FSMSymbolStringQueue  stringq;

private:
  NGramTable::iterator ngti;
  bool is_eos;

  // -- training variables
  set<FSMSymbolString> *curtags;
  set<FSMSymbolString>::iterator cti;

  NGramVector      theNgram;
  set<NGramVector> *curngrams;
  set<NGramVector> *nextngrams;
  set<NGramVector> *tmpngrams;

  set<NGramVector>::iterator ngi;

  //FSMSymbolStringQueue::reverse_iterator qri;
  FSMSymbolStringQueue::iterator qi;
  set<FSMSymbolString>::iterator qii;


  // -- methods
public:
  // -- public methods: constructor/destructor
  dwds_tagger_trainer(FSMSymSpec *mysyms=NULL, FSM *mymorph=NULL) {};
  ~dwds_tagger_trainer();

  // -- public methods: FSM generation
  FSM *generate_unknown_fsa();
  FSM *generate_disambig_fsa(); // NYI

  // -- mid-/low-level methods: FSM generation
  set<FSMState> fsm_add_pos_arc(FSM *fsm, const FSMState qfrom, const FSMSymbolString &pos,
				const FSMWeight cost = FSM_default_cost_structure.freecost());

  // -- public methods: param-generation: (output-file = DEBUG)
  bool train_from_strings(int argc, char **argv, FILE *out=stdout);
  bool train_from_stream(FILE *in=stdin, FILE *out=stdout);
  // -- mid-level
  inline void train_next_token(void);
  inline void train_eos(void);

  // -- public methods: param-generation: parameter-file read/write
  bool load_param_file(FILE *in=stdin,const char *filename=NULL);  // NYI
  bool save_param_file(FILE *out=stdout);

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

  // --- public methods: low-level : initialize & cleanup
  inline bool init_training_temps(FILE *in=NULL, FILE *out=NULL);
  inline bool cleanup_training_temps();
};

#endif // _DWST_TRAINER_H_



