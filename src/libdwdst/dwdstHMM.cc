/*--------------------------------------------------------------------------
 * File: dwdstHMM.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger for DWDS project : 1st-order HMM tagger/disambiguator
 *--------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
# include <dwdstConfig.h>
#endif

#include <stdio.h>
#include "dwdstHMM.h"

using namespace std;

/*--------------------------------------------------------------------------
 * Reset
 *--------------------------------------------------------------------------*/

void dwdstHMM::clear(void)
{
  tokids.clear();
  tagids.clear();
  ngprobs1.clear();

  for (LexProbTable::iterator lpi = lexprobs.begin(); lpi != lexprobs.end(); lpi++) {
    if (lpi->second != NULL) {
      lpi->second->clear();
      delete lpi->second;
      lpi->second = NULL;
    }
  }
  lexprobs.clear();

  for (BigramProbTable::iterator bgi = ngprobs2.begin(); bgi != ngprobs2.end(); bgi++) {
    if (bgi->second != NULL) {
      bgi->second->clear();
      delete bgi->second;
      bgi->second = NULL;
    }
  }
  ngprobs2.clear();
}


/*--------------------------------------------------------------------------
 * Compilation
 *--------------------------------------------------------------------------*/
bool dwdstHMM::compile(const dwdstLexfreqs &lexfreqs,
		       const dwdstNgrams &ngrams,
		       const dwdstLexfreqs::LexfreqCount unknownLexThreshhold)
{
  fprintf(stderr, "dwdstHMM::compile(): not yet implemented!\n");

  dwdstLexfreqs::LexfreqCount unTotal;  //-- total "unknown" token count

  //-- prepare tokids
  for (dwdstLexfreqs::LexfreqTotalTable::const_iterator lftoti = lexfreqs.lftotals.begin();
       lftoti != lexfreqs.lftotals.end();
       lftoti++)
    {
      if (lftoti->second > unknownLexThreshhold) {
	//-- it's a kosher token
	if (!tokids.nameExists(lftoti->first))
	  tokids.insert(lftoti->first);
      }
      else {
	//-- otherwise, it's below the unknown threshhold, so we can skip assigning it an id
	unTotal += lftoti->second;
      }
      
      //-- get tag ids from Lexfreqs, too...
      dwdstLexfreqs::LexfreqSubtable *subt = lexfreqs.lftable[lftoti->first];
      if (subt != NULL) {
	for (dwdstLexfreqs::LexfreqSubtable::const_iterator subti = subt->begin();
	     subti != subt->end();
	     subti++)
	  {
	    if (!tagids.nameExists(subti->first))
	      tagids.insert(subti->first);
	  }
      }
    }
  
  //-- get tag-ids from ngrams
  for (dwdstNgrams::NgramStringTable::const_iterator ngti = ngrams.ngtable.begin();
       ngti != ngrams.ngtable.end();
       ngti++)
    {
      if (ngti.size() == 1) {
	//-- congratulations, it's a unigram...
	if (!tagids.nameExists(ngti->first))
	  tagids.insert(ngti->first);
      }
      //-- otherwise, skip it.
    }

  //-- compute lexical probabilities.
  TokID tokid;
  TagID tagid;
  for (dwdstLexfreqs::LexfreqStringTable::const_iterator lfi = lexfreqs.lftable.begin();
       lfi != lexfreqs.lftable.end();
       lfi++)
    {
      if (lfi->second == NULL) continue; //-- skip empty subtables

      //--- ARGH. continue here!

      //-- ensure we have a subtable for this token
      tokid = tokids.name2id(lfi->first);
      if (lexprobs.find(tokid) == lexprobs.end() || lexprobs[tokid] == NULL)
	lexprobs[tokid] = new TagProbTable();

      for (dwdstLexfreqs::LexfreqSubtable::const_iterator lfsi = lfi->second->begin();
	   lfsi != lfi->second->end();
	   lfsi++)
	{
	  tagid = tagids.name2id(lfsi->first);
	  
	}
    }



  return false;
}
