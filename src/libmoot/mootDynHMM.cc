/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2010 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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
 *   + moot PoS tagger : dynamic HMM : guts
 *--------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
# include <mootConfig.h>
#endif

#include <mootDynHMM.h>
#include <mootMIParser.h>

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
    if (writer) {
      if ((writer->tw_format & tiofTrace)) this->tag_dump_trace(*sent);
      writer->put_sentence(*sent);
    }
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
bool mootDynLexHMM::load_model(const string &modelname,
			       const mootTagString &start_tag_str,
			       const char *myname,
			       bool  do_estimate_nglambdas,
			       bool  do_estimate_wlambdas,
			       bool  do_estimate_clambdas,
			       bool  do_build_suffix_trie,
			       bool  do_compute_logprobs)
{
  //-- inherited
  if (!mootHMM::load_model(modelname,
			   start_tag_str,
			   myname,
			   do_estimate_nglambdas,
			   do_estimate_wlambdas,
			   do_estimate_clambdas,
			   do_build_suffix_trie,
			   do_compute_logprobs))
    {
      return false;
    }

  //-- get "new" tag ID
  newtag_id = tagids.name2id(newtag_str);

  //-- save original number of tags in model
  tagids_size_orig = tagids.size();

  return true;
}

//--------------------------------------------------------------
bool mootDynLexHMM::compile(mootLexfreqs &lexfreqs,
			    mootNgrams &ngrams,
			    mootClassfreqs &classfreqs,
			    const mootTagString &start_tag_str)
{
  //-- insert "new" tag
  if (ngrams.ngtable.find(newtag_str) == ngrams.ngtable.end()) {
    ngrams.add_count(newtag_str, newtag_f);
  }

  //-- inherited
  return mootHMM::compile(lexfreqs,ngrams,classfreqs,start_tag_str);
};

//--------------------------------------------------------------
void mootDynLexHMM::tw_put_info(moot::TokenWriter *tw)
{
  tw->printf_raw("  +DynHMM class      : %s\n", "mootDynLexHMM");
  tw->printf_raw("   New Tag           : %s\n", newtag_str.c_str());
  tw->printf_raw("   Ftw_eps           : %g\n", Ftw_eps);
};

//--------------------------------------------------------------
void mootDynLexHMM::tag_hook_pre(mootSentence &sent)
{
  //-- ensure everything is cleared
  dynlex_clear();

  //-- prepare lexmap
  for (mootSentence::iterator si = sent.begin(); si != sent.end(); si++) {
    if (si->toktype() != TokTypeVanilla) continue; //-- ignore non-vanilla tokens
    mootToken &tok = *si;
    for (mootToken::Analyses::iterator ani=tok.tok_analyses.begin(); ani!=tok.tok_analyses.end(); ani++) {
      mootToken::Analysis &a = *ani;
      ProbT f_tw = this->dynlex_analysis_freq(tok,a);
      a.prob = f_tw; //-- DEBUG
      if (f_tw <= 0) continue; //-- ignore
      Ftw[a.tag][tok.text()] += f_tw;
      Fw[tok.text()] += f_tw;
      Ft[a.tag] += f_tw;
    }
  }

  //-- populate HMM::lexprobs
  this->dynlex_populate_lexprobs();
}

//--------------------------------------------------------------
void mootDynLexHMM::tag_hook_post(mootSentence &sent)
{
#if 0
  //-- DEBUG: dump model
  FILE *txtout = fopen("mootdyn.txtdump","wb");
  assert(txtout != NULL);
  txtdump(txtout, true, true, false, false, true);
  fclose(txtout);
#endif

  //-- clear
  dynlex_clear();

  return;
}

