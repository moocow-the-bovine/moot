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
 * File: mootSuffixTrie.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moocow's PoS tagger : suffix trie
 *--------------------------------------------------------------------------*/

#include <mootSuffixTrie.h>
#include <stdio.h>
#include <math.h>

moot_BEGIN_NAMESPACE

using namespace std;

//-- define to smooth trie probabilities a la Brants(2000)
//#define SMOOTH_ALA_BRANTS

//-- define to smooth trie probabilities using theoretically correct standard deviation
//#define SMOOTH_ALA_STDDEV

//-- define to smooth trie probabilities a la Brants & Samuelsson(1995)
//   : this seems to work best
#define SMOOTH_ALA_SAMUELSSON

//-- smoothing logic
#if defined(SMOOTH_ALA_BRANTS)
# undef SMOOTH_ALA_STDDEV
# undef SMOOTH_ALA_SAMUELSSON
#elif defined(SMOOTH_ALA_STDDDEV)
# undef SMOOTH_ALA_BRANTS
# undef SMOOTH_ALA_SAMUELSSON
#elif defined(SMOOTH_ALA_SAMUELSSON)
# undef SMOOTH_ALA_BRANTS
# undef SMOOTH_ALA_STDDEV
#endif


//-- define this for verbose debugging
//#define SUFFIX_TRIE_DEBUG
#ifdef SUFFIX_TRIE_DEBUG
# define DEBUG(code) code
#else
# define DEBUG(code)
#endif

/*--------------------------------------------------------------
 * build()
 */
