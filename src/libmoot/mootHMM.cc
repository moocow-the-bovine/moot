/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2005 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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

#include <mootHMM.h>
#include <mootTokenIO.h>
#include <mootCIO.h>
#include <mootZIO.h>
#include <mootBinIO.h>
#include <mootUtils.h>

using namespace std;
using namespace mootBinIO;
using namespace mootio;

/*--------------------------------------------------------------------------
 * Behavioral Flags
 *--------------------------------------------------------------------------*/
//-- define this to resort to hapax counts when no suffix matches
#define NO_SUFFIX_USE_HAPAX

//-- define this to sort lexical assoc vectors by probability on compute_logprobs()
//#define LEX_SORT_BYVALUE

/*--------------------------------------------------------------------------
 * Constructor
 *--------------------------------------------------------------------------*/
mootHMM::mootHMM(void)
  : verbose(1),
    ndots(0),
    save_ambiguities(false),
    save_flavors(false),
    save_mark_unknown(false),
    save_dump_trellis(false),
    use_lex_classes(true),
    start_tagid(0),
    unknown_lex_threshhold(1.0),
    unknown_class_threshhold(1.0),
    nglambda1(mootProbEpsilon),
    nglambda2(1.0 - mootProbEpsilon),
    wlambda0(mootProbEpsilon),
    wlambda1(1.0 - mootProbEpsilon),
    clambda0(mootProbEpsilon),
    clambda1(1.0 - mootProbEpsilon),
    beamwd(1000),
    n_tags(0),
    n_toks(0),
    n_classes(0),
#if !defined(MOOT_USE_TRIGRAMS)
    ngprobs2(NULL),
#elif !defined(MOOT_HASH_TRIGRAMS)
    ngprobs3(NULL),
#endif
    vtable(NULL),
    nsents(0),
    ntokens(0),
    nnewtokens(0),
    nunclassed(0),
    nnewclasses(0),
    nunknown(0),
    nfallbacks(0),
    trash_nodes(NULL),
#ifdef MOOT_USE_TRIGRAMS
    trash_rows(NULL),
#endif
    trash_columns(NULL), 
    trash_pathnodes(NULL),
    vbestpn(NULL),
    vbestpath(NULL)
{
  //-- create special token entries
  for (TokID i = 0; i < NTokFlavors; i++) { flavids[i] = 0; }
  unknown_token_name("@UNKNOWN");
  unknown_tag_name("UNKNOWN");
  uclass = LexClass();
};


/*--------------------------------------------------------------------------
 * clear, freeing dynamic data
 *--------------------------------------------------------------------------*/

void mootHMM::clear(bool wipe_everything, bool unlogify)
{
  //-- iterator variables
  ViterbiColumn *col, *col_next;
#ifdef MOOT_USE_TRIGRAMS
  ViterbiRow    *row, *row_next;
#endif
  ViterbiNode   *nod, *nod_next;

  //-- free trellis: columns, rows, and nodes
  for (col = vtable; col != NULL; col = col_next) {
    col_next      = col->col_prev;
#ifdef MOOT_USE_TRIGRAMS
    for (row = col->rows; row != NULL; row = row_next)
      {
	row_next    = row->row_next;
	for (nod = row->nodes; nod != NULL; nod = nod_next)
#else
	for (nod = col->rows; nod != NULL; nod = nod_next)
#endif
	  {
	    nod_next  = nod->nod_next;
	    delete nod;
	  }
#ifdef MOOT_USE_TRIGRAMS
	delete row;
      }
#endif
    delete col;
  }
  vtable = NULL;

  //-- free trashed trellis cols
  for (col = trash_columns; col != NULL; col = col_next) {
    col_next = col->col_prev;
    delete col;
  }
  trash_columns = NULL;

#ifdef MOOT_USE_TRIGRAMS
  //-- free trashed trellis rows
  for (row = trash_rows; row != NULL; row = row_next) {
    row_next = row->row_next;
    delete row;
  }
  trash_rows = NULL;
#endif

  //-- free trashed trellis nodes
  for (nod = trash_nodes; nod != NULL; nod = nod_next) {
    nod_next = nod->nod_next;
    delete nod;
  }
  trash_nodes = NULL;

  //-- free best-path nodes
  ViterbiPathNode *pnod, *pnod_next;
  for (pnod = vbestpath; pnod != NULL; pnod = pnod_next) {
    pnod_next = pnod->path_next;
    delete pnod;
  }
  vbestpath = NULL;

  //-- free trashed path nodes
  for (pnod = trash_pathnodes; pnod != NULL; pnod = pnod_next) {
    pnod_next       = pnod->path_next;
    delete pnod;
  }
  trash_pathnodes = NULL;

  //-- free n-gram probabilitiy table(s)
#ifdef MOOT_USE_TRIGRAMS
# ifdef MOOT_HASH_TRIGRAMS
  ngprobs3.clear();
# else
  if (ngprobs3) {
    free(ngprobs3);
    ngprobs3 = NULL;
  }
# endif
#else
  if (ngprobs2) {
    free(ngprobs2);
    ngprobs2 = NULL;
  }
#endif // MOOT_USE_TRIGRAMS

  //-- free lexical probabilities
  lexprobs.clear();

  //-- free lexical-class probabilities
  lcprobs.clear();

  //-- reset to default "empty" values
  vbestpn = NULL;
  nsents = 0;
  ntokens = 0;
  nnewtokens = 0;
  nunclassed = 0;
  nnewclasses = 0;
  nunknown = 0;

  //-- un-logify constants
  if (unlogify) {
    nglambda1 = exp(nglambda1);
    nglambda2 = exp(nglambda2);
#ifdef MOOT_USE_TRIGRAMS
    nglambda3 = exp(nglambda3);
#endif
    wlambda0 = exp(wlambda0);
    wlambda1 = exp(wlambda1);
    clambda0 = exp(clambda0);
    clambda1 = exp(clambda1);
    if (beamwd) beamwd = exp(beamwd);
  }

  if (wipe_everything) {
    //-- free id-tables
    tokids.clear();
    tagids.clear();

    for (int i = 0; i < NTokFlavors; i++) {
      flavids[i] = 0;
    }

    start_tagid = 0;
    unknown_lex_threshhold = 1;
    unknown_class_threshhold = 1;
    n_tags = 0;
    n_toks = 0;
    n_classes = 0;
  }
}

/*--------------------------------------------------------------------------
 * Compilation
 *--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
 * Compilation : load_model()
 */
