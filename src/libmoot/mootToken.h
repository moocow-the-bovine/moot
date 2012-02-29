/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2010 by Bryan Jurish <moocow@cpan.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/*--------------------------------------------------------------------------
 * File: mootToken.h
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moocow's PoS tagger : token information
 *--------------------------------------------------------------------------*/

/**
\file mootToken.h
\brief classes and utilities for tokens and associated analyses
*/

#ifndef _moot_TOKEN_H
#define _moot_TOKEN_H

#include <ctype.h>

#include <list>
#include <vector>
#include <mootTypes.h>

/**
 * \def MOOT_TNT_COMPAT
 * Whether to typify token text as TnT does it, or the "right" way
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


/*----------------------------------------------------------------------
 * Basic Token Types
 *----------------------------------------------------------------------*/
enum mootTokenTypeE {
  /* Output token-types */
  TokTypeUnknown,    /**< we dunno what it is -- could be anything  */
  TokTypeVanilla,    /**< plain "vanilla" token (+/-besttag,+/-analyses) */
  TokTypeLibXML,     /**< plain XML token; much like 'Vanilla' */
  TokTypeXMLRaw,     /**< Raw XML text (for lossless XML I/O) */
  TokTypeComment,    /**< a comment, should be ignored by processing routines */
  TokTypeEOS,        /**< end-of-sentence */
  TokTypeEOF,        /**< end-of-file */
  TokTypeUser,       /**< user-defined token type: use in conjunction with 'tok_data' */
  NTokTypes          /**< number of token-types (not a type itself) */
};
typedef mootTokenTypeE mootTokenType;

/** Useful for debugging token types */
extern const char* mootTokenTypeNames[NTokTypes];

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

  /** \brief Type for a single morphological analysis */
  class Analysis {
  public:
    /** Type flag */
    //mootTokenType type;

    /** PoS Tag */
    mootTagString tag;

    /** Full analysis string (possibly with features) */
    mootTagString details;

    /** Analysis probability (or "cost" or "weight" or ...) */
    ProbT prob;

    /**
     * Additional user data for this analysis, defaults to NULL.
     * User is responsible for memory management.
     */
    void *data;

    /*--------------------------------------------------
     * Constructor / Destructor
     */
    /** Default constructor */
    Analysis(void)
      : prob(0),
	data(NULL)
    {};

    /** Constructor given only tag: full analysis defaults to empty */
    Analysis(const mootTagString &my_tag
	     //, mootTokenType typ=TokTypeVanilla
	     )
      : tag(my_tag),
	prob(0),
	data(NULL)
    {};

    /** Constructor given tag and full analysis */
    Analysis(const mootTagString &my_tag,
	     const mootTagString &my_details)
      : tag(my_tag),
	details(my_details),
	prob(0),
	data(NULL)
    {};

    /** Constructor given tag, full analysis, and probability */
    Analysis(const mootTagString &my_tag,
	     const mootTagString &my_details,
	     const ProbT my_prob)
      : tag(my_tag),
	details(my_details),
	prob(my_prob),
	data(NULL)
    {};

    /** Clear this object (except for data) */
    inline void clear(void) {
      tag.clear();
      details.clear();
      prob = 0;
    };

    /** Check for empty analysis*/
    inline bool empty(void) const {
      return tag.empty() && details.empty();
    }

    /** Comparsion operator */
    friend bool operator<(const Analysis &x, const Analysis &y)
    {
      if (x.prob != y.prob) return x.prob < y.prob;
      int tcomp = x.tag.compare(y.tag);
      return (tcomp < 0
	      ? true
	      : (tcomp > 0
		 ? false
		 : x.details < y.details));
    };

    /** Equality operator */
    friend bool operator==(const Analysis &x, const Analysis &y)
    {
      return x.prob == y.prob && x.tag == y.tag && x.details == y.details;
    }
    
  }; //-- /mootToken::Analysis


  /** Type for multiple concurrent analyses : tag => set<Analysis> */
  //typedef set<Analysis> AnalysisSet;
  //typedef vector<Analysis> Analyses;
  typedef list<Analysis> Analyses;

  /** Type for token locations (in input stream) */
  class Location {
  public:
    OffsetT offset;  ///< (byte) offset in input stream (default=0)
    OffsetT length;  ///< (byte) length in input stream (default=0)
  public:
    /** Default constructor */
    inline Location(const OffsetT my_offset=0, const OffsetT my_length=0)
      : offset(my_offset),
	length(my_length)
    {};
    /** Clear this object (reset to defaults) */
    inline void clear(void)
    {
      offset = 0;
      length = 0;
    };
  }; //-- /mootToken::Location

