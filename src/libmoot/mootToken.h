/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2004 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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

/*--------------------------------------------------------------------------
 * File: mootToken.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moocow's PoS tagger : token information
 *--------------------------------------------------------------------------*/

#ifndef _moot_TOKEN_H
#define _moot_TOKEN_H

#define MOOT_TOKEN_VERSION 1
#define MOOT_TOKEN_REVISION 0

#include <ctype.h>

#include <list>
#include <set>
#include <string>

#if defined(__GNUC__)
# include <float.h>

/** Cost value to use when finding lower bounds in mootToken methods */
# define MOOT_COST_LB -FLT_MAX

/** Cost value to use when finding upper bounds in mootToken methods */
# define MOOT_COST_UB +FLT_MAX

#else /* defined(__GNUC__) */

/** Cost value to use when finding lower bounds in mootToken methods */
# define MOOT_COST_LB -1E+37F

/** Cost value to use when finding upper bounds in mootToken methods */
# define MOOT_COST_UB +1E+37F

#endif /* defined(__GNUC__) */

/**
 * \def MOOT_TNT_COMPAT
 * Whether to typify tokens the "right" way, or as TnT does it
 */
#define MOOT_TNT_COMPAT 1
//#undef MOOT_TNT_COMPAT

namespace moot {
  using namespace std;

/*----------------------------------------------------------------------
 * Basic Types
 *----------------------------------------------------------------------*/

/** Tag-string type */
typedef string mootTagString;

/** Token-string type */
typedef string mootTokString;

/** Tagset (read "lexical class") type */
typedef set<mootTagString> mootTagSet;

/*--------------------------------------------------------------------------
 * mootToken
 *--------------------------------------------------------------------------*/
/**
 * \brief High-level token information object
 */
class mootToken {
public:
  /*---------------------------------------------------------------------
   * Embedded Types
   */

  /** Type for analysis weights */
  typedef float Cost;
 
  /** Type for a single morphological analysis */
  class Analysis {
  public:
    /** PoS Tag */
    mootTagString tag;

    /** Full analysis string (possibly with features) */
    mootTagString details;

    /** "Cost" of this analysis (lower=better) */
    Cost          cost;

    /*--------------------------------------------------
     * Constructor / Destructor
     */
    /** Default constructor */
    Analysis(const mootToken::Cost my_cost=0.0)
      : tag(""), details(""), cost(my_cost)
    {};

    /** Constructor given only tag: full analysis defaults to empty */
    Analysis(const mootTagString &my_tag, const mootToken::Cost my_cost=0.0)
      : tag(my_tag), details(""), cost(my_cost)
    {};

    /** Constructor given tag and full analysis */
    Analysis(const mootTagString &my_tag,
	     const mootTagString &my_details,
	     const Cost my_cost=0.0)
      : tag(my_tag), details(my_details), cost(my_cost)
    {};

    /** Clear this object */
    inline void clear(void) {
      tag = "";
      details = "";
      cost = 0.0;
    };

    /** Comparsion operator */
    friend bool operator <(const Analysis &x, const Analysis &y)
    {
      int tcomp = x.tag.compare(y.tag);
      if (tcomp < 0) return true;
      else if (tcomp > 0) return false;
      return (x.cost == y.cost
	      ? x.details < y.details
	      : x.cost < y.cost);
    };

    /** Equality operator */
    friend bool operator ==(const Analysis &x, const Analysis &y)
    {
      return x.tag == y.tag && x.cost == y.cost && x.details == y.details;
    }
    
  }; //-- /mootToken::Analysis


  /** Type for multiple concurrent analyses : tag => set<Analysis> */
  typedef set<Analysis> AnalysisSet;

public:
  /*---------------------------------------------------------------------*
   * Data Members
   */

  /**
   * Literal token text.
   * \warning Use the text() method(s) instead of accessing this directly!
   */
  mootTokString   tok_text;
  
  /**
   * Set of possible analyses as a mootToken::AnalysisSet
   * \warning Use the analyses() method(s) instead of accessing this directly!
   */
  AnalysisSet     tok_analyses;

  /**
   * Best tag for this token.
   * \warning Use the besttag() method(s) instead of accessing this directly!
   */
  mootTagString   tok_besttag;

public:
  /*------------------------------------------------------------
   * Constructors / Destructors
   */
  /** Default constructor: empty text, no analyses */
  mootToken(void)
    : tok_text(""), tok_besttag("")
  {};

  /** Constructor given only token text: no analyses */
  mootToken(const mootTokString &text)
    : tok_text(text), tok_besttag("")
  {};

  /** Constructor given text & analyses */
  mootToken(const mootTokString &text,
	    const AnalysisSet &analyses)
    : tok_text(text), tok_analyses(analyses), tok_besttag("")
  {};

