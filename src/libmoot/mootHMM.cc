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
 * File: mootHMM.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moot PoS tagger : 1st-order HMM tagger/disambiguator : guts
 *--------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
# include <mootConfig.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

#include "mootHMM.h"
#include "mootTokenIO.h"

#include <zlib.h>
#include "mootBinIO.h"
#include "mootBinStream.h"

using namespace std;
using namespace mootBinIO;

/*--------------------------------------------------------------------------
 * clear, freeing dynamic data
 *--------------------------------------------------------------------------*/

void mootHMM::clear(bool wipe_everything)
{
  //-- iterator variables
  ViterbiColumn *col, *col_next;
  ViterbiNode   *nod, *nod_next;

  //-- free state-table: columns and nodes
  for (col = vtable; col != NULL; col = col_next) {
    col_next      = col->col_prev;
    for (nod = col->nodes; nod != NULL; nod = nod_next) {
      nod_next    = nod->row_next;
      delete nod;
    }
    delete col;
  }

  //-- free trashed state cols
  for (col = trash_columns; col != NULL; col = col_next) {
    col_next = col->col_prev;
    delete col;
  }

  //-- free trashed state nodes
  for (nod = trash_nodes; nod != NULL; nod = nod_next) {
    nod_next = nod->row_next;
    delete nod;
  }

  //-- free best-path nodes
  ViterbiPathNode *pnod, *pnod_next;
  for (pnod = vbestpath; pnod != NULL; pnod = pnod_next) {
    pnod_next       = pnod->path_next;
    delete pnod;
  }
  vbestpath = NULL;

  //-- free trashed path nodes
  for (pnod = trash_pathnodes; pnod != NULL; pnod = pnod_next) {
    pnod_next       = pnod->path_next;
    delete pnod;
  }
  trash_pathnodes = NULL;

  //-- free lexical probabilities
  lexprobs.clear();

  //-- free lexical-class probabilities
  lcprobs.clear();

  //-- clear trigram table
#ifdef moot_USE_TRIGRAMS
  ngprobs3.clear();
#endif

  //-- reset to default "empty" values
  ngprobs1 = NULL;
  ngprobs2 = NULL;
  vtable = NULL;
  trash_nodes = NULL;
  trash_columns = NULL;
  vbestpn = NULL;
  nsents = 0;
  ntokens = 0;
  nunclassed = 0;
  nnewclasses = 0;
  nunknown = 0;

  if (wipe_everything) {
    //-- free id-tables
    tokids.clear();
    tagids.clear();

    for (int i = 0; i < NTokTypes; i++) {
      typids[i] = 0;
    }

    start_tagid = 0;
    unknown_lex_threshhold = 1;
    n_tags = 0;
    n_toks = 0;
    n_classes = 0;
  }
}

/*--------------------------------------------------------------------------
 * Compilation
 *--------------------------------------------------------------------------*/
