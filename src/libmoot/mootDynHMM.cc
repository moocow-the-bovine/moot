/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2009 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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
 * File: mootDynHMM.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moot PoS tagger : 1st-order HMM tagger/disambiguator : guts
 *--------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
# include <mootConfig.h>
#endif

#include <mootDynHMM.h>
#include <mootTokenIO.h>
#include <mootCIO.h>
#include <mootZIO.h>
#include <mootBinIO.h>
#include <mootUtils.h>

using namespace std;
using namespace mootBinIO;
using namespace mootio;

moot_BEGIN_NAMESPACE

/*======================================================================
 * mootDynHMM
 */

//--------------------------------------------------------------
void mootDynHMM::tag_io(TokenReader *reader, TokenWriter *writer)
{
  int rtok;
  mootSentence *sent;
  while (reader && (rtok = reader->get_sentence()) != TokTypeEOF) {
    sent = reader->sentence();
    if (!sent) continue;
    this->tag_sentence(*sent);
    if (writer) writer->put_sentence(*sent);
  }
}

//--------------------------------------------------------------
void mootDynHMM::tag_sentence(mootSentence &sentence)
{
  this->tag_hook_pre(sentence);
  mootHMM::tag_sentence(sentence);
  this->tag_hook_post(sentence);
}


/*======================================================================
 * mootDynLexHMM
 */

//--------------------------------------------------------------
void mootDynLexHMM::tag_hook_pre(mootSentence &sent)
{
  //-- ensure everything is cleared
  lex_clear();
  dynlex.clear();
  tagids_size_orig = n_tags;

  //-- prepare lexmap
  for (mootSentence::const_iterator si = sent.begin(); si != sent.end(); si++) {
    if (si->toktype() != TokTypeVanilla) continue; //-- ignore non-vanilla tokens
    const mootToken &tok = *si;
    for (mootToken::Analyses::const_iterator ani=tok.analyses().begin(); ani!=tok.analyses().end(); ani++) {
      this->dynlex_add_analysis(tok, *ani);
    }
  }

  //-- populate HMM::lexprobs
  this->dynlex_populate_lexprobs();

#if 1
  //-- DEBUG dump
  FILE *txtout = fopen("mootdyn.txtdump","wb");
  assert(txtout != NULL);
  txtdump(txtout, true, true, false, false, true);
  fclose(txtout);
#endif
}

//--------------------------------------------------------------
void mootDynLexHMM::tag_hook_post(mootSentence &sent)
{
  //-- clear: lex
  lex_clear();
  dynlex.clear();

  //-- clear: n-grams
  for (TagID t=tagids_size_orig; t<n_tags; t++) {
    ngprobsh.erase(Trigram(0,0,t));
  }

  //-- clear: tagset
  tagids.resize(tagids_size_orig);
  n_tags = tagids_size_orig;
}

//--------------------------------------------------------------
void mootDynLexHMM::dynlex_populate_lexprobs(void)
{
  TagID tagid_new  = tagids.name2id(tagstr_new);
  ProbT  logp_new  = tagp(tagid_new);
  ProbT  logp_zero = MOOT_PROB_ZERO;
  if (static_cast<double>(logp_new)==static_cast<double>(logp_zero)) {
    logp_new = log(1e-12); //-- HACK
  }

  //-- populate lexicon from pseudo-frequencies in dynlex
  for (TagTokProbMap::const_iterator twpi=dynlex.begin(); twpi!=dynlex.end(); twpi++) {
    const TagStr &tagstr = twpi->first;
    TagID          tagid = get_tagid(tagstr);

    //-- copy 'newtagstr' unigram tag probability for new tags
    if (tagid >= tagids_size_orig) {
      set_ngram_prob(logp_new, 0,0,tagid);
    }

    //-- get f_t ~= f(tag) = \sum_{w} f(w,tag)
    ProbT f_t = 0;
    for (TokProbMap::const_iterator wpi=twpi->second.begin(); wpi!=twpi->second.end(); wpi++) {
      f_t += wpi->second + wtflambda0;
    }

    //-- populate HMM::lexprobs as log(wlambda1_p*(f_wt/f_t))
    //   + since wlambda1_p is stored in compiled model as logwlambda1=mootHMM::wlambda1=log(wlambda1_p),
    //     the formula is: log_wlambda1 + log(f_wt/f_t) = log(wlambda1_p) + log(f_wt/f_t)
    for (TokProbMap::const_iterator wpi=twpi->second.begin(); wpi!=twpi->second.end(); wpi++) {
      const TokStr &tokstr = wpi->first;
      const TokID    tokid = lex_get_tokid(tokstr);
      ProbT           f_wt = wpi->second + wtflambda0;
      ProbT          p_wgt = f_wt > 0 ? (wlambda1 + log(f_wt / f_t)) : MOOT_PROB_ZERO;
      lexprobs[tokid][tagid] = p_wgt;
    }
  }
}

/*======================================================================
 * test
 */
/*
void __testme__(void) {
  mootDynHMM<LPFUniform> hmmu;
  }*/

moot_END_NAMESPACE
