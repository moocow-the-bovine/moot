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
#include <stdarg.h>

#include "dwdstHMM.h"
#include "dwdstTaggerLexer.h"

using namespace std;

/*--------------------------------------------------------------------------
 * clear, freeing dynamic data
 *--------------------------------------------------------------------------*/

void dwdstHMM::clear(bool wipe_everything)
{
  //-- iterator variables
  ViterbiColumn *col, *col_next;
  ViterbiNode   *nod, *nod_next;

  //-- free columns and nodes
  for (col = vtable; col != NULL; col = col_next) {
    col_next      = col->col_prev;
    for (nod = col->nodes; nod != NULL; nod = nod_next) {
      nod_next    = nod->row_next;
      delete nod;
    }
    delete col;
  }

  //-- free trashed cols
  for (col = trash_columns; col != NULL; col = col_next) {
    col_next = col->col_prev;
    delete col;
  }

  //-- free trashed nodes
  for (nod = trash_nodes; nod != NULL; nod = nod_next) {
    nod_next = nod->row_next;
    delete nod;
  }

  //-- free lexical probabilities
  lexprobs.clear();

  //-- clear trigram table
#ifdef DWDST_USE_TRIGRAMS
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

  if (wipe_everything) {
    //-- free id-tables
    tokids.clear();
    tagids.clear();

    start_tagid = 0;
    n_tags = 0;
    n_toks = 0;
  }
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
    carp("dwdstHMM::compile(): Error: bad unigram total in 'ngrams'!\n");
    return false;
  }

  //-- assign IDs
  assign_ids_lf(lexfreqs, unknownLexThreshhold);
  assign_ids_ng(ngrams);

  //-- get or assign start-tag-ID
  start_tagid = tagids.nameExists(start_tag_str)
    ? tagids.name2id(start_tag_str)
    : tagids.insert(start_tag_str);

  //-- save n_tags, n_toks
  n_tags = tagids.size();
  n_toks = tokids.size();

  //-- estimate lambdas
  //estimate_lambdas(ngrams);

  //-- allocate lookup tables
  clear(false);
  lexprobs.resize(tokids.size());

  //-- allocate: unigrams
  ngprobs1 = (ProbT *)malloc(n_tags*sizeof(ProbT));
  if (!ngprobs1) {
    carp("dwdstHMM::compile(): Error: could not allocate unigram table.\n");
    return false;
  }

  //-- allocate: bigrams
  ngprobs2 = (ProbT *)malloc((n_tags*n_tags)*sizeof(ProbT));
  if (!ngprobs2) {
    carp("dwdstHMM::compile(): Error: could not allocate bigram table.\n");
    return false;
  }

  //-- zero lookup-table contents
  memset(ngprobs1, 0, n_tags*sizeof(ProbT));
  memset(ngprobs2, 0, (n_tags*n_tags)*sizeof(ProbT));

  //-- compilation variables
  TokID                       tokid;          //-- current token-ID
  TagID                       tagid;          //-- current tag-ID
  TagID                       ptagid;         //-- previous tag-ID (for bigrams)
#ifdef DWDST_USE_TRIGRAMS
  TagID                      pptagid;         //-- previous-previous tag-ID (for trigrams)
