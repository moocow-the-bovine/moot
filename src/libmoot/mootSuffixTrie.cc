/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2007 by Bryan Jurish <moocow@cpan.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/*--------------------------------------------------------------------------
 * File: mootSuffixTrie.h
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moocow's PoS tagger : suffix trie
 *--------------------------------------------------------------------------*/

#include <mootConfig.h>
#include <mootSuffixTrie.h>
#include <stdio.h>
#include <math.h>

moot_BEGIN_NAMESPACE

using namespace std;

/*--------------------------------------------------------------------------
 * Smoothing Methods
 */
//-- define to smooth trie probabilities a la Brants(2000)
#define SMOOTH_ALA_BRANTS

//-- define to smooth trie probabilities using theoretically correct standard deviation
//#define SMOOTH_ALA_STDDEV

//-- define to smooth trie probabilities a la acopost (NYI)
//#define SMOOTH_ALA_ACOPOST

//-- define to smooth trie probabilities a la Brants & Samuelsson(1995)
//   : this seems to work best
//#define SMOOTH_ALA_SAMUELSSON

//-- smoothing logic
#if defined(SMOOTH_ALA_BRANTS)
# undef SMOOTH_ALA_STDDEV
# undef SMOOTH_ALA_ACOPOST
# undef SMOOTH_ALA_SAMUELSSON
#elif defined(SMOOTH_ALA_STDDDEV)
# undef SMOOTH_ALA_BRANTS
# undef SMOOTH_ALA_ACOPOST
# undef SMOOTH_ALA_SAMUELSSON
#elif defined(SMOOTH_ALA_ACOPOST)
# undef SMOOTH_ALA_BRANTS
# undef SMOOTH_ALA_STDDEV
# undef SMOOTH_ALA_SAMUELSSON
#elif defined(SMOOTH_ALA_SAMUELSSON)
# undef SMOOTH_ALA_BRANTS
# undef SMOOTH_ALA_ACOPOST
# undef SMOOTH_ALA_STDDEV
#else
# define SMOOTH_ALA_SAMUELSSON
#endif

/*--------------------------------------------------------------------------
 * Smoothing Initialization methods
 */
//-- Define this to use global P(t) for empty suffix (a la Brants 2000)
#define SMOOTH_EMPTY_AS_GLOBAL

//-- Define this to use local P_{low}(t) for empty suffix  (a la acopost)
//#define SMOOTH_EMPTY_AS_EMPTY

//-- Define this to use lexical entriy @UNKNOWN for empty suffix (also a la acopost, variant)
//   (NYI)
//#define SMOOTH_EMTPY_AS_UNKNOWN

#if defined(SMOOTH_EMPTY_AS_GLOBAL)
# undef SMOOTH_EMPTY_AS_EMPTY
# undef SMOOTH_EMTPY_AS_UNKNOWN
#elif defined(SMOOTH_EMPTY_AS_EMPTY)
# undef SMOOTH_EMPTY_AS_GLOBAL
# undef SMOOTH_EMTPY_AS_UNKNOWN
/*
#elif defined(SMOOTH_EMPTY_AS_UNKNOWN)
# undef SMOOTH_EMPTY_AS_GLOBAL
# undef SMOOTH_EMTPY_AS_EMPTY
*/
#else
# define SMOOTH_EMPTY_AS_GLOBAL
#endif


/*--------------------------------------------------------------------------
 * Inversion Methods (equivalent)
 */
//-- define this to use "Pure" Bayesian inversion: P(suf|t) = P(suf)*P(t|suf)/P(t)
#define INVERT_PURE

//-- define this to use word-constant inversion: P(suf|t) = P(t|suf)/P(t)
//#define INVERT_NOSCALE

//-- define this to use acopost-style inversion: P(suf|t) = P(t|suf)/f(t)
//#define INVERT_ACOPOST

#if defined(INVERT_PURE)
# undef INVERT_NOSCALE
# undef INVERT_ACOPOST
#elif defined(INVERT_NOSCALE)
# undef INVERT_PURE
# undef INVERT_ACOPOST
#elif defined(INVERT_ACOPOST)
# undef INVERT_PURE
# undef INVERT_NOSCALE
#else
# define INVERT_PURE
#endif

/*--------------------------------------------------------------------------
 * Suffix Item Selection
 */