bool mootHMM::compile(const mootLexfreqs &lexfreqs,
		      const mootNgrams &ngrams,
		      const mootClassfreqs &classfreqs,
		      const mootTagString &start_tag_str)
{
  //-- sanity check
  if (ngrams.ugtotal == 0) {
    carp("mootHMM::compile(): Error: bad unigram total in 'ngrams'!\n");
    return false;
  }

  //-- assign IDs
  assign_ids_lf(lexfreqs);
  assign_ids_ng(ngrams);
  assign_ids_cf(classfreqs);

  //-- get or assign start-tag-ID
  start_tagid = tagids.nameExists(start_tag_str)
    ? tagids.name2id(start_tag_str)
    : tagids.insert(start_tag_str);

  //-- save n_tags, n_toks, n_classes
  n_tags = tagids.size();
  n_toks = tokids.size();
  n_classes = classids.size();

  //-- estimate lambdas
  //estimate_lambdas(ngrams);

  //-- allocate lookup tables : lex
  clear(false);
  lexprobs.resize(tokids.size());

  //-- allocate lookup tables : classes
  lcprobs.resize(classids.size());

  //-- allocate: unigrams
  ngprobs1 = (ProbT *)malloc(n_tags*sizeof(ProbT));
  if (!ngprobs1) {
    carp("mootHMM::compile(): Error: could not allocate unigram table.\n");
    return false;
  }

  //-- allocate: bigrams
  ngprobs2 = (ProbT *)malloc((n_tags*n_tags)*sizeof(ProbT));
  if (!ngprobs2) {
    carp("mootHMM::compile(): Error: could not allocate bigram table.\n");
    return false;
  }

  //-- zero lookup-table contents
  memset(ngprobs1, 0, n_tags*sizeof(ProbT));
  memset(ngprobs2, 0, (n_tags*n_tags)*sizeof(ProbT));

  //-- compilation variables
  TokID                       tokid;          //-- current token-ID
  TokenType                   toktyp;         //-- current token-type
  LexClass                    lclass;         //-- current lexical class
  TagID                       tagid;          //-- current tag-ID
  TagID                       tagid2;         //-- next tag-ID (for bigrams)
#ifdef moot_USE_TRIGRAMS
  TagID                       tagid3;         //-- next-next tag-ID (for trigrams)
#endif
  mootLexfreqs::LexfreqCount unTotal = 0 ;    //-- total "unknown" token count
  LexProbSubTable            &untagcts        //-- "unknown" tag counts (later, probabilites)
      = lexprobs[0];
  //bool                     is_valid_token;    //-- watch out for special tokens (@CARD & friends)
  //int                      i;

  //-- compile lexical probabilities : let lexfreqs figure out specials (elsewhere!)
  //lexfreqs.compute_specials();

  //-- compile lexical probabilities : for all tokens (lfti)
  for (mootLexfreqs::LexfreqTokTable::const_iterator lfti = lexfreqs.lftable.begin();
       lfti != lexfreqs.lftable.end();
       lfti++)
    {
      const mootTokString &tokstr = lfti->first;
      const mootLexfreqs::LexfreqEntry &entry = lfti->second;
      const mootLexfreqs::LexfreqCount toktotal = entry.count;

      //-- sanity check
      if (toktotal == 0) continue;

      //-- get token type, id
      toktyp = token2type(tokstr);
      tokid  = toktyp != TokTypeUnknown ? tokids.name2id(tokstr) : typids[toktyp];

      //-- ... for all tags occurring with this token(lftagi)
      for (mootLexfreqs::LexfreqSubtable::const_iterator lftagi = entry.freqs.begin();
	   lftagi != entry.freqs.end();
	   lftagi++)
	{
	  const mootTagString &tagstr = lftagi->first;
	  const mootLexfreqs::LexfreqCount tagcount = lftagi->second;
	  const mootLexfreqs::LexfreqCount tagtotal = lexfreqs.taglookup(tagstr);
	  
	  //-- sanity check
	  if (tagtotal == 0) continue;
  
	  //-- get tag-ID
	  tagid  = tagids.name2id(tagstr);
	  
	  //-- "unknown" token check
	  if (toktyp == TokTypeAlpha && toktotal <= unknown_lex_threshhold) {
	    //-- "unknown" token: just store the raw counts for now
	    
	    //-- ... and add to "unknown" counts
	    unTotal += tagcount;
	    LexProbSubTable::iterator lpsi = untagcts.find(tagid);
	    if (lpsi == untagcts.end()) {
	      untagcts[tagid] = tagcount;
	    } else {
	      lpsi->second += tagcount;
	    }
	  }
	  if (tokid != 0) {
	    //-- it's a kosher token (too?): compute lexical probability: p(tok|tag)
	    lexprobs[tokid][tagid] = tagcount / tagtotal;
	  }
	}
    }

  //-- Normalize "unknown" lexical probabilities
  if (lexprobs.size() == 0) lexprobs.resize(1);
  for (LexProbSubTable::iterator lpsi = untagcts.begin();
       lpsi != untagcts.end();
       lpsi++)
    {
      if (lpsi->second == 0) continue;
      const mootTagString &tagstr = tagids.id2name(lpsi->first);
      const mootLexfreqs::LexfreqCount tagtotal = lexfreqs.taglookup(tagstr);
      lpsi->second /= tagtotal;
    }

  //-- compile class probabilities : for all classes (lcti)
  for (mootClassfreqs::ClassfreqTable::const_iterator lcti = classfreqs.lctable.begin();
       lcti != classfreqs.lctable.end();
       lcti++)
    {
      const mootTagSet &tagset = lcti->first;
      const mootClassfreqs::ClassfreqEntry &entry = lcti->second;
      const CountT classtotal = entry.count;

      //-- get class id
      lclass.clear();
      tagset2lexclass(tagset,&lclass,false);
      ClassID classid = classids.name2id(lclass);

      //-- check for empty/unknown class
      if (classid == 0 || lclass.empty()) classid = uclassid;

      //-- sanity check
      if (classtotal == 0 || classid == 0) continue;

      //-- ... for all tags assigned to this class (lctagi)
      for (mootClassfreqs::ClassfreqSubtable::const_iterator lctagi = entry.freqs.begin();
	   lctagi != entry.freqs.end();
	   lctagi++)
	{
	  const mootTagString &ctagstr   = lctagi->first;
	  const CountT         ctagcount = lctagi->second;
	  const CountT         ctagtotal = classfreqs.taglookup(ctagstr);
	  
	  //-- sanity check
	  if (ctagtotal == 0) continue;
  
	  //-- get tag-ID
	  tagid  = tagids.name2id(ctagstr);

	  //-- compute class probability: p(class|tag)
	  lcprobs[classid][tagid] = ctagcount / ctagtotal;
	}
    }

  //-- Compute ngram probabilites
  for (mootNgrams::NgramTable::const_iterator ngi1 = ngrams.ngtable.begin();
       ngi1 != ngrams.ngtable.end();
       ngi1++)
    {
      //-- look at unigrams first : get ID
      tagid = tagids.name2id(ngi1->first);

      //-- compute unigram probability
      ngprobs1[tagid] = ngi1->second.count / ngrams.ugtotal;

      //-- ignore zero probabilities
      if (ngi1->second.count == 0) continue;

      //-- next, look at bigrams
      for (mootNgrams::BigramTable::const_iterator ngi2 = ngi1->second.freqs.begin();
	   ngi2 != ngi1->second.freqs.end();
	   ngi2++)
	{
	  //-- get ID
	  tagid2 = tagids.name2id(ngi2->first);

	  //-- compute bigram probability
	  ngprobs2[(n_tags*tagid)+tagid2] = ngi2->second.count / ngi1->second.count;

#ifdef moot_USE_TRIGRAMS
	  //-- look at trigrams now
	  for (mootNgrams::TrigramTable::const_iterator ngi3 = ngi2->second.freqs.begin();
	       ngi3 != ngi2->second.freqs.end();
	       ngi3++)
	    {
	      //-- get ID
	      tagid3 = tagids.name2id(ngi3->first);

	      //-- compute trigram probability
	      ngprobs3[Trigram(tagid,tagid2,tagid3)] = ngi3->second / ngi2->second.count;
	    }
#endif // moot_USE_TRIGRAMS
	}
    }

  viterbi_clear();
  return true;
}

/*--------------------------------------------------------------------------
 * Compilation utilities: ID-assignment
 *--------------------------------------------------------------------------*/
void mootHMM::assign_ids_lf(const mootLexfreqs &lexfreqs)
{
  //-- add special type-tokens
  for (TokID i = 0; i < NTokTypes; i++) {
    if (i == TokTypeAlpha || i == TokTypeUnknown) { continue; }
    typids[i] =
      tokids.nameExists(typnames[i])
      ? tokids.name2id(typnames[i])
      : tokids.insert(typnames[i]);
  }
  typids[TokTypeAlpha] = 0;
  typids[TokTypeUnknown] = 0;

  //-- compile lexical IDs
  for (mootLexfreqs::LexfreqTokTable::const_iterator lfti = lexfreqs.lftable.begin();
       lfti != lexfreqs.lftable.end();
       lfti++)
    {
      const mootTokString &tokstr = lfti->first;
      const mootLexfreqs::LexfreqEntry &entry = lfti->second;
      //const mootLexfreqs::LexfreqCount toktotal = entry.total;

      TokenType typ = token2type(tokstr);

      //-- ... for all tags occurring with this token(lftagi)
      for (mootLexfreqs::LexfreqSubtable::const_iterator lftagi = entry.freqs.begin();
	   lftagi != entry.freqs.end();
	   lftagi++)
	{
	  const mootTagString &tagstr   = lftagi->first;
	  //const CountT        &tagcount = lftagi->second;

	  //-- always assign a tag-id
	  if (!tagids.nameExists(tagstr)) tagids.insert(tagstr);

	  if (typ != TokTypeAlpha)
	    //-- ignore non-alphabetics
	    continue; 

	  if (
            #ifndef moot_LEX_UNKNOWN_TOKENS
	      //-- unknown threshhold check
	      toktotal > unknown_lex_threshhold &&
            #endif // moot_LEX_UNKNOWN_TOKENS
	      !tokids.nameExists(tokstr))
	    {
	      //-- it's a kosher token : assign a token-ID
	      tokids.insert(tokstr);
	    }
       
	  //-- otherwise, it's below the unknown threshhold, so we ignore it here
	}
    }

  //-- update number of tags, classes
  n_tags = tagids.size();
  n_toks = tokids.size();

  return;
}

