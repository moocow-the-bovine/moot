/*-*- Mode: C++ -*-*/
/*----------------------------------------------------------------------
 * Name: dwdstTypes.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + shared typedefs for libdwdst
 *----------------------------------------------------------------------*/
/** \file dwdstTypes.h
 * \brief Common typedefs and constants.
 */

#ifndef _DWDST_TYPES_H
#define _DWDST_TYPES_H

#include <string>
#include <deque>
#include <map>
#include <set>
//#include <pair>
//#include <hash_map>
//#include <hash_set>

#include <hash_map.h>
#include <hash_set.h>
#if defined(__GNUC__)
#  if __GNUC__ >= 3
#    if __GNUC_MINOR__ == 0
#      define DWDST_STL_NAMESPACE std
#    else
#      define DWDST_STL_NAMESPACE __gnu__cxx
#    endif /* __GNUC_MINOR__ == 0 */
#  else  /* __GNUC__ >= 3 */
#  endif /* __GNUC__ >= 3 */
#else  /* defined(__GNUC__) */
#  define DWDST_STL_NAMESPACE std
#endif /* defiend(__GNUC__) */

#include <FSM.h>
#include <FSMSymSpec.h>

/** Whether to load symspecs in AT&T-compatibility mode */
#define DWDST_SYM_ATT_COMPAT true

/* Namespace definitions: not yet ready */
#define DWDST_NAMESPACE dwdst
#define DWDST_BEGIN_NAMESPACE namespace dwdst {
#define DWDST_END_NAMESPACE };
//#define DWDST_NAMESPACE
//#define DWDST_BEGIN_NAMESPACE
//#define DWDST_END_NAMESPACE

DWDST_BEGIN_NAMESPACE

using namespace std;

/*----------------------------------------------------------------------
 * Basic Types
 *----------------------------------------------------------------------*/

/** Tag-string type */
typedef string dwdstTagString;

/** Token-string type */
typedef string dwdstTokString;

/** Count types (for raw frequencies) */
typedef float CountT;

/*----------------------------------------------------------------------
 * Token Typification
 *----------------------------------------------------------------------*/

//namespace dwdstTokenUtils {
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
    inline TokenType token2type(const dwdstTokString &token)
    {
	dwdstTokString::const_iterator ti = token.begin();
    
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

//}; //-- namespace dwdstTokUtils

/*----------------------------------------------------------------------
 * Symbol Vectors (binary)
 *----------------------------------------------------------------------*/

/** \brief unused
 *
 *  STL symbol-vector utilities
 */
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

/** \brief unused
 *
 * Maps a symbol-vector (sorted ambiguity-set) to a single symbol (ambiguity class)
 */
typedef
  hash_map<FSM::FSMSymbolVector,
	   FSMSymbol,
	   dwdstSymbolVectorUtils::HashFcn,
	   dwdstSymbolVectorUtils::EqualFcn>
  dwdstSymbolVector2SymbolMap;

DWDST_END_NAMESPACE

#endif /* _DWDST_TYPES_H */