//-- define this to ignore non-alphabetic tokens in trie construction
//#define IGNORE_NON_ALPHAS

/*--------------------------------------------------------------------------
 * Debug
 */
//-- define this for verbose debugging
#define SUFFIX_TRIE_DEBUG

#if defined(MOOT_DEBUG_ENABLED) && defined(SUFFIX_TRIE_DEBUG)
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
  //-- sanity check
  if (!maxlen()) return true;

  //-- Report
  if (verbose) { fprintf(stderr, "("); fflush(stderr); }

  //-- insert: enqueue maximum-length suffixes
  if (verbose) { fprintf(stderr, "insert"); fflush(stderr); }
  if (!_build_insert(lf)) {
    fprintf(stderr, "\nSuffixTrie::build(): insertion FAILED.\n");
    return false;
  }
  DEBUG(fprintf(stderr, "\n->DEBUG: post-insert: pending size=%u\n", trie_pending.size()));


  //-- compile: populate adjaceny table
  if (verbose) { fprintf(stderr, ", compile"); fflush(stderr); }
  compile();

  //-- associate: assign count data to terminal & branching suffixes
  if (verbose) { fprintf(stderr, ", associate"); fflush(stderr); }
  if (!_build_assoc(lf,tagids)) {
    fprintf(stderr, "\nSuffixTrie::build(): frequency association failed.\n");
    return false;
  }

  //-- sanity check
  if (empty() || begin()->data.size() <= 2) {
    fprintf(stderr, "\nSuffixTrie::build(): empty trie -- aborting build.\n");
    return false;
  }
  DEBUG(fprintf(stderr, "\n->DEBUG: begin()->data.size()=%u\n", begin()->data.size()));


  //-- smooth: theta: compute smoothing constants
  if (verbose) { fprintf(stderr, ", smooth"); fflush(stderr); }
  if (theta==0) {
    if (!_build_compute_theta(lf,ng,tagids,eos_tagid)) {
      fprintf(stderr, "\nSuffixTrie::build(): could not compute theta.\n");
      return false;
    }
  }

  //-- smooth: mle: compute MLE probabilities P(tag|suffix)
  if (!_build_compute_mles(lf,ng,tagids,eos_tagid)) {
    fprintf(stderr, "\nSuffixTrie::build(): could not compute MLEs.\n");
    return false;
  }

  //-- smooth: invert: compute MLE probabilities P(tag|suffix)
  if (!_build_invert_mles(ng,tagids,eos_tagid)) {
    fprintf(stderr, "\nSuffixTrie::build(): could not invert MLEs.\n");
    return false;
  }

  //-- report
  if (verbose) { fprintf(stderr, ")"); fflush(stderr); }

  return true;
};


/*--------------------------------------------------------------
 * _build_insert()
 */
bool SuffixTrie::_build_insert(const mootLexfreqs &lf)
{
  mootLexfreqs::LexfreqTokTable::const_iterator lfi;
  const mootTaster *taster = lf.taster;

  for (lfi = lf.lftable.begin(); lfi != lf.lftable.end(); lfi++) {
    const mootLexfreqs::LexfreqEntry &lfe = lfi->second;
      
    if (lfe.count > maxcount
#ifdef IGNORE_NON_ALPHAS
	|| (taster && taster->flavor(lfi->first) != taster.nolabel)
#endif
	|| (taster && taster->has_label(lfi->first))
	)
      continue;

    trie_rinsert(lfi->first);
  }
  return true;
}

/*--------------------------------------------------------------
 * _build_assoc()
 */
