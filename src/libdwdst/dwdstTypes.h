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
#include <FSMSymSpec.h>

using namespace std;

/// Type for a single 'n-gram' (used by training methods)
typedef deque<FSMSymbolString> NGramVector, *NGramVectorPtr;

/// N-gram->count lookup table (used by training methods)
typedef map<NGramVector,float> NGramTable, *NGramTablePtr;

/// Used by training methods to construct n-grams
typedef deque<set<FSMSymbolString> * > FSMSymbolStringQueue, FSMSymbolStringQueuePtr;

#endif /* _DWDST_TYPES_H */