  /** Constructor given text & analyses & best tag */
  mootToken(const mootTokString &text,
	    const AnalysisSet &analyses,
	    const mootTagString &besttag)
    : tok_text(text), tok_analyses(analyses), tok_besttag(besttag)
  {};

  /*------------------------------------------------------------
   * Operators
   */
  /** Equality operator */
  friend bool operator==(const mootToken &x, const mootToken &y)
  {
    return
      x.tok_text == y.tok_text
      && x.tok_besttag == y.tok_besttag
      && x.tok_analyses == y.tok_analyses;
  };

  /** Comparison operaor */
  friend bool operator <(const mootToken &x, const mootToken &y)
  {
    return
      x.tok_text < y.tok_text
      || x.tok_besttag < y.tok_besttag
      || x.tok_analyses < y.tok_analyses;
  };
 

  /*------------------------------------------------------------
   * Manipulators: General
   */
  /** Clear this object */
  inline void clear(void) {
    tok_text.clear();
    tok_analyses.clear();
    tok_besttag.clear();
  };

  /*------------------------------------------------------------
   * Manipulators: specific
   */
  /** Get token text */
  inline const mootTokString &text(void) const {
    return tok_text;
  };
  /** Set token text */
  inline mootTokString &text(const mootTokString &text) {
    tok_text = text;
    return tok_text;
  };

  /** Get best tag */
  inline const mootTagString &besttag(void) const {
    return tok_besttag;
  };
  /** Set best tag */
  inline mootTagString &besttag(const mootTagString &besttag) {
    tok_besttag = besttag;
    return tok_besttag;
  };

  /** Get token analyses */
  inline const AnalysisSet &analyses(void) const {
    return tok_analyses;
  };
  /** Set token analyses */
  inline const AnalysisSet &analyses(const AnalysisSet &analyses)
  {
    tok_analyses = analyses;
    return tok_analyses;
  };
  /** Insert an analysis */
  inline void insert(const Analysis &analysis)
  {
    tok_analyses.insert(analysis);
  };
  /** Remove an analysis */
  inline void erase(const Analysis &analysis)
  {
    tok_analyses.erase(analysis);
  };
  /** Prune analyses, retaining only those for 'besttag' */
  inline void prune(void)
  {
    Analysis bound(tok_besttag,MOOT_COST_LB);
    tok_analyses.erase(tok_analyses.begin(),
		       tok_analyses.lower_bound(bound));
    bound.cost = MOOT_COST_UB;
    tok_analyses.erase(tok_analyses.upper_bound(bound),
		       tok_analyses.end());
  };

  /** Find first analysis (if any) whose tag is <= tag */
  inline AnalysisSet::const_iterator lower_bound(const mootTagString &tag) const
  {
    return tok_analyses.lower_bound(Analysis(tag,MOOT_COST_LB));
  };
  /** Find first analysis (if any) whose tag is > tag */
  inline AnalysisSet::const_iterator upper_bound(const mootTagString &tag) const
  {
    return tok_analyses.upper_bound(Analysis(tag,MOOT_COST_UB));
  };

  /*------------------------------------------------------------
   * Compatibility
   */
  /**
   * For backwards-compatibility: convert old-style 'text,tags' pair
   * to a mootToken.
   *
   * \warning current analysis-set is NOT cleared by this method.
   */
  inline void tokImport(const mootTokString *src_toktext=NULL,
			const mootTagSet    *src_tagset=NULL)
  {
    if (src_toktext) tok_text = *src_toktext;
    if (src_tagset) {
      for (mootTagSet::const_iterator tsi = src_tagset->begin();
	   tsi != src_tagset->end();
	   tsi++)
	{
	  tok_analyses.insert(Analysis(*tsi));
	}
    }
  };

