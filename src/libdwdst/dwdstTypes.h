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

/*----------------------------------------------------------------------
 * Basic Types
 *----------------------------------------------------------------------*/

/** Tag-string type */
typedef string dwdstTagString;

/** Token-string type */
typedef string dwdstTokString;

/*----------------------------------------------------------------------
 * Symbol Vectors (binary)
 *----------------------------------------------------------------------*/

/** STL utilities for symbol-vectors (currently unused) */
class dwdstSymbolVectorUtils {
public:
  struct HashFcn {
    inline size_t operator()(const FSM::FSMSymbolVector &x) const {
      size_t hv = 0;
      for (FSM::FSMSymbolVector::const_iterator xi = x.begin(); xi != x.end(); xi++) {
	hv += 5*hv + *xi;
      }
      return hv;
    }
  };
  struct EqualFcn {
    inline size_t operator()(const FSM::FSMSymbolVector &x, const FSM::FSMSymbolVector &y) const
    {
      return x==y;
    }
  };
};

/*----------------------------------------------------------------------
 * Symbol Vector -> Symbol mapping
 *----------------------------------------------------------------------*/

/** Maps a symbol-vector (sorted ambiguity-set) to a single symbol (ambiguity class) */
typedef
  hash_map<FSM::FSMSymbolVector,
	   FSMSymbol,
	   dwdstSymbolVectorUtils::HashFcn,
	   dwdstSymbolVectorUtils::EqualFcn>
  dwdstSymbolVector2SymbolMap;


#endif /* _DWDST_TYPES_H */
