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
  viterbi_clear();
  tokids.clear();
  tagids.clear();
  lexprobs.clear();
  ngprobs1.clear();
  ngprobs2.clear();
}


/*--------------------------------------------------------------------------
 * Compilation
 *--------------------------------------------------------------------------*/
bool dwdstHMM::compile(const dwdstLexfreqs &lexfreqs,
		       const dwdstNgrams &ngrams,
		       const dwdstTagString &start_tag_str,
		       const dwdstLexfreqs::LexfreqCount unknownLexThreshhold)
{
  //-- sanity check
  if (ngrams.ugtotal == 0) {
    fprintf(stderr, "dwdstHMM::compile(): Error bad unigram total in 'ngrams'!\n");
    return 0;
  }

  TokID                       tokid;          //-- current token-ID
  TagID                       tagid;          //-- current tag-ID
  TagID                       ptagid;         //-- previous tag-ID (for bigrams)
  dwdstNgrams::NgramString    ngtmp;          //-- temporary string-ngram (for lookup)
  dwdstNgrams::NgramCount     ptTotal;        //-- previous tag-total (for bigrams)
  dwdstLexfreqs::LexfreqCount unTotal = 0;    //-- total "unknown" token count
  TagProbTable                untagcts;       //-- "unknown" tag counts

  //-- compile lexical probabilities : for all lexical keys (lfti)
  for (dwdstLexfreqs::LexfreqStringTable::const_iterator lfti = lexfreqs.lftable.begin();
       lfti != lexfreqs.lftable.end();
       lfti++)
    {
      const dwdstTokString &tokstr = lfti->first.first;
      const dwdstTagString &tagstr = lfti->first.second;
      const dwdstLexfreqs::LexfreqCount toktotal = lexfreqs.lookup(tokstr);
      const dwdstNgrams::NgramCount     tagtotal = ngrams.lookup(tagstr);

      //-- always get or assign a tag-id
      tagid = tagids.nameExists(tagstr)
	? tagids.name2id(tagstr)
	: tagids.insert(tagstr);

      //-- sanity check
      if (toktotal == 0 || tagtotal == 0) continue;

      //-- unknown threshhold check
      if (toktotal > unknownLexThreshhold) {
	//-- it's a kosher token : get or assign token-ID
	tokid = tokids.nameExists(tokstr)
	  ? tokids.name2id(tokstr)
	  : tokids.insert(tokstr);

	//-- ... and compute lexical probability: p(tok|tag)
	lexprobs[IDPair(tokid,tagid)] = lfti->second / tagtotal;
	//-- ... and compute lexical probability: p(tag|tok) [IMPURE]
	//lexprobs[IDPair(tokid,tagid)] = lfti->second / toktotal;
      }
      else {
	//-- otherwise, it's below the unknown threshhold, so we assign it the constant "unknown" ID
	tokid = 0;

	//-- get or assign tag-ID
	tagid = tagids.nameExists(tagstr)
	  ? tagids.name2id(tagstr)
	  : tagids.insert(tagstr);
	    
	//-- ... and add to "unknown" counts
	unTotal += lfti->second;
	if (untagcts.find(tagid) == untagcts.end()) {
	  untagcts[tagid] = lfti->second;
	} else {
	  untagcts[tagid] += lfti->second;
	}
      }

    }

  //-- Compute "unknown" lexical probabilities
  if (unTotal != 0) {
    IDPair unkey(0,0);
    for (TagProbTable::iterator upi = untagcts.begin(); upi != untagcts.end(); upi++) {
      unkey.second = upi->first;
      lexprobs[unkey] = upi->second / unTotal;
    }
  }

  //-- Compute ngram probabilites
  for (dwdstNgrams::NgramStringTable::const_iterator ngti = ngrams.ngtable.begin();
       ngti != ngrams.ngtable.end();
       ngti++)
    {
      if (ngti->first.size() == 1) {
	//-- congratulations, it's a unigram...
	tagid = tagids.nameExists(ngti->first[0])
	  ? tagids.name2id(ngti->first[0])
	  : tagids.insert(ngti->first[0]);

	//-- compute unigram probability
	ngprobs1[tagid] = ngti->second / ngrams.ugtotal;
      }
      else if (ngti->first.size() == 2) {
	//-- it's a bigram

	//-- get or assign previous tag-id
	ptagid = tagids.nameExists(ngti->first[0])
	  ? tagids.name2id(ngti->first[0])
	  : tagids.insert(ngti->first[0]);

	//-- get or assign current tag-id
	tagid  = tagids.nameExists(ngti->first[1])
	  ? tagids.name2id(ngti->first[1])
	  : tagids.insert(ngti->first[1]);

	//-- get unigram count for previous tag
	ngtmp.clear();
	ngtmp.push_back(ngti->first[0]);
	ptTotal = ngrams.lookup(ngtmp);
	if (ptTotal == 0) continue;

	//-- compute bigram probability
	ngprobs2[IDPair(ptagid,tagid)] = ngti->second / ptTotal;
      }
      //-- Otherwise, it's neither unigram nor bigram: ignore it
    }

  //-- get or assign start-tag-ID
  start_tagid = tagids.nameExists(start_tag_str)
    ? tagids.name2id(start_tag_str)
    : tagids.insert(start_tag_str);

  //-- allocate Viterbi state table(s)
  vtable.resize(tagids.size());
  pvtable.resize(tagids.size());
  viterbi_clear();

  return true;
}

