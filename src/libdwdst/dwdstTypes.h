/*-*- Mode: C++ -*-*/
/*----------------------------------------------------------------------
 * Name: dwdstTypes.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + shared typedefs for libdwdst
 *----------------------------------------------------------------------*/

#ifndef _DWDST_TYPES_H
#define _DWDST_TYPES_H

#include <string>
#include <deque>
#include <hash_map>
#include <hash_set>
#include <map>
#include <set>
//#include <pair>


#include <FSM.h>
#include <FSMSymSpec.h>

/** Whether to load symspecs in AT&T-compatibility mode */
#define DWDST_SYM_ATT_COMPAT true

using namespace std;

/** Tag-string type */
typedef string dwdstTagString;

/** STL utilities for symbol-vectors */
class dwdstSymbolVectorUtils {
public:
  struct HashFcn {
    inline size_t operator()(const FSM::FSMSymbolVector &x) const {
      size_t hv = 0;
      for (FSM::FSMSymbolVector::const_iterator xi = x.begin(); xi != x.end(); xi++) {
	//hv += (hv<<5)-hv + *xi;
	hv += 5*hv + *xi;
      }
      return hv;
    }
  };
  struct EqualFcn {
    inline size_t operator()(const FSM::FSMSymbolVector &x, const FSM::FSMSymbolVector &y) const {
      return x==y;
    }
  };
};


/** Maps a symbol-vector (sorted ambiguity-set) to a single symbol (ambiguity class) */
typedef
  hash_map<FSM::FSMSymbolVector,FSMSymbol,dwdstSymbolVectorUtils::HashFcn,dwdstSymbolVectorUtils::EqualFcn>
  dwdstSymbolVector2SymbolMap;


/** Type for a single 'n-gram' (used by training methods) */
typedef  deque<FSMSymbolString> NGramVector;

/** Utilities for NGramVector */
class NGramVectorUtils {
public:
  // -- STL utilities
  struct HashFcn {
  public:
    inline size_t operator()(const NGramVector &x) const {
      size_t hv = 0;
      //FSMSymbolString::const_iterator xii;
      for (NGramVector::const_iterator xi = x.begin(); xi != x.end(); xi++) {
	/*
	for (xii = xi->begin(); *xii != '\0' && xii != xi->end(); xii++) {
	  hv += (hv<<5)-hv + *xii;
	}*/
	hv += 5*hv + __stl_hash_string(xi->c_str());
      }
      return hv;
    };
  };
  struct EqualFcn {
  public:
    inline size_t operator()(const NGramVector &x, const NGramVector &y) const {
      return x==y;
    };
  };
  //--End STL utilities
};

/** set of ngrams, used by training methods */
//typedef hash_set<NGramVector,NGramVectorUtil::HashFcn,NGramVectorUtil::EqualFcn> NGramVectorSet;
typedef set<NGramVector> NGramVectorSet;

/** N-gram->count lookup table (used by training methods) */
typedef
  //map<NGramVector,float> //--slower
  hash_map<NGramVector,float,NGramVectorUtils::HashFcn,NGramVectorUtils::EqualFcn>
  NGramTable;

/** Used by training methods to construct n-grams */
typedef set<FSMSymbolString> FSMSymbolStringSet;

/** Used by training methods to construct n-grams */
typedef deque<FSMSymbolStringSet *> FSMSymbolStringQueue;

/** Used for disambig-fsa generation */
typedef
  //map<FSMSymbolString,FSMSymbol> //--slower
  hash_map<FSMSymbolString,FSMSymbol>
  dwdstStringToSymbolMap;

/** Used for disambig-fsa generation */
typedef vector<FSMSymbolStringSet::iterator> tagSetIterVector;

/** Used for disambig-fsa generation.
 * First element of value-pair is the nGram-start state, second
 * element is the nGram-final state.
 */
typedef
  // -- moocow: hash_map appears to be a bit slower than sorted map<> here
  //hash_map<NGramVector,FSMState,NGramVectorUtils::HashFcn,NGramVectorUtils::EqualFcn>
  map<NGramVector,FSMState>
  NGramToStateMap;


/** Used for disambig-fsa generation.
 * First element is a count, second element is the number of
 * shortening-adjustments (fallback iterations) required to find
 * the count.
 */
class NGramCountFallbacksPair : public pair<float,float> {
public:
  inline float &count() { return first; };
  inline float &fallbacks() { return second; };
};

// Used by dwdstTrainer::add_pos_arc() [not too kosher]
// --this is defined by FSMTypes.h !!!
//typedef hash_set<FSMState> FSMStateSet;


#endif /* _DWDST_TYPES_H */
