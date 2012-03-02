/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2012 by Bryan Jurish <moocow@cpan.org>

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
 * File: mootHMM.cc
 * Author: Bryan Jurish <moocow@cpan.org>
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
#include <mootModelSpec.h>
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
 * - (inlined)
 */

/*--------------------------------------------------------------------------
 * clear, freeing dynamic data
 *--------------------------------------------------------------------------*/

void mootHMM::clear(bool wipe_everything, bool unlogify)
{
  //-- iterator variables
  ViterbiColumn *col, *col_next;
  ViterbiRow    *row, *row_next;
  ViterbiNode   *nod, *nod_next;

  //-- free trellis: columns, rows, and nodes
  for (col = vtable; col != NULL; col = col_next) {
    col_next      = col->col_prev;
    for (row = col->rows; row != NULL; row = row_next)
      {
	row_next    = row->row_next;
	for (nod = row->nodes; nod != NULL; nod = nod_next)
	  {
	    nod_next  = nod->nod_next;
	    delete nod;
	  }
	delete row;
      }
    delete col;
  }
  vtable = NULL;

  //-- free trashed trellis cols
  for (col = trash_columns; col != NULL; col = col_next) {
    col_next = col->col_prev;
    delete col;
  }
  trash_columns = NULL;

  //-- free trashed trellis rows
  for (row = trash_rows; row != NULL; row = row_next) {
    row_next = row->row_next;
    delete row;
  }
  trash_rows = NULL;

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
  ngprobsh.clear();  //-- clear: hash
  if (ngprobsa) {    //-- clear: array
    free(ngprobsa);
    ngprobsa = NULL;
  }

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
    nglambda3 = exp(nglambda3);
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
    taster.clear();

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
  mootModelSpec ms(modelname);

  //-- sanity check
  if (ms.binfile.empty() && ms.lexfile.empty() && ms.ngfile.empty() && ms.lcfile.empty()) {
    carp("%s: Error: no model found for `%s'!\n", myname, modelname.c_str());
    return false;
  }

  //-- load model: binary
  if (!ms.binfile.empty()) {
    if (verbose >= vlProgress)
      carp("%s: loading binary HMM model file '%s'...", myname, ms.binfile.c_str());
    if (!load(ms.binfile.c_str())) {
      carp("\n%s: load FAILED for binary HMM model file '%s'\n", myname, ms.binfile.c_str());
      return false;
    }
    else if (verbose >= vlProgress) carp(" loaded.\n");
  }
  else {
    //-- load model: frequency data
    mootLexfreqs   lexfreqs(32767);
    mootClassfreqs classfreqs(512);
    mootNgrams     ngfreqs;
    mootTaster     mtaster; //-- default: built-in

    //-- load model: lexical frequencies
    if (!ms.lexfile.empty() && moot_file_exists(ms.lexfile)) {
      if (verbose >= vlProgress)
	carp("%s: loading lexical frequency file '%s'...", myname, ms.lexfile.c_str());

      if (!lexfreqs.load(ms.lexfile.c_str())) {
	carp("\n%s: load FAILED for lexical frequency file `%s'\n", myname, ms.lexfile.c_str());
	return false;
      }
      else if (verbose >= vlProgress) carp(" loaded.\n");
    }

    // -- load model: n-gram frequencies
    if (!ms.ngfile.empty() && moot_file_exists(ms.ngfile)) {
      if (verbose >= vlProgress)
	carp("%s: loading n-gram frequency file '%s'...", myname, ms.ngfile.c_str());

      if (!ngfreqs.load(ms.ngfile.c_str())) {
	carp("\n%s: load FAILED for n-gram frequency file `%s'\n", myname, ms.ngfile.c_str());
	return false;
      }
      else if (verbose >= vlProgress) carp(" loaded.\n");
    }

    // -- load model: class frequencies
    if (use_lex_classes && !ms.lcfile.empty() && moot_file_exists(ms.lcfile)) {
      if (verbose >= vlProgress)
	carp("%s: loading class frequency file '%s'...", myname, ms.lcfile.c_str());

      if (!classfreqs.load(ms.lcfile.c_str())) {
	carp("\n%s: load FAILED for class frequency file `%s'\n", myname, ms.lcfile.c_str());
	return false;
      }
      else if (verbose >= vlProgress) carp(" loaded.\n");
    }

    // -- load model: flavors
    if (use_flavors && !ms.flafile.empty() && moot_file_exists(ms.flafile)) {
      if (verbose >= vlProgress)
	carp("%s: loading flavor definition file '%s'...", myname, ms.flafile.c_str());

      if (!mtaster.load(ms.flafile)) {
	carp("\n%s: load FAILED for flavor definition file `%s'\n", myname, ms.flafile.c_str());
	return false;
      }
      else if (verbose >= vlProgress) carp(" loaded.\n");
    }
    else if (use_flavors) {
      if (verbose >= vlProgress) carp("%s: using built-in flavor definitions\n", myname);
      //mtaster.set_default_rules(); //-- default
    }
    else {
      //-- flavors disabled
      if (verbose >= vlProgress) carp("%s: disabling token flavors\n", myname);
      mtaster.clear();
    }

    //-- compile HMM
    if (verbose >= vlProgress) carp("%s: compiling HMM...", myname);

    //-- ensure taster-flavors are computed for lexfreqs
    //   + this has to happen here rather than in compile(), since compile() params (e.g. lexfreqs) are const
    lexfreqs.unknown_threshhold = unknown_lex_threshhold;
    lexfreqs.compute_specials((use_flavors ? &mtaster : NULL), true);

    //-- compile guts (virtualized)
    if (!this->compile(lexfreqs,ngfreqs,classfreqs,start_tag_str,mtaster)) {
      carp("\n%s: HMM compilation FAILED\n", myname);
      return false;
    }
    else if (verbose >= vlProgress) carp(" compiled.\n");

    //-- check whether to use classes
    if (lcprobs.size() <= 2 && use_lex_classes) {
      use_lex_classes = false;
      moot_msg(verbose, vlWarnings, "%s: Warning: no class frequencies available: disabling lexical classes!\n", myname);
    }

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

#ifdef MOOT_ENABLE_SUFFIX_TRIE
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
#endif //--MOOT_ENABLE_SUFFIX_TRIE

    //-- compute log-probabilities
    if (do_compute_logprobs) {
      if (verbose >= vlProgress)
	carp("%s: computing log-probabilities [hash_ngrams=%d]...", myname, static_cast<int>(hash_ngrams));
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
		      const mootTagString &start_tag_str,
		      const mootTaster &mtaster)
{
  //--------------------------------------
  // compile: preliminaries

  //-- sanity check
  if (ngrams.ugtotal <= 0) {
    carp("mootHMM::compile(): Error: bad unigram total in 'ngrams' - looks like a bogus model!\n");
    return false;
  }

  //-- setup flavors
  taster = mtaster;

  //-- assign IDs
  assign_ids_fl();
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

  //-- allocate: n-gram arrays (if requested)
  if (!hash_ngrams) {
    size_t nga_bytes = sizeof(ProbT) * n_tags * n_tags * n_tags; //-- trigrams
    ngprobsa = reinterpret_cast<ProbT *>(malloc(nga_bytes));
    if (!ngprobsa) {
      carp("mootHMM::compile(): Error: could not allocate dense n-gram table of %lu bytes.\n", static_cast<long unsigned>(nga_bytes));
      return false;
    }
    memset(ngprobsa, 0, nga_bytes);
  }
  ngprobsh.clear();

  //--------------------------------------
  // compile: common variables
  TokID                       tokid;          //-- current token-ID
  FlavorID                    flavid;         //-- current token-flavor-ID
  LexClass                    lclass;         //-- current lexical class
  ClassID                     classid;        //-- current lexical class-ID
  TagID                       tagid;          //-- current tag-ID
  TagID                       tagid2;         //-- next tag-ID (for bigrams)
  TagID                       tagid3;         //-- next-next tag-ID (for trigrams)
  /*-- v2.0.9-1: disabled special handling of "unknown" token and class
  mootLexfreqs::LexfreqCount unTotal = 0 ;    //-- total "unknown" token count
  LexProbSubTable            &untagcts        //-- "unknown" tag counts (later, probabilites)
      = lexprobs[0];
  LexClassProbSubTable       &unctagcts       //-- "unknown" class-class counts (later, probabilites)
      = lcprobs[0];
  */

  //--------------------------------------
  // compile: lexfreqs

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
      tokid  = token2id(tokstr);  //-- token or flavor id
      flavid = taster.flavor_id(tokstr);

      //-- ... for all tags occurring with this token(lftagi)
      for (mootLexfreqs::LexfreqSubtable::const_iterator lftagi = entry.freqs.begin(); lftagi != entry.freqs.end(); ++lftagi) {
	const mootTagString &tagstr = lftagi->first;
	const mootLexfreqs::LexfreqCount tagcount = lftagi->second;
	const mootLexfreqs::LexfreqCount tagtotal = lexfreqs.f_tag(tagstr);
	
	//-- sanity check
	if (tagtotal == 0) continue;
	
	//-- get tag-ID
	tagid  = tagids.name2id(tagstr);
	
#if 0
	//-- "unknown" token check (disabled, v2.0.9-1)
	if (flavid == taster.noid && toktotal <= unknown_lex_threshhold) { //-- dubious
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
#endif
	if (tokid != 0) {
	  //-- it's a kosher token (too?): compute lexical probability: p(tok|tag)
	  lexprobs[tokid][tagid] = tagcount / tagtotal;
	}
      }
    }

  if (lexprobs.size() == 0) lexprobs.resize(1); //-- ensure at least a lexical entry for the "unknown" token
#if 0
  //-- Normalize "unknown" lexical probabilities (disabled, v2.0.9-1)
  for (LexProbSubTable::iterator lpsi = untagcts.begin(); lpsi != untagcts.end(); ++lpsi) {
    if (lpsi->second == 0) continue;
    const mootTagString &tagstr = tagids.id2name(lpsi->first);
    const mootLexfreqs::LexfreqCount tagtotal = lexfreqs.taglookup(tagstr);
    lpsi->second /= tagtotal;
  }
#endif

  //--------------------------------------
  // compile: lexical classes

  if (use_lex_classes) {
    //-- compile class probabilities : for all stringy classes (lcti ~ lclass ~ classid)
    for (mootClassfreqs::ClassfreqTable::const_iterator lcti = classfreqs.lctable.begin(); lcti != classfreqs.lctable.end(); ++lcti) {
      const mootTagSet &tagset = lcti->first;
      const mootClassfreqs::ClassfreqEntry &entry = lcti->second;
      //const CountT classtotal = entry.count;

      //-- get class id
      lclass.clear();
      tagset2lexclass(tagset,&lclass,false);
      classid = classids.name2id(lclass);

      //-- check for empty/unknown class
      if (lclass.empty()) classid = 0;

      //-- ... for all tags assigned to this class (lctagi)
      for (mootClassfreqs::ClassfreqSubtable::const_iterator lctagi=entry.freqs.begin(); lctagi != entry.freqs.end(); ++lctagi) {
	const mootTagString &ctagstr   = lctagi->first;
	const CountT         ctagcount = lctagi->second;
	const CountT         ctagtotal = classfreqs.taglookup(ctagstr);
	  
	//-- sanity check
	if (ctagtotal == 0) continue;
  
	//-- get tag-ID
	tagid  = tagids.name2id(ctagstr);

#if 0
	//-- unknown class check (v2.0.9-1: disabled)
	if ( !uclass.empty() && (classtotal <= unknown_class_threshhold || lclass.empty()) ) {
	  //-- "unknown" class: just store the raw counts for now
	  LexClassProbSubTable::iterator lcpsi = unctagcts.find(tagid);
	  if (lcpsi == unctagcts.end()) {
	    unctagcts[tagid] = ctagcount;
	  } else {
	    lcpsi->second += ctagcount;
	  }
	}
#endif
	if (classid != 0) {
	  //-- it's a kosher class (too?): compute class probability: p(class|tag)
	  lcprobs[classid][tagid] = ctagcount / ctagtotal;
	}
      }
    }

#if 0
    //--------------------------------------
    // compile: lexical classes: unknown (v2.0.9-1: disabled)

    //-- Normalize "unknown" class probabilities
    if (!unctagcts.empty()) {
      for (LexClassProbSubTable::iterator lcpsi = unctagcts.begin(); lcpsi != unctagcts.end(); ++lcpsi) {
	const mootTagString   &tagstr = tagids.id2name(lcpsi->first);
	const CountT        ctagtotal = classfreqs.taglookup(tagstr);
	lcpsi->second                /= ctagtotal;
      }
    }
#endif
  }

  //--------------------------------------
  // compile: n-grams

  //-- Compute ngram probabilites
  ProbT ugtotal = ngrams.ugtotal - ngrams.lookup(start_tag_str);
  ProbT f;

  for (mootNgrams::NgramTable::const_iterator ngi1 = ngrams.ngtable.begin();
       ngi1 != ngrams.ngtable.end();
       ngi1++)
    {
      //-- look at unigrams first : get ID
      tagid = tagids.name2id(ngi1->first);
      f     = ngi1->second.count;

      //-- compute unigram probability, storing as '0 0 $tagid'
      set_ngram_prob((ngi1->second.count / ugtotal), 0,0,tagid);

      //-- ignore zero probabilities
      if (ngi1->second.count == 0) continue;

      //-- next, look at bigrams
      for (mootNgrams::BigramTable::const_iterator ngi2 = ngi1->second.freqs.begin();
	   ngi2 != ngi1->second.freqs.end();
	   ngi2++)
	{
	  //-- get ID
	  tagid2 = tagids.name2id(ngi2->first);
	  f      = ngi2->second.count;

	  //-- compute bigram probability, storing as '0 $tagid1 $tagid2'
	  set_ngram_prob((ngi2->second.count / ngi1->second.count), 0,tagid,tagid2);

	  //-- look at trigrams now
	  for (mootNgrams::TrigramTable::const_iterator ngi3 = ngi2->second.freqs.begin();
	       ngi3 != ngi2->second.freqs.end();
	       ngi3++)
	    {
	      //-- get ID
	      tagid3 = tagids.name2id(ngi3->first);
	      f      = ngi3->second;

	      //-- compute trigram probability, storing as '$tagid1 $tagid2 $tagid3'
	      set_ngram_prob((ngi3->second / ngi2->second.count), tagid,tagid2,tagid3);
	    }
	}
    }

  //--------------------------------------
  // compile: cleanup

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

//----------------------------------------------------------------------
void mootHMM::assign_ids_fl(void)
{
  //-- add flavor-ids (except for default flavor)
  taster.noid = 0;
  for (mootTaster::Rules::iterator ri=taster.rules.begin(); ri!=taster.rules.end(); ++ri) {
    ri->id = (ri->lab == taster.nolabel) ? taster.noid : tokids.get_id(ri->lab);
  }
}

//----------------------------------------------------------------------
void mootHMM::assign_ids_lf(const mootLexfreqs &lexfreqs)
{
  //-- add flavor-ids
  //taster = lexfreqs.taster; //???
  taster.noid = 0;
  for (mootTaster::Rules::iterator fri=taster.rules.begin(); fri!=taster.rules.end(); ++fri) {
    fri->id = (fri->lab == taster.nolabel) ? taster.noid : tokids.get_id(fri->lab);
  }

  //-- compile lexical IDs
  for (mootLexfreqs::LexfreqTokTable::const_iterator lfti = lexfreqs.lftable.begin();
       lfti != lexfreqs.lftable.end();
       lfti++)
    {
      const mootTokString &tokstr = lfti->first;
      const mootLexfreqs::LexfreqEntry &entry = lfti->second;
#   ifndef MOOT_LEX_NONALPHA
      mootFlavorID flavid = taster.flavor_id(tokstr);
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
	  if (flavid != taster.noid)
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

//----------------------------------------------------------------------
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

//----------------------------------------------------------------------
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
  ProbT   f_t123;                     //-- current  trigram  count: f(t1,t2,t3)
  ProbT   f_t23;                      //-- current  bigram   count: f(t2,t3)
  ProbT   f_t12;                      //-- previous bigram   count: f(t1,t2)
  ProbT   f_t3;                       //-- current  unigram  count: f(t3)
  ProbT   f_t2;                       //-- previous unigram  count: f(t2)
  ProbT   f_t1;                       //-- pprevious unigram count: f(t1)
  ProbT   f_N;                        //-- corpus size (unigram total)

  ProbT ngp123;  //-- 3grams: adjusted trigram probability : (f(t1,t2,t3) - 1) / (f(t1,t2)    - 1)
  ProbT ngp23;   //-- 3grams: adjusted bigram probability  : (f(t2,t3)    - 1) / (f(t2)       - 1)
  ProbT ngp3;    //-- 3grams: adjusted unigram probability : (f(t3)       - 1) / (corpus_size - 1)

  ProbT ngp12;   //-- 2grams: adjusted bigram probability  : (f(t1,t2)    - 1) / (f(t1)       - 1)
  ProbT ngp2;    //-- 2grams: adjusted unigram probability : (f(t2)       - 1) / (corpus_size - 1)

  //-- initialize
  //f_N       = ngrams.ugtotal;
  f_N       = ngrams.ugtotal - ngrams.lookup(tagids.id2name(start_tagid));
  if (f_N <= 0) f_N = 1.0; //-- sanitize
  nglambda1 = 0.0;
  nglambda2 = 0.0;
  nglambda3 = 0.0;

  //---- get best-guess counts: (n>=1)-grams
  for (mootNgrams::NgramTable::const_iterator ngi1 = ngrams.ngtable.begin();
       ngi1 != ngrams.ngtable.end();
       ngi1++)
    {
      f_t1 = ngi1->second.count;

      if (ngi1->second.freqs.empty()) {
	//---- best-guess counts: 1-gram only
	nglambda1 += f_t1;
      }
      else {
	//---- get best-guess counts: (n>=2)-grams
	for (mootNgrams::BigramTable::const_iterator ngi2 = ngi1->second.freqs.begin();
	     ngi2 != ngi1->second.freqs.end();
	     ngi2++)
	  {
	    //-- previous bigram count: f(t1,t2)
	    f_t12 = ngi2->second.count;

	    //-- previous unigram count : f(t2)
	    f_t2 = ngrams.lookup(ngi2->first);

	    if (ngi2->second.freqs.empty()) {
	      //---- best-guess counts: 1- and 2-grams only

	      //-- compute adjusted probabilities
	      ngp12  =  f_t1  == 1  ?  0  : (f_t12  - 1.0) / (f_t1  - 1.0);
	      ngp2   =  f_N   == 1  ?  0  : (f_t2   - 1.0) / (f_N   - 1.0);

	      //-- adjust lambdas
	      if (ngp12 >= ngp2)
		nglambda2 += f_t12;
	      else
		nglambda1 += f_t12;
	    }
	    else {
	      //---- best-guess counts: 1-, 2-, and 3-grams
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
		  ngp23  =  f_t2  == 1  ?  0  : (f_t23  - 1.0) / (f_t2  - 1.0);
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
      }
    }

  //-- sanity check
  if (nglambda3==0 && nglambda2==0 && nglambda1==0) {
    carp("mootHMM::estimate_lambdas(): WARNING: all pre-normalization smoothing counts are zero; using n-gram length\n");
    nglambda1 = 1.0;
    nglambda2 = 2.0;
    nglambda3 = 3.0;
  }

  //-- normalize lambdas
  ProbT nglambda_total = nglambda1 + nglambda2 + nglambda3;
  nglambda3 /= nglambda_total;
  nglambda2 /= nglambda_total;
  nglambda1 /= nglambda_total;

  return true;
}


/*--------------------------------------------------------------------------
 * Compilation utilities: smoothing constant estimation : lexical
 *--------------------------------------------------------------------------*/
bool mootHMM::estimate_wlambdas(const mootLexfreqs &lf)
{
  //-- estimate lexical smoothing constants
  if (lf.n_tokens > 0) {
    wlambda0 = 0.5 / static_cast<ProbT>(lf.n_tokens);
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
    clambda0 = 0.5 / static_cast<ProbT>(cf.totalcount);
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
  if (!hash_ngrams) {
    ProbT   p3=0, p23=0, p=0;
    TagID   tag1=0, tag2=0, tag3=0;

    //-- trigram probabilities: stored as '$tagid1,$tagid2,$tagid3'
    for (tag3 = 1; tag3 < n_tags; tag3++) {
      p3 = tagp(tag3);
      for (tag2 = 1; tag2 < n_tags; tag2++) {
	p23 = tagp(tag2,tag3);
	for (tag1 = 1; tag1 < n_tags; tag1++) {
	  //-- trigram probabilities: stored as '$tagid1,$tagid2,$tagid3'
	  p = ( (nglambda1 * p3)
		+ (nglambda2 * p23)
		+ (nglambda3 * tagp(tag1,tag2,tag3)) );
	  p = (p == 0.0 ? MOOT_PROB_ZERO : log(p));
	  ngprobsa[(n_tags*((n_tags*tag1)+tag2))+tag3] = p;
	}
	//-- bigram probabilities: stored as '0,$tagid2,$tagid3'
	p = ( (nglambda1 * p3)
	      + (nglambda2 * p23) );
	p = (p == 0.0 ? MOOT_PROB_ZERO : log(p));
	ngprobsa[(n_tags*tag2)+tag3] = p;
      }
      //-- unigram probabilities: stored as '0,0,$tagid3'
      p = nglambda1 * p3;
      p = (p == 0.0 ? MOOT_PROB_ZERO : log(p));
      ngprobsa[tag3] = p;
    }
    //-- UNKNOWN unigram probability: stored as '0,0,0'
    p = nglambda1 * tagp(0);
    p = (p == 0.0 ? MOOT_PROB_ZERO : log(p));
    ngprobsa[0] = p;

  } else {                // +hash
    ProbT p = 0;

    //-- compute log probabilities: trigrams: <t1,t2,t3> -> l3*p(t3|t1,t2) + l2*p(t3|t2) + l1*p(t3)
    for (NgramProbHash::iterator ngpi = ngprobsh.begin(); ngpi != ngprobsh.end(); ngpi++) {
      const NgramProbKey &ngram = ngpi->first;
      if (ngram.tag1 == 0 || ngram.tag2 == 0) continue;
      p    = ( (nglambda1   * tagp(ngram.tag3))
	       + (nglambda2 * tagp(ngram.tag2,ngram.tag3))
	       + (nglambda3 * ngpi->second) );
      ngpi->second = (p == 0.0 ? MOOT_PROB_ZERO : log(p));
    }

    //-- compute log probabilities: bigrams: <0,t2,t3> -> l2*p(t3|t2) + l1*p(t3)
    for (NgramProbHash::iterator ngpi = ngprobsh.begin(); ngpi != ngprobsh.end(); ngpi++) {
      const NgramProbKey &ngram = ngpi->first;
      if (ngram.tag1 != 0 || ngram.tag2 == 0) continue;
      p    = ( (nglambda1   * tagp(ngram.tag3))
	       + (nglambda2 * ngpi->second) );
      ngpi->second = (p == 0.0 ? MOOT_PROB_ZERO : log(p));
    }

    //-- compute log probabilities: unigrams: <0,0,t3> -> l1*p(t3)
    for (NgramProbHash::iterator ngpi = ngprobsh.begin(); ngpi != ngprobsh.end(); ngpi++) {
      const NgramProbKey &ngram = ngpi->first;
      if (ngram.tag1 != 0 || ngram.tag2 != 0) continue;
      p    = ( (nglambda1   * ngpi->second) );
      ngpi->second = (p == 0.0 ? MOOT_PROB_ZERO : log(p));
    }
  }

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

#ifdef MOOT_ENABLE_SUFFIX_TRIE
  //-- suffix-trie probabilities
  for (SuffixTrie::iterator sti = suftrie.begin(); sti != suftrie.end(); sti++) {
    for (SuffixTrieDataT::iterator stdi = sti->data.begin(); stdi != sti->data.end(); stdi++) {
      stdi->second = log(stdi->second);
    }
# ifdef LEX_SORT_BYVALUE
    //-- sort it
    sti->data.sort_byvalue();
# endif //-- LEX_SORT_BY_VALUE
  }
#endif //-- MOOT_ENABLE_SUFFIX_TRIE

  //-- smoothing constants
  nglambda1 = log(nglambda1);
  nglambda2 = log(nglambda2);
  nglambda3 = log(nglambda3);
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
  ViterbiRow     *row, *row_next;
  ViterbiNode    *nod, *nod_next;
  for (col = vtable; col != NULL; col = col_next) {
    col_next      = col->col_prev;
    col->col_prev = trash_columns;
    trash_columns = col;
    for (row = col->rows; row != NULL; row = row_next) {
      row_next      = row->row_next;
      row->row_next = trash_rows;
      trash_rows    = row;
      for (nod = row->nodes; nod != NULL; nod = nod_next) {
	nod_next      = nod->nod_next;
	nod->nod_next = trash_nodes;
	trash_nodes   = nod;
      }
    }
  }
  //viterbi_clear_bestpath();

  //-- add BOS entry
  vtable             = viterbi_get_column();
  vtable->col_prev   = NULL;

  //-- BOS: initialize beam-pruning stuff
  vtable->bbestpr    = MOOT_PROB_ONE;
  vtable->bpprmin    = MOOT_PROB_NEG;

  row = vtable->rows = viterbi_get_row();
  row->tagid         = start_tagid;
  row->row_next      = NULL;
  row->wprob         = MOOT_PROB_ONE;

  nod = row->nodes   = viterbi_get_node();
  nod->tagid         = start_tagid;
  nod->ptagid        = start_tagid;
  nod->lprob         = MOOT_PROB_ONE;
  //nod->row           = row;

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
#ifndef MOOT_ENABLE_SUFFIX_TRIE
  const LexProbSubTable *lps = &(lexprobs[tokid]);
#else
  const LexProbSubTable *lps;
  if (tokid != 0) {
    lps = &(lexprobs[tokid]);
  } else {
    size_t matchlen;
    lps = &(suftrie.sufprobs(toktext,&matchlen));
# ifdef NO_SUFFIX_USE_HAPAX
    if (!matchlen) lps = &lexprobs[tokid];
# endif //-- NO_SUFFIX_USE_HAPAX
  }
#endif //-- MOOT_ENABLE_SUFFIX_TRIE

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
#ifndef MOOT_ENABLE_SUFFIX_TRIE
    lps = &(lcprobs[classid]);
#else
    if (classid != 0) {
      lps   = &(lcprobs[classid]);
    } else {
      size_t matchlen;
      lps = &(suftrie.sufprobs(toktext,&matchlen));
# ifdef NO_SUFFIX_USE_HAPAX
      if (!matchlen) lps = &(lcprobs[classid]);
# endif //-- NO_SUFFIX_USE_HAPAX
    }
#endif //-- MOOT_ENABLE_SUFFIX_TRIE
  }
  else {
#ifdef MOOT_ENABLE_SUFFIX_TRIE
    size_t matchlen;
    lps = &(suftrie.sufprobs(toktext,&matchlen));
# ifdef NO_SUFFIX_USE_HAPAX
    if (!matchlen) lps = &(lexprobs[0]);
# endif
#else //-- NO_SUFFIX_USE_HAPAX
    lps = &(lexprobs[0]);
#endif //-- MOOT_ENABLE_SUFFIX_TRIE
    wclambda0 = wlambda0;
  }

  //-- Get next column
  ViterbiColumn *col = NULL;

  //-- Update beam-pruning variables
  //bpprmin = vtable->bbestpr - beamwd;
  //bbestpr = MOOT_PROB_NEG;

  //-- for each possible destination tag 'vtagid'
  if (relax) {
    /*
     * RELAX: Iterate over actual probability-table entries:
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
  } else {
    /*
     * NORELAX: Iterate over actual actual class specified:
     *
     * This is about 2K tok/sec slower than the above, and gives 3.9% more
     * errors (96.52% vs. 96.66% correct), but retains almost-mandatory
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
  }
  //--/relax

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
	sri->tok_analyses.push_back(mootToken::Analysis(taster.flavor(sri->text())));
      }

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

      if (save_mark_unknown && tokids.name2id(sri->text()) == 0) {
	sri->tok_analyses.push_back(mootToken::Analysis("*","*"));
      }
    }
  }

};

/*--------------------------------------------------------------------------
 * Trace: in-sentence Viterbi trace
 */
void mootHMM::tag_dump_trace(mootSentence &sentence)
{
  ViterbiColumn *vcol;
  std::list<ViterbiColumn*> vcols;
  mootSentence s; //-- output temporary

  //-- get column list in sentence order
  for (vcol = vtable; vcol != NULL; vcol=vcol->col_prev) {
    vcols.push_front(vcol);
  }
  if (!vcols.empty()) vcols.pop_front();   //-- omit BOS
  //if (!vcols.empty()) vcols.pop_back();  //-- omit EOS (no!)


  //-- traverse trellis columns and sentence in parallel
  mootSentence::const_iterator         si=sentence.begin();
  std::list<ViterbiColumn*>::iterator vci=vcols.begin();
  const mootToken *tokp;
  mootToken eostok(tagids.id2name(start_tagid));

  //-- iterate: sentence tokens ~ Viterbi columns 
  while (vci != vcols.end()) {
    vcol = *vci;
    if (si != sentence.end()) {
      s.push_back(*si);
      if (si->toktype() != TokTypeVanilla) { //-- ignore non-vanilla tokens
	si++;
	continue;
      }
      tokp = &(*si);
    } else {
      tokp = &eostok;
    }
    TokID tokid = token2id(tokp->text());
    mootTokenFlavor flav = tokenFlavor(tokp->text());
 
    //-- comment: token
    sentence_printf_append(s, TokTypeComment,
			   "moot:trace\tWORD %d:%s\tflavor=(%d:%s) bpprmin=%g bbestpr=%g",
			   tokid, (tokp==&eostok ? tokp->text().c_str() : tokids.id2name(tokid).c_str()),
			   flav, mootTokenFlavorNames[flav],
			   vcol->bpprmin, vcol->bbestpr);

    //-- iterate: Viterbi rows (current tags)
    for (ViterbiRow *vrow=vcol->rows; vrow != NULL; vrow=vrow->row_next) {

      //-- iteratate: Viterbi nodes (previous tags)
      for (ViterbiNode *vnod=vrow->nodes; vnod != NULL; vnod = vnod->nod_next) {
	ViterbiNode *vpnod = vnod->pth_prev;
	sentence_printf_append(s, TokTypeComment, "moot:trace\t%cNODE %d:%s\t%d:%s\t%d:%s\twprob=%g lprob=%g",
			       (vnod->lprob==vcol->bbestpr ? '*' : ' '),
			       vpnod->ptagid,  tagids.id2name(vpnod->ptagid).c_str(),
			       vnod->ptagid, tagids.id2name(vnod->ptagid).c_str(),
			       vrow->tagid,  tagids.id2name(vrow->tagid).c_str(),
			       vrow->wprob, vnod->lprob
			       );

	//-- iteratate: tag ids (next tags)
	for (TagID nxtid=0; nxtid < tagids.size(); nxtid++) {
	  ProbT vnxtpr = tagp(vnod->ptagid, vrow->tagid, nxtid);
	  sentence_printf_append(s, TokTypeComment, "moot:trace\t  NEXT %d:%s\t%d:%s\t%d:%s\tntxtprob=%g",
				 vnod->ptagid, tagids.id2name(vnod->ptagid).c_str(),
				 vrow->tagid,  tagids.id2name(vrow->tagid).c_str(),
				 nxtid,        tagids.id2name(nxtid).c_str(),
				 vnxtpr
				 );
	}
      }
    }

    //-- increment iterator(s)
    si++;
    vci++;
  }

  //-- swap input and output sentences
  sentence.swap(s);
}


/*--------------------------------------------------------------------------
 * Debug / HMM Dump
 *--------------------------------------------------------------------------*/
void mootHMM::txtdump(FILE *file, bool dump_constants, bool dump_lexprobs, bool dump_classprobs, bool dump_suftrie, bool dump_ngprobs)
{
  fprintf(file, "%%%% mootHMM text dump\n");

  if (dump_constants) {
    fprintf(file, "\n");
    fprintf(file, "%%%%-----------------------------------------------------\n");
    fprintf(file, "%%%% Constants\n");
    fprintf(file, "%%%%-----------------------------------------------------\n");
    fprintf(file, "start_tagid\t%u(\"%s\")\n", start_tagid, tagids.id2name(start_tagid).c_str());
    fprintf(file, "nglambda1\t%e (=%e)\n", nglambda1, exp(nglambda1));
    fprintf(file, "nglambda2\t%e (=%e)\n", nglambda2, exp(nglambda2));
    fprintf(file, "nglambda3\t%e (=%e)\n", nglambda3, exp(nglambda3));
    fprintf(file, "wlambda0\t%e (=%e)\n", wlambda0, exp(wlambda0));
    fprintf(file, "wlambda1\t%e (=%e)\n", wlambda1, exp(wlambda1));
    //
    fprintf(file, "clambda0\t%e (=%e)\n", clambda0, exp(clambda0));
    fprintf(file, "clambda1\t%e (=%e)\n", clambda1, exp(clambda1));
    //
    fprintf(file, "hash_ngrams\t%d\n", hash_ngrams ? 1 : 0);
    fprintf(file, "relax\t%d\n", relax ? 1 : 0);
    fprintf(file, "use_lex_classes\t%d\n", use_lex_classes ? 1 : 0);
    fprintf(file, "use_flavors\t%d\n", use_flavors ? 1 : 0);
    //
    fprintf(file, "beamwd\t%e (=%e)\n", beamwd, exp(beamwd));
    //
    fputs("uclass\t", file);
    for (LexClass::const_iterator lci = uclass.begin();  lci != uclass.end(); lci++) {
      if (lci!=uclass.begin()) fputc(' ', file);
      fputs(tagids.id2name(*lci).c_str(), file);
    }
    fputc('\n', file);

    //-- dump: flavors
    fprintf(file, "\n");
    fprintf(file, "%%%%-----------------------------------------------------\n");
    fprintf(file, "%%%% Flavors (LC_CTYPE=%s)\n", moot_lc_ctype());
    fprintf(file, "%%%% Id\tLabel\tRegex\n");
    fprintf(file, "%%%%-----------------------------------------------------\n");
    for (mootTaster::Rules::const_iterator ri=taster.rules.begin(); ri!=taster.rules.end(); ++ri) {
      fprintf(file, "%u\t%s\t%s\n", ri->id, ri->lab.c_str(), ri->re_s.c_str());
    }
    fprintf(file, "%u\t%s\n", taster.noid, taster.nolabel.c_str());
  }
  //--/dump_constants

  //-- common variables
  TokID tokid;
  TagID tagid, ptagid;
  ProbT prob;
 

  if (dump_lexprobs) {
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
  }
  //--/dump_lexprobs

  if (dump_classprobs) {
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
  }
  //--/dump_classprobs

  if (dump_suftrie) {
    fprintf(file, "\n");
    fprintf(file, "%%%%-----------------------------------------------------\n");
    fprintf(file, "%%%% Suffix Trie\n");
    fprintf(file, "%%%% maxlen=%zd ; theta=%e\n", suftrie.maxlen(), suftrie.theta);
    fprintf(file, "%%%% \"Suffix\"\tTagID(\"TagStr\")\tlog(p(Suffix|TagID))\tp\n");
    fprintf(file, "%%%%-----------------------------------------------------\n");
#ifdef MOOT_ENABLE_SUFFIX_TRIE
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
#else
    fprintf(file, "%%%% DISABLED (re-compile with MOOT_ENABLE_SUFFIX_TRIE defined to enable)\n");
#endif //-- MOOT_ENABLE_SUFFIX_TRIE
  }
  //--/dump_suftrie

  if (dump_ngprobs) {
    fprintf(file, "\n");
    fprintf(file, "%%%%-----------------------------------------------------\n");
    fprintf(file, "%%%% N-gram (Uni-, Bi-, and Trigram) Probabilities\n");
    fprintf(file, "%%%%  + n_tags = %u\n", n_tags);
    fprintf(file, "%%%%  + tagids_size = %u\n", tagids.size());
    fprintf(file, "%%%%  + hash_size = %u\n", ngprobsh.size());
    fprintf(file, "%%%%  + array_size = %u\n", (hash_ngrams ? 0 : (n_tags*n_tags*n_tags)));
    fprintf(file, "%%%% Tag1Id(\"Tag1Str\")\tTag2Id(\"Tag2Str\")\tTag3Id(\"Tag3Str\")\tlog(p(Tag3|Tag1,Tag2))\tp\n");
    fprintf(file, "%%%%-----------------------------------------------------\n");
    TagID pptagid;

    if ( (hash_ngrams && !ngprobsh.empty()) || (!hash_ngrams && ngprobsa!=NULL) ) {
      if (!hash_ngrams) {
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
      } else { //-- if (hash_ngrams)
	for (NgramProbHash::const_iterator ngi = ngprobsh.begin(); ngi != ngprobsh.end(); ngi++) {
	  prob    = ngi->second;
	  pptagid = ngi->first.tag1;
	  ptagid  = ngi->first.tag2;
	  tagid   = ngi->first.tag3;
	  fprintf(file, "%u(\"%s\")\t%u(\"%s\")\t%u(\"%s\")\t%e\t%e\n",
		  pptagid,  tagids.id2name(pptagid).c_str(),
		  ptagid,  tagids.id2name(ptagid).c_str(),
		  tagid, tagids.id2name(tagid).c_str(),
		  prob,
		  exp(prob));
	}
      }
    } else {
      fprintf(file, "%%%% (NULL | empty)\n");
    }
  }
  //-- /dump_ngprobs

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

  w->put_comment_block_end();
}

/*--------------------------------------------------------------------------
 * Binary I/O: save
 *--------------------------------------------------------------------------*/

/* v??? .. ???
const HeaderInfo::VersionT BINCOMPAT_MIN_VER = 1;
const HeaderInfo::VersionT BINCOMPAT_MIN_REV = 3;
const HeaderInfo::VersionT BINCOMPAT_VER = 1;
const HeaderInfo::VersionT BINCOMPAT_REV = 3;
*/
/* v2.?? .. v2.0.7-0 
const HeaderInfo::VersionT BINCOMPAT_MIN_VER = 2;
const HeaderInfo::VersionT BINCOMPAT_MIN_REV = 5;
const HeaderInfo::VersionT BINCOMPAT_VER = 2;
const HeaderInfo::VersionT BINCOMPAT_REV = 5;
*/
/* v2.0.7-0 .. 2.0.8-5
const HeaderInfo::VersionT BINCOMPAT_VER = 3;          //-- we save files as BINCOMPAT_$(VER.REV)
const HeaderInfo::VersionT BINCOMPAT_REV = 0;
const HeaderInfo::VersionT BINCOMPAT_MIN_VER_SAVE = 3; //-- our files can be loaded by libs >= BINCOMPAT_MIN_VER_SAVE_$(VER.REV)
const HeaderInfo::VersionT BINCOMPAT_MIN_REV_SAVE = 0;
const HeaderInfo::VersionT BINCOMPAT_MIN_VER_LOAD = 2; //-- we can load files >= BINCOMPAT_MIN_VER_LOAD_$(VER.REV)
const HeaderInfo::VersionT BINCOMPAT_MIN_REV_LOAD = 5;
*/
/* v2.0.9-0
const HeaderInfo::VersionT BINCOMPAT_VER = 3;          //-- we save files as BINCOMPAT_$(VER.REV)
const HeaderInfo::VersionT BINCOMPAT_REV = 1;
#if MOOT_32BIT_FORCE
const HeaderInfo::VersionT BINCOMPAT_MIN_VER_SAVE = 3; //-- 32bit: our files can be loaded by libs >= BINCOMPAT_MIN_VER_SAVE_$(VER.REV)
const HeaderInfo::VersionT BINCOMPAT_MIN_REV_SAVE = 1;
const HeaderInfo::VersionT BINCOMPAT_MIN_VER_LOAD = 3; //-- 32bit: we can load files >= BINCOMPAT_MIN_VER_LOAD_$(VER.REV)
const HeaderInfo::VersionT BINCOMPAT_MIN_REV_LOAD = 1;
#else
const HeaderInfo::VersionT BINCOMPAT_MIN_VER_SAVE = 3; //-- native: our files can be loaded by libs >= BINCOMPAT_MIN_VER_SAVE_$(VER.REV)
const HeaderInfo::VersionT BINCOMPAT_MIN_REV_SAVE = 0;
const HeaderInfo::VersionT BINCOMPAT_MIN_VER_LOAD = 2; //-- native: we can load files >= BINCOMPAT_MIN_VER_LOAD_$(VER.REV)
const HeaderInfo::VersionT BINCOMPAT_MIN_REV_LOAD = 5;
#endif
*/
/* v2.0.9-1 .. CURRENT */
const HeaderInfo::VersionT BINCOMPAT_VER = 3;          //-- we save files as BINCOMPAT_$(VER.REV)
const HeaderInfo::VersionT BINCOMPAT_REV = 2;
#if MOOT_32BIT_FORCE
const HeaderInfo::VersionT BINCOMPAT_MIN_VER_SAVE = 3; //-- 32bit: our files can be loaded by libs >= BINCOMPAT_MIN_VER_SAVE_$(VER.REV)
const HeaderInfo::VersionT BINCOMPAT_MIN_REV_SAVE = 2;
const HeaderInfo::VersionT BINCOMPAT_MIN_VER_LOAD = 3; //-- 32bit: we can load files >= BINCOMPAT_MIN_VER_LOAD_$(VER.REV)
const HeaderInfo::VersionT BINCOMPAT_MIN_REV_LOAD = 1;
#else
const HeaderInfo::VersionT BINCOMPAT_MIN_VER_SAVE = 3; //-- native: our files can be loaded by libs >= BINCOMPAT_MIN_VER_SAVE_$(VER.REV)
const HeaderInfo::VersionT BINCOMPAT_MIN_REV_SAVE = 1;
const HeaderInfo::VersionT BINCOMPAT_MIN_VER_LOAD = 2; //-- native: we can load files >= BINCOMPAT_MIN_VER_LOAD_$(VER.REV)
const HeaderInfo::VersionT BINCOMPAT_MIN_REV_LOAD = 5;
#endif


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
    carp("mootHMM::save(): open failed for \"%s\": %s\n", filename, ofs.errmsg().c_str());
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
		BINCOMPAT_VER,          BINCOMPAT_REV,
		BINCOMPAT_MIN_VER_SAVE, BINCOMPAT_MIN_REV_SAVE,
		0);
  Item<HeaderInfo> hi_item;
  Item<size_t>     size_item;
  Item<char *>     cmt_item;
  char comment[512];
  sprintf(comment, "\nmootHMM Version %u.%u\n", BINCOMPAT_VER, BINCOMPAT_REV);

  //-- get checksum
  size_t crc = start_tagid + n_tags + n_toks + n_classes
#ifdef MOOT_ENABLE_SUFFIX_TRIE
    + suftrie.size()
#endif //-- MOOT_ENABLE_SUFFIX_TRIE
    ;
  if (! (hi_item.save(obs, hi)
	 && size_item.save(obs, crc)
	 && cmt_item.save(obs, comment)
	 ))
    {
      carp("mootHMM::save(): could not save header%s%s\n",
	   (filename ? " to file " : ""), (filename ? filename : ""));
      return false;
    }

  return _bindump(obs, hi, filename);
}

bool mootHMM::_bindump(mootio::mostream *obs, const HeaderInfo &hdr, const char *filename)
{
  //-- variables
  Item<size_t> size_item;
  Item<TagID> tagid_item;
  Item<ProbT> probt_item;
  Item<bool> bool_item;
  Item<LexClass> lclass_item;
  Item<TokIDTable> tokids_item;
  Item<TagIDTable> tagids_item;
  Item<ClassIDTable> classids_item;
  Item<LexProbTable> lexprobs_item;
  Item<LexClassProbTable> lcprobs_item;
  Item<NgramProbHash> nghash_item;
  Item<mootTaster> taster_item;

#ifdef MOOT_ENABLE_SUFFIX_TRIE
  Item<SuffixTrie> trie_item;
#endif

  if (! (tagid_item.save(obs, start_tagid)
	 && probt_item.save(obs, unknown_lex_threshhold)
	 && probt_item.save(obs, nglambda1)
	 && probt_item.save(obs, nglambda2)
	 && probt_item.save(obs, nglambda3)
	 && probt_item.save(obs, wlambda0)
	 && probt_item.save(obs, wlambda1)

	 && bool_item.save(obs, hash_ngrams)
	 && bool_item.save(obs, relax)
	 && bool_item.save(obs, use_lex_classes)
	 && bool_item.save(obs, use_flavors)   //-- v2.0.9-1 / binfmt 3.2
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
	 && (hash_ngrams
	     ? nghash_item.save(obs, ngprobsh)
	     : probt_item.save_n(obs, ngprobsa, n_tags*n_tags*n_tags)
	     )
	 ))
    {
      carp("mootHMM::save(): could not save data%s%s\n",
	   (filename ? " to file " : ""), (filename ? filename : ""));
      return false;
    }

#ifdef MOOT_ENABLE_SUFFIX_TRIE
  if (!trie_item.save(obs, suftrie))
    {
      carp("mootHMM::save(): could not save suffix trie%s%s\n",
	   (filename ? " to file " : ""), (filename ? filename : ""));
      return false;
    }
#endif //--MOOT_ENABLE_SUFFIX_TRIE

  if (!taster_item.save(obs, taster)) { //-- v2.0.9-1 / binfmt 3.2
    carp("mootHMM::save(): could not save flavor data%s%s\n",
	 (filename ? " to file " : ""), (filename ? filename : ""));
    return false;
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

  Item<HeaderInfo> hi_item;
  Item<size_t>     size_item;
  Item<bool>       bool_item;
  Item<string>     cmt_item;
  string comment;

  //-- load raw header
  if (!hi_item.load(ibs, hi))
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
  else if (hi.version < BINCOMPAT_MIN_VER_LOAD
	   || (hi.version == BINCOMPAT_MIN_VER_LOAD && hi.revision < BINCOMPAT_MIN_REV_LOAD)
	   || BINCOMPAT_VER < hi.minver
	   || (BINCOMPAT_VER == hi.minver && BINCOMPAT_REV < hi.minrev))
    {
      carp("mootHMM::load(): incompatible file versions: %s=[ver:%u.%u,min:%u.%u]; lib=[ver:%u.%.u,min:%u.%u]\n",
	   (filename ? filename : "stored"),
	   hi.version, hi.revision,
	   BINCOMPAT_MIN_VER_LOAD, BINCOMPAT_MIN_REV_LOAD);
      return false;
    }

  //-- load auxilliary header data
  if (!size_item.load(ibs, crc)) {
      carp("mootHMM::load(): could not load checksum%s%s\n",
	   (filename ? " from file " : ""), (filename ? filename : ""));
      return false;
  }
  if (hi.version==2 && hi.revision==5) {
    //-- hack: load dummy fields in binary v2.5 format
    carp("mootHMM::load(): attempting to load obsolete model format%s%s: consider re-compiling the model\n",
	 (filename ? " from file " : ""), (filename ? filename : ""));
    bool saved_use_trigrams;
    if (! (bool_item.load(ibs, saved_use_trigrams)
	   && bool_item.load(ibs, hash_ngrams)
	   ))
      {
	carp("mootHMM::load(): could not load obsolete header flags%s%s: recompile the model\n",
	     (filename ? " from file " : ""), (filename ? filename : ""));
	return false;
      }
    if (!saved_use_trigrams) {
      carp("mootHMM::load(): obsolete bigram-only model detected%s%s: recompile the model\n",
	   (filename ? " in file " : ""), (filename ? filename : ""));
      return false;
    }
  }
  if (!cmt_item.load(ibs, comment)) {
    carp("mootHMM::load(): could not load model comment%s%s\n",
	 (filename ? " from file " : ""), (filename ? filename : ""));
    return false;
  }


  if(!_binload(ibs, hi, filename))
    return false;

  if (crc != (start_tagid + n_tags + n_toks + n_classes
#ifdef MOOT_ENABLE_SUFFIX_TRIE
	      + suftrie.size()
#endif
	      ))
    {
      carp("mootHMM::load(): checksum failed%s%s\n",
	   (filename ? " for file " : ""), (filename ? filename : ""));
    }

  viterbi_clear(); //-- (re-)initialize Viterbi table
  return true;
}


bool mootHMM::_binload(mootio::mistream *ibs, const HeaderInfo &hdr, const char *filename)
{
  //-- variables
  Item<size_t> size_item;
  Item<TagID> tagid_item;
  Item<ProbT> probt_item;
  Item<bool> bool_item;
  Item<LexClass> lclass_item;
  Item<TokIDTable> tokids_item;
  Item<TagIDTable> tagids_item;
  Item<ClassIDTable> classids_item;
  Item<LexProbTable> lexprobs_item;
  Item<LexClassProbTable> lcprobs_item;
  //Item<mootTokString>  tokstr_item;
  Item<NgramProbHash> nghash_item;
  size_t ngprobsa_size = 0;
#ifdef MOOT_ENABLE_SUFFIX_TRIE
  Item<SuffixTrie> trie_item;
#endif

  if (! (tagid_item.load(ibs, start_tagid)
	 && probt_item.load(ibs, unknown_lex_threshhold)
	 && probt_item.load(ibs, nglambda1)
	 && probt_item.load(ibs, nglambda2)
	 && probt_item.load(ibs, nglambda3)
	 && probt_item.load(ibs, wlambda0)
	 && probt_item.load(ibs, wlambda1)

	 && (hdr.version < 3 ? true : bool_item.load(ibs, hash_ngrams))
	 && (hdr.version < 3 ? true : bool_item.load(ibs, relax))
	 && bool_item.load(ibs, use_lex_classes)
	 && (hdr.version < 3 || (hdr.version==3 && hdr.revision < 2) ? true : bool_item.load(ibs, use_flavors))
	 && lclass_item.load(ibs, uclass)
	 && probt_item.load(ibs, clambda0)
	 && probt_item.load(ibs, clambda1)
	 && probt_item.load(ibs, beamwd)
	 ))
    {
      carp("mootHMM::load(): could not load model constants%s%s\n",
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
	 && (hash_ngrams
	     ? nghash_item.load(ibs, ngprobsh)
	     : probt_item.load_n(ibs, ngprobsa, ngprobsa_size))
	 ))
    {
      carp("mootHMM::load(): could not load table data%s%s\n",
	   (filename ? " from file " : ""), (filename ? filename : ""));
      return false;
    }

#ifdef MOOT_ENABLE_SUFFIX_TRIE
  if (!trie_item.load(ibs, suftrie))
    {
      carp("mootHMM::load(): could not load trie data%s%s\n",
	   (filename ? " from file " : ""), (filename ? filename : ""));
      return false;
    }
#endif

  //-- flavors
  if (hdr.version < 3 || (hdr.version==3 && hdr.revision < 2)) {
    //-- binfmt < 3.2 (libmoot < v2.0.9-1): use built-in flavors and just load ids: Alpha,Card,CardPunct,CardSuffix,CardSeps,Unknown
    //const char *fl_names[6] = {"@ALPHA"/*id=0*/,"@CARD","@CARDPUNCT","@CARDSUFFIX","@CARDSEPS","@UNKNOWN"/*id=0*/};
    Item<TokID> tokid_item;
    TokID x;
    int i;
    //-- load data
    for (i=0; i < 6; i++) {
      if (!tokid_item.load(ibs, x)) {
	carp("mootHMM::load(): could not load old-style flavor data%s%s\n", (filename ? " from file " : ""), (filename ? filename : ""));
	return false;
      }
    }
    //-- just assign ids; we should already have them in tokids
    use_flavors = true;
    taster.set_default_rules();
    assign_ids_fl();
  }
  else {
    //-- binfmt >= 3.2 (moot >= 2.0.9-1): load actual taster
    Item<mootTaster> taster_item;
    if (!taster_item.load(ibs, taster)) return false;
  }

  return true;
}


/*--------------------------------------------------------------------------
 * Error reporting
 *--------------------------------------------------------------------------*/
void mootHMM::carp(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}