public:
  /*---------------------------------------------------------------------*
   * Data Members
   */

  /**
   * Token content type.
   */
  mootTokenType   tok_type;

  /**
   * Literal token text.
   */
  mootTokString   tok_text;
  
  /**
   * Best tag for this token.
   */
  mootTagString   tok_besttag;

  /**
   * Set of possible analyses as a mootToken::AnalysisSet
   */
  Analyses       tok_analyses;

  /**
   * Token location in input stream, for backtraces (optional)
   */
  Location       tok_location;

  /**
   * Additional user data for this token, defaults to NULL.
   * User is responsible for memory management.
   */
  void *tok_data;

public:
  /*------------------------------------------------------------
   * Constructors / Destructors
   */
  /** Default constructor: empty text, no analyses */
  mootToken(mootTokenType type=TokTypeVanilla)
    : tok_type(type),
      tok_data(NULL)
  {};

  /** Constructor given only token text: no analyses */
  mootToken(const mootTokString &text, mootTokenType type=TokTypeVanilla)
    : tok_type(type),
      tok_text(text),
      tok_data(NULL)
  {};

  /** Constructor given text & analyses */
  mootToken(const mootTokString &text,
	    const Analyses &analyses)
    : tok_type(TokTypeVanilla),
      tok_text(text),
      tok_analyses(analyses),
      tok_data(NULL)
  {};

  /** Constructor given text & analyses & best tag */
  mootToken(const mootTokString &text,
	    const Analyses &analyses,
	    const mootTagString &besttag)
    : tok_type(TokTypeVanilla),
      tok_text(text),
      tok_besttag(besttag),
      tok_analyses(analyses),
      tok_data(NULL)
  {};

  /** Constructor for text-only tokens from C buffer of known length */
  /*
  mootToken(mootTokenType type=TokTypeVanilla, const char *text, size_t len)
    : tok_type(type),
      tok_text(text,len),
      tok_data(NULL)
  {};
  */

  /* Destructor */
  ~mootToken(void) {};

  /*------------------------------------------------------------
   * Operators
   */
  /** Equality operator */
  friend bool operator==(const mootToken &x, const mootToken &y)
  {
    return
      x.tok_type == y.tok_type
      && x.tok_text == y.tok_text
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
  /** Clear this object (except for tok_data) */
  inline void clear(void) {
    tok_type = TokTypeVanilla;
    tok_text.clear();
    tok_besttag.clear();
    tok_analyses.clear();
    tok_location.clear();
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
  /** Set token text */
  inline mootTokString &text(const char *s, size_t len) {
    tok_text.assign(s,len);
    return tok_text;
  }; 
  /** Append to token text, returns new token-text */
  inline mootTokString &textAppend(const mootTokString &text) {
    tok_text.append(text);
    return tok_text;
  };
  /** Append to token text, returns new token-text */
  inline mootTokString &textAppend(const char *s, size_t len) {
    tok_text.append(s, len);
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
  /** Append to 'besttag' text, returns new text */
  inline mootTagString &besttagAppend(const mootTagString &text) {
    tok_besttag.append(text);
    return tok_besttag;
  };
  /** Append to 'besttag' text, returns new text */
  inline mootTagString &besttagAppend(const char *s, size_t len) {
    tok_besttag.append(s, len);
    return tok_besttag;
  };

  /** Get token type */
  inline mootTokenType toktype(void) const { return tok_type; }
  /** Set token type */
  inline mootTokenType toktype(const mootTokenType type) {
    tok_type = type;
    return tok_type;
  };

  /** Get token analyses */
  inline const Analyses &analyses(void) const { return tok_analyses; };
  /** Set token analyses */
  inline const Analyses &analyses(const Analyses &analyses) {
    tok_analyses = analyses;
    return tok_analyses;
  };
  /** Insert an analysis */
  inline void insert(const Analysis &analysis)
  {
    //tok_analyses.insert(analysis);
    tok_analyses.push_back(analysis);
  };
  /** Insert a new analysis */
  inline void insert(const mootTagString &tag, const mootTagString &details, ProbT p=0)
  {
    //insert(Analysis(tag,details));
    tok_analyses.push_back(Analysis());
    tok_analyses.back().tag = tag;
    tok_analyses.back().details = details;
    tok_analyses.back().prob = p;
  };
  /** Insert a new analysis, C-style */
  inline void insert(const char *tag, const char *details, ProbT p=0)
  {
    //insert(Analysis(tag,details));
    tok_analyses.push_back(Analysis());
    tok_analyses.back().tag = tag;
    tok_analyses.back().details = details;
    tok_analyses.back().prob = p;
  };
  /** Test presence of an analysis for 'tag' */
  inline bool has_analysis_for_tag(const mootTagString &tag) const
  {
    for (Analyses::const_iterator asi = tok_analyses.begin();
	 asi != tok_analyses.end();
	 asi++)
      {
	if (asi->tag == tag) return true;
      }
    return false;
  };
  /** Remove an analysis */
  inline void erase(const Analysis &analysis)
  {
    for (Analyses::iterator asi = tok_analyses.begin();
	 asi != tok_analyses.end();
	 )
      {
	if (*asi == analysis) tok_analyses.erase(asi);
	else asi++;
      }
  };
  /** Prune analyses, retaining only those for 'besttag' */
  inline void prune(void)
  {
    for (Analyses::iterator asi = tok_analyses.begin();
	 asi != tok_analyses.end();
	 )
      {
	if (asi->tag != tok_besttag) tok_analyses.erase(asi);
	else asi++;
      }
  };

  /** Get token location */
  inline const Location &location(void) const
  { return tok_location; }

  /** Set token location */
  inline const Location &location(const Location &loc)
  { tok_location=loc; return location(); }

  /** Set token location directly from member variables */
  inline const Location &location(const OffsetT offset, const OffsetT length=0)
  { return location(Location(offset,length)); };

  /** Get token location offset */
  inline OffsetT loc_offset(void) const { return tok_location.offset; };

  /** Set token location offset */
  inline OffsetT loc_offset(const OffsetT off)
  { tok_location.offset=off; return loc_offset(); };

  /** Get token location length */
  inline OffsetT loc_length(void) const { return tok_location.length; };

  /** Set token location length */
  inline OffsetT loc_length(const OffsetT len)
  { tok_location.length=len; return loc_length(); };


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
	  insert(Analysis(*tsi));
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
      for (Analyses::const_iterator asi = tok_analyses.begin();
	   asi != tok_analyses.end();
	   asi++
	     //asi = upper_bound(asi->tag)
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

/** Sentences are just lists of mootToken objects */
typedef list<mootToken> mootSentence;

/** Sentences are just vectors of mootToken objects */
//typedef vector<mootToken> mootSentence;

/** Utilitiy method to add a printf()-formatted token at the end of \c s */
mootToken &sentence_printf_append(mootSentence &s, mootTokenType typ, const char *fmt, ...);

/*----------------------------------------------------------------------
 * Pattern-based Typification
 *----------------------------------------------------------------------*/

#if 1
/** Enum for TnT-style token typification */
typedef enum {
  TokFlavorAlpha,      /**< (Mostly) alphabetic token: "foo", "bar", "foo2bar" */
  TokFlavorCard,       /**< \p \@CARD: Digits-only: "42" */
  TokFlavorCardPunct,  /**< \p \@CARDPUNCT: Digits single-char punctuation suffix: "42." */
  TokFlavorCardSuffix, /**< \p \@CARDSUFFIX: Digits with (almost any) suffix: "42nd" */
  TokFlavorCardSeps,   /**< \p \@CARDSEPS: Digits with interpunctuation: "420.24/7" */
  TokFlavorUnknown,    /**< \p \@UNKNOWN: Special "Unknown" token-type */
  //TokFlavorSpecial,    /* A literal "\@CARD", "\@CARDPUNCT", etc. */
  NTokFlavors          /**< Not really a token-type */
} mootTokenFlavor;


/** Convert token-types to symbolic names */
extern const char *mootTokenFlavorNames[NTokFlavors];

/** TnT compatibility hack */
inline bool tokenFlavor_isCardPunctChar(const char c) {
  return
#if !defined(MOOT_TNT_COMPAT)
    (ispunct(c));             //-- This is the "right" way to do it
#else
    (c=='.'||c==','||c=='-'); //-- ... but TnT seems to do it this way
#endif // MOOT_TNT_COMPAT
};

/** TnT compatibility hack */
inline bool tokenFlavor_isCardSuffixChar(const char c) {
  //bool answer = !tokenFlavor_isCardPunctChar(c);
  //bool answer = !ispunct(c);
  //fprintf(stderr, "tokenFlavor_isCardSuffixChar(%c)=%d\n", c, answer);
  //return answer;
  return true;
};

/** Get the TokenType for a given token */
inline mootTokenFlavor tokenFlavor(const mootTokString &token)
{
  mootTokString::const_iterator ti = token.begin();
  bool leading_punct = false;

  if (ti==token.end()) return TokFlavorAlpha;
  else if (tokenFlavor_isCardPunctChar(*ti)) {
    leading_punct = true;
    ti++;
  }
  
  if (!isdigit(*ti))
    return TokFlavorAlpha;
    
  //-- ^[:digit:]
  for (ti++; ti != token.end() && isdigit(*ti); ti++) {;}  //-- find first non-digit
  //-- ^([:digit:]+)
  
  if (ti == token.end()) {
    //-- ^([:digit:]+)$
    if (!leading_punct) return TokFlavorCard;
    else return TokFlavorCardSeps;
  }

  else if (tokenFlavor_isCardPunctChar(*ti)) {
    //-- ^([:digit:]+)([:CardPunct:])

    if (++ti == token.end())
      //-- ^([:digit:]+)([:CardPunct:])$
      return TokFlavorCardPunct;

    else if (isdigit(*ti)  || tokenFlavor_isCardPunctChar(*ti)) {
      //-- ^([:digit:]+)([:CardPunct:])([:digit:])
      for (ti++; ti != token.end() && (isdigit(*ti) || tokenFlavor_isCardPunctChar(*ti)); ti++) {;}
      //-- ^([:digit:]+)([:CardPunct:])([[:digit:][:CardPunct:]]+)
      if (ti == token.end())
	//-- ^([:digit:]+)([:CardPunct:])([[:digit:]|[:CardPunct:]]+)$
	return TokFlavorCardSeps;
    }
  }

#if defined(MOOT_TNT_COMPAT)
  //-- allow only suffixes of length <= 3 characters
  for (int i=0 ; ti != token.end() && i < 3 ; ti++, i++) {;}
  //-- ^([:digit:]+)([[:digit:][:CardPunct]]*)([^[:digit:][:CardPunct:]])(.{0,3})

  if (ti == token.end())
    //-- ^([:digit:]+)([[:digit:][:CardPunct]]*)([^[:digit:][:CardPunct:]])(.{0,3})$
    return TokFlavorCardSuffix;

#else // !defined(MOOT_TNT_COMPAT)
  //-- allow suffixes of arbitrary length
  //for ( ; ti != token.end() && tokenFlavor_isCardSuffixChar(*ti); ti++) {;}
  //-- ^([:digit:]+)([[:digit:][:CardPunct]]*)(([^[:digit:][:CardPunct:]]+)?)([:CardSuffixChar:]*)
  return TokFlavorCardSuffix;

#endif // MOOT_TNT_COMPAT
  
  return TokFlavorAlpha;
};

/** Returns true iff \a tokstr is a pseudo-identifier for a non-alpha type
 *  Used during HMM and trie compilation */
inline bool isTokFlavorName(const mootTokString &tokstr)
{
  int i;
  for (i = 0; i < NTokFlavors; i++) {
    if (tokstr == mootTokenFlavorNames[i]) return true;
  }
  return (tokstr == "@USECASE");
};

#endif /* if 0: flavors */

}; /* namespace moot */

#endif /* _moot_TOKEN_H */