bool SuffixTrie::_build_assoc(const mootLexfreqs &lf, const TagIDTable &tagids)
{
  TagID tagid;
  mootLexfreqs::LexfreqTokTable::const_iterator lfi;
  mootLexfreqs::LexfreqSubtable::const_iterator lfsi;
  iterator ti, momi;
  const mootTaster *taster = lf.taster;

  for (lfi = lf.lftable.begin(); lfi != lf.lftable.end(); lfi++) {
    const mootLexfreqs::LexfreqEntry &lfe = lfi->second;

    if (lfe.count > maxcount
#ifdef IGNORE_NON_ALPHAS
	|| (taster && taster->flavor(lfi->first) != taster.nolabel)
#endif
	|| (taster && taster->has_label(lfi->first))
	)
      continue;

    ti  = rfind_longest(lfi->first);
    for (momi = ti; momi != end(); momi = find_mother(*momi)) {
      if (momi != ti && momi->ndtrs == 1) continue; //-- ignore non-branching nodes

      //-- HACK: reserve full complement of tagid slots for each mom
      //if (momi->data.empty()) momi->data.reserve(tagids.size());
   
      //-- HACK: store total counts in (node.data[0])
      if (momi->data.find(0) != momi->data.end()) {
	momi->data[0] += lfe.count;
      } else {
	momi->data[0]  = lfe.count;
      }

      for (lfsi = lfe.freqs.begin(); lfsi != lfe.freqs.end(); lfsi++) {
	const string &tagstr = lfsi->first;
	tagid = tagids.name2id(tagstr);
	if (tagid != 0) {
	  if (momi->data.find(tagid) != momi->data.end()) {
	    momi->data[tagid] += lfsi->second;
	  } else {
	    momi->data[tagid]  = lfsi->second;
	  }
	}
      }
    }
  }
  return true;
}

/*--------------------------------------------------------------
 * _build_compute_theta()
 */
bool SuffixTrie::_build_compute_theta(const mootLexfreqs &lf,
					  const mootNgrams   &ng,
					  const TagIDTable   &tagids,
					  TagID eos_tagid)
{
  ProbT ugtotal  = ng.ugtotal - ng.lookup(tagids.id2name(eos_tagid));
  theta          = 0;

#if defined(SMOOTH_ALA_BRANTS)

  //---------------------------------------------------------------
  // theta[Brants]: compute weighted average unigram MLEs
  ProbT ntags    = tagids.size()-2;               //-- use HMM tag count (except UNKNOWN,EOS)
  ProbT pavg     = 1/static_cast<ProbT>(ntags);   //-- use uniform distribution over tag-probs
  //for (SuffixTrieDataT::const_iterator tdi = begin()->data.begin(); tdi != begin()->data.end(); tdi++)
  for (TagID tagid = 1; tagid < tagids.size(); tagid++)
    {
      if (tagid == eos_tagid) continue;
      ProbT tagp = static_cast<ProbT>(ng.lookup(tagids.id2name(tagid))) / ugtotal;
      theta += pow(tagp-pavg, 2);
    }
  theta = sqrt(theta/(ntags-1));           //-- Var_{uniform(|Tags|)} (P_t(·)) [Brants,2000]
  //---- /theta[Brants]
  //---------------------------------------------------------------

  DEBUG(fprintf(stderr, \
		"\n-> DEBUG[Brants]: size=%u; lextotal=%g ; ugtotal=%g ; ntags=%g; pavg=%g; theta=%g\n",\
		size(), lf.n_tokens, ugtotal, ntags, pavg, theta));

#elif defined(SMOOTH_ALA_STDDEV)

  //---------------------------------------------------------------
  // theta[stddev]: compute weighted average unigram MLEs

  //-- get E(P(t))
  ProbT pavg     = 0;          //-- E(P(t))
  //for (SuffixTrieDataT::const_iterator tdi = begin()->data.begin(); tdi != begin()->data.end(); tdi++)
  for (TagID tagid = 1; tagid < tagids.size(); tagid++)
    {
      if (tagid == eos_tagid) continue;
      ProbT tagp = ((ProbT)ng.lookup(tagids.id2name(tagid))) / ugtotal;
      //tagid  = tdi->key();
      //tagp   = tdi->value() / suftotal;
      //ntags += 1.0;
      //--
      pavg  += (tagp*tagp);    //-- E_{P_t} (P_t(·)) [weighted avg: stddev]
    }

  //for (SuffixTrieDataT::const_iterator tdi = begin()->data.begin(); tdi != begin()->data.end(); tdi++)
  for (tagid = 1; tagid < tagids.size(); tagid++)
    {
      if (tagid == eos_tagid) continue;
      ProbT tagp = ((ProbT)ng.lookup(tagids.id2name(tagid))) / ugtotal;
      theta += tagp * pow(tagp-pavg, 2);
    }
  theta = sqrt(theta);                     //-- Var_{P_t} (P_t(·)) [weighted avg: stddev]
  //---- /theta[stdddev]
  //---------------------------------------------------------------

  DEBUG(fprintf(stderr, \
		"\n-> DEBUG[stddev]: size=%u; lextotal=%g ; ugtotal=%g ; ntags=%g; pavg=%g; theta=%g\n",\
		size(), lf.n_tokens, ugtotal, ntags, pavg, theta));

#elif defined(SMOOTH_ALA_ACOPOST)

  //---------------------------------------------------------------
  // theta[acopost]: compute weighted average low-count unigram MLEs

  ProbT ntags    = tagids.size()-2;        //-- use HMM tag count (except UNKNOWN,EOS)
  ProbT pavg     = 1/ntags;                //-- E(P(t)) [uniform: acopost]
  ProbT suftotal = begin()->data[0];
  for (SuffixTrieDataT::const_iterator tdi = begin()->data.begin(); tdi != begin()->data.end(); tdi++) {
    tagid  = tdi->key();
    if (tagid == 0 || tagid == eos_tagid) continue;

    ProbT tagp = tdi->value() / suftotal - pavg;
    theta += tagp * tagp;
  }
  theta = sqrt(theta/(ntags-1));           //-- ????
  //---- /theta[acopost]
  //---------------------------------------------------------------

  DEBUG(fprintf(stderr, \
		"\n-> DEBUG[acopost]: size=%u; lextotal=%g ; ugtotal=%g ; ntags=%g; theta=%g\n",\
		size(), lf.n_tokens, ugtotal, ntags, theta));

#elif defined(SMOOTH_ALA_SAMUELSSON)

  //---- theta[Samuelsson] : ignore
  theta = sqrt(((ProbT)begin()->data[0])/((ProbT)size()));
  DEBUG(fprintf(stderr, \
		"\n-> DEBUG[Samuelsson]: size=%u; lextotal=%g ; ugtotal=%g ; suftotal=%g; avg(theta)=%g\n",\
		size(), lf.n_tokens, ugtotal, begin()->data[0], theta));

#endif // SMOOTH_ALA_BRANTS / SMOOTH_ALA_STDDEV / SMOOTH_ALA_ACOPOST / SMOOTH_ALA_SAMUELSSON

  return true;
}

