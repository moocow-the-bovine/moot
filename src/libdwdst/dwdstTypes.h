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
#include <map>
#include <set>
//#include <pair>
#include <FSMSymSpec.h>

using namespace std;

/// Type for a single 'n-gram' (used by training methods)
typedef deque<FSMSymbolString> NGramVector, *NGramVectorPtr;

/// N-gram->count lookup table (used by training methods)
typedef map<NGramVector,float> NGramTable, *NGramTablePtr;

/// Used by training methods to construct n-grams
typedef deque<set<FSMSymbolString> * > FSMSymbolStringQueue, FSMSymbolStringQueuePtr;

/// Used for disambig-fsa generation
typedef map<FSMSymbolString,FSMSymbol> dwdstStringToSymbolMap;

/// Used for disambig-fsa generation
typedef vector<set<FSMSymbolString>::iterator> tagSetIterVector;

/// Used for disambig-fsa generation.
/// First element of value-pair is the nGram-start state, second
/// element is the nGram-final state.
typedef map<NGramVector,pair<FSMState,FSMState> > NGramToStateMap;

/// Used for disambig-fsa generation.
/// First element is a count, second element is the number of
/// shortening-adjustments (fallback iterations) required to find
/// the count.
class NGramCountFallbacksPair : public pair<float,float> {
public:
  inline float &count() { return first; };
  inline float &fallbacks() { return second; };
};


#endif /* _DWDST_TYPES_H */