#endif
  dwdstNgrams::NgramString    ngtmp;          //-- temporary string-ngram (for lookup)
  dwdstNgrams::NgramCount     pnTotal;        //-- parent ngram-total (for bigrams/trigrams)
  dwdstLexfreqs::LexfreqCount unTotal = 0;    //-- total "unknown" token count
  LexProbSubTable             &untagcts       //-- "unknown" tag counts (later, probabilites)
      = lexprobs[0];
 
  //-- compile lexical probabilities : for all lexical keys (lfti)
  for (dwdstLexfreqs::LexfreqStringTable::const_iterator lfti = lexfreqs.lftable.begin();
       lfti != lexfreqs.lftable.end();
       lfti++)
    {
      const dwdstTokString &tokstr = lfti->first.first;
      const dwdstTagString &tagstr = lfti->first.second;
      const dwdstLexfreqs::LexfreqCount toktotal = lexfreqs.lookup(tokstr);
      const dwdstLexfreqs::LexfreqCount tagtotal = lexfreqs.taglookup(tagstr);

      //-- sanity check
      if (toktotal == 0 || tagtotal == 0) continue;

      //-- get IDs
      tagid = tagids.name2id(tagstr);
      tokid = tokids.name2id(tokstr);

      //-- "unknown" token check
      if (toktotal <= unknownLexThreshhold) {
	//-- "unknown" token: just store the raw counts for now
	
	//-- ... and add to "unknown" counts
	unTotal += lfti->second;
	LexProbSubTable::iterator lpsi = untagcts.find(tagid);
	if (lpsi == untagcts.end()) {
	  untagcts[tagid] = lfti->second;
	} else {
	  lpsi->second += lfti->second;
	}
      }
      if (tokid != 0) {
	//-- it's a kosher token (too?): compute lexical probability: p(tok|tag)
	lexprobs[tokid][tagid] = lfti->second / tagtotal;
      }
    }

  //-- Normakize "unknown" lexical probabilities
  if (lexprobs.size() == 0) lexprobs.resize(1);
  for (LexProbSubTable::iterator lpsi = untagcts.begin();
       lpsi != untagcts.end();
       lpsi++)
      {
	  if (lpsi->second == 0) continue;
	  const dwdstTagString &tagstr = tagids.id2name(lpsi->first);
	  const dwdstLexfreqs::LexfreqCount tagtotal = lexfreqs.taglookup(tagstr);
	  lpsi->second /= tagtotal;
      }

  //-- Compute ngram probabilites
  for (dwdstNgrams::NgramStringTable::const_iterator ngti = ngrams.ngtable.begin();
       ngti != ngrams.ngtable.end();
       ngti++)
    {
      if (ngti->first.size() == 1) {
	//-- congratulations, it's a unigram: getID
	tagid = tagids.name2id(ngti->first[0]);  

	//-- compute unigram probability
	ngprobs1[tagid] = ngti->second / ngrams.ugtotal;
      }
      else if (ngti->first.size() == 2) {
	//-- it's a bigram

	//-- get IDs
	ptagid = tagids.name2id(ngti->first[0]);
	tagid  = tagids.name2id(ngti->first[1]);

	//-- get unigram count for previous tag
	ngtmp.clear();
	ngtmp.push_back(ngti->first[0]);
	pnTotal = ngrams.lookup(ngtmp);
	if (pnTotal == 0) continue;

	//-- compute bigram probability
	ngprobs2[(n_tags*ptagid)+tagid] = ngti->second / pnTotal;
      }
      else if (ngti->first.size() == 3) {
	//-- it's a trigram

#ifdef DWDST_USE_TRIGRAMS
	//-- get IDs
	pptagid = tagids.name2id(ngti->first[0]);
	ptagid  = tagids.name2id(ngti->first[1]);
	tagid   = tagids.name2id(ngti->first[2]);

	//-- get bigram count for bigram (pptagid,ptagid)
	ngtmp.clear();
	ngtmp.push_back(ngti->first[0]);
	ngtmp.push_back(ngti->first[1]);
	pnTotal = ngrams.lookup(ngtmp);
	if (pnTotal == 0) continue;

	//-- compute trigram probability
	ngprobs3[Trigram(pptagid,ptagid,tagid)] = ngti->second / pnTotal;
#else /* DWDST_USE_TRIGRAMS */
	;
#endif /* DWDST_USE_TRIGRAMS */
      }
      else {
	//-- Otherwise, it's not a uni-, bi-, or tri-gram: ignore it
	carp("dwdstHMM::compile(): ignoring %u-gram!\n", ngti->first.size());
      }
    }

  viterbi_clear();
  return true;
}

/*--------------------------------------------------------------------------
 * Compilation utilities: ID-assignment
 *--------------------------------------------------------------------------*/
void dwdstHMM::assign_ids_lf(const dwdstLexfreqs &lexfreqs,
			     const dwdstLexfreqs::LexfreqCount unknownLexThreshhold)
{
  //-- compile lexical IDs
  for (dwdstLexfreqs::LexfreqStringTable::const_iterator lfti = lexfreqs.lftable.begin();
       lfti != lexfreqs.lftable.end();
       lfti++)
    {
      const dwdstTokString &tokstr = lfti->first.first;
      const dwdstTagString &tagstr = lfti->first.second;
      #ifndef DWDST_LEX_UNKNOWN_TOKENS
        const dwdstLexfreqs::LexfreqCount toktotal = lexfreqs.lookup(tokstr);
      #endif

      //-- always assign a tag-id
      if (!tagids.nameExists(tagstr)) tagids.insert(tagstr);

      if (
      #ifndef DWDST_LEX_UNKNOWN_TOKENS
	  //-- unknown threshhold check
          toktotal > unknownLexThreshhold &&
      #endif
	  !tokids.nameExists(tokstr))
	{
	  //-- it's a kosher token : assign a token-ID
	  tokids.insert(tokstr);
	}
       
      //-- otherwise, it's below the unknown threshhold, so we ignore it here
    }

  //-- update number of tags, toks
  n_tags = tagids.size();
  n_toks = tokids.size();

  return;
}

