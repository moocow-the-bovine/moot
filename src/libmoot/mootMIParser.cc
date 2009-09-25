/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2009 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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
 * File: mootMIParser.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moot PoS tagger : MI parser
 *--------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
# include <mootConfig.h>
#endif

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
 * mootMIParser
 */

//--------------------------------------------------------------
bool mootMIParser::load_model(const string &modelname,
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

  return true;
}


//--------------------------------------------------------------
bool mootMIParser::compile(mootLexfreqs &lexfreqs,
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
void mootMIParser::tag_sentence(mootSentence &sent)
{
  TokenPtrVector sentpv(sent.size());  //-- token pointers into sentence
  SentIndex sidx=0;
  for (mootSentence::iterator si=sent.begin(); si!=sent.end(); si++) {
    if (si->toktype() != TokTypeVanilla) continue; //-- ignore non-vanilla tokens
    sentpv[sidx] = &(*si);
    sidx++;
    ntokens++; //-- stats
  }
  sentpv.resize(sidx);
 
  //-- populate pmiset with pairs <i,pmi(w_{i-1},w_{i})>
  //   where:
  //      pmi(t1,v) = log( p(t1,t2) / (p(t1)*p(v)) )
  //               = log(p(t1,t2)) - log(p(t1)*p(t2))
  //               = log(p(t1,t2)) - log(p(t1)) - log(p(t2))
  //               = log(p(t2|t1)*p(t1)) -log(p(t1)) -log(p(t2))
  //               = log(p(t2|t1)) +log(p(t1)) -log(p(t1)) -log(p(t2))
  //               = log(p(t2|t1)) -log(p(t2))
  ProbIndexPairVector pmiv(sentpv.size()); // +1
  TagID             tagid1 = start_tagid;
  for (sidx=0; sidx < sentpv.size(); sidx++) {
    mootToken *tokp = sentpv[sidx];
    const TagStr &tagstr2 = text_tags ? tokp->text() : tokp->besttag();
    TagID   tagid2  = tagids.name2id(tagstr2);
    if (tagid2 == 0) {
      tagid2 = newtag_id;
      nunknown++;
    }

    ProbT pmi = tagp(tagid1,tagid2) - tagp(tagid2);
    if (pmi==0 && tagid2 != newtag_id) {
      pmi = tagp(tagid1,newtag_id) - tagp(newtag_id);
      nunknown++;
    }
    pmiv[sidx] = std::make_pair(pmi,sidx);

    tagid1 = tagid2;
  }
  //-- insert final boundary (useless, since we can't index it as a token)
  //tagid2 = final_tagid;
  //pmiv[sidx] = std::make_pair(tagp(tagid1,tagid2)-tagp(tagid2), sidx);

  //-- sort the temp vector in ascending order by stored PMI
  std::sort(pmiv.begin(), pmiv.end());

  //-- mark cut boundaries by adding analyses
  SentIndex bidx;
  char detailbuf[32];
  for (bidx=0; bidx < pmiv.size(); bidx++) {
    const ProbIndexPair &pip = pmiv[bidx];
    ProbT pmi = pip.first;
    sidx      = pip.second;
    mootToken *tokp = sentpv[sidx];

    //-- build & insert new analysis
    mootTagString details = output_prefix;
    sprintf(detailbuf, "%lu <%g>", static_cast<long unsigned int>(bidx), pmi);
    details.append(detailbuf);
    tokp->insert(output_tag, details, pmi);
  }

  //-- update some stats
  nsents++;
};

moot_END_NAMESPACE
