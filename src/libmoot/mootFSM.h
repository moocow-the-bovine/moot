/*-*- Mode: C++ -*-*/

/*
   libmoot version 1.0.4 : moocow's part-of-speech tagging library
   Copyright (C) 2003 by Bryan Jurish <moocow@ling.uni-potsdam.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

/*----------------------------------------------------------------------
 * Name: mootFSM.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moot-relevant FSM includes (or dummies)
 *----------------------------------------------------------------------*/
/**
 * \file mootFSM.h
 * \brief FSM interface for libmoot.
 * \detail May define dummy classes, etc. if HAVE_LIBFSM is not defines
 */

#ifndef _moot_FSM_H
#define _moot_FSM_H

#ifdef HAVE_CONFIG_H
# include <mootConfig.h>
#endif

#ifdef HAVE_LIBFSM
# include <FSMTypes.h>
# include <FSM.h>
# include <FSMSymSpec.h>
#else /* HAVE_LIBFSM */

#include <vector>
#include <string>

/*-------------------------------------------------------------
 * gcc hacks
 */
#ifdef __GNUC__
  #if __GNUC__ >= 3
    #include <ext/stl_hash_fun.h>
    #if __GNUC_MINOR__ == 0
      #define FSM_STL_NAMESPACE std
    #else
      #define FSM_STL_NAMESPACE __gnu_cxx
    #endif /* __GNUC_MINOR__ == 0 */
    /** gcc-3 STL can't hash strings: argh */
    namespace FSM_STL_NAMESPACE
    {
      template<> struct hash<std::string> {
        std::size_t operator()(const string &s) const {
          return __stl_hash_string(s.c_str());
        }
      };
    }
  #endif /* __GNUC => 3 */
#else /* __GNUC__ */
  #define FSM_STL_NAMESPACE std
#endif /* __GNUC__ */ 

using namespace std;

/*-------------------------------------------------------------
 * FSMTypes
 */
typedef string FSMSymbolString;

typedef short int FSMSymbol;

typedef float FSMWeight;

/*-------------------------------------------------------------
 * FSMSymSpec
 */
/** Dummy FSMSymSpec class */
class FSMSymSpec {
 public:
};

/*-------------------------------------------------------------
 * FSM
 */
/** Dummy FSM class */
class FSM {
 public:
  typedef vector<FSMSymbol> FSMSymbolVector;

  template <class T> class FSMWeightedIOPair
  {
  public:
    T istr;           ///< input part of the value-pair
    T ostr;           ///< output part of the value-pair [UNUSED!]
    FSMWeight weight; ///< weight associated with this value-pair
    
    // -- Constructors
    FSMWeightedIOPair(const T& is) : istr(is), ostr(T()), weight(0) {};
    FSMWeightedIOPair(const T& is, FSMWeight w) : istr(is), ostr(T()), weight(w) {};
    FSMWeightedIOPair(const T& is, const T& os) : istr(is), ostr(os), weight(0) {};
    FSMWeightedIOPair(const T& is, const T& os, FSMWeight w) : istr(is), ostr(os), weight(w) {};


    //-- Comparators
    friend bool operator<(const FSMWeightedIOPair& x, const FSMWeightedIOPair& y)
    {
      if (x.weight < y.weight) {
	return true;
      }
      else if (x.weight == y.weight) {
	if (x.istr < y.istr) {
	  return true;
	}
	else if (x.istr == y.istr) {
	  return x.ostr < y.ostr;
	}
      }
      return false;
    }
  };

  typedef FSMWeightedIOPair<FSMSymbolVector> FSMWeightedSymbolVector;
 public:
};


#endif /* HAVE_LIBFSM */

#endif /* _moot_FSM_H */