void mootHMM::assign_ids_ng(const mootNgrams   &ngrams)
{
  //-- Compile tag IDs (from unigrams only!)
  for (mootNgrams::NgramTable::const_iterator ngi1 = ngrams.ngtable.begin();
       ngi1 != ngrams.ngtable.end();
       ngi1++)
    {
      if (!tagids.nameExists(ngi1->first)) tagids.insert(ngi1->first);
    }

  //-- update number of tags
  n_tags = tagids.size();

  return;
}

void mootHMM::assign_ids_cf(const mootClassfreqs &classfreqs)
{
  LexClass lclass;

  //-- compile class IDs : for each string class (lcti) :
  for (mootClassfreqs::ClassfreqTable::const_iterator lcti = classfreqs.lctable.begin();
       lcti != classfreqs.lctable.end();
       lcti++)
    {
      const mootTagSet &tagset = lcti->first;
      const mootClassfreqs::ClassfreqEntry &entry = lcti->second;

      //-- ... convert to an id-class, adding tags
      lclass.clear();
      tagset2lexclass(tagset,&lclass,true);

      //-- assign class-id (don't autopopulate)
      if (!classids.nameExists(lclass)) classids.insert(lclass);

      //-- ... for all tags assigned to this class (lctagi)
      for (mootClassfreqs::ClassfreqSubtable::const_iterator lctagi = entry.freqs.begin();
	   lctagi != entry.freqs.end();
	   lctagi++)
	{
	  const mootTagString &tagstr   = lctagi->first;
	  //-- just assign a tag-id
	  if (!tagids.nameExists(tagstr)) tagids.insert(tagstr);
	}
    }

  //-- instantiate 'uclassid'
  lclass.clear();
  if (!unknown_tagset.empty()) {
    //-- ... convert to an id-class, adding tags
    tagset2lexclass(unknown_tagset,&lclass,true);

    //-- get or assign class-id (don't autopopulate)
    uclassid = classids.name2id(lclass);
    if (uclassid==0) uclassid = classids.insert(lclass);
  }
  else {
    //-- uclass is empty : try the empty class (still might be zero)
    uclassid = classids.name2id(LexClass());
    mootClassfreqs::ClassfreqTable::const_iterator cfi = classfreqs.lctable.find(mootTagSet());
    if (uclassid != 0 && cfi != classfreqs.lctable.end()) {
      //-- we found the empty class: populate it with actual tags from training data
      const mootClassfreqs::ClassfreqEntry &uentry = cfi->second;

      //-- ... for all tags assigned to this class (uctagi)
      for (mootClassfreqs::ClassfreqSubtable::const_iterator uctagi = uentry.freqs.begin();
	   uctagi != uentry.freqs.end();
	   uctagi++)
	{
	  //-- add tag-id to 'lclass'
	  lclass.insert(tagids.name2id(uctagi->first));
	}

      //-- is it still empty?
      if (lclass.empty()) {
	uclassid = 0; //-- argh
      } else {
	//-- get or assign class-id (don't autopopulate)
	uclassid = classids.name2id(lclass);
	if (uclassid==0) uclassid = classids.insert(lclass);
      }
    }

    //-- is it STILL zero????
    if (!uclassid) {
      //-- buggrit : instantiate a 'universal' class
      lclass.clear();
      for (TagID t = 1; t < tagids.size(); t++) { lclass.insert(t); }
      //-- get or assign class-id (don't autopopulate)
      uclassid = classids.name2id(lclass);
      if (uclassid==0) uclassid = classids.insert(lclass);
    }
  }

  //-- update number of tags, classes
  n_tags = tagids.size();
  n_classes = classids.size();

  return;
}


/*--------------------------------------------------------------------------
 * Compilation utilities: smoothing constant estimation
 *--------------------------------------------------------------------------*/