void dwdstHMM::assign_ids_ng(const dwdstNgrams   &ngrams)
{
  //-- Compile tag IDs
  for (dwdstNgrams::NgramStringTable::const_iterator ngti = ngrams.ngtable.begin();
       ngti != ngrams.ngtable.end();
       ngti++)
    {
      for (dwdstNgrams::NgramString::const_iterator ngsi = ngti->first.begin();
	   ngsi != ngti->first.end();
	   ngsi++)
	{
	  if (!tagids.nameExists(*ngsi)) tagids.insert(*ngsi);
	}
    }

  //-- update number of tags
  n_tags = tagids.size();

  return;
}



/*--------------------------------------------------------------------------
 * Compilation utilities: smoothing constant estimation
 *--------------------------------------------------------------------------*/
/** TODO: add better DWDST_USE_TRIGRAMS check here. */
bool dwdstHMM::estimate_lambdas(const dwdstNgrams &ngrams)
{
  //-- sanity check
  if (ngrams.ugtotal <= 1) {
    fprintf(stderr, "dwdstHMM::estimate_lambdas(): Error bad unigram total in 'ngrams'!\n");
    return false;
  }

  dwdstNgrams::NgramString  ngtmp;    //-- temporary string-ngram (for lookup)

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
#ifdef DWDST_USE_TRIGRAMS
  nglambda3 = 0.0;
#else
  ProbT nglambda3 = 0.0; //-- Hack!
#endif

  //-- adjust lambdas
  for (dwdstNgrams::NgramStringTable::const_iterator ngti = ngrams.ngtable.begin();
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

  //-- normalize lambdas
  ProbT nglambda_total = nglambda1 + nglambda2;
#ifdef DWDST_USE_TRIGRAMS
  nglambda_total += nglambda3;
  nglambda3 /= nglambda_total;
#endif /* DWDST_USE_TRIGRAMS */

  nglambda2 /= nglambda_total;
  nglambda1 /= nglambda_total;

  return true;
}


/*--------------------------------------------------------------------------
 * Top-level
 *--------------------------------------------------------------------------*/
void dwdstHMM::tag_stream(FILE *in, FILE *out, char *infilename)
{
  dwdstTaggerLexer lexer;

  //-- prepare lexer
  lexer.step_streams(in,out);
  lexer.theLine   = 1;
  lexer.theColumn = 0;

  //-- prepare variables
  int tok;
  tokens.clear();
  curtags.clear();
  viterbi_clear();

  while ((tok = lexer.yylex()) != dwdstTaggerLexer::DTEOF) {
      switch (tok) {

      case dwdstTaggerLexer::TOKEN:
	tokens.push_back((const char *)lexer.yytext);
	break;

      case dwdstTaggerLexer::TAG:
        curtags.insert(tagids.name2id((const char *)lexer.yytext));
	break;

      case dwdstTaggerLexer::EOT:
	if (curtags.empty()) {
	  viterbi_step(tokens.back());
	} else {
	  viterbi_step(tokids.name2id(tokens.back()), curtags);
	  curtags.clear();
	}
        ntokens++;
        break;

      case dwdstTaggerLexer::EOS:
	viterbi_finish();
	tag_print_best_path(out);
	tokens.clear();
	curtags.clear();
	viterbi_clear();
	nsents++;
	break;

      default:
	carp("%s: Error: unknown token '%s' in file '%s' at line %d, column %d\n",
	     "dwdstHMM::tag_stream()",
	     (infilename ? infilename : "(unknown)"),
	     lexer.yytext,
	     lexer.theLine,
	     lexer.theColumn);
	break;
      }
  }
}


/*--------------------------------------------------------------------------
 * Debug / Dump
 *--------------------------------------------------------------------------*/
void dwdstHMM::txtdump(FILE *file)
{
  fprintf(file, "%%%% dwdstHMM text dump\n");

  fprintf(file, "\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  fprintf(file, "%%%% Smoothing Constants\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  fprintf(file, "%%nglambda1\t%g\n", nglambda1);
  fprintf(file, "%%nglambda2\t%g\n", nglambda2);
#ifdef DWDST_USE_TRIGRAMS
  fprintf(file, "%%nglambda3\t%g\n", nglambda3);
#endif
  fprintf(file, "%%wlambda1\t%g\n", wlambda1);
  fprintf(file, "%%wlambda2\t%g\n", wlambda2);


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

#ifdef DWDST_USE_TRIGRAMS
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
#endif /* DWDST_USE_TRIGRAMS */

  fprintf(file, "\n");
}

void dwdstHMM::viterbi_txtdump(FILE *file)
{
  fprintf(file, "%%%% dwdstHMM Viterbi column text dump\n");
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
 * Error reporting
 *--------------------------------------------------------------------------*/
void dwdstHMM::carp(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}
