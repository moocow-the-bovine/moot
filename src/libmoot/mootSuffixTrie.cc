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

/*--------------------------------------------------------------
 * compile()
 */
void SuffixTrie::compile(const mootLexfreqs &lf,
			 const TagIDTable &tagids,
			 bool  verbose)
{
  //---- sanity check
  if (!maxlen()) return;

  //---- Report
  if (verbose) { fprintf(stderr, "Building trie ("); fflush(stderr); }

  //---- insert: enqueue maximum-length suffixes
  if (verbose) { fprintf(stderr, "insert"); fflush(stderr); }
  mootLexfreqs::LexfreqTokTable::const_iterator lfi;
  for (lfi = lf.lftable.begin(); lfi != lf.lftable.end(); lfi++) {
    const mootLexfreqs::LexfreqEntry &lfe = lfi->second;
    if (lfe.count > maxcount) continue;
    trie_rinsert(lfi->first);
  }
  //---- /insert

  //---- compile: populate adjaceny table
  if (verbose) { fprintf(stderr, ", compile"); fflush(stderr); }
  TrieType::compile();
  //---- /compile

  //---- associate: assign count data to all suffixes
  if (verbose) { fprintf(stderr, ", associate"); fflush(stderr); }
  TagID tagid;
  mootLexfreqs::LexfreqSubtable::const_iterator lfsi;
  iterator ti;
  for (lfi = lf.lftable.begin(); lfi != lf.lftable.end(); lfi++) {
    const mootLexfreqs::LexfreqEntry &lfe = lfi->second;
    if (lfe.count > maxcount) continue;

    for (ti=rfind_longest(lfi->first); ti != end(); ti = find_mother(*ti)) {
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
    fprintf(stderr, "): empty trie?!\n");
    return;
  }

  //---- theta: compute standard deviation of unigram MLEs (suffix len=0)
  if (verbose) { fprintf(stderr, ", theta"); fflush(stderr); }
  SuffixTrieDataT::iterator  tdi;

  //ProbT ugtotal = begin()->data[0]         //-- count suffix-candidates only!
  ProbT ugtotal = lf.n_tokens;             //-- count all lexemes
  ProbT ntags   = begin()->data.size()-1;  //-- really |tags|, but we stored total!
  ProbT pavg    = lf.n_tokens;             //-- E(P(t))
  for (tdi = begin()->data.begin(); tdi != begin()->data.end(); tdi++) {
    if (tdi->key() == 0) continue;
    pavg += (tdi->value() / ugtotal);
  }
  pavg /= ntags;

  theta = 0;
  for (tdi = begin()->data.begin(); tdi != begin()->data.end(); tdi++) {
    if (tdi->key() == 0) continue;
    theta += pow(((tdi->value()/ugtotal)-pavg), 2);
  }
  theta /= (ntags-1);
  //---- /theta


  //------ smooth: smooth increasingly more specific suffix probabilities
  if (verbose) { fprintf(stderr, ", smooth"); fflush(stderr); }
  SuffixTrieDataT::iterator        tdi_zero;
  list<iterator>                   queue;
  queue.push_front(begin());
  while (!queue.empty()) {
    ti = queue.front();
    queue.pop_front();

    //-- get total, and set pseudo-tag 0 to MLE P(suffix)
    ProbT            ticount = tdi_zero->value();
    tdi_zero                 = ti->data.find(0);
    tdi_zero->value()        = ticount / ugtotal;

    //-- get mother Id and iterator
    NodeId             momid = ti->mother;
    const_iterator      momi = find_mother(*ti);

    //-- process current and all sister nodes
    for ( ; ti->mother == momid; ti++) {
      for (tdi = ti->data.begin(); tdi != ti->data.end(); tdi++) {
	if (tdi->key() == 0) continue;

	//-- first compute P_{MLE}(t|suffix) for this node only
	tdi->value() /= ticount;

	//-- root node check
	if (momid != NoNode) {
	  //-- non-root: smooth MLE probs with mother's
	  ProbT                             momp = 0;
	  SuffixTrieDataT::const_iterator  momdi = momi->data.find(tdi->key());
	  if (momdi != momi->data.end())    momp = momdi->value();

	  tdi->value() = (tdi->value() + theta*momp) / (1+theta);
	}
      }

      //-- enqueue first daughter (if any)
      ti = first_dtr(*ti);
      if (ti != end()) queue.push_back(ti);
    }
  }
  //---- /smooth


  //------ invert: Bayesian inversion: compute P(suffix|t) from P(t|suffix),P(suffix),P(t)
  if (verbose) { fprintf(stderr, ", invert"); fflush(stderr); }
  SuffixTrieDataT ugdata = begin()->data;
  for (ti = begin(); ti != end(); ti++) {
    //-- save p(suffix) and erase its pseudo-tag 0 (zero)
    tdi_zero   = ti->data.find(0);
    ProbT psuf = tdi_zero->value();
    ti->data.erase(tdi_zero);

    for (tdi = ti->data.begin(); tdi != ti->data.end(); tdi++) {
      tdi->value() = log( (tdi->value()*psuf) / ugdata[tdi->key()] );
    }
  }
  //------ /invert

  //---- report
  if (verbose) { fprintf(stderr, "): built\n"); fflush(stderr); }
}

moot_END_NAMESPACE