/** TODO: add better moot_USE_TRIGRAMS check here. */
bool mootHMM::estimate_lambdas(const mootNgrams &ngrams)
{
  //-- sanity check
  if (ngrams.ugtotal <= 1) {
    fprintf(stderr, "mootHMM::estimate_lambdas(): Error bad unigram total in 'ngrams'!\n");
    return false;
  }

  //mootNgrams::NgramString  ngtmp;    //-- temporary string-ngram (for lookup)

  ProbT   f_t123;                     //-- current  trigram  count: f(t1,t2,t3)
  ProbT   f_t23;                      //-- current  bigram   count: f(t2,t3)
  ProbT   f_t12;                      //-- previous bigram   count: f(t1,t2)
  ProbT   f_t3;                       //-- current  unigram  count: f(t3)
  ProbT   f_t2;                       //-- previous unigram  count: f(t2)
  ProbT   f_N;                        //-- corpus size (unigram total)

  ProbT ngp123;  //-- adjusted trigram probability : (f(t1,t2,t3) - 1) / (f(t1,t2)    - 1)
  ProbT ngp23;   //-- adjusted bigram probability  : (f(t2,t3)    - 1) / (f(t2)       - 1)
  ProbT ngp3;    //-- adjusted unigram probability : (f(t3)       - 1) / (corpus_size - 1)

  //-- initialize
  f_N       = ngrams.ugtotal;
  nglambda1 = 0.0;
  nglambda2 = 0.0;
#ifdef moot_USE_TRIGRAMS
  nglambda3 = 0.0;
#else
  ProbT nglambda3 = 0.0; //-- Hack!
#endif

  //-- adjust lambdas
  for (mootNgrams::NgramTable::const_iterator ngi1 = ngrams.ngtable.begin();
       ngi1 != ngrams.ngtable.end();
       ngi1++)
    {
      for (mootNgrams::BigramTable::const_iterator ngi2 = ngi1->second.freqs.begin();
	   ngi2 != ngi1->second.freqs.end();
	   ngi2++)
	{
	  //-- previous bigram count: f(t1,t2)
	  f_t12 = ngi2->second.count;

	  //-- previous unigram count : f(t2)
	  f_t2 = ngrams.lookup(ngi2->first);

	  for (mootNgrams::TrigramTable::const_iterator ngi3 = ngi2->second.freqs.begin();
	       ngi3 != ngi2->second.freqs.end();
	       ngi3++)
	    {
	      //-- current trigram count : f(t1,t2,t3)
	      f_t123 = ngi3->second;

	      //-- current bigram count : f(t2,t3)
	      f_t23 = ngrams.lookup(ngi2->first,ngi3->first);

	      //-- current unigram count : f(t3)
	      f_t3 = ngrams.lookup(ngi3->first);

	      //-- compute adjusted probabilities
	      ngp123 =  f_t12 == 1  ?  0  : (f_t123 - 1.0) / (f_t12 - 1.0);
	      ngp23  =  f_t23 == 1  ?  0  : (f_t23  - 1.0) / (f_t2  - 1.0);
	      ngp3   =  f_N   == 1  ?  0  : (f_t3   - 1.0) / (f_N   - 1.0);

	      //-- adjust lambdas
	      if (ngp123 >= ngp23  && ngp123 >= ngp3)
		nglambda3 += f_t123;
	      else if (ngp23  >= ngp123 && ngp23  >= ngp3)
		nglambda2 += f_t123;
	      else
		nglambda1 += f_t123;
	    }
	}
    }

  /*---- OLD ---- */
  /*
  //-- adjust lambdas
  for (mootNgrams::NgramStringTable::const_iterator ngti = ngrams.ngtable.begin();
       ngti != ngrams.ngtable.end();
       ngti++)
    {
      if (ngti->first.size() == 3) {
	//-- it's a trigram

	//-- current trigram count : f(t1,t2,t3)
	f_t123 = ngti->second;

	//-- current bigram count : f(t2,t3)
	ngtmp = ngti->first;
	ngtmp.pop_front();
	f_t23 = ngrams.lookup(ngtmp);

	//-- current unigram count : f(t3)
	ngtmp.pop_front();
	f_t3 = ngrams.lookup(ngtmp);

	//-- previous bigram count: f(t1,t2)
	ngtmp = ngti->first;
	ngtmp.pop_back();
	f_t12 = ngrams.lookup(ngtmp);

	//-- previous unigram count : f(t2)
	ngtmp.pop_front();
	f_t2 = ngrams.lookup(ngtmp);

	//-- compute adjusted probabilities
	ngp123 =  f_t12 == 1  ?  0  : (f_t123 - 1.0) / (f_t12 - 1.0);
	ngp23  =  f_t23 == 1  ?  0  : (f_t23  - 1.0) / (f_t2  - 1.0);
	ngp3   =  f_N   == 1  ?  0  : (f_t3   - 1.0) / (f_N   - 1.0);

	//-- adjust lambdas
	if (ngp123 >= ngp23  && ngp123 >= ngp3)
	  nglambda3 += f_t123;
	else if (ngp23  >= ngp123 && ngp23  >= ngp3)
	  nglambda2 += f_t123;
	else
	  nglambda1 += f_t123;
      }
      //-- Otherwise, it's not a (N_max)-gram: ignore it
    }
  */

  //-- normalize lambdas
  ProbT nglambda_total = nglambda1 + nglambda2;
#ifdef moot_USE_TRIGRAMS
  nglambda_total += nglambda3;
  nglambda3 /= nglambda_total;
#endif /* moot_USE_TRIGRAMS */

  nglambda2 /= nglambda_total;
  nglambda1 /= nglambda_total;

  return true;
}


bool mootHMM::estimate_wlambdas(const mootLexfreqs &lf)
{
  //-- estimate lexical smoothing constants
  if (lf.n_tokens > 0) {
    wlambda2 = 1.0 / (ProbT)(lf.n_tokens);
    wlambda1 = 1.0 - wlambda2;
  } else {
    wlambda2 = DBL_EPSILON;
    wlambda1 = 1.0 - wlambda2;
  }
  return true;
}

/*--------------------------------------------------------------
 * Viterbi: clear
 */
void mootHMM::viterbi_clear(void)
{
  //-- move to trash: state-table
  ViterbiColumn *col, *col_next;
  ViterbiNode   *nod, *nod_next;
  for (col = vtable; col != NULL; col = col_next) {
    col_next      = col->col_prev;
    col->col_prev = trash_columns;
    trash_columns = col;
    for (nod = col->nodes; nod != NULL; nod = nod_next) {
      nod_next      = nod->row_next;
      nod->row_next = trash_nodes;
      trash_nodes   = nod;
    }
  }

  //viterbi_clear_bestpath();

  //-- add BOS entry
  vtable = viterbi_get_column();
  //vtable->tokid = start_tokid;
  vtable->col_prev = NULL;
  nod = vtable->nodes = viterbi_get_node();
  nod->tagid = start_tagid;
  nod->prob  = 1.0;
  nod->row_next = NULL;
  nod->pth_prev = NULL;
}


/*--------------------------------------------------------------
 * Viterbi: step : TokID
 */
void mootHMM::viterbi_step(TokID tokid)
{
  //-- Get next column
  ViterbiColumn *col = viterbi_get_column();
  ViterbiNode   *nod;
  col->col_prev = vtable;
  col->nodes = NULL;

  //-- sanity check
  if (tokid >= n_toks) tokid = 0;

  //-- info: check for "unknown" token
  if (tokid==0) nunknown++;

  //-- Get map of possible destination tags
  const LexProbSubTable &lps = lexprobs[tokid];

  //-- for each possible destination tag 'vtagid'
  for (LexProbSubTable::const_iterator lpsi = lps.begin(); lpsi != lps.end(); lpsi++)
    {
      vtagid  = lpsi->first;

      //-- ignore "unknown" tag
      if (vtagid == 0) continue;

      //-- get lexical probability: p(tok|tag) 
      vwordpr = ( (wlambda1 * lpsi->second) + wlambda2 );

      //-- find best previous tag by n-gram probabilites: store information in vbestpr,vbestpn
      viterbi_find_best_prevnode(vtagid, tagp(vtagid));

      //-- skip zero-probabilities
      //if (vbestpr <= 0) continue;

      //-- update state table column for current destination tag
      nod           = viterbi_get_node();
      nod->tagid    = vtagid;
      nod->prob     = (vbestpr * vwordpr);
      nod->pth_prev = vbestpn;
      nod->row_next = col->nodes;
      col->nodes    = nod;
    }

  if (col->nodes == NULL) {
    //-- we might not have found anything...
    _viterbi_step_fallback(tokid, col);
  } else{
    //-- add new column to state table
    vtable = col;
  }
};