bool SuffixTrie::build(const mootLexfreqs &lf,
		       const mootNgrams   &ng,
		       const TagIDTable   &tagids,
		       TagID eos_tagid,
		       bool  verbose)
{
  //---- sanity check
  if (!maxlen()) return true;

  //---- Report
  if (verbose) { fprintf(stderr, "("); fflush(stderr); }

  //---- insert: enqueue maximum-length suffixes
  if (verbose) { fprintf(stderr, "insert"); fflush(stderr); }
  mootLexfreqs::LexfreqTokTable::const_iterator lfi;
  for (lfi = lf.lftable.begin(); lfi != lf.lftable.end(); lfi++) {
    const mootLexfreqs::LexfreqEntry &lfe = lfi->second;
    if (lfe.count > maxcount
	|| tokenFlavor(lfi->first) != TokFlavorAlpha
	|| isTokFlavorName(lfi->first))
      continue;
    trie_rinsert(lfi->first);
  }
  //---- /insert

  //---- compile: populate adjaceny table
  if (verbose) { fprintf(stderr, ", compile"); fflush(stderr); }
  compile();
  //---- /compile

  //---- associate: assign count data to all suffixes
  if (verbose) { fprintf(stderr, ", associate"); fflush(stderr); }
  TagID tagid;
  mootLexfreqs::LexfreqSubtable::const_iterator lfsi;
  iterator ti;
  for (lfi = lf.lftable.begin(); lfi != lf.lftable.end(); lfi++) {
    const mootLexfreqs::LexfreqEntry &lfe = lfi->second;
    if (lfe.count > maxcount
	|| tokenFlavor(lfi->first) != TokFlavorAlpha
	|| isTokFlavorName(lfi->first))
      continue;

    for (ti  = rfind_longest(lfi->first);
	 ti != end();
	 ti  = find_mother(*ti))
      {
	//-- HACK: store total counts in (node.data[0])
	if (ti->data.find(0) != ti->data.end()) {
	  ti->data[0] += lfe.count;
	} else {
	  ti->data[0]  = lfe.count;
	}

	for (lfsi = lfe.freqs.begin(); lfsi != lfe.freqs.end(); lfsi++) {
	  const string &tagstr = lfsi->first;
	  tagid = tagids.name2id(tagstr);
	  if (tagid != 0) {
	    if (ti->data.find(tagid) != ti->data.end()) {
	      ti->data[tagid] += lfsi->second;
	    } else {
	      ti->data[tagid]  = lfsi->second;
	    }
	  }
	}
      }
  }
  //---- /associate

  //---- sanity check
  if (empty() || begin()->data.size() <= 2) {
    fprintf(stderr, " -> empty trie?!)");
    return false;
  }


  //---- smooth: compute smoothing constants and MLE probabilities P(tag|suffix)
  if (verbose) { fprintf(stderr, ", smooth"); fflush(stderr); }

  //-- some useful variables
  SuffixTrieDataT::iterator  tdi;
  ProbT ugtotal  = ng.ugtotal - ng.lookup(tagids.id2name(eos_tagid));
  ProbT lextotal = lf.n_tokens;              //-- count all lexemes (use real ngrams too!)
  ProbT tagp;


#if defined(SMOOTH_ALA_BRANTS) || defined(SMOOTH_ALA_STDDEV)

# ifdef SMOOTH_ALA_BRANTS
  ProbT ntags    = tagids.size()-2;          //-- use HMM tag count (except UNKNOWN,EOS)
# endif

  //---------------------------------------------------------------
  // theta[Brants|stddev]: compute standard deviation of unigram MLEs (suffix len=0)

  ProbT pavg     = 0;                        //-- E(P(t))

  //-- get average tag probability
  //for (tdi = begin()->data.begin(); tdi != begin()->data.end(); tdi++)
  for (tagid = 1; tagid < tagids.size(); tagid++)
    {
      if (tagid == eos_tagid) continue;
      tagp   = ((ProbT)ng.lookup(tagids.id2name(tagid))) / ugtotal;
      //tagid  = tdi->key();
      //tagp   = tdi->value() / suftotal;
      //ntags += 1.0;
      //--
# ifdef SMOOTH_ALA_BRANTS
      pavg  += tagp;           //-- E_{uniform(|Tags|)} (P_t(·)) [Brants,2000]
# else
      pavg  += (tagp*tagp);    //-- E_{P_t}             (P_t(·)) [stddev]
# endif
    }
# ifdef SMOOTH_ALA_BRANTS
  pavg /= ntags;               //-- E_{uniform(|Tags|)} (P_t(·)) [Brants,2000]
# endif
  //--


  //-- get standard deviation of tag probability
  theta = 0;
  //for (tdi = begin()->data.begin(); tdi != begin()->data.end(); tdi++)
  for (tagid = 1; tagid < tagids.size(); tagid++)
    {
      if (tagid == eos_tagid) continue;
      tagp   = ((ProbT)ng.lookup(tagids.id2name(tagid))) / ugtotal;
# ifdef SMOOTH_ALA_BRANTS
      theta += pow(tagp-pavg, 2);          //-- Var_{uniform(|Tags|)} (P_t(·)) [Brants,2000]
# else
      theta += tagp * pow(tagp-pavg, 2);   //-- Var_{P_t}             (P_t(·)) [stddev]
# endif
    }
# ifdef SMOOTH_ALA_BRANTS
  theta /= (ntags-1);                      //-- Var_{uniform(|Tags|)} (P_t(·)) [Brants,2000]
# endif
  theta = sqrt(theta);                     //-- Var_{P_t}             (P_t(·)) [stddev]
  //---- /theta[Brants|stdddev]

  DEBUG(fprintf(stderr, \
		"\n-> DEBUG: size=%u; lextotal=%g ; ugtotal=%g ; ntags=%g; pavg=%g; theta=%g\n",\
		size(), lextotal, ugtotal, ntags, pavg, theta));
#else // defined(SMOOTH_ALA_BRANTS) || defined(SMOOTH_ALA_STDDEV)

  DEBUG(fprintf(stderr, \
		"\n-> DEBUG: size()=%u; lextotal=%g ; ugtotal=%g ; ntags=%g\n", \
		size(), lextotal, ugtotal, ntags));

#endif // defined(SMOOTH_ALA_BRANTS) || defined(SMOOTH_ALA_STDDEV)

  //---- theta[Samuelsson] : ignore



  //------ smooth:abstract: smooth increasingly more specific suffix probabilities
  SuffixTrieDataT::iterator  tdi_zero;
  list<iterator>             queue;
  queue.push_front(begin());
  while (!queue.empty()) {
    ti = queue.front();
    queue.pop_front();

    //-- get mother Id and iterator
    NodeId           momid = ti->mother;
    const_iterator    momi = find_mother(*ti);

    //-- process current and all sister nodes
    for ( ; ti->mother == momid; ti++) {

      //-- get total, and set pseudo-tag 0 to MLE P(suffix)
      tdi_zero                  = ti->data.find(0);
      ProbT             ticount = tdi_zero->value();
      tdi_zero->value()         = ticount / lextotal;
#ifdef SMOOTH_ALA_SAMUELSSON
      ProbT             sqrtN   = sqrt(ticount);  //-- Samuelsson
#endif

      for (tdi = ti->data.begin(); tdi != ti->data.end(); tdi++) {
	//-- ignore pseudo-key 0 (zero)
	if (tdi->key() == 0) continue;

	//-- root node check
	if (momid == NoNode) {
	  //-- root-node: use P_{MLE}(t)
	  tdi->value() = ((ProbT)ng.lookup(tagids.id2name(tdi->key()))) / ugtotal;
	}
	else {
	  //-- non-root: smooth MLE probs with mother's

	  //... first compute P_{MLE}(t|suffix) for this pair (t,suffix) only
	  tdi->value() /= ticount;

	  ProbT                             momp = 0;
	  SuffixTrieDataT::const_iterator  momdi = momi->data.find(tdi->key());
	  if (momdi != momi->data.end())    momp = momdi->value();

#ifdef SMOOTH_ALA_SAMUELSSON
	  tdi->value() = (sqrtN*tdi->value() + momp) / (1.0+sqrtN);
#else
	  tdi->value() = (tdi->value() + theta*momp) / (1.0+theta);
#endif
	}
      }

      //-- enqueue first daughter (if any)
      queue.push_back(first_dtr(*ti));
      if (queue.back() == end()) queue.pop_back();
    }
  }
  //---- /smooth:abstract

  //DEBUG(return true);

  //------ invert: Bayesian inversion: compute P(suffix|t) from P(t|suffix),P(suffix),P(t)
  if (verbose) { fprintf(stderr, ", invert"); fflush(stderr); }

  for (ti = begin(); ti != end(); ti++) {
    //-- save p(suffix) and erase its pseudo-tag 0 (zero)
    tdi_zero   = ti->data.find(0);
    ProbT psuf = tdi_zero->value();
    ti->data.erase(tdi_zero);

    if (ti == begin()) continue;

    for (tdi = ti->data.begin(); tdi != ti->data.end(); tdi++) {
      tagid = tdi->key();
      tagp  = ((ProbT)ng.lookup(tagids.id2name(tagid))) / ugtotal;
      if (tagp != 0) {
	tdi->value() =
	  //log(
	  (tdi->value() * psuf) / tagp;
	  //)
	  ;
 
      } else {
	//tdi->value() = MOOT_PROB_ZERO;
	tdi->value() = 0;
      }
    }
  }
  //------ /invert

  //-- report
  if (verbose) { fprintf(stderr, ")"); fflush(stderr); }

  return true;
};

#undef DEBUG

moot_END_NAMESPACE
