/* -*- Mode: C++ -*- */
/*--------------------------------------------------------------------------
 * File: dwdstHMM.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger for DWDS project : Hidden Markov Model (Disambiguator): headers
 *--------------------------------------------------------------------------*/

#ifndef _DWDST_HMM_H
#define _DWDST_HMM_H

#include <hash_map>

#include "dwdstTypes.h"
#include "dwdstLexfreqs.h"
#include "dwdstNgrams.h"
#include "dwdstEnum.h"

/*--------------------------------------------------------------------------
 * dwdstHMM : HMM class
 *--------------------------------------------------------------------------*/

/**
 * \brief 1st-order Hidden Markov Model Tagger/Disambiguator class.
 */
class dwdstHMM {
public:
  // ------ public typedefs
  /** Type for a single probability value */
  typedef float dwdstProb;

  /**
   * Type for a single lexical probability entry (subtable), numeric form.
   * Keys are tag-ids, values are counts.
   */
  typedef hash_map<dwdstEnumID,dwdstProb> LexProbSubtable;
  
public:
  // ------ public data
  
private:
  // ------ private data
  
public:
  // ------ public methods: constructor/destructor
  
};


#endif /* _DWDST_HMM_H */