/*--------------------------------------------------------------
 * Viterbi: step : (TokID,ClassID,LexClass)
 */
void mootHMM::viterbi_step(TokID tokid, ClassID classid, const LexClass &lclass)
{
  //-- Get next column
  ViterbiColumn *col = viterbi_get_column();
  ViterbiNode   *nod;
  col->col_prev = vtable;
  col->nodes = NULL;

  //-- sanity check(s)
  if (tokid >= n_toks) tokid = 0;
  if (classid >= n_classes) classid = 0;

  //-- unknown class check(s)
  if (!classid) classid = uclassid;

  //-- info: check for "unknown" token + class
  if (tokid==0 && classid==uclassid) nunknown++;

  //-- Get map of possible destination tags
  const LexProbSubTable      &lps  = lexprobs[tokid];
  const LexClassProbSubTable &lcps = lcprobs[classid];

  //-- Iterators
  LexProbSubTable::const_iterator      lpsi;
  LexClassProbSubTable::const_iterator lcpsi;

  //-- for each possible destination tag 'vtagid'
  for (LexClass::const_iterator lci = lclass.begin(); lci != lclass.end(); lci++)
    {
      vtagid  = *lci;

      //-- ignore "unknown" tag(s)
      if (vtagid >= n_tags || vtagid == 0) continue;

      //-- get lexical probability:
      if (tokid != 0) {
	//-- Known token: use "real" lexical probabilities
	//   P(tok|tag) := lambda_{w1}*p(tok|tag) + lambda_{w2}
	lpsi = lps.find(vtagid);
	vwordpr = (wlambda2 + (lpsi  == lps.end()  ? 0 : (wlambda1 * lpsi->second)) );
      }
      //else if (classid != uclassid && classid != 0) {
      else if (classid != 0) {
	//-- Unknown token, known class: use class probabilities
	//   P(tok|tag) := lambda_{c}*p(class|tag) + lambda_{w2}
	lcpsi = lcps.find(vtagid);
	vwordpr = (wlambda2 + (lcpsi  == lcps.end()  ? 0 : (wlambda1 * lcpsi->second)) );
      }
      else { // (classid == uclassid || classid == 0)
	//-- Totally unknown token: use "unknown" token probabilities
	//   P(tok|tag) := lambda_{w1}p(0|tag) + lambda_{w2}
	lpsi = lps.find(vtagid);
	vwordpr = (wlambda2 
		   + (lpsi  == lps.end()  ? 0 : (wlambda1 * lpsi->second)) );
      }

      //-- find best previous tag by n-gram probabilites: store information in vbestpr,vbestpn
      viterbi_find_best_prevnode(vtagid, tagp(vtagid));

      //-- skip zero-probabilities
      //if (vbestpr <= 0) continue;

      //-- update state table column for current destination tag
      nod           = viterbi_get_node();
      nod->tagid    = vtagid;
      nod->prob     = (vbestpr * vwordpr);
      nod->pth_prev = vbestpn;
      nod->row_next = col->nodes;
      col->nodes    = nod;
    }

  if (col->nodes == NULL) {
    //-- oops: we haven't found anything...
    _viterbi_step_fallback(tokid, col);
  } else{
    //-- add new column to state table
    vtable = col;
  }
}


/*--------------------------------------------------------------
 * Viterbi: single iteration: (TokID,TagID,col=NULL)
 */
void mootHMM::viterbi_step(TokID tokid, TagID tagid, ViterbiColumn *col)
{
  ViterbiNode   *nod;
  if (col==NULL) {
    //-- Get next column
    col = viterbi_get_column();
    col->col_prev = vtable;
    col->nodes = NULL;
  }

  //-- sanity check
  if (tokid >= n_toks) tokid = 0;

  //-- for the destination tag 'vtagid'
  vtagid = tagid >= n_tags ? 0 : tagid;

  //-- get lexical probability: p(tok|tag) 
  vwordpr = ( (wlambda1 * wordp(tokid,tagid)) + wlambda2 );

  //-- find best previous tag by n-gram probabilites: store information in vbestpr,vbestpn
  viterbi_find_best_prevnode(vtagid, tagp(vtagid));

  //-- update state table column for current destination tag
  nod           = viterbi_get_node();
  nod->tagid    = vtagid;
  nod->prob     = (vbestpr * vwordpr);
  nod->pth_prev = vbestpn;
  nod->row_next = col->nodes;
  col->nodes    = nod;

  //-- add new column to state table
  vtable = col;
}


/*------------------------------------------------------------
 * Viterbi: fallback
 */
void mootHMM::_viterbi_step_fallback(TokID tokid, ViterbiColumn *col)
{
  //-- info
  nfallbacks++;

  //-- sanity
  if (tokid >= n_toks) tokid = 0;
  if (col==NULL) {
    //-- Get next column
    col = viterbi_get_column();
    col->col_prev = vtable;
    col->nodes = NULL;
  }

  //-- variables
  ViterbiNode                     *nod;
  const LexProbSubTable           &lps = lexprobs[tokid];
  LexProbSubTable::const_iterator  lpsi;

  //-- for each possible destination tag 'vtagid' (except "UNKNOWN")
  for (vtagid = 1; vtagid < n_tags; vtagid++) {

    //-- get lexical probability: p(tok|tag) 
    lpsi = lps.find(vtagid);
    if (lpsi != lps.end()) {
      vwordpr = ( (wlambda1 * lpsi->second) + wlambda2 );
    } else {
      vwordpr = wlambda2;
    }

    //-- find best previous tag by n-gram probabilites: store information in vbestpr,vbestpn
    viterbi_find_best_prevnode(vtagid, tagp(vtagid));

    //-- skip zero-probabilities
    //if (vbestpr <= 0) continue;

    //-- update state table column for current destination tag
    nod           = viterbi_get_node();
    nod->tagid    = vtagid;
    nod->prob     = (vbestpr * vwordpr);
    nod->pth_prev = vbestpn;
    nod->row_next = col->nodes;
    col->nodes    = nod;
  }

  if (col->nodes == NULL) {
    //-- we STILL might not have found anything...
    viterbi_step(tokid, 0, col);
  } else{
    //-- add new column to state table
    vtable = col;
  }
}


