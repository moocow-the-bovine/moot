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
 *   + moot-relevant FSM includes (or placebos, if the real thing is unavailable)
 *----------------------------------------------------------------------*/
/**
 * \file mootFSM.h
 * \brief FSM interface for libmoot.
 * \detail May define placebo classes, etc. if HAVE_LIBFSM is not defined
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
# include <FSMCost.h>
#else /* HAVE_LIBFSM */

#include <vector>
#include <string>
#include <list>
#include <set>

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
        std::size_t operator()(const std::string &s) const {
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

#define EPSILON 0
#define FSMNOLABEL -1

void FSMplacebo(const char *name);

/*-------------------------------------------------------------
 * FSMSymSpec
 */
/** Placebo FSMSymSpec class */
class FSMSymSpec {
public:
  /*------------------------------------------------
   * typedefs
   */

public:
  /*------------------------------------------------
   * data
   */
  list<std::string> *messages;

public:
  /*------------------------------------------------
   * methods
   */
  FSMSymSpec(char *filename=NULL, list<std::string> *msgs=NULL, bool att_compat=true)
    : messages(msgs) {};

  std::string &symbol_vector_to_string(const vector<FSMSymbol> &vec,
				       std::string &str,
				       const bool use_categories=false,
				       const bool warn_on_undefined=true)
  {
    FSMplacebo("FSMSymSpec::symbol_vector_to_string()");
    return str;
  };

  const set<FSMSymbolString> *signs(void) {
    FSMplacebo("FSMSymSpec::signs()");
    return NULL;
  }
  const set<FSMSymbolString> *symbols(void) {
    FSMplacebo("FSMSymSpec::symbols()");
    return NULL;
  }

  const FSMSymbolString *symbol_to_symbolname(FSMSymbol sym) {
    FSMplacebo("FSMSymSpec::symbol_to_symbolname()");
    return NULL;
  }

  operator bool() { return false; }
};

/*-------------------------------------------------------------
 * FSM
 */
/** Placebo FSM class */
class FSM {
 public:
  /*------------------------------------------------
   * typedefs
   */
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
      FSMplacebo("FSM::FSMWeightedIOPair::operator<");
      return false;
    }
  };

  typedef FSMWeightedIOPair<FSMSymbolVector> FSMWeightedSymbolVector;
  typedef FSMWeightedIOPair<FSMSymbolString> FSMStringWeight;

  typedef set<FSMWeightedSymbolVector> FSMWeightedSymbolVectorSet;

 public:
  /*------------------------------------------------
   * methods
   */

  FSM(const char *filename=NULL) {
    FSMplacebo("FSM::FSM()");
  }

  bool fsm_use_symbol_spec(FSMSymSpec *)
  {
    FSMplacebo("FSM::fsm_use_symbol_spec()");
    return false;
  }

  void fsm_clear(void) { return; }

  FSM *fsm_lookup(FSMSymbolString &s, FSM *result=NULL, bool do_connect=true)
  {
    FSMplacebo("FSM::fsm_lookup()");
    return result;
  }

  set<FSMWeightedSymbolVector> &fsm_symbol_vectors(set<FSMWeightedSymbolVector> &vectors,
						  bool cycle_test=true)
  {
    FSMplacebo("FSM::fsm_symbol_vectors()");
    return vectors;
  }

  FSM *fsm_lookup_vector(const FSMSymbolVector &v, FSM *result=NULL, bool connect=true)
  {
    FSMplacebo("FSM::fsm_lookup_vector()");
    return result;
  }

  operator bool() const { return false; }

  size_t fsm_no_of_states() { return 0; }
  size_t fsm_no_of_final_states() { return 0; }
  size_t fsm_no_of_transitions() { return 0; }
};

/*-------------------------------------------------------------
 * FSM
 */
/** Placebo FSMRegexCompiler class */
class FSMRegexCompiler {
public:
  /*------------------------------------------------
   * typedefs
   */

public:
  /*------------------------------------------------
   * methods
   */
  void use_symbol_spec(FSMSymSpec *symspec) {
    FSMplacebo("FSMRegexCompiler::use_symbol_spec()");
  }

  FSM *parse_from_string(const char *str, const char *srcname=NULL)
  {
    FSMplacebo("FSMRegexCompiler::parse_from_string()");
    return NULL;
  }
  

};


#endif /* HAVE_LIBFSM */

#endif /* _moot_FSM_H */
