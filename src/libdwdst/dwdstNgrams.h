/* -*- Mode: C++ -*- */

/*============================================================================
 * File: dwdstNgrams.h
 * Author:  Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *    Class for storage & retrieval of N-Gram counts
 *============================================================================*/

#ifndef _DWDST_NGRAMS_H
#define _DWDST_NGRAMS_H

#include <dwdstTypes.h>

/**
 * Class for storage & retrieval of N-Gram counts
 */
class dwdstNgrams {
public:
  //------ public typedefs
  
  /** Type for a single N-Gram, string form */
  typedef  deque<dwdstTagString> NgramString;
  
  /** Type for an N-Gram count */
  typedef float NgramCount;

  /** Utilities for dwdstNgrams::NgramString */
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
	    for (dwdstTagString::const_iterator xii = xi->begin();
		 *xii != '\0' && xii != xi->end();
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
  }; //-- end dwdstNgrams::NgramStringUtils

  /** Actual N-Gram->Count lookup table typedef */
  typedef
    //map<dwdstNGramString,float> //--slower
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
  dwdstNgrams() {};

  //------ public methods: manipulation

  /** Add 'count' to the current count for 'ngram', returns new count */
  inline NgramCount add_count(const NgramString &ngram, const NgramCount count)
  {
    if (ngtable.find(ngram) != ngtable.end()) {
      ngtable[ngram] = count;
    } else {
      ngtable[ngram] += count;
    }
    if (ngram.size() == 1) ugtotal += count;
    return ngtable[ngram];
  };


  //------ public methods: lookup

  /** Returns current count for ngram, returns 0 if unknown */
  inline const NgramCount lookup(const NgramString &ngram)
  {
    NgramStringTable::const_iterator ngti = ngtable.find(ngram);
    return ngti == ngtable.end() ? 0 : ngti->second;
  }


  //------ public methods: i/o

  /** Load n-grams from a TnT-style parameter file */
  bool load(char *filename);

  /** Load n-grams from a TnT-style parameter file (stream version) */
  bool load(FILE *file, char *filename = NULL);

  /** Save n-grams to a TnT-style paramater file */
  bool save(char *filename);

  /** Save n-grams to a TnT-style paramater file (stream version) */
  bool save(FILE *file, char *filename = NULL);
};



#endif /* _DWDST_NGRAMS_H */