/*--------------------------------------------------------------------------
 * Top-level: stream
 *--------------------------------------------------------------------------*/
void mootHMM::tag_stream(FILE *in, FILE *out, char *srcname)
{
  TokenReader treader(false,input_ignore_first_analysis);
  TokenWriter twriter(output_best_only);

  treader.select_stream(in,srcname);
  do {
    mootSentence &sent = treader.get_sentence();
    //if (sent.empty()) break;
    if (treader.lexer.lasttyp == mootTokenLexer::TLEOF) break;

    tag_sentence(sent);

    twriter.sentence_put(out,sent);
  } while (1);
}


/*--------------------------------------------------------------------------
 * Top-level: strings
 *--------------------------------------------------------------------------*/
void mootHMM::tag_strings(int argc, char **argv, FILE *out, char *infilename)
{
  //-- prepare variables
  mootSentence sent;
  for ( ; --argc >= 0; argv++) {
    sent.push_back(mootToken(mootTokString((const char *)*argv)));
  }
  tag_sentence(sent);
  nsents++;
}

/*--------------------------------------------------------------------------
 * Mid-level: output
 *--------------------------------------------------------------------------*/
void mootHMM::tag_mark_best(mootSentence &sentence)
{
  //-- populate 'vbestpath' with (ViterbiPathNode*)s
  ViterbiPathNode *pnod = viterbi_best_path();
  mootSentence::iterator senti;

  if (pnod) pnod = pnod->path_next;  //-- skip boundary tag

  for (senti = sentence.begin(); senti != sentence.end(); senti++) {
    if (pnod && pnod->node) {
      senti->besttag(tagids.id2name(pnod->node->tagid));
      pnod = pnod->path_next;
    }
    else {
      //-- this should never actually happen, but it has...
      carp("%s: Error: no best tag for token '%s'!\n",
	   "mootHMM::tag_mark_best()", senti->text().c_str());
      senti->besttag(tagids.id2name(0)); //-- use 'unknown' tag
    }
  }
};


/*--------------------------------------------------------------------------
 * Debug / Dump
 *--------------------------------------------------------------------------*/