bool mootHMM::load_model(const string &modelname,
			 const mootTagString &start_tag_str,
			 const char *myname,
			 bool  do_estimate_nglambdas,
			 bool  do_estimate_wlambdas,
			 bool  do_estimate_clambdas,
			 bool  do_build_suffix_trie,
			 bool  do_compute_logprobs)
{
  string binfile, lexfile, ngfile, lcfile;
  hmm_parse_model_name(modelname, binfile,lexfile,ngfile,lcfile);

  //-- sanity check
  if (binfile.empty() && lexfile.empty() && ngfile.empty() && lcfile.empty()) {
    carp("%s: Error: no model found for `%s'!\n", myname, modelname.c_str());
    return false;
  }

  //-- load model: binary
  if (!binfile.empty()) {
    if (verbose >= vlProgress)
      carp("%s: loading binary HMM model file '%s'...", myname, binfile.c_str());
    if (!load(binfile.c_str())) {
      carp("\n%s: load FAILED for binary HMM model file '%s'\n", myname, binfile.c_str());
      return false;
    }
    else if (verbose >= vlProgress) carp(" loaded.\n");
  }
  else {
    //-- load model: frequency data
    mootLexfreqs   lexfreqs(32767);
    mootClassfreqs classfreqs(512);
    mootNgrams     ngfreqs;

    //-- load model: lexical frequencies
    if (!lexfile.empty() && moot_file_exists(lexfile.c_str())) {
      if (verbose >= vlProgress)
	carp("%s: loading lexical frequency file '%s'...", myname, lexfile.c_str());

      if (!lexfreqs.load(lexfile.c_str())) {
	carp("\n%s: load FAILED for lexical frequency file `%s'\n", myname, lexfile.c_str());
	return false;
      }
      else if (verbose >= vlProgress) carp(" loaded.\n");
    }

    // -- load model: n-gram frequencies
    if (!ngfile.empty() && moot_file_exists(ngfile.c_str())) {
      if (verbose >= vlProgress)
	carp("%s: loading n-gram frequency file '%s'...", myname, ngfile.c_str());

      if (!ngfreqs.load(ngfile.c_str())) {
	carp("\n%s: load FAILED for n-gram frequency file `%s'\n", myname, ngfile.c_str());
	return false;
      }
      else if (verbose >= vlProgress) carp(" loaded.\n");
    }

    // -- load model: class frequencies
    if (use_lex_classes && !lcfile.empty() && moot_file_exists(lcfile.c_str())) {
      if (verbose >= vlProgress)
	carp("%s: loading class frequency file '%s'...", myname, lcfile.c_str());

      if (!classfreqs.load(lcfile.c_str())) {
	carp("\n%s: load FAILED for class frequency file `%s'\n", myname, lcfile.c_str());
	return false;
      }
      else if (verbose >= vlProgress) carp(" loaded.\n");
    }

    //-- compile HMM
    if (verbose >= vlProgress) carp("%s: compiling HMM...", myname);

    lexfreqs.compute_specials();
    if (!compile(lexfreqs,ngfreqs,classfreqs,start_tag_str)) {
      carp("\n%s: HMM compilation FAILED\n", myname);
      return false;
    }
    else if (verbose >= vlProgress) carp(" compiled.\n");

    //-- check whether to use classes
    if (lcprobs.size() <= 2) use_lex_classes = false;

    //-- estimate smoothing constants: lexical probabiltiies (wlambdas)
    if (do_estimate_wlambdas) {
      if (verbose >= vlProgress)
	carp("%s: estimating lexical lambdas...", myname);
      if (!estimate_wlambdas(lexfreqs)) {
	carp("\n%s: lexical lambda estimation FAILED.\n", myname);
	return false;
      }
      else if (verbose >= vlProgress) carp(" done.\n");
    }

    //-- estimate smoothing constants: n-gram probabiltiies (nglambdas)
    if (do_estimate_nglambdas) {
      if (verbose >= vlProgress)
	carp("%s: estimating n-gram lambdas...", myname);
      if (!estimate_lambdas(ngfreqs)) {
	carp("\n%s: n-gram lambda estimation FAILED.\n", myname);
	return false;
      }
      else if (verbose >= vlProgress) carp(" done.\n");
    }

    //-- estimate smoothing constants: class probabiltiies (clambdas)
    if (use_lex_classes && do_estimate_clambdas) {
      if (verbose >= vlProgress)
	carp("%s: estimating class lambdas...", myname);
      if (!estimate_clambdas(classfreqs)) {
	carp("\n%s: class lambda estimation FAILED.\n", myname);
	return false;
      }
      else if (verbose >= vlProgress) carp(" done.\n");
    }

    //-- build suffix trie
    if (do_build_suffix_trie && suftrie.maxlen() != 0) {
      if (verbose >= vlProgress)
	carp("%s: Building suffix trie ", myname);
      if (!build_suffix_trie(lexfreqs, ngfreqs, (verbose>=vlProgress)))
	{
	  carp("\n%s: suffix trie construction FAILED.\n", myname);
	  return false;
	}
      else if (verbose >= vlProgress)
	carp(": built.\n");
    }

    //-- compute log-probabilities
    if (do_compute_logprobs) {
      if (verbose >= vlProgress)
	carp("%s: computing log-probabilities...", myname);
      if (!compute_logprobs()) {
	carp("\n%s: log-probability computation FAILED.\n", myname);
	return false;
      }
      else if (verbose >= vlProgress) carp(" done.\n");
    }
  }

  return true;
}


/*--------------------------------------------------------------------------
 * Compilation : compile()
 */
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
  if (use_lex_classes) {
    assign_ids_cf(classfreqs);

    //-- compile unknown lex class
    compile_unknown_lexclass(classfreqs);
  }

  //-- get or assign start-tag-ID
  start_tagid = tagids.nameExists(start_tag_str)
    ? tagids.name2id(start_tag_str)
    : tagids.insert(start_tag_str);

  //-- save n_tags, n_toks, n_classes
  n_tags = tagids.size();
  n_toks = tokids.size();
  n_classes = classids.size();

  //-- estimate lambdas (NOT HERE!)
  //estimate_lambdas(ngrams);

  //-- allocate lookup tables : lex
  clear(false,false);
  lexprobs.resize(tokids.size());

  //-- allocate lookup tables : classes
  lcprobs.resize(classids.size());

#if !defined(MOOT_USE_TRIGRAMS)
  //-- allocate: bigrams
  ngprobs2 = (ProbT *)malloc((n_tags*n_tags)*sizeof(ProbT));
  if (!ngprobs2) {
    carp("mootHMM::compile(): Error: could not allocate bigram table.\n");
    return false;
  }
  memset(ngprobs2, 0, (n_tags*n_tags)*sizeof(ProbT));
#elif !defined(MOOT_HASH_TRIGRAMS)
  //-- allocate: bigrams
  ngprobs3 = (ProbT *)malloc((n_tags*n_tags*n_tags)*sizeof(ProbT));
  if (!ngprobs3) {
    carp("mootHMM::compile(): Error: could not allocate trigram table.\n");
    return false;
  }
  memset(ngprobs3, 0, (n_tags*n_tags*n_tags)*sizeof(ProbT));
#endif // MOOT_USE_TRIGRAMS

  //-- compilation variables
  TokID                       tokid;          //-- current token-ID
  mootTokenFlavor             tokflav;        //-- current token-flavor
  LexClass                    lclass;         //-- current lexical class
  ClassID                     classid;        //-- current lexical class-ID
  TagID                       tagid;          //-- current tag-ID
  TagID                       tagid2;         //-- next tag-ID (for bigrams)
#ifdef MOOT_USE_TRIGRAMS
  TagID                       tagid3;         //-- next-next tag-ID (for trigrams)
#endif
  mootLexfreqs::LexfreqCount unTotal = 0 ;    //-- total "unknown" token count
  LexProbSubTable            &untagcts        //-- "unknown" tag counts (later, probabilites)
      = lexprobs[0];
  LexClassProbSubTable       &unctagcts       //-- "unknown" class-class counts (later, probabilites)
      = lcprobs[0];

  //-- compile lexical probabilities : let lexfreqs figure out specials
  //   : this must happen elsewhere (we have const here and want it that way!)
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

      //-- get token flavor, id
      tokflav = tokenFlavor(tokstr);
      tokid   = token2id(tokstr);
      //tokid   = tokflav != TokFlavorUnknown ? tokids.name2id(tokstr) : flavids[tokflav];

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
	  if (tokflav == TokFlavorAlpha && toktotal <= unknown_lex_threshhold) //-- dubious
	    {
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

  if (use_lex_classes) {
    //-- compile class probabilities : for all stringy classes (lcti)
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
	classid = classids.name2id(lclass);

	//-- check for empty/unknown class (non-gaspode)
	if (lclass.empty()) classid = 0;

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

	    //-- unknown class check
	    if (!uclass.empty()
		&& (classtotal <= unknown_class_threshhold
		    || lclass.empty())
		)
	      {
		//-- "unknown" class: just store the raw counts for now
		LexClassProbSubTable::iterator lcpsi = unctagcts.find(tagid);
		if (lcpsi == unctagcts.end()) {
		  unctagcts[tagid] = ctagcount;
		} else {
		  lcpsi->second += ctagcount;
		}
	      }

	    if (classid != 0) {
	      //-- it's a kosher class (too?): compute class probability: p(class|tag)
	      lcprobs[classid][tagid] = ctagcount / ctagtotal;
	    }
	  }
      }

    //-- Normalize "unknown" class probabilities
    if (!unctagcts.empty()) {
      for (LexClassProbSubTable::iterator lcpsi = unctagcts.begin();
	   lcpsi != unctagcts.end();
	   lcpsi++)
	{
	  const mootTagString   &tagstr = tagids.id2name(lcpsi->first);
	  const CountT        ctagtotal = classfreqs.taglookup(tagstr);
	  lcpsi->second                /= ctagtotal;
	}
    }
  }

  //-- Compute ngram probabilites
  //ProbT ugtotal = ngrams.ugtotal;
  ProbT ugtotal = ngrams.ugtotal - ngrams.lookup(start_tag_str);

  for (mootNgrams::NgramTable::const_iterator ngi1 = ngrams.ngtable.begin();
       ngi1 != ngrams.ngtable.end();
       ngi1++)
    {
      //-- look at unigrams first : get ID
      tagid = tagids.name2id(ngi1->first);

      //-- compute unigram probability
#ifdef MOOT_USE_TRIGRAMS
      //   : store as bigram 'UNKNOWN $tagid'
# ifdef MOOT_HASH_TRIGRAMS
      ngprobs3[Trigram(0,0,tagid)] = ngi1->second.count / ugtotal;
# else //!MOOT_HASH_TRIGRAMS
      ngprobs3[tagid] = ngi1->second.count / ugtotal;
# endif //MOOT_HASH_TRIGRAMS
#else // !MOOT_USE_TRIGRAMS
      //   : store as bigram 'UNKNOWN $tagid'
      //ngprobs1[tagid] = ngi1->second.count / ugtotal;
      //ngprobs2[n_tags*tagid] = ngi1->second.count / ugtotal;
      ngprobs2[tagid] = ngi1->second.count / ugtotal;
#endif // MOOT_USE_TRIGRAMS

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
#ifdef MOOT_USE_TRIGRAMS
	  //   : store as trigram(0,tagid1,tagid2)
# ifdef MOOT_HASH_TRIGRAMS
	  ngprobs3[Trigram(0,tagid,tagid2)] = ngi2->second.count / ngi1->second.count;
# else // !MOOT_HASH_TRIGRAMS
	  ngprobs3[(n_tags*tagid)+tagid2] = ngi2->second.count / ngi1->second.count;
# endif // MOOT_HASH_TRIGRAMS
#else // !MOOT_USE_TRIGRAMS
	  //   : store as bigram[tagid1][tagid2]
	  ngprobs2[(n_tags*tagid)+tagid2] = ngi2->second.count / ngi1->second.count;
#endif // MOOT_USE_TRIGRAMS


#ifdef MOOT_USE_TRIGRAMS
	  //-- look at trigrams now
	  for (mootNgrams::TrigramTable::const_iterator ngi3 = ngi2->second.freqs.begin();
	       ngi3 != ngi2->second.freqs.end();
	       ngi3++)
	    {
	      //-- get ID
	      tagid3 = tagids.name2id(ngi3->first);

	      //-- compute trigram probability
# ifdef MOOT_HASH_TRIGRAMS
	      ngprobs3[Trigram(tagid,tagid2,tagid3)] = ngi3->second / ngi2->second.count;
# else // !MOOT_HASH_TRIGRAMS
	      ngprobs3[(n_tags*((n_tags*tagid)+tagid2))+tagid3] = ngi3->second / ngi2->second.count;
# endif // MOOT_HASH_TRIGRAMS
	    }
#endif // MOOT_USE_TRIGRAMS
	}
    }

  viterbi_clear();
  return true;
}

