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

#include <list>
#include <set>
#include <string>

/*#include "mootTypes.h"*/

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
      return x.cost < y.cost || x.tag < y.tag || x.details < y.details;
    };

    /** Equality operator */
    friend bool operator ==(const Analysis &x, const Analysis &y)
    {
      return x.cost == y.cost && x.tag == y.tag && x.details == y.details;
    }
    
  }; //-- /mootToken::Analysis


  /** Type for multiple concurrent analyses : tag => set<Analysis> */
  typedef set<Analysis> AnalysisSet;

public:
  /*---------------------------------------------------------------------*
   * Data Members
   */

  /** Literal token text */
  mootTokString toktext;

  /** Set of possible analyses as a mootToken::AnalysisSet */
  AnalysisSet   analyses;

  /** Best tag for this token */
  mootTagString         besttag;

  /*---------------------------------------------------------------------*
   * Constructors / Destructors
   */
  /** Default constructor: empty text, no analyses */
  mootToken(void)
    : toktext(""), besttag("")
  {};

  /** Constructor given only token text: no analyses */
  mootToken(const mootTokString &my_text)
    : toktext(my_text), besttag("")
  {};

  /** Constructor given text & analyses */
  mootToken(const mootTokString &my_text,
	    const mootToken::AnalysisSet &my_analyses)
    : toktext(my_text), analyses(my_analyses), besttag("")
  {};

  /** Clear this object */
  void clear(void) {
    toktext = "";
    analyses.clear();
    besttag = "";
  };


  /**
   * For backwards-compatibility: convert old-style 'text,tags' pair
   * to a mootToken.
   *
   * NOTE: current analysis-set is NOT cleared by this method.
   */
  inline void tokImport(const mootTokString *src_toktext=NULL,
			const set<mootTagString> *src_tagset=NULL)
  {
    if (src_toktext) toktext = *src_toktext;
    if (src_tagset) {
      for (set<mootTagString>::const_iterator tsi = src_tagset->begin();
	   tsi != src_tagset->end();
	   tsi++)
	{
	  analyses.insert(Analysis(*tsi));
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
    if (dst_toktext) *dst_toktext = toktext;
    if (dst_tagset) {
      for (AnalysisSet::const_iterator asi = analyses.begin();
	   asi != analyses.end();
	   asi++)
	{
	  dst_tagset->insert(asi->tag);
	}
    }
  };

  
}; //-- /mootToken


/** Sentences are just lists of mootToken objects */
typedef list<mootToken> mootSentence;

}; /* namespace moot */

#endif /* _moot_TOKEN_H */