void dwdstHMM::txtdump(FILE *file)
{
  fprintf(file, "%% dwdstHMM text dump\n");

  fprintf(file, "\n");
  fprintf(file, "%%-----------------------------------------------------\n");
  fprintf(file, "%% Lexical Probabilities\n");
  fprintf(file, "%% TokID(\"TokStr\")\tTagID(\"TagStr\")\tp(TokID|TagID)\n");
  fprintf(file, "%%-----------------------------------------------------\n");
  for (LexProbTable::const_iterator lpi = lexprobs.begin(); lpi != lexprobs.end(); lpi++) {
    fprintf(file, "%u(\"%s\")\t%u(\"%s\")\t%g\n",
	    lpi->first.first, tokids.id2name(lpi->first.first).c_str(),
	    lpi->first.second, tagids.id2name(lpi->first.second).c_str(),
	    lpi->second);
  }
  fprintf(file, "\n");

  fprintf(file, "%%-----------------------------------------------------\n");
  fprintf(file, "%% Unigram Probabilities\n");
  fprintf(file, "%% TagID(\"TagStr\")\tp(TagID)\n");
  fprintf(file, "%%-----------------------------------------------------\n");
  for (TagProbTable::const_iterator tpi = ngprobs1.begin(); tpi != ngprobs1.end(); tpi++) {
    fprintf(file, "%u(\"%s\")\t%g\n",
	    tpi->first, tagids.id2name(tpi->first).c_str(),
	    tpi->second);
  }
  fprintf(file, "\n");

  fprintf(file, "%%-----------------------------------------------------\n");
  fprintf(file, "%% Bigram Probabilities\n");
  fprintf(file, "%% PrevTagID(\"PrevTagStr\")\tTagID(\"TagStr\")\tp(TagID|PrevTagID)\n");
  fprintf(file, "%%-----------------------------------------------------\n");
  for (BigramProbTable::const_iterator bpi = ngprobs2.begin(); bpi != ngprobs2.end(); bpi++) {
    fprintf(file, "%u(\"%s\")\t%u(\"%s\")\t%g\n",
	    bpi->first.first,  tagids.id2name(bpi->first.first).c_str(),
	    bpi->first.second, tagids.id2name(bpi->first.second).c_str(),
	    bpi->second);
  }

  fprintf(file, "\n");
}

void dwdstHMM::viterbi_txtdump(FILE *file)
{
  fprintf(file, "%% dwdstHMM Viterbi table text dump\n");
  fprintf(file, "%% TagID(\"TagString\")\t:\t< PATH >\t:\tProb\n");

    for (vtagid = 0             , vsi  = vtable.begin();
	 vtagid < tagids.size() , vsi != vtable.end();
	 vtagid ++              , vsi ++)
      {
	fprintf(file, "%u(%s)\t:\t< ", vtagid, tagids.id2name(vtagid).c_str());
	for (StateSeq::const_iterator pathi = vsi->path.begin();
	     pathi != vsi->path.end();
	     pathi++)
	  {
	    fprintf(file, "%u(\"%s\") ", *pathi, tagids.id2name(*pathi).c_str());
	  }
	fprintf(file, ">\t:\t%g\n", vsi->prob);
      }
}