/*--------------------------------------------------------------------------
 * Compilation utilities: Unknown class compilation
 *--------------------------------------------------------------------------*/
void mootHMM::compile_unknown_lexclass(const mootClassfreqs &classfreqs)
{
  uclass.clear();

  //-- do we have any classes at all?
  if (!use_lex_classes) return;

  for (mootClassfreqs::ClassfreqTable::const_iterator lcti = classfreqs.lctable.begin();
       lcti != classfreqs.lctable.end();
       lcti++)
    {
      const mootTagSet                        &tagset = lcti->first;
      const mootClassfreqs::ClassfreqEntry     &entry = lcti->second;
      const CountT                         classtotal = entry.count;

      if (classtotal > unknown_class_threshhold && !tagset.empty())
	continue;

      //-- add IDs to 'uclass'
      tagset2lexclass(tagset, &uclass, false);

      //-- add target tags for empty classes, too
      if (tagset.empty()) {
	//-- ... for all tags assigned to this class (lctagi)
	for (mootClassfreqs::ClassfreqSubtable::const_iterator lctagi = entry.freqs.begin();
	     lctagi != entry.freqs.end();
	     lctagi++)
	  {
	    uclass.insert(tagids.name2id(lctagi->first));
	  }
      }
    }

  //-- update n_classes
  n_classes = classids.size();
}

/*--------------------------------------------------------------------------
 * Compilation utilities: ID-assignment
 *--------------------------------------------------------------------------*/