/*--------------------------------------------------------------
 * _build_compute_mles()
 */
bool SuffixTrie::_build_compute_mles(const mootLexfreqs &lf,
					 const mootNgrams   &ng,
					 const TagIDTable   &tagids,
					 TagID eos_tagid)
{
  iterator                    ti;
  SuffixTrieDataT::iterator   tdi;
  SuffixTrieDataT::iterator   tdi_zero;
  ProbT                       lextotal = lf.n_tokens;
#ifndef SMOOTH_EMPTY_AS_EMPTY
  ProbT                      ugtotal  = ng.ugtotal - ng.lookup(tagids.id2name(eos_tagid));
#endif
  list<iterator>             queue;
  queue.push_front(begin());

  //------ smooth:abstract: smooth increasingly more specific suffix probabilities
  while (!queue.empty()) {
    ti = queue.front();
    queue.pop_front();

    //-- get mother Id and iterator
    NodeId           momid = ti->mother;
    const_iterator    momi = find_mother(*ti);
    size_t          nesteps= 0;
    const_iterator  nemomi = const_find_ancestor_nonempty(momi, &nesteps);
    SuffixTrieDataT::const_iterator  nemomdi;

    //-- process current and all sister nodes
    for ( ; ti->mother == momid; ti++) {

      //-- get total, and set pseudo-tag 0 to MLE P(suffix)
      if (!ti->data.empty()) {
	tdi_zero                  = ti->data.find(0);
	ProbT             ticount = tdi_zero->value();
	tdi_zero->value()         = ticount / lextotal;

#ifdef SMOOTH_ALA_SAMUELSSON
	ProbT             sqrtN    = sqrt(ticount);  //-- Samuelsson
#endif

	if (momid == NoNode) {
	  //-- root node: iterate over data directly
	  for (tdi = ti->data.begin(); tdi != ti->data.end(); tdi++) {
	    if (tdi->key() == 0) continue;
#ifdef SMOOTH_EMPTY_AS_EMPTY
	    tdi->value() /= ticount;
#else
	    tdi->value() =  static_cast<ProbT>(ng.lookup(tagids.id2name(tdi->key()))) / ugtotal;
#endif
	  }
	}
	else {
	  //-- non-root: iterate over mom data, smoothing MLE probs with mom's
	  ti->data.reserve(nemomi->data.size());

	  for (nemomdi = nemomi->data.begin(); nemomdi != nemomi->data.end(); nemomdi++) {
	    TagID tagid = nemomdi->key();
	    ProbT  momp = nemomdi->value();
	    if (tagid == 0) continue;

	    //-- repeatedly smooth momp nesteps times
	    for (size_t ns = 0; ns < -nesteps; ns++) {
#ifdef SMOOTH_ALA_SAMUELSSON
	      ProbT sqrtMomN = sqrt(nemomi->data[0]*lextotal);
	      momp           = (sqrtMomN*momp + momp) / (1.0+sqrtMomN);
#else
	      momp = (momp + theta*momp) / (1.0+theta);
#endif
	    }
	    
	    //-- get (possibly new) association for this key
	    tdi = ti->data.find(tagid);
	    if (tdi == ti->data.end()) tdi = ti->data.insert(tagid,0);
	    
	    //-- compute P_{MLE}(t|suffix) for this pair (t,suffix) only
	    tdi->value() /= ticount;
	    
#ifdef SMOOTH_ALA_SAMUELSSON
	    tdi->value() = (sqrtN*tdi->value() + momp) / (1.0+sqrtN);
#else
	    tdi->value() = (tdi->value() + theta*momp) / (1.0+theta);
#endif
	  }
	}
      }

      //-- enqueue first daughter (if any)
      queue.push_back(first_dtr(*ti));
      if (queue.back() == end()) queue.pop_back();
    }
  }

  return true;
}