//--------------------------------------------------------------
#define LOGP_NEW_MIN -100
void mootDynLexHMM::dynlex_populate_lexprobs(void)
{
  ProbT  logp_new  = tagp(newtag_id);
  ProbT  logp_zero = MOOT_PROB_ZERO;
  if (static_cast<double>(logp_new)==static_cast<double>(logp_zero)) {
    logp_new = LOGP_NEW_MIN;
  }

  //-- populate lexicon from pseudo-frequencies in dynlex
  for (TagTokProbMap::const_iterator twpi=Ftw.begin(); twpi!=Ftw.end(); twpi++) {
    const TagStr &tagstr = twpi->first;
    TagID          tagid = get_tagid(tagstr);

    //-- copy 'newtagstr' unigram tag probability for new tags
    if (tagid >= tagids_size_orig) {
      set_ngram_prob(logp_new, 0,0,tagid);
    }

    //-- get f_t ~= f(tag) = \sum_{w} f(w,tag)
    ProbT f_t = Ft[tagstr];

    //-- populate HMM::lexprobs[w][t] = logp_wt = log(wlambda1_p * (f_wt/Z_wt)), where:
    //   + f_wt = Ftw[t][w]
    //   + Z_wt = / Fw[w] = f_w = \sum_t f(w,t)    : if invert_lexp==true
    //            \ Ft[t] = f_t = \sum_w f(w,t)    : otherwise
    //   + since wlambda1_p is stored in compiled model as
    //       log_wlambda1=mootHMM::wlambda1=log(wlambda1_p),
    //     the formula is:
    //       logp_wt = log_wlambda1 + log(f_wt/f_t) = log(wlambda1_p) + log(f_wt/f_t)
    for (TokProbMap::const_iterator wpi=twpi->second.begin(); wpi!=twpi->second.end(); wpi++) {
      const TokStr &tokstr = wpi->first;
      const TokID    tokid = lex_get_tokid(tokstr);
      ProbT           f_wt = wpi->second;
      ProbT           Z_wt = invert_lexp ? Fw[tokstr] : f_t;
      ProbT           p_wt = Z_wt > 0 ? (f_wt / Z_wt) : 0;
      ProbT        logp_wt = wlambda1 + (p_wt > 0 ? log(p_wt) : MOOT_PROB_ZERO);
      lexprobs[tokid][tagid] = logp_wt;
#if 0 //-- DEBUG
      fprintf(stderr,"log p(W=%u[\"%s\"] | T=%u[\"%s\"]):=%g, logp=%g; f_wt=%g, f_w=%g, f_t=%g\n",
	      tokid, tokstr.c_str(), tagid, tagstr.c_str(), p_wt, logp_wgt, f_wt, f_w, f_t);
#endif
    }
  }
}

//--------------------------------------------------------------
void mootDynLexHMM::dynlex_clear(void)
{
  //-- clear: lex
  lex_clear();
  Ftw.clear();
  Fw.clear();
  Ft.clear();

  //-- clear: n-grams
  for (TagID t=tagids_size_orig; t<n_tags; t++) {
    ngprobsh.erase(Trigram(0,0,t));
  }

  //-- clear: tagset
  tagids.resize(tagids_size_orig);
  n_tags = tagids_size_orig;
}


/*======================================================================
 * Utilities
 */
const char *DynHMMClassNames[dheN] = {
  "Unknown",
  "Freq",
  "Boltzmann",
  "MIParser"
};

mootDynHMM *newDynHMM(DynHMMClassId which, const mootDynHMMOptions &opts)
{
  mootDynHMM *hmmp = NULL;
  switch (which) {
  case dheFreq:
    hmmp = new mootDynLexHMM();
    break;
  case dheBoltzmann:
    hmmp = new mootDynLexHMM_Boltzmann();
    break;
  case dheMIParser:
    hmmp = new mootMIParser();
    break;
  default:
    return NULL;
  }
  if (hmmp) hmmp->set_options(opts);
  return hmmp;
}

mootDynHMM *newDynHMM(const std::string &which, const mootDynHMMOptions &opts)
{
  unsigned int e;
  for (e=0; e < dheN; e++) {
    if (which==DynHMMClassNames[e]) return newDynHMM(static_cast<DynHMMClassId>(e),opts);
  }
  return newDynHMM(dheUnknown, opts);
}


moot_END_NAMESPACE