void mootHMM::assign_ids_lf(const mootLexfreqs &lexfreqs)
{
  //-- add special flavor-tokens
  for (TokID i = 0; i < NTokFlavors; i++) {
    if (i == TokFlavorAlpha || i == TokFlavorUnknown) { continue; }
    flavids[i] =
      tokids.nameExists(mootTokenFlavorNames[i])
      ? tokids.name2id(mootTokenFlavorNames[i])
      : tokids.insert(mootTokenFlavorNames[i]);
  }
  flavids[TokFlavorAlpha] = 0;
  flavids[TokFlavorUnknown] = 0;

  //-- compile lexical IDs
  for (mootLexfreqs::LexfreqTokTable::const_iterator lfti = lexfreqs.lftable.begin();
       lfti != lexfreqs.lftable.end();
       lfti++)
    {
      const mootTokString &tokstr = lfti->first;
      const mootLexfreqs::LexfreqEntry &entry = lfti->second;

#   ifndef MOOT_LEX_NONALPHA
      mootTokenFlavor flav = tokenFlavor(tokstr);
#   endif

      //-- ... for all tags occurring with this token(lftagi)
      for (mootLexfreqs::LexfreqSubtable::const_iterator lftagi = entry.freqs.begin();
	   lftagi != entry.freqs.end();
	   lftagi++)
	{
	  const mootTagString &tagstr   = lftagi->first;
	  //const CountT        &tagcount = lftagi->second;

	  //-- always assign a tag-id
	  if (!tagids.nameExists(tagstr)) tagids.insert(tagstr);

#       ifndef MOOT_LEX_NONALPHA
	  if (flav != TokFlavorAlpha)
	    //-- ignore non-alphabetics
	    continue;
#       endif

	  if (
#       ifndef MOOT_LEX_UNKNOWN_TOKENS
	      //-- unknown threshhold check
	      entry.count > unknown_lex_threshhold &&
#       endif
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

      if (
#ifndef MOOT_LEX_UNKNOWN_CLASSES
	  //-- unknown threshhold check
	  entry.count > unknown_class_threshhold &&
#endif // MOOT_LEX_UNKNOWN_CLASSES
	  !classids.nameExists(lclass) )
	{
	  //-- it's a kosher class : assign a class-ID (don't autopopulate
	  classids.insert(lclass);
	}

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

  //-- update number of tags, classes
  n_tags = tagids.size();
  n_classes = classids.size();

  return;
}


/*--------------------------------------------------------------------------
 * Compilation utilities: smoothing constant estimation : ngrams
 *--------------------------------------------------------------------------*/
bool mootHMM::estimate_lambdas(const mootNgrams &ngrams)
{
  //-- sanity check
  if (ngrams.ugtotal <= 1) {
    fprintf(stderr, "mootHMM::estimate_lambdas(): Error bad unigram total in 'ngrams'!\n");
    return false;
  }

#ifdef MOOT_USE_TRIGRAMS
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
  //f_N       = ngrams.ugtotal;
  f_N       = ngrams.ugtotal - ngrams.lookup(tagids.id2name(start_tagid));

  nglambda1 = 0.0;
  nglambda2 = 0.0;
  nglambda3 = 0.0;

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

  //-- normalize lambdas
  ProbT nglambda_total = nglambda1 + nglambda2 + nglambda3;
  nglambda3 /= nglambda_total;
  nglambda2 /= nglambda_total;
  nglambda1 /= nglambda_total;

#else //-- !MOOT_USE_TRIGRAMS

  ProbT   f_t12;                      //-- current  biigram  count: f(t1,t2)
  ProbT   f_t2;                       //-- current  unigram  count: f(t2)
  ProbT   f_t1;                       //-- previous unigram  count: f(t1)
  ProbT   f_N;                        //-- corpus size (unigram total)

  ProbT ngp12;   //-- adjusted bigram probability  : (f(t1,t2)    - 1) / (f(t1)       - 1)
  ProbT ngp2;    //-- adjusted unigram probability : (f(t2)       - 1) / (corpus_size - 1)

  //-- initialize
  //f_N       = ngrams.ugtotal
  f_N       = ngrams.ugtotal - ngrams.lookup(tagids.id2name(start_tagid));
  nglambda1 = 0.0;
  nglambda2 = 0.0;

  //-- adjust lambdas
  for (mootNgrams::NgramTable::const_iterator ngi1 = ngrams.ngtable.begin();
       ngi1 != ngrams.ngtable.end();
       ngi1++)
    {
      //-- previous unigram count: f(t1)
      f_t1 = ngi1->second.count;

      for (mootNgrams::BigramTable::const_iterator ngi2 = ngi1->second.freqs.begin();
	   ngi2 != ngi1->second.freqs.end();
	   ngi2++)
	{
	  //-- current bigram count: f(t1,t2)
	  f_t12 = ngi2->second.count;

	  //-- current unigram count : f(t2)
	  f_t2 = ngrams.lookup(ngi2->first);

	  //-- ignore eos
	  if (ngi2->first == tagids.id2name(start_tagid)) continue;
	 
	  //-- compute adjusted probabilities
	  ngp12  =  f_t12 == 1  ?  0  : (f_t12  - 1.0) / (f_t1  - 1.0);
	  ngp2   =  f_N   == 1  ?  0  : (f_t2   - 1.0) / (f_N   - 1.0);

	  //-- adjust lambdas
	  if (ngp12 >= ngp2)
	    nglambda2 += f_t12;
	  else
	    nglambda1 += f_t12;
	}
    }

  //-- normalize lambdas
  ProbT nglambda_total = nglambda1 + nglambda2;
  nglambda2 /= nglambda_total;
  nglambda1 /= nglambda_total;

#endif // MOOT_USE_TRIGRAMS

  return true;
}


/*--------------------------------------------------------------------------
 * Compilation utilities: smoothing constant estimation : lexical
 *--------------------------------------------------------------------------*/
bool mootHMM::estimate_wlambdas(const mootLexfreqs &lf)
{
  //-- estimate lexical smoothing constants
  if (lf.n_tokens > 0) {
    wlambda0 = 0.5 / (ProbT)(lf.n_tokens);
    //wlambda0 = 1.0 / (ProbT)(lf.n_tokens);
    wlambda1 = 1.0 - wlambda0;
  } else {
    wlambda0 = mootProbEpsilon;
    wlambda1 = 1.0 - wlambda0;
  }
  return true;
}

/*--------------------------------------------------------------------------
 * Compilation utilities: smoothing constant estimation : lexical-class
 *--------------------------------------------------------------------------*/
bool mootHMM::estimate_clambdas(const mootClassfreqs &cf)
{
  //-- estimate lexical-class smoothing constants
  if (cf.totalcount > 0) {
    clambda0 = 0.5 / (ProbT)(cf.totalcount);
    //clambda0 = 1.0 / (ProbT)(cf.totalcount);
    clambda1 = 1.0 - clambda0;
  } else {
    clambda0 = mootProbEpsilon;
    clambda1 = 1.0 - clambda0;
  }
  return true;
}

/*--------------------------------------------------------------------------
 * Compilation utilities: probability normalization
 *--------------------------------------------------------------------------*/
bool mootHMM::compute_logprobs(void)
{

#ifdef MOOT_USE_TRIGRAMS
# ifdef MOOT_HASH_TRIGRAMS
  ProbT   t3p = 0, t23p = 0, t123p = 0;
  Trigram tg3(0,0,0), tg23(0,0,0), tg123(0,0,0);

  //-- trigram probabilities: stored as '$tagid1,$tagid2,$tagid3'
  for (tg123.tag1 = 1; tg123.tag1 < n_tags; tg123.tag1++) {

    for (tg123.tag2 = 1; tg123.tag2 < n_tags; tg123.tag2++) {
      tg23.tag2 = tg123.tag2;

      for (tg123.tag3 = 0; tg123.tag3 < n_tags; tg123.tag3++) {
	tg23.tag3 = tg123.tag3;
	tg3.tag3 = tg123.tag3;

	t123p = ( (nglambda1 * tagp(tg3,0))
		  +
		  (nglambda2 * tagp(tg23,0))
		  +
		  (nglambda3 * tagp(tg123,0)) );

      if (t123p != 0.0) ngprobs3[tg123] = log(t123p);
      }
    }
  }

  //-- bigram probabilities: stored as '0 $tagid1 $tagid2'
  for (tg23.tag2 = 1; tg23.tag2 < n_tags; tg23.tag2++) {

    for (tg23.tag3 = 0; tg23.tag3 < n_tags; tg23.tag3++) {
      tg3.tag3 = tg23.tag3;

      t23p = ( (nglambda1 * tagp(tg3,0))
	       +
	       (nglambda2 * tagp(tg23,0)) );

      if (t23p != 0.0) ngprobs3[tg23] = log(t23p);
    }
  }

  //-- unigram probabilities: stored as '0 0 $tagid'
  for (tg3.tag3 = 0; tg3.tag3 < n_tags; tg3.tag3++) {
    t3p = nglambda1 * tagp(tg3,0);
    if (t3p != 0.0) ngprobs3[tg3] = log(t3p);
  }

# else //-- !MOOT_HASH_TRIGRAMS

  ProbT   t3p = 0, t23p = 0, t123p = 0;
  TagID   tag1 = 0, tag2 = 0, tag3 = 0;

  //-- trigram probabilities: stored as '$tagid1,$tagid2,$tagid3'
  for (tag1 = 1; tag1 < n_tags; tag1++) {
    for (tag2 = 1; tag2 < n_tags; tag2++) {
      for (tag3 = 0; tag3 < n_tags; tag3++) {

	t123p = ( (nglambda1 * tagp(tag3))
		  +
		  (nglambda2 * tagp(tag2,tag3))
		  +
		  (nglambda3 * tagp(tag1,tag2,tag3)) );

	if (t123p != 0.0)
	  ngprobs3[(n_tags*((n_tags*tag1)+tag2))+tag3] = log(t123p);
	else
	  ngprobs3[(n_tags*((n_tags*tag1)+tag2))+tag3] = MOOT_PROB_ZERO;
      }
    }
  }

  //-- bigram probabilities: stored as '0 $tagid1 $tagid2'
  for (tag2 = 1; tag2 < n_tags; tag2++) {

    for (tag3 = 0; tag3 < n_tags; tag3++) {

      t23p = ( (nglambda1 * tagp(tag3))
	       +
	       (nglambda2 * tagp(tag2,tag3)) );

      if (t23p != 0.0)
	ngprobs3[(n_tags*tag2)+tag3] = log(t23p);
      else
	ngprobs3[(n_tags*tag2)+tag3] = MOOT_PROB_ZERO;

      //-- catch in-between probs: (tag2,UNKNOWN,tag3)
      ngprobs3[(n_tags*((n_tags*tag2)+0))+tag3] = MOOT_PROB_ZERO;
    }
  }

  //-- unigram probabilities: stored as '0 0 $tagid'
  for (tag3 = 0; tag3 < n_tags; tag3++) {
    t3p = nglambda1 * tagp(tag3);
    if (t3p != 0.0)
      ngprobs3[tag3] = log(t3p);
    else
      ngprobs3[tag3] = MOOT_PROB_ZERO;
  }

# endif //-- MOOT_HASH_TRIGRAMS

#else //-- !MOOT_USE_TRIGRAMS

  TagID   tag1 = 0, tag2 = 0;
  ProbT   t1p  = 0, t12p = 0;

  //-- bigram probabilities: stored as '$tagid1 $tagid2'
  for (tag1 = 1; tag1 < n_tags; tag1++) {
    for (tag2 = 0; tag2 < n_tags; tag2++) {
      t12p = ( (nglambda1 * ngprobs2[tag2])
	       +
	       (nglambda2 * ngprobs2[(n_tags*tag1)+tag2]) );

      if (t12p == 0.0)
	ngprobs2[(n_tags*tag1)+tag2] = MOOT_PROB_ZERO;
      else
	ngprobs2[(n_tags*tag1)+tag2] = log(t12p);
    }
  }

  //-- unigram probabilities: stored as 'UNKNOWN $tagid'
  for (tag1 = 0; tag1 < n_tags; tag1++) {
    t1p = nglambda1 * ngprobs2[tag1];
    if (t1p == 0.0)
      ngprobs2[tag1] = MOOT_PROB_ZERO;
    else
      ngprobs2[tag1] = log(t1p);
  }

#endif //-- MOOT_USE_TRIGRAMS


  //-- lexical probabilities
  for (LexProbTable::iterator lpi = lexprobs.begin(); lpi != lexprobs.end(); lpi++) {
    for (LexProbSubTable::iterator lpsi = lpi->begin();
	 lpsi != lpi->end();
	 lpsi++)
      {
	//lpsi->second = log(wlambda0 + (wlambda1 * lpsi->second));
	lpsi->second = log(wlambda1 * lpsi->second);
      }
#ifdef LEX_SORT_BYVALUE
    //-- sort it
    lpi->sort_byvalue();
#endif
  }

  //-- class probabilities
  for (LexClassProbTable::iterator lcpi = lcprobs.begin(); lcpi != lcprobs.end(); lcpi++) {
    for (LexClassProbSubTable::iterator lcpsi = lcpi->begin();
	 lcpsi != lcpi->end();
	 lcpsi++)
      {
	//lcpsi->second = log(clambda0 + (clambda1 * lcpsi->second));
	lcpsi->second = log(clambda1 * lcpsi->second);
      }
#ifdef LEX_SORT_BYVALUE
    //-- sort it
    lcpi->sort_byvalue();
#endif
  }

  //-- suffix-trie probabilities
  for (SuffixTrie::iterator sti = suftrie.begin(); sti != suftrie.end(); sti++) {
    for (SuffixTrieDataT::iterator stdi = sti->data.begin(); stdi != sti->data.end(); stdi++) {
      stdi->second = log(stdi->second);
    }
#ifdef LEX_SORT_BYVALUE
    //-- sort it
    sti->data.sort_byvalue();
#endif
  }

  //-- smoothing constants
  nglambda1 = log(nglambda1);
  nglambda2 = log(nglambda2);
#ifdef MOOT_USE_TRIGRAMS
  nglambda3 = log(nglambda3);
#endif //MOOT_USE_TRIGRAMS
  wlambda0  = log(wlambda0);
  wlambda1  = log(wlambda1);
  clambda0  = log(clambda0);
  clambda1  = log(clambda1);

  if (beamwd) beamwd = log(beamwd);

  return true;
}

/*--------------------------------------------------------------
 * Viterbi: clear
 */
void mootHMM::viterbi_clear(void)
{
  //-- move to trash: trellis
  ViterbiColumn  *col, *col_next;
#ifdef MOOT_USE_TRIGRAMS
  ViterbiRow     *row, *row_next;
#endif
  ViterbiNode    *nod, *nod_next;
  for (col = vtable; col != NULL; col = col_next) {
    col_next      = col->col_prev;
    col->col_prev = trash_columns;
    trash_columns = col;
#ifdef MOOT_USE_TRIGRAMS
    for (row = col->rows; row != NULL; row = row_next)
      {
	row_next      = row->row_next;
	row->row_next = trash_rows;
	trash_rows    = row;
	for (nod = row->nodes; nod != NULL; nod = nod_next)
#else
	for (nod = col->rows; nod != NULL; nod = nod_next)
#endif
	  {
	    nod_next      = nod->nod_next;
	    nod->nod_next = trash_nodes;
	    trash_nodes   = nod;
	  }
#ifdef MOOT_USE_TRIGRAMS
      }
#endif
  }

  //viterbi_clear_bestpath();

  //-- add BOS entry
  vtable             = viterbi_get_column();
  vtable->col_prev   = NULL;

  //-- BOS: initialize beam-pruning stuff
  vtable->bbestpr    = MOOT_PROB_ONE;
  vtable->bpprmin    = MOOT_PROB_NEG;


#ifdef MOOT_USE_TRIGRAMS
  row = vtable->rows = viterbi_get_row();
  row->tagid         = start_tagid;
  row->row_next      = NULL;
  row->wprob         = MOOT_PROB_ONE;

  nod = row->nodes   = viterbi_get_node();
  nod->tagid         = start_tagid;
  nod->ptagid        = start_tagid;
  nod->lprob         = MOOT_PROB_ONE;
  //nod->row           = row;

#else // !MOOT_USE_TRIGRAMS

  nod = vtable->rows = viterbi_get_node();
  nod->tagid         = start_tagid;
  nod->wprob         = MOOT_PROB_ONE;
  nod->lprob         = MOOT_PROB_ONE;

#endif // MOOT_USE_TRIGRAMS

  nod->pth_prev      = NULL;
  nod->nod_next      = NULL;
}


/*--------------------------------------------------------------
 * Viterbi: step : TokID
 */
void mootHMM::viterbi_step(TokID tokid, const mootTokString &toktext)
{
  //-- pointer to next trellis column
  ViterbiColumn *col = NULL;

  //-- sanity check
  if (tokid >= n_toks) tokid = 0;

  //-- info: check for "unknown" token
  if (tokid==0) nnewtokens++;

  //-- Update beam-pruning variable(s)
  //bpprmin = vtable->bbestpr - beamwd;
  //bbestpr = MOOT_PROB_NEG;

  //-- get map of possible tags
  const LexProbSubTable *lps;
  if (tokid != 0) {
    lps = &(lexprobs[tokid]);
  } else {
    size_t matchlen;
    lps = &(suftrie.sufprobs(toktext,&matchlen));
#ifdef NO_SUFFIX_USE_HAPAX
    if (!matchlen) lps = &lexprobs[tokid];
#endif
  }

  //-- for each possible destination tag 'vtagid'
  for (LexProbSubTable::const_iterator lpsi = lps->begin(); lpsi != lps->end(); lpsi++) {
    vtagid  = lpsi->first;

    //-- ignore "unknown" tag
    if (vtagid == 0) continue;

    //-- get lexical probability
    vwordpr = lpsi->second;

    //-- populate new row for this tag
    col = viterbi_populate_row(vtagid, vwordpr, col);
  }

  if (!viterbi_column_ok(col)) {
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
void mootHMM::viterbi_step(TokID tokid,
			   ClassID classid,
			   const LexClass &lclass,
			   const mootTokString &toktext)
{
  //-- sanity check(s)
  if (tokid >= n_toks) tokid = 0;
  if (classid >= n_classes) classid = 0;

  //-- unknown class check(s)
  //if (!classid) classid = 0; //-- uh, yeah... sure...

  //-- info: check for "unknown" token + class
  if (tokid==0) {
    nnewtokens++;
    if (classid == 0) nunknown++;
  }

  //-- set constants
  const LexProbSubTable *lps;
  ProbT wclambda0;
  if (tokid != 0) {
    lps   = &(lexprobs[tokid]);
    wclambda0 = wlambda0;
  }
  else if (use_lex_classes) {
    wclambda0 = wlambda0;
    if (classid != 0) {
      lps   = &(lcprobs[classid]);
    } else {
      size_t matchlen;
      lps = &(suftrie.sufprobs(toktext,&matchlen));
#ifdef NO_SUFFIX_USE_HAPAX
      if (!matchlen) lps = &(lcprobs[classid]);
#endif
    }
  }
  else {
    size_t matchlen;
    lps = &(suftrie.sufprobs(toktext,&matchlen));
#ifdef NO_SUFFIX_USE_HAPAX
    if (!matchlen) lps = &(lexprobs[0]);
#endif
    wclambda0 = wlambda0;
  }

  //-- Get next column
  ViterbiColumn *col = NULL;

  //-- Update beam-pruning variables
  //bpprmin = vtable->bbestpr - beamwd;
  //bbestpr = MOOT_PROB_NEG;

  //-- for each possible destination tag 'vtagid'
#ifdef MOOT_RELAX
  /*
   * Iterate over actual probability-table entries:
   *
   * This is about 3% (2K tok/sec) faster, and gives 3.9% fewer errors
   * (96.66% vs. 96.52% correct), but it loses us almost-mandatory
   * internal coverage ("strictness" of specified class: only 99.61%
   * vs. 100%), so we don't do it this way by default.
   */
  for (LexProbSubTable::const_iterator lpsi = lps->begin(); lpsi != lps->end(); lpsi++)
    {
      vtagid  = lpsi->first;

      //-- ignore "unknown" tag(s)
      if (vtagid >= n_tags || vtagid == 0) continue;

      //-- get lexical probability
      vwordpr = lpsi->second;

      //-- populate a new row for this tag
      col = viterbi_populate_row(vtagid, vwordpr, col);
    }

#else // !MOOT_RELAX

  /*
   * Iterate over actual actual class specified:
   *
   * This is about 2K tok/sec slower than the above, and gives 3.9% more
   * errors (96.52% vs. 96.66% correct), and retains almost-mandatory
   * internal coverage ("strictness" of specified class), so we
   * do it this way instead of the "relaxed" way by default.
   */
  LexProbSubTable::const_iterator lpsi;
  for (LexClass::const_iterator lci = lclass.begin(); lci != lclass.end(); lci++)
    {
      vtagid  = *lci;

      //-- ignore "unknown" tag(s)
      if (vtagid >= n_tags || vtagid == 0) continue;

      //-- get lexical probability
      lpsi = lps->find(vtagid);
      vwordpr = (lpsi==lps->end() ? wclambda0 : lpsi->second);

      //-- populate a new row for this tag
      col = viterbi_populate_row(vtagid, vwordpr, col);
    }
#endif // MOOT_RELAX

  if (!viterbi_column_ok(col)) {
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
  //-- sanity check
  if (tokid >= n_toks) tokid = 0;

  //-- for the destination tag 'vtagid'
  vtagid = tagid >= n_tags ? 0 : tagid;

  //-- get lexical probability: p(tok|tag) : BAD at EOS!
  //vwordpr = wordp(tokid,tagid);
  vwordpr = MOOT_PROB_ONE;

  //-- hack: disable beam-pruning cutoff
  //bpprmin = MOOT_PROB_NEG;

  //-- populate a new row for this tag
  col = viterbi_populate_row(vtagid, vwordpr, col, MOOT_PROB_NEG);

  //-- add new column to state table
  vtable        = col;
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

  //-- variables
  const LexProbSubTable           &lps = lexprobs[tokid];
  LexProbSubTable::const_iterator  lpsi;

  //-- hack: disable beam-pruning cutoff
  //bpprmin = MOOT_PROB_NEG;

  //-- for each possible destination tag 'vtagid' (except "UNKNOWN")
  for (vtagid = 1; vtagid < n_tags; vtagid++) {

    //-- get lexical probability: p(tok|tag) 
    lpsi = lps.find(vtagid);
    if (lpsi != lps.end()) {
      vwordpr = lpsi->second;
    } else {
      vwordpr = wlambda0;
    }

    //-- populate a new row for this tag
    col = viterbi_populate_row(vtagid, vwordpr, col, MOOT_PROB_NEG);
  }

  if (!viterbi_column_ok(col)) {
    //-- we STILL might not have found anything...
    viterbi_step(tokid, 0, col);
  } else {
    //-- add new column to state table
    vtable = col;
  }
}



/*--------------------------------------------------------------------------
 * Top-level: tag_strings
 *--------------------------------------------------------------------------*/
/*
void mootHMM::tag_strings(int argc, char **argv, FILE *out)
{
  //-- prepare variables
  mootSentence sent;
  for ( ; --argc >= 0; argv++) {
    sent.push_back(mootToken(mootTokString((const char *)*argv)));
  }
  tag_sentence(sent);
  nsents++;

  //if (ndots) fputc('\n', stderr);
}
*/

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
    if (senti->toktype() != TokTypeVanilla) continue; //-- ignore non-vanilla tokens
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

  if (save_ambiguities) {
    mootSentence::reverse_iterator sri;
    ViterbiColumn *c = vtable && vtable->col_prev ? vtable->col_prev : NULL;
    if (!c) return;

    for (sri=sentence.rbegin(); c != NULL && sri != sentence.rend(); sri++, c=c->col_prev) {
      if (sri->toktype() != TokTypeVanilla) continue; //-- ignore non-vanilla tokens

      if (save_flavors) {
	sri->tok_analyses.push_back
	  (mootToken::Analysis(mootTokenFlavorNames[tokenFlavor(sri->text())]));
      }

#ifdef MOOT_USE_TRIGRAMS
      //-- get total column probability
      ViterbiRow *r;
      ViterbiNode *n;
      ProbT pcolsum = 0;
      ProbT trowpr;
      for (r = c->rows; r != NULL; r = r->row_next) {
	trowpr = MOOT_PROB_NEG;
	for (n = r->nodes; n != NULL; n = n->nod_next) {
	  if (n->lprob > trowpr) trowpr = n->lprob;
	}
	pcolsum += exp(trowpr);
      }
      //-- dump analyses to mootToken object
      for (r = c->rows; r != NULL; r = r->row_next) {
	trowpr = MOOT_PROB_NEG;
	for (n = r->nodes; n != NULL; n = n->nod_next) {
	  if (n->lprob > trowpr) trowpr = n->lprob;
	}
	sri->tok_analyses.push_back
	  (mootToken::Analysis(tagids.id2name(r->tagid),
			       "",
			       exp(trowpr)/pcolsum));

      }
#else //-- !MOOT_USE_TRIGRAMS
      //-- get total column probability
      ViterbiNode *n;
      ProbT pcolsum = 0;
      for (n = c->rows; n != NULL; n = n->nod_next) {
	pcolsum += exp(n->lprob);
      }
      //-- dump analyses to mootToken object
      for (n = c->rows; n != NULL; n = n->nod_next) {
	sri->tok_analyses.push_back
	  (mootToken::Analysis(tagids.id2name(n->tagid),
			       "",
			       exp(n->lprob)/pcolsum));
      }
#endif //-- MOOT_USE_TRIGRAMS

      if (save_mark_unknown && tokids.name2id(sri->text()) == 0) {
	sri->tok_analyses.push_back(mootToken::Analysis("*","*"));
      }
    }
  }

};


/*--------------------------------------------------------------------------
 * Debug / HMM Dump
 *--------------------------------------------------------------------------*/
void mootHMM::txtdump(FILE *file)
{
  fprintf(file, "%%%% mootHMM text dump\n");

  fprintf(file, "\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  fprintf(file, "%%%% Constants\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  fprintf(file, "start_tagid\t%u(\"%s\")\n", start_tagid, tagids.id2name(start_tagid).c_str());
  fprintf(file, "nglambda1\t%e (=%e)\n", nglambda1, exp(nglambda1));
  fprintf(file, "nglambda2\t%e (=%e)\n", nglambda2, exp(nglambda2));
#ifdef MOOT_USE_TRIGRAMS
  fprintf(file, "nglambda3\t%e (=%e)\n", nglambda3, exp(nglambda3));
#endif
  fprintf(file, "wlambda0\t%e (=%e)\n", wlambda0, exp(wlambda0));
  fprintf(file, "wlambda1\t%e (=%e)\n", wlambda1, exp(wlambda1));

  fprintf(file, "clambda0\t%e (=%e)\n", clambda0, exp(clambda0));
  fprintf(file, "clambda1\t%e (=%e)\n", clambda1, exp(clambda1));
  fprintf(file, "use_lex_classes\t%d\n", use_lex_classes ? 1 : 0);

  fprintf(file, "beamwd\t%e (=%e)\n", beamwd, exp(beamwd));

  fputs("uclass\t", file);
  for (LexClass::const_iterator lci = uclass.begin();  lci != uclass.end(); lci++) {
    if (lci!=uclass.begin()) fputc(' ', file);
    fputs(tagids.id2name(*lci).c_str(), file);
  }
  fputc('\n', file);


  //-- common variables
  TokID tokid;
  TagID tagid, ptagid;
  ProbT prob;

  fprintf(file, "\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  fprintf(file, "%%%% Lexical Probabilities\n");
  fprintf(file, "%%%% TokID(\"TokStr\")\tTagID(\"TagStr\")\tlog(p(TokID|TagID))\tp\n");
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
	  fprintf(file, "%u(\"%s\")\t%u(\"%s\")\t%e\t%e\n",
		  tokid, tokids.id2name(tokid).c_str(),
		  tagid, tagids.id2name(tagid).c_str(),
		  prob,
		  exp(prob));
	}
    }

  fprintf(file, "\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  fprintf(file, "%%%% Class Probabilities\n");
  fprintf(file, "%%%% ClassID(\"ClassStr\")\tTagID(\"TagStr\")\tlog(p(ClassID|TagID))\tp\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  LexClassProbTable::const_iterator cpi;
  ClassID cid;
  for (cpi = lcprobs.begin() , cid = 0;
       cpi != lcprobs.end()  ;
       cpi++                 , cid++)
    {
      const LexClass &lclass = classids.id2name(cid);
      string classname = "";

      for (LexClass::const_iterator lci = lclass.begin();  lci != lclass.end(); lci++) {
	if (lci!=lclass.begin()) classname.push_back(' ');
	classname.append(tagids.id2name(*lci));
      }

      if (cpi->empty()) {
	fprintf(file, "%u(\"%s\")\t--EMPTY--\n", cid, classname.c_str());
	continue;
      }

      for (LexClassProbSubTable::const_iterator cpsi = cpi->begin(); cpsi != cpi->end(); cpsi++)
	{
	  TagID ctagid = cpsi->first;
	  ProbT cprob  = cpsi->second;

	  fprintf(file, "%u(\"%s\")\t%u(\"%s\")\t%e\t%e\n",
		  cid, classname.c_str(),
                  ctagid, tagids.id2name(ctagid).c_str(), cprob, exp(cprob));
	}
    }


#ifdef MOOT_USE_TRIGRAMS
  fprintf(file, "\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  fprintf(file, "%%%% Uni-, Bi-, and Trigram Probabilities\n");
  fprintf(file, "%%%% PrevPrevPrevTagID(\"PrevPrevTagStr\")\tPrevTagID(\"PrevTagStr\")\tTagID(\"TagStr\")\tlog(p(TagID|PrevPrevTagID,PrevTagID))\tp\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  TagID pptagid;
# ifdef MOOT_HASH_TRIGRAMS
  if (!ngprobs3.empty())
# else
  if (ngprobs3 != NULL)
# endif
    {
      for (pptagid = 0; pptagid < n_tags; pptagid++) {
	for (ptagid = 0; ptagid < n_tags; ptagid++) {
	  for (tagid = 0; tagid < n_tags; tagid++) {
	    prob = tagp(pptagid, ptagid, tagid);
	    if (prob != MOOT_PROB_ZERO) {
	      fprintf(file, "%u(\"%s\")\t%u(\"%s\")\t%u(\"%s\")\t%e\t%e\n",
		      pptagid,  tagids.id2name(pptagid).c_str(),
		      ptagid,  tagids.id2name(ptagid).c_str(),
		      tagid, tagids.id2name(tagid).c_str(),
		      prob,
		      exp(prob));
	    }
	  }
	}
      }
    }
  else
    {
      fprintf(file, "%%%% (NULL | empty)\n");
    }

#else // !MOOT_USE_TRIGRAMS

  fprintf(file, "\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  fprintf(file, "%%%% Unigram and Bigram Probabilities\n");
  fprintf(file, "%%%% PrevTagID(\"PrevTagStr\")\tTagID(\"TagStr\")\tlog(p(TagID|PrevTagID))\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  if (ngprobs2 != NULL) {
    for (ptagid = 0; ptagid < n_tags; ptagid++) {
      for (tagid = 0; tagid < n_tags; tagid++) {
	prob = ngprobs2[(n_tags*ptagid)+tagid];
	//if (prob == 0) continue;
	fprintf(file, "%u(\"%s\")\t%u(\"%s\")\t%e\t%e\n",
		ptagid,  tagids.id2name(ptagid).c_str(),
		tagid, tagids.id2name(tagid).c_str(),
		prob,
		exp(prob));
      }
    }
  } else {
    fprintf(file, "%% (NULL)\n");
  }
#endif // MOOT_USE_TRIGRAMS

  fprintf(file, "\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  fprintf(file, "%%%% Suffix Trie\n");
  fprintf(file, "%%%% maxlen=%u ; theta=%e\n", suftrie.maxlen(), suftrie.theta);
  fprintf(file, "%%%% \"Suffix\"\tTagID(\"TagStr\")\tlog(p(Suffix|TagID))\tp\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  for (SuffixTrie::const_iterator sti = suftrie.begin(); sti != suftrie.end(); sti++) {
    string suf = suftrie.node_string(*sti);
    for (SuffixTrieDataT::const_iterator stdi=sti->data.begin(); stdi != sti->data.end(); stdi++)
      {
	fprintf(file, "\"%s\"\t%u(\"%s\")\t%e\t%e\n",
		suf.c_str(),
		stdi->first,
		tagids.id2name(stdi->first).c_str(),
		stdi->second,
		exp(stdi->second));
      }
  }

  fprintf(file, "\n");
}

/*--------------------------------------------------------------------------
 * Debug: Viterbi Trellis Dump
 *--------------------------------------------------------------------------*/
void mootHMM::viterbi_txtdump(TokenWriter *w, int ncols)
{
  w->put_comment_block_begin();
  w->printf_raw("%%%%*********************************************************************\n");
  w->printf_raw("%%%% BEGIN mootHMM Viterbi Trellis text dump\n");
  w->printf_raw("%%%%*********************************************************************\n");

  ViterbiColumn  *col;
  size_t         coli;

  for (coli = 0, col = vtable; col != NULL; col = col->col_prev, coli++) {
    viterbi_txtdump_col(w,col,ncols-coli);
  }

  w->printf_raw("%%%%*********************************************************************\n");
  w->printf_raw("%%%% END mootHMM Viterbi Trellis text dump\n");
  w->printf_raw("%%%%*********************************************************************\n");
  w->put_comment_block_end();
}

/*--------------------------------------------------------------------------
 * Debug: Viterbi Column Dump
 *--------------------------------------------------------------------------*/
void mootHMM::viterbi_txtdump_col(TokenWriter *w, ViterbiColumn *col, int colnum)
{
  w->put_comment_block_begin();

#ifdef MOOT_USE_TRIGRAMS
  ViterbiRow     *row;
  ViterbiNode    *node;
  size_t          rowi;

  w->printf_raw("%%%%=================================================================\n");
  w->printf_raw("%%%% COLUMN %3d: (log) beamwd=%e ; bbestpr=%e ; bpprmin=%e ; cutoff=%e\n",
		colnum, beamwd, col->bbestpr, col->bpprmin, col->bbestpr-beamwd);
  w->printf_raw("%%%%           : (exp) beamwd=%e ; bbestpr=%e ; bpprmin=%e ; cutoff=%e\n",
		exp(beamwd), exp(col->bbestpr), exp(col->bpprmin), exp(col->bbestpr-beamwd));


  for (rowi = 0, row = col->rows; row != NULL; row = row->row_next, rowi++) {
    w->printf_raw("%%%%-----------------------------------------------------\n");
    w->printf_raw("%%%% ROW %d.%u [tag=%u(\"%s\")] ; l(wordp)=%e ; wordp=%e\n",
		  colnum, rowi, row->tagid, tagids.id2name(row->tagid).c_str(),
		  row->wprob, exp(row->wprob));
    w->printf_raw
      ("%%%% TagID(\"Str\")\t [PrevTagID(\"PStr\")]\t <PPrevTagID(\"PPStr\")>:\t log(p) (=p)\n");

    for (node = row->nodes; node != NULL; node = node->nod_next) {
      if (node->pth_prev == NULL) {
	//-- BOS
	w->printf_raw("%u(\"%s\")\t [%u(\"%s\")]\t <(NULL)>\t: %e\t (=%e)\n",
		      node->tagid,   tagids.id2name(node->tagid).c_str(),
		      node->ptagid,  tagids.id2name(node->ptagid).c_str(),
		      node->lprob,
		      exp(node->lprob)
		      );
      } else {
	w->printf_raw("%u(\"%s\")\t [%u(\"%s\")]\t <%u(\"%s\")>\t: %e\t (=%e)\n",
		      node->tagid,             tagids.id2name(node->tagid).c_str(),
		      node->ptagid,            tagids.id2name(node->ptagid).c_str(),
		      node->pth_prev->ptagid,  tagids.id2name(node->pth_prev->ptagid).c_str(),
		      node->lprob,
		      exp(node->lprob)
		      );
      }
    }
  }

#else // !MOOT_USE_TRIGRAMS

  ViterbiNode    *node;

  w->printf_raw("%%%%=================================================================\n");
  w->printf_raw("%%%% COLUMN %3d: (log) beamwd=%e ; bbestpr=%e ; bpprmin=%e ; cutoff=%e\n",
		colnum, beamwd, col->bbestpr, col->bpprmin, col->bbestpr-beamwd);
  w->printf_raw("%%%%           : (exp) beamwd=%e ; bbestpr=%e ; bpprmin=%e ; cutoff=%e\n",
		exp(beamwd), exp(col->bbestpr), exp(col->bpprmin), exp(col->bbestpr-beamwd));

  w->printf_raw("%%%% TagID(\"Str\")\t <PrevTagID(\"PStr\")>:\t log(p)\t (=p)\t log(wp)\t (=wp)\n");

  for (node = col->rows; node != NULL; node = node->nod_next) {
    if (node->pth_prev == NULL) {
      //-- BOS
      w->printf_raw("%u(\"%s\")\t <(NULL)>\t: %e\t (=%e)\n",
		    node->tagid,   tagids.id2name(node->tagid).c_str(),
		    node->lprob,
		    exp(node->lprob),
		    node->wprob,
		    exp(node->wprob)
		    );
    } else {
      w->printf_raw("%u(\"%s\")\t <%u(\"%s\")>\t: %e\t (=%e)\t %e (=%e)\n",
		    node->tagid,             tagids.id2name(node->tagid).c_str(),
		    node->pth_prev->tagid,   tagids.id2name(node->pth_prev->tagid).c_str(),
		    node->lprob,
		    exp(node->lprob),
		    node->wprob,
		    exp(node->wprob)
		    );
    }
  }

#endif // MOOT_USE_TRIGRAMS

  w->put_comment_block_end();
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
const HeaderInfo::VersionT BINCOMPAT_MIN_REV = 5;

const HeaderInfo::VersionT BINCOMPAT_VER = 2;
const HeaderInfo::VersionT BINCOMPAT_REV = 5;

bool mootHMM::save(const char *filename, int compression_level)
{
  //-- open file
#ifdef MOOT_ZLIB_ENABLED
  mootio::mozfstream 
#else 
  mootio::mofstream
#endif
    ofs(filename,"wb");

  if (!ofs.valid()) {
    carp("mootHMM::save(): open failed for \"%s\": %s\n",
	 filename, ofs.errmsg().c_str());
    return false;
  }
  ofs.setparams(compression_level);

  //-- ... and save
  bool rc = save(&ofs, filename);
  ofs.close();
  return rc;
}

bool mootHMM::save(mootio::mostream *obs, const char *filename)
{
  if (!obs || !obs->valid()) return false;

  HeaderInfo hi(string("mootHMM"),
		BINCOMPAT_VER,     BINCOMPAT_REV,
		BINCOMPAT_MIN_VER, BINCOMPAT_MIN_REV,
		0);
  Item<HeaderInfo> hi_item;
  Item<size_t>     size_item;
  Item<char *>     cmt_item;
  Item<bool>       bool_item;
  char comment[512];
  sprintf(comment, "\nmootHMM Version %u.%u\n", BINCOMPAT_VER, BINCOMPAT_REV);

#ifdef MOOT_USE_TRIGRAMS
  bool using_trigrams = true;
# ifdef MOOT_HASH_TRIGRAMS
  bool using_hash = true;
# else
  bool using_hash = false;
# endif
#else
  bool using_trigrams = false;
  bool using_hash     = false;
#endif

  //-- get checksum
  size_t crc = start_tagid + n_tags + n_toks + n_classes + suftrie.size();
  if (! (hi_item.save(obs, hi)
	 && size_item.save(obs, crc)
	 && bool_item.save(obs, using_trigrams)
	 && bool_item.save(obs, using_hash)
	 && cmt_item.save(obs, comment)
	 ))
    {
      carp("mootHMM::save(): could not save header%s%s\n",
	   (filename ? " to file " : ""), (filename ? filename : ""));
      return false;
    }

  return _bindump(obs, filename);
}

bool mootHMM::_bindump(mootio::mostream *obs, const char *filename)
{
  //-- variables
  Item<size_t> size_item;
  Item<TagID> tagid_item;
  Item<TokID> tokid_item;
  Item<ProbT> probt_item;
  Item<bool> bool_item;
  Item<LexClass> lclass_item;
  Item<TokIDTable> tokids_item;
  Item<TagIDTable> tagids_item;
  Item<ClassIDTable> classids_item;
  Item<LexProbTable> lexprobs_item;
  Item<LexClassProbTable> lcprobs_item;
#if defined(MOOT_USE_TRIGRAMS) && defined(MOOT_HASH_TRIGRAMS)
  Item<TrigramProbTable> trigrams_item;
#endif
  Item<SuffixTrie> trie_item;

  if (! (tagid_item.save(obs, start_tagid)
	 && probt_item.save(obs, unknown_lex_threshhold)
	 && probt_item.save(obs, nglambda1)
	 && probt_item.save(obs, nglambda2)
#ifdef MOOT_USE_TRIGRAMS
	 && probt_item.save(obs, nglambda3)
#endif
	 && probt_item.save(obs, wlambda0)
	 && probt_item.save(obs, wlambda1)

	 && bool_item.save(obs, use_lex_classes)
	 && lclass_item.save(obs, uclass)
	 && probt_item.save(obs, clambda0)
	 && probt_item.save(obs, clambda1)

	 && probt_item.save(obs, beamwd)

	 && tokids_item.save(obs, tokids)
	 && tagids_item.save(obs, tagids)
	 && classids_item.save(obs, classids)
	 && size_item.save(obs, n_tags)
	 && size_item.save(obs, n_toks)
	 && size_item.save(obs, n_classes)
	 && lexprobs_item.save(obs, lexprobs)
	 && lcprobs_item.save(obs, lcprobs)
#ifdef MOOT_USE_TRIGRAMS
# ifdef MOOT_HASH_TRIGRAMS
	 && trigrams_item.save(obs, ngprobs3)
# else
	 && probt_item.save_n(obs, ngprobs3, n_tags*n_tags*n_tags)
# endif
#else
	 && probt_item.save_n(obs, ngprobs2, n_tags*n_tags)
#endif //MOOT_USE_TRIGRAMS
	 ))
    {
      carp("mootHMM::save(): could not save data%s%s\n",
	   (filename ? " to file " : ""), (filename ? filename : ""));
      return false;
    }

  if (!trie_item.save(obs, suftrie))
    {
      carp("mootHMM::save(): could not save suffix trie%s%s\n",
	   (filename ? " to file " : ""), (filename ? filename : ""));
      return false;
    }

  int i;
  for (i = 0; i < NTokFlavors; i++) {
    if (!tokid_item.save(obs, flavids[i])) {
      carp("mootHMM::save(): could not save flavor data%s%s\n",
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
  //-- open file
#ifdef MOOT_ZLIB_ENABLED
  mootio::mizfstream
#else //-- !MOOT_ZLIB_ENABLED
  mootio::mifstream
#endif
    ifs(filename,"rb");

  if (!ifs.valid()) {
    carp("mootHMM::load(): open failed for \"%s\": %s\n",
	 filename, ifs.errmsg().c_str());
    return false;
  }

  //... and load
  bool rc = load(&ifs, filename);
  ifs.close();
  return rc;
}

bool mootHMM::load(mootio::mistream *ibs, const char *filename)
{
  clear(true,false); //-- make sure the object is totally empty

  HeaderInfo hi, hi_magic(string("mootHMM"));
  size_t     crc;
  bool       saved_uses_trigrams;
  bool       saved_hashes_trigrams;
#ifdef MOOT_USE_TRIGRAMS
  bool       i_use_trigrams = true;
# ifdef MOOT_HASH_TRIGRAMS
  bool       i_hash_trigrams = true;
# else
  bool       i_hash_trigrams = false;
# endif
#else
  bool       i_use_trigrams = false;
  bool       i_hash_trigrams = false;
#endif

  Item<HeaderInfo> hi_item;
  Item<size_t>     size_item;
  Item<bool>       bool_item;
  Item<string>     cmt_item;
  string comment;

  //-- load headers
  if (! (hi_item.load(ibs, hi)
	 && size_item.load(ibs, crc)
	 && bool_item.load(ibs, saved_uses_trigrams)
	 && bool_item.load(ibs, saved_hashes_trigrams)
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
  else if (saved_uses_trigrams != i_use_trigrams)
    {
      carp("mootHMM::load(): incompatible use_trigam flag (local=%u / saved=%u)%s%s\n",
	   i_use_trigrams, saved_uses_trigrams,
	   (filename ? " in file " : ""), (filename ? filename : ""));
    }
  else if (saved_hashes_trigrams != i_hash_trigrams)
    {
      carp("mootHMM::load(): incompatible hash_trigrams flag (local=%u / saved=%u)%s%s\n",
	   i_hash_trigrams, saved_hashes_trigrams,
	   (filename ? " in file " : ""), (filename ? filename : ""));
    }


  if(!_binload(ibs, filename))
    return false;

  if (crc != (start_tagid + n_tags + n_toks + n_classes + suftrie.size())) {
    carp("mootHMM::load(): checksum failed%s%s\n",
	 (filename ? " for file " : ""), (filename ? filename : ""));
  }

  viterbi_clear(); //-- (re-)initialize Viterbi table
  return true;
}


bool mootHMM::_binload(mootio::mistream *ibs, const char *filename)
{
  //-- variables
  Item<size_t> size_item;
  Item<TagID> tagid_item;
  Item<TokID> tokid_item;
  Item<ProbT> probt_item;
  Item<bool> bool_item;
  Item<LexClass> lclass_item;
  Item<TokIDTable> tokids_item;
  Item<TagIDTable> tagids_item;
  Item<ClassIDTable> classids_item;
  Item<LexProbTable> lexprobs_item;
  Item<LexClassProbTable> lcprobs_item;
  //Item<mootTokString>  tokstr_item;
#ifdef MOOT_USE_TRIGRAMS
# ifdef MOOT_HASH_TRIGRAMS
  Item<TrigramProbTable> trigrams_item;
# else
  size_t ngprobs3_size = 0;
# endif
#else
  size_t ngprobs2_size  = 0;
#endif
  Item<SuffixTrie> trie_item;


  if (! (tagid_item.load(ibs, start_tagid)
	 && probt_item.load(ibs, unknown_lex_threshhold)
	 && probt_item.load(ibs, nglambda1)
	 && probt_item.load(ibs, nglambda2)
#ifdef MOOT_USE_TRIGRAMS
	 && probt_item.load(ibs, nglambda3)
#endif
	 && probt_item.load(ibs, wlambda0)
	 && probt_item.load(ibs, wlambda1)

	 && bool_item.load(ibs, use_lex_classes)
	 && lclass_item.load(ibs, uclass)
	 && probt_item.load(ibs, clambda0)
	 && probt_item.load(ibs, clambda1)

	 && probt_item.load(ibs, beamwd)
	 ))
    {
      carp("mootHMM::load(): could not load smoothing constants%s%s\n",
	   (filename ? " from file " : ""), (filename ? filename : ""));
      return false;
    }

  if (!(tokids_item.load(ibs, tokids)
	&& tagids_item.load(ibs, tagids)
	&& classids_item.load(ibs, classids)))
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
#ifdef MOOT_USE_TRIGRAMS
# ifdef MOOT_HASH_TRIGRAMS
	 && trigrams_item.load(ibs, ngprobs3)
# else
	 && probt_item.load_n(ibs, ngprobs3, ngprobs3_size)
# endif
#else
	 && probt_item.load_n(ibs, ngprobs2, ngprobs2_size)
#endif // MOOT_USE_TRIGRAMS
	 ))
    {
      carp("mootHMM::load(): could not load table data%s%s\n",
	   (filename ? " from file " : ""), (filename ? filename : ""));
      return false;
    }

  if (!trie_item.load(ibs, suftrie))
    {
      carp("mootHMM::load(): could not load trie data%s%s\n",
	   (filename ? " from file " : ""), (filename ? filename : ""));
      return false;
    }

  int i;
  for (i = 0; i < NTokFlavors; i++) {
    if (!tokid_item.load(ibs, flavids[i])) {
      carp("mootHMM::save(): could not load flavor data%s%s\n",
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
