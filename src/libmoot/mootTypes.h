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
 * Name: mootTypes.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + shared typedefs for libmoot
 *----------------------------------------------------------------------*/
/** \file mootTypes.h
 * \brief Common typedefs and constants.
 */

#ifndef _moot_TYPES_H
#define _moot_TYPES_H

#include <ctype.h>

#include <string>
#include <deque>
#include <map>
#include <set>

#if defined(__GNUC__)
#  if __GNUC__ >= 3
#    if __GNUC_MINOR__ == 0
#      define moot_STL_NAMESPACE std
#    else
#      define moot_STL_NAMESPACE __gnu_cxx
#    endif /* __GNUC_MINOR__ == 0 */
#    include <ext/hash_map>
#    include <ext/hash_set>
#  else  /* __GNUC__ >= 3 */
#    define moot_STL_NAMESPACE std
#    include <hash_map>
#    include <hash_set>
#  endif /* __GNUC__ >= 3 */
#else  /* defined(__GNUC__) */
#  include <hash_map>
#  include <hash_set>
#  define moot_STL_NAMESPACE std
#endif /* defiend(__GNUC__) */

#include <mootFSM.h>

/** Whether to load symspecs in AT&T-compatibility mode */
#define moot_SYM_ATT_COMPAT true

/* Namespace definitions: not yet ready */
#define moot_NAMESPACE moot
#define moot_BEGIN_NAMESPACE namespace moot {
#define moot_END_NAMESPACE };
//#define moot_NAMESPACE
//#define moot_BEGIN_NAMESPACE
//#define moot_END_NAMESPACE

moot_BEGIN_NAMESPACE

using namespace std;
using namespace moot_STL_NAMESPACE;

/*----------------------------------------------------------------------
 * Basic Types
 *----------------------------------------------------------------------*/

/** Tag-string type */
typedef string mootTagString;

/** Token-string type */
typedef string mootTokString;

/** Count types (for raw frequencies) */
typedef float CountT;

/*----------------------------------------------------------------------
 * Token Typification
 *----------------------------------------------------------------------*/

//namespace mootTokenUtils {
//using namespace std;

    /** Enum typedef for token2id() */
    typedef enum {
	TokTypeAlpha,      /**< (Mostly) alphabetic token: "foo", "bar", "foo2bar" */
	TokTypeCard,       /**< @CARD: Digits-only: "42" */
	TokTypeCardPunct,  /**< @CARDPUNCT: Digits with punctuation suffix: "42." */
	TokTypeCardSuffix, /**< @CARDSUFFIX: Digits with any suffix: "42nd" */
	TokTypeCardSeps,   /**< @CARDEPS: Digits with interpunctuation: "420.24/7" */
	TokTypeUnknown,    /**< @UNKNOWN: Special "Unknown" token-type */
	//TokTypeSpecial,    /**< A literal '@CARD', '@CARDPUNCT', etc. */
	NTokTypes          /**< Not really a token-type */
    } TokenType;

    /** Get the TokenType for a given token */
    inline TokenType token2type(const mootTokString &token)
    {
	mootTokString::const_iterator ti = token.begin();
    
	if (ti==token.end() || !isdigit(*ti)) return TokTypeAlpha;
    
	//-- ^[:digit:]
	for (ti++; ti != token.end() && isdigit(*ti); ti++) {;}  //-- find first non-digit
	//-- ^([:digit:]+)
    
	if (ti == token.end())  //-- ^([:digit:]+)$
	    return TokTypeCard;
    
	else if (ispunct(*ti)) {
	    //-- ^([:digit:]+)([:punct:])
	    for (ti++; ti != token.end() && ispunct(*ti); ti++) {;}
	    //-- ^([:digit:]+)([:punct:]+)
      
	    if (ti == token.end())      //-- ^([:digit:]+)([:punct:]+)$
		return TokTypeCardPunct;
      
	    else if (isdigit(*ti)) {
		//-- ^([:digit:]+)([:punct:]+)([:digit:])
		for (ti++; ti != token.end() && (isdigit(*ti) || ispunct(*ti)); ti++) {;}
		//-- ^([:digit:]+)([:punct:]+)(([:digit:]|[:punct:]+))
		if (ti == token.end())
		    //-- ^([:digit:]+)([:punct:]+)(([:digit:]|[:punct:]+))$
		    return TokTypeCardSeps;
	    }
	}
    
	//-- ^([:digit:]+)([[:digit:][:punct]]*)([^[:digit:][:punct:]])
	for (ti++; ti != token.end() && !isdigit(*ti); ti++) {;}
	//-- ^([:digit:]+)([[:digit:][:punct]]*)([^[:digit:][:punct:]])([^[:digit]]*)
    
	if (ti == token.end())
	    //-- ^([:digit:]+)([[:digit:][:punct]]*)([^[:digit:][:punct:]])([^[:digit]]*)$
	    return TokTypeCardSuffix;
    
	return TokTypeAlpha;
    };

//}; //-- namespace mootTokUtils

/*----------------------------------------------------------------------
 * Symbol Vectors (binary)
 *----------------------------------------------------------------------*/

/** \brief unused
 *
 *  STL symbol-vector utilities
 */
class mootSymbolVectorUtils {
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

/** \brief unused
 *
 * Maps a symbol-vector (sorted ambiguity-set) to a single symbol (ambiguity class)
 */
typedef
  hash_map<FSM::FSMSymbolVector,
	   FSMSymbol,
	   mootSymbolVectorUtils::HashFcn,
	   mootSymbolVectorUtils::EqualFcn>
  mootSymbolVector2SymbolMap;

moot_END_NAMESPACE

#endif /* _moot_TYPES_H */
