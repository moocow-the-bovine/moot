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

namespace moot {
  using namespace std;

/*----------------------------------------------------------------------
 * Basic Types
 *----------------------------------------------------------------------*/

/** Tag-string type */
typedef string mootTagString;

/** Token-string type */
typedef string mootTokString;

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
    void clear(void) {
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
   * NOTE: current analysis-set is NOT cleared by this method.
   */
  inline void tokImport(const mootTokString *src_toktext=NULL,
			const set<mootTagString> *src_tagset=NULL)
  {
    if (src_toktext) tok_text = *src_toktext;
    if (src_tagset) {
      for (set<mootTagString>::const_iterator tsi = src_tagset->begin();
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
   * NOTE: argument tagset is NOT cleared by this method.
   */
  inline void tokExport(mootTokString *dst_toktext=NULL,
			set<mootTagString> *dst_tagset=NULL) const
  {
    if (dst_toktext) *dst_toktext = tok_text;
    if (dst_tagset) {
      for (AnalysisSet::const_iterator asi = tok_analyses.begin();
	   asi != tok_analyses.end();
	   asi++)
	{
	  dst_tagset->insert(asi->tag);
	}
      if (!tok_besttag.empty()) dst_tagset->insert(tok_besttag);
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

}; /* namespace moot */

#endif /* _moot_TOKEN_H */