void mootHMM::txtdump(FILE *file)
{
  fprintf(file, "%%%% mootHMM text dump\n");

  fprintf(file, "\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  fprintf(file, "%%%% Constants\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  fprintf(file, "%%start_tag\tID=%u(\"%s\")\n", start_tagid, tagids.id2name(start_tagid).c_str());
  fprintf(file, "%%nglambda1\t%g\n", nglambda1);
  fprintf(file, "%%nglambda2\t%g\n", nglambda2);
#ifdef moot_USE_TRIGRAMS
  fprintf(file, "%%nglambda3\t%g\n", nglambda3);
#endif
  fprintf(file, "%%wlambda1\t%g\n", wlambda1);
  fprintf(file, "%%wlambda2\t%g\n", wlambda2);
  fprintf(file, "%%uclassid\t%u\n", uclassid);


  //-- common variables
  TokID tokid;
  TagID tagid, ptagid;
  ProbT prob;

  fprintf(file, "\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  fprintf(file, "%%%% Lexical Probabilities\n");
  fprintf(file, "%%%% TokID(\"TokStr\")\tTagID(\"TagStr\")\tp(TokID|TagID)\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  LexProbTable::const_iterator lpi;
  for (lpi = lexprobs.begin() , tokid = 0;
       lpi != lexprobs.end()  ;
       lpi++                  , tokid++)
    {
      for (LexProbSubTable::const_iterator lpsi = lpi->begin(); lpsi != lpi->end(); lpsi++)
	{
	  TagID tagid = lpsi->first;
	  ProbT prob  = lpsi->second;
	  fprintf(file, "%u(\"%s\")\t%u(\"%s\")\t%g\n",
		  tokid, tokids.id2name(tokid).c_str(),
		  tagid, tagids.id2name(tagid).c_str(),
		  prob);
	}
    }

  fprintf(file, "\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  fprintf(file, "%%%% Class Probabilities\n");
  fprintf(file, "%%%% ClassID(\"ClassStr\")\tTagID(\"TagStr\")\tp(ClassID|TagID)\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  LexClassProbTable::const_iterator cpi;
  ClassID cid;
  for (cpi = lcprobs.begin() , cid = 0;
       cpi != lcprobs.end()  ;
       cpi++                 , cid++)
    {
      const LexClass &lclass = classids.id2name(cid);
      for (LexClassProbSubTable::const_iterator cpsi = cpi->begin(); cpsi != cpi->end(); cpsi++)
	{
	  TagID ctagid = cpsi->first;
	  ProbT cprob  = cpsi->second;

	  fprintf(file, "%u(\"", cid);
	  for (LexClass::const_iterator lci = lclass.begin(); lci != lclass.end(); lci++)
	    {
	      fprintf(file, "%s%s",
		      (lci == lclass.begin() ? "" : " "),
		      tagids.id2name(*lci).c_str());
	    }

	  fprintf(file, "\")\t%u(\"%s\")\t%g\n",
		  ctagid, tagids.id2name(ctagid).c_str(),
		  cprob);
	}
    }

  fprintf(file, "\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  fprintf(file, "%%%% Unigram Probabilities\n");
  fprintf(file, "%%%% TagID(\"TagStr\")\tp(TagID)\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  if (ngprobs1 != NULL) {
    for (tagid = 0; tagid < n_tags; tagid++) {
      prob = ngprobs1[tagid];
      if (prob == 0) continue;
      fprintf(file, "%u(\"%s\")\t%g\n",
	      tagid, tagids.id2name(tagid).c_str(),
	      prob);
    }
  }
  else {
    fprintf(file, "%% (NULL)\n");
  }

  fprintf(file, "\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  fprintf(file, "%%%% Bigram Probabilities\n");
  fprintf(file, "%%%% PrevTagID(\"PrevTagStr\")\tTagID(\"TagStr\")\tp(TagID|PrevTagID)\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  if (ngprobs2 != NULL) {
    for (ptagid = 0; ptagid < n_tags; ptagid++) {
      for (tagid = 0; tagid < n_tags; tagid++) {
	prob = ngprobs2[(n_tags*ptagid)+tagid];
	if (prob == 0) continue;
	fprintf(file, "%u(\"%s\")\t%u(\"%s\")\t%g\n",
		ptagid,  tagids.id2name(ptagid).c_str(),
		tagid, tagids.id2name(tagid).c_str(),
		prob);
      }
    }
  } else {
    fprintf(file, "%% (NULL)\n");
  }

#ifdef moot_USE_TRIGRAMS
  fprintf(file, "\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  fprintf(file, "%%%% Trigram Probabilities\n");
  fprintf(file, "%%%% PrevPrevPrevTagID(\"PrevPrevTagStr\")\tPrevTagID(\"PrevTagStr\")\tTagID(\"TagStr\")\tp(TagID|PrevPrevTagID,PrevTagID)\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  TagID pptagid;
  if (!ngprobs3.empty()) {
    for (TrigramProbTable::const_iterator tgti = ngprobs3.begin();
	 tgti != ngprobs3.end();
	 tgti++)
      {
	pptagid = tgti->first.tag1;
	ptagid  = tgti->first.tag2;
	tagid   = tgti->first.tag3;
	prob    = tgti->second;
	if (prob == 0) continue;
	fprintf(file, "%u(\"%s\")\t%u(\"%s\")\t%u(\"%s\")\t%g\n",
		pptagid,  tagids.id2name(pptagid).c_str(),
		ptagid,  tagids.id2name(ptagid).c_str(),
		tagid, tagids.id2name(tagid).c_str(),
		prob);
      }
  }
  else {
    fprintf(file, "%% (empty)\n");
  }
#endif /* moot_USE_TRIGRAMS */

  fprintf(file, "\n");
}

void mootHMM::viterbi_txtdump(FILE *file)
{
  fprintf(file, "%%%% mootHMM Viterbi column text dump\n");
  if (vtable == NULL) {
    fprintf(file, "%% (NULL)\n");
    return;
  }

  fprintf(file, "%%%% TagID(\"TagString\")\t: <PrevTagID(\"PrevTagString\")>\t: Prob\n");
  ViterbiNode *node;
  for (node = vtable->nodes; node != NULL; node = node->row_next) {
    if (node->pth_prev == NULL) {
      //-- BOS
      fprintf(file, "%u(\"%s\")\t: <(NULL)>\t: %g\n",
	      node->tagid, tagids.id2name(node->tagid).c_str(),
	      node->prob);
    } else {
      fprintf(file, "%u(\"%s\")\t: <%u(\"%s\")>\t: %g\n",
	      node->tagid,           tagids.id2name(node->tagid).c_str(),
	      node->pth_prev->tagid, tagids.id2name(node->pth_prev->tagid).c_str(),
	      node->prob);
    }
  }
}

/*--------------------------------------------------------------------------
 * Binary I/O: save
 *--------------------------------------------------------------------------*/

/*
const HeaderInfo::VersionT BINCOMPAT_MIN_VER = 1;
const HeaderInfo::VersionT BINCOMPAT_MIN_REV = 3;
const HeaderInfo::VersionT BINCOMPAT_VER = 1;
const HeaderInfo::VersionT BINCOMPAT_REV = 3;
*/
const HeaderInfo::VersionT BINCOMPAT_MIN_VER = 2;
const HeaderInfo::VersionT BINCOMPAT_MIN_REV = 0;

const HeaderInfo::VersionT BINCOMPAT_VER = 2;
const HeaderInfo::VersionT BINCOMPAT_REV = 0;

bool mootHMM::save(const char *filename, int compression_level)
{
  //-- sanity checks
  if (compression_level != Z_DEFAULT_COMPRESSION
      && (compression_level > Z_BEST_COMPRESSION || compression_level < Z_NO_COMPRESSION))
    {
      carp("mootHMM::save(): bad compression level %d defaults to %d\n",
	   compression_level, Z_DEFAULT_COMPRESSION);
      compression_level = Z_DEFAULT_COMPRESSION;
    }

  //-- open file
  gzFile gzf = gzopen(filename, "wb");
  if (!gzf) {
    carp("mootHMM::save(): open failed for file '%s': %s\n",
	 filename, gzerror(gzf, &errno));
    return false;
  }
  gzsetparams(gzf, compression_level, Z_DEFAULT_STRATEGY);


  //-- setup stream-hack
  mootBinStream::ozBinStream ozs(gzf);

  //-- and save
  bool rc = save(ozs, filename);
  ozs.close();
  return rc;
}

bool mootHMM::save(mootBinStream::oBinStream &obs, const char *filename)
{
  HeaderInfo hi(string("mootHMM"),
		BINCOMPAT_VER,     BINCOMPAT_REV,
		BINCOMPAT_MIN_VER, BINCOMPAT_MIN_REV,
		0);
  Item<HeaderInfo> hi_item;
  Item<size_t>     size_item;
  Item<char *>     cmt_item;
  char comment[512];
  sprintf(comment, "\nmootHMM Version %u.%u\n", BINCOMPAT_VER, BINCOMPAT_REV);

  //-- get checksum
  size_t crc = start_tagid + n_tags + n_toks + n_classes;
  if (! (hi_item.save(obs, hi)
	 && size_item.save(obs, crc)
	 && cmt_item.save(obs, comment)
	 ))
    {
      carp("mootHMM::save(): could not save header%s%s\n",
	   (filename ? " to file " : ""), (filename ? filename : ""));
      return false;
    }

  return _bindump(obs, filename);
}

bool mootHMM::_bindump(mootBinStream::oBinStream &obs, const char *filename)
{
  //-- variables
  Item<size_t> size_item;
  Item<TagID> tagid_item;
  Item<TokID> tokid_item;
  Item<ProbT> probt_item;
  Item<TokIDTable> tokids_item;
  Item<TagIDTable> tagids_item;
  Item<LexProbTable> lexprobs_item;
  Item<LexClassProbTable> lcprobs_item;
  Item<mootTokString> tokstr_item;

  if (! (tagid_item.save(obs, start_tagid)
	 && tagid_item.save(obs, uclassid)
	 && probt_item.save(obs, unknown_lex_threshhold)
	 && probt_item.save(obs, nglambda1)
	 && probt_item.save(obs, nglambda2)
#ifdef moot_USE_TRIGRAMS
	 && probt_item.save(obs, nglambda3)
#else
	 && probt_item.save(obs, 0.0)
#endif
	 && probt_item.save(obs, wlambda1)
	 && probt_item.save(obs, wlambda2)
	 && tokids_item.save(obs, tokids)
	 && tagids_item.save(obs, tagids)
	 && size_item.save(obs, n_tags)
	 && size_item.save(obs, n_toks)
	 && size_item.save(obs, n_classes)
	 && lexprobs_item.save(obs, lexprobs)
	 && lcprobs_item.save(obs, lcprobs)
	 && probt_item.save_n(obs, ngprobs1, n_tags)
	 && probt_item.save_n(obs, ngprobs2, n_tags*n_tags)
	 ))
    {
      carp("mootHMM::save(): could not save data%s%s\n",
	   (filename ? " to file " : ""), (filename ? filename : ""));
      return false;
    }

  int i;
  for (i = 0; i < NTokTypes; i++) {
    if (!tokid_item.save(obs, typids[i]) && tokstr_item.save(obs, typnames[i])) {
      carp("mootHMM::save(): could not save type data%s%s\n",
	   (filename ? " to file " : ""), (filename ? filename : ""));
      return false;
    }
  }

  return true;
}

/*--------------------------------------------------------------------------
 * Binary I/O: load
 *--------------------------------------------------------------------------*/

bool mootHMM::load(const char *filename)
{
  //-- setup gzFile
  gzFile gzs = gzopen(filename, "rb");
  if (!gzs) {
    carp("mootHMM::load(): could not open file '%s' for read: %s\n",
	 filename, strerror(errno));
    return false;
  }

  //-- setup stream-hack
  mootBinStream::izBinStream izs(gzs);

  bool rc = load(izs, filename);
  izs.close();
  return rc;
}

bool mootHMM::load(mootBinStream::iBinStream &ibs, const char *filename)
{
  clear(true); //-- make sure the object is totally empty

  HeaderInfo hi, hi_magic(string("mootHMM"));
  size_t     crc;

  Item<HeaderInfo> hi_item;
  Item<size_t>     size_item;
  Item<string>     cmt_item;
  string comment;

  //-- load headers
  if (! (hi_item.load(ibs, hi)
	 && size_item.load(ibs, crc)
	 && cmt_item.load(ibs, comment)
	 ))
    {
      carp("mootHMM::load(): could not load header%s%s\n",
	   (filename ? " from file " : ""), (filename ? filename : ""));
      return false;
    }
  else if (hi.magic != hi_magic.magic)
    {
      carp("mootHMM::load(): bad magic 0x%x%s%s\n",
	   hi.magic,
	   (filename ? " in file " : ""), (filename ? filename : ""));
      return false;
    }
  else if (hi.version < BINCOMPAT_MIN_VER
	   || (hi.version == BINCOMPAT_MIN_VER && hi.revision < BINCOMPAT_MIN_REV)
	   || BINCOMPAT_VER < hi.minver
	   || (BINCOMPAT_VER == hi.minver && BINCOMPAT_MIN_REV < hi.minrev))
    {
      carp("mootHMM::load(): incompatible file version %u.%u%s%s\n",
	   hi.version, hi.revision,
	   (filename ? " in file " : ""), (filename ? filename : ""));
    }

  if(!_binload(ibs, filename))
    return false;

  if (crc != (start_tagid + n_tags + n_toks + n_classes)) {
    carp("mootHMM::load(): checksum failed%s%s\n",
	 (filename ? " for file " : ""), (filename ? filename : ""));
  }

  viterbi_clear(); //-- (re-)initialize Viterbi table
  return true;
}


bool mootHMM::_binload(mootBinStream::iBinStream &ibs, const char *filename)
{
  //-- variables
  Item<size_t> size_item;
  Item<TagID> tagid_item;
  Item<TokID> tokid_item;
  Item<ProbT> probt_item;
  Item<TokIDTable> tokids_item;
  Item<TagIDTable> tagids_item;
  Item<LexProbTable> lexprobs_item;
  Item<LexClassProbTable> lcprobs_item;
  Item<mootTokString>  tokstr_item;
#ifndef moot_USE_TRIGRAMS
  ProbT dummy_nglambda3;
#endif

  size_t ngprobs1_size  = 0;
  size_t ngprobs2_size  = 0;

  if (! (tagid_item.load(ibs, start_tagid)
	 && tagid_item.load(ibs, uclassid)
	 && probt_item.load(ibs, unknown_lex_threshhold)
	 && probt_item.load(ibs, nglambda1)
	 && probt_item.load(ibs, nglambda2)
#ifdef moot_USE_TRIGRAMS
	 && probt_item.load(ibs, nglambda3)
#else
	 && probt_item.load(ibs, dummy_nglambda3)
#endif
	 && probt_item.load(ibs, wlambda1)
	 && probt_item.load(ibs, wlambda2)
	 ))
    {
      carp("mootHMM::load(): could not load smoothing constants%s%s\n",
	   (filename ? " from file " : ""), (filename ? filename : ""));
      return false;
    }

  if (!(tokids_item.load(ibs, tokids)
	&& tagids_item.load(ibs, tagids)))
    {
      carp("mootHMM::load(): could not load ID data%s%s\n",
	   (filename ? " from file " : ""), (filename ? filename : ""));
      return false;
    }

  if (! (size_item.load(ibs, n_tags)
	 && size_item.load(ibs, n_toks)
	 && size_item.load(ibs, n_classes)) )
    {
      carp("mootHMM::load(): could not load base data%s%s\n",
	   (filename ? " from file " : ""), (filename ? filename : ""));
      return false;
    }

  if (! (lexprobs_item.load(ibs, lexprobs)
	 && lcprobs_item.load(ibs, lcprobs)
	 && probt_item.load_n(ibs, ngprobs1, ngprobs1_size)
	 && probt_item.load_n(ibs, ngprobs2, ngprobs2_size)
	 ))
    {
      carp("mootHMM::load(): could not load table data%s%s\n",
	   (filename ? " from file " : ""), (filename ? filename : ""));
      return false;
    }

  int i;
  for (i = 0; i < NTokTypes; i++) {
    if (!tokid_item.load(ibs, typids[i]) && tokstr_item.load(ibs, typnames[i])) {
      carp("mootHMM::save(): could not load type data%s%s\n",
	   (filename ? " to file " : ""), (filename ? filename : ""));
      return false;
    }
  }
  
  return true;
}


/*--------------------------------------------------------------------------
 * Error reporting
 *--------------------------------------------------------------------------*/
void mootHMM::carp(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}