/*--------------------------------------------------------------
 * _build_invert_mles
 */
bool SuffixTrie::_build_invert_mles(const mootNgrams &ng,
					const TagIDTable &tagids,
					TagID eos_tagid)
{
  iterator ti;
  SuffixTrieDataT::iterator tdi;
  SuffixTrieDataT::iterator tdi_zero;
#if defined(INVERT_PURE) || defined(INVERT_NOSCALE)
  ProbT ugtotal = ng.ugtotal - ng.lookup(tagids.id2name(eos_tagid));
#endif

  for (ti = begin(); ti != end(); ti++) {
    if (ti->data.empty()) continue;

    //-- save p(suffix) and erase its pseudo-tag 0 (zero)
    tdi_zero   = ti->data.find(0);
#if defined(INVERT_PURE)
    ProbT psuf = tdi_zero->value();
#endif
    ti->data.erase(tdi_zero);

    if (ti == begin()) continue;

    for (tdi = ti->data.begin(); tdi != ti->data.end(); tdi++) {
      TagID tagid = tdi->key();
#if defined(INVERT_ACOPOST)
      ProbT tagp  = ((ProbT)ng.lookup(tagids.id2name(tagid)));
      ProbT tagp  = static_cast<ProbT>(ng.lookup(tagids.id2name(tagid)));
#else
      ProbT tagp  = static_cast<ProbT>(ng.lookup(tagids.id2name(tagid))) / ugtotal;
#endif

      if (tagp != 0) {
#if defined(INVERT_PURE)
	tdi->value() = (tdi->value() * psuf) / tagp;
#elif defined(INVERT_NOSCALE) || defined(INVERT_ACOPOST)
	tdi->value() = (tdi->value() / tagp);
#endif
       } else {
	//tdi->value() = MOOT_PROB_ZERO;
	tdi->value() = 0;
      }
    }
  }
  //------ /invert
  return true;
}

/*--------------------------------------------------------------
 * txtdump()
 */
void SuffixTrie::txtdump(FILE *out, const TagIDTable &tagids) const
{
  for (const_iterator i = begin(); i != end(); i++) {
    string s = node_string(*i);
    fwrite(s.data(), s.size(), 1, out);
    fputc('\n', out);
    for (SuffixTrieDataT::const_iterator di = i->data.begin(); di != i->data.end(); di++) {
      TagID tagid = di->key();
      ProbT tagp  = di->value();
      fwrite(s.data(), s.size(), 1, out);
      fprintf(out, "\t%s\t%g\n", tagids.id2name(tagid).c_str(), tagp);
    }
  }
};

#undef DEBUG

moot_END_NAMESPACE
