/* -*- Mode: C++ -*- */

/*============================================================================
 * File: mootNgrams.h
 * Author:  Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *    Class for storage & retrieval of N-Gram counts
 *============================================================================*/

#ifndef _moot_NGRAMS_H
#define _moot_NGRAMS_H

#include <mootTypes.h>

moot_BEGIN_NAMESPACE

/**
 * \brief Class for storage & retrieval of raw N-Gram frequencies.
 */
class mootNgrams {

public:
  //------ public typedefs
  
  /** Type for a single N-Gram, string form */
  typedef  deque<mootTagString> NgramString;

  /** Type for an N-Gram count */
  typedef CountT NgramCount;
  
  /** Utilities for mootNgrams::NgramString */
  class NgramStringUtils {
  public:
    // -- STL utilities
    struct HashFcn {
    public:
      inline size_t operator()(const NgramString &x) const {
	size_t hv = 0;
	for (NgramString::const_iterator xi = x.begin();
	     xi != x.end();
	     xi++)
	  {
	    for (mootTagString::const_iterator xii = xi->begin();
		 // (*xii) != '\0' &&
		 xii != xi->end();
		 xii++)
	      {
		hv += (hv<<5)-hv + *xii;
	      }
	    //hv += 5*hv + __stl_hash_string(xi->c_str());
	  }
	return hv;
      };
    };
    struct EqualFcn {
    public:
      inline size_t operator()(const NgramString &x, const NgramString &y) const {
	return x==y;
      };
    };
  }; //-- end mootNgrams::NgramStringUtils

  /** Actual N-Gram->Count lookup table typedef */
  typedef
    //map<mootNGramString,float> //--slower
    hash_map<NgramString,
	     float,
	     NgramStringUtils::HashFcn,
	     NgramStringUtils::EqualFcn>
    NgramStringTable;

public:
  //------ public data
  NgramStringTable ngtable;  /**< N-Gram to count lookup table */
  NgramCount       ugtotal;  /**< total number of unigrams */

public:
  //------ public methods
  /** Default constructor */
  mootNgrams(void) : ugtotal(0) {};

  /** Default destructor */
  ~mootNgrams() {
    clear(); 
  };

  //------ public methods: manipulation

  /** Clear the ngrams object */
  void clear(void) {
    ngtable.clear();
    ugtotal = 0;
  };

  /** Add 'count' to the current count for 'ngram', returns new count */
  inline NgramCount add_count(const NgramString &ngram, const NgramCount count)
  {
    /*
    if (ngtable.find(ngram) != ngtable.end()) {
      ngtable[ngram] = count;
    } else {
      ngtable[ngram] += count;
    }
    */
    if (ngram.size() == 1) ugtotal += count;
    return ngtable[ngram] += count;
  };


  //------ public methods: lookup

  /** Returns current count for ngram, returns 0 if unknown */
  inline const NgramCount lookup(const NgramString &ngram) const
  {
    NgramStringTable::const_iterator ngti = ngtable.find(ngram);
    return ngti == ngtable.end() ? 0 : ngti->second;
  };

  /** Returns current count for unigram, returns 0 if unknown */
  inline const NgramCount lookup(const mootTagString &tagstr)
    const
  {
    NgramString ngs;
    ngs.push_back(tagstr);
    return lookup(ngs);
  };

  /** Returns current count for bigram, returns 0 if unknown */
  inline const NgramCount lookup(const mootTagString &prevtagstr, const mootTagString &tagstr)
    const
  {
    NgramString ngs;
    ngs.push_back(prevtagstr);
    ngs.push_back(tagstr);
    return lookup(ngs);
  };

  /** Returns current count for trigram, returns 0 if unknown */
  inline const NgramCount lookup(const mootTagString &prevprevtagstr,
				 const mootTagString &prevtagstr,
				 const mootTagString &tagstr)
    const
  {
    NgramString ngs;
    ngs.push_back(prevprevtagstr);
    ngs.push_back(prevtagstr);
    ngs.push_back(tagstr);
    return lookup(ngs);
  };

  //------ public methods: i/o

  /** Load n-grams from a TnT-style parameter file */
  bool load(char *filename);

  /** Load n-grams from a TnT-style parameter file (stream version) */
  bool load(FILE *file, char *filename = NULL);

  /** Save n-grams to a TnT-style paramater file */
  bool save(char *filename, bool compact=false);

  /** Save n-grams to a TnT-style paramater file (stream version) */
  bool save(FILE *file, char *filename = NULL, bool compact=false);
};

moot_END_NAMESPACE

#endif /* _moot_NGRAMS_H */