  /**
   * For lazy backwards-compatibility:
   * convert mootToken back to old-style 'text,tagset' pair.
   *
   * @param dst_toktext pointer to destination token-text. NULL (default) means don't export text.
   * @param dst_tagset pointer to destination tagset. NULL (default) means don't export tagset.
   * @param want_besttag_in_tagset whether to include besttag in exported tagset.  Default=true.

   * \warning argument tagset is NOT cleared by this method.
   */
  inline void tokExport(mootTokString *dst_toktext=NULL,
			mootTagSet *dst_tagset=NULL,
			bool want_besttag_in_tagset = true) const
  {
    if (dst_toktext) *dst_toktext = tok_text;
    if (dst_tagset) {
      for (AnalysisSet::const_iterator asi = tok_analyses.begin();
	   asi != tok_analyses.end();
	   //asi++
	   asi = upper_bound(asi->tag)
	   )
	{
	  dst_tagset->insert(asi->tag);
	}
      if (want_besttag_in_tagset && !tok_besttag.empty())
	dst_tagset->insert(tok_besttag);
    }
  };
  
}; //-- /mootToken


/*--------------------------------------------------------------------------
 * mootSentence
 *--------------------------------------------------------------------------*/

/**
 * Sentences are just lists of mootToken objects
 */
typedef list<mootToken> mootSentence;


/*----------------------------------------------------------------------
 * Pattern-based Typification
 *----------------------------------------------------------------------*/

/** Enum for TnT-style token typification */
typedef enum {
  TokTypeAlpha,      /**< (Mostly) alphabetic token: "foo", "bar", "foo2bar" */
  TokTypeCard,       /**< @CARD: Digits-only: "42" */
  TokTypeCardPunct,  /**< @CARDPUNCT: Digits with single-char punctuation suffix: "42." */
  TokTypeCardSuffix, /**< @CARDSUFFIX: Digits with (almost any) suffix: "42nd" */
  TokTypeCardSeps,   /**< @CARDEPS: Digits with interpunctuation: "420.24/7" */
  TokTypeUnknown,    /**< @UNKNOWN: Special "Unknown" token-type */
  //TokTypeSpecial,    /* A literal '@CARD', '@CARDPUNCT', etc. */
  NTokTypes          /**< Not really a token-type */
} TokenType;

/** Convert token-types to symbolic names */
const char TokenTypeNames[NTokTypes][16] =
  {
    "@ALPHA",
    "@CARD",
    "@CARDPUNCT",
    "@CARDSUFFIX",
    "@CARDSEPS",
    "@UNKNOWN"
  };

/** TnT compatibility hack */
inline bool tok2type_isCardPunctChar(const char c) {
  return
#if !defined(MOOT_TNT_COMPAT)
    (ispunct(c));             //-- This is the "right" way to do it
#else
    (c=='.'||c==','||c=='-'); //-- ... but TnT seems to do it this way
#endif // MOOT_TNT_COMPAT
};

/** TnT compatibility hack */
inline bool tok2type_isCardSuffixChar(const char c) {
  //bool answer = !tok2type_isCardPunctChar(c);
  //bool answer = !ispunct(c);
  //fprintf(stderr, "tok2type_isCardSuffixChar(%c)=%d\n", c, answer);
  //return answer;
  return true;
};

/** Get the TokenType for a given token */
inline TokenType token2type(const mootTokString &token)
{
  mootTokString::const_iterator ti = token.begin();
  bool leading_punct = false;

  if (ti==token.end()) return TokTypeAlpha;
  else if (tok2type_isCardPunctChar(*ti)) {
    leading_punct = true;
    ti++;
  }
  
  if (!isdigit(*ti))
    return TokTypeAlpha;
    
  //-- ^[:digit:]
  for (ti++; ti != token.end() && isdigit(*ti); ti++) {;}  //-- find first non-digit
  //-- ^([:digit:]+)
  
  if (ti == token.end()) {
    //-- ^([:digit:]+)$
    if (!leading_punct) return TokTypeCard;
    else return TokTypeCardSeps;
  }

  else if (tok2type_isCardPunctChar(*ti)) {
    //-- ^([:digit:]+)([:CardPunct:])

    if (++ti == token.end())
      //-- ^([:digit:]+)([:CardPunct:])$
      return TokTypeCardPunct;

    else if (isdigit(*ti)  || tok2type_isCardPunctChar(*ti)) {
      //-- ^([:digit:]+)([:CardPunct:])([:digit:])
      for (ti++; ti != token.end() && (isdigit(*ti) || tok2type_isCardPunctChar(*ti)); ti++) {;}
      //-- ^([:digit:]+)([:CardPunct:])([[:digit:][:CardPunct:]]+)
      if (ti == token.end())
	//-- ^([:digit:]+)([:CardPunct:])([[:digit:]|[:CardPunct:]]+)$
	return TokTypeCardSeps;
    }
  }

#if defined(MOOT_TNT_COMPAT)
  //-- allow at most 3-character suffixes
  for (int i=0 ; ti != token.end() && i < 3 ; ti++, i++) {;}
  //-- ^([:digit:]+)([[:digit:][:CardPunct]]*)([^[:digit:][:CardPunct:]])(.{0,3})

  if (ti == token.end())
    //-- ^([:digit:]+)([[:digit:][:CardPunct]]*)([^[:digit:][:CardPunct:]])(.{0,3})$
    return TokTypeCardSuffix;

#else // !defined(MOOT_TNT_COMPAT)
  //-- allow suffixes of arbitrary length
  //for ( ; ti != token.end() && tok2type_isCardSuffixChar(*ti); ti++) {;}
  //-- ^([:digit:]+)([[:digit:][:CardPunct]]*)(([^[:digit:][:CardPunct:]]+)?)([:CardSuffixChar:]*)
  return TokTypeCardSuffix;

#endif // MOOT_TNT_COMPAT
  
  return TokTypeAlpha;
};


}; /* namespace moot */

#endif /* _moot_TOKEN_H */
