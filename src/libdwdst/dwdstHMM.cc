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
#include <errno.h>
#include <string.h>

#include "dwdstHMM.h"
#include "dwdstTaggerLexer.h"

#include <zlib.h>
#include "dwdstBinIO.h"
#include "dwdstBinStream.h"

using namespace std;
using namespace dwdstBinIO;

/*--------------------------------------------------------------------------
 * clear, freeing dynamic data
 *--------------------------------------------------------------------------*/

void dwdstHMM::clear(bool wipe_everything)
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

    for (int i = 0; i < NTokTypes; i++) {
      typids[i] = 0;
    }

    start_tagid = 0;
    unknown_lex_threshhold = 1;
    n_tags = 0;
    n_toks = 0;
  }
}

/*--------------------------------------------------------------------------
 * Compilation
 *--------------------------------------------------------------------------*/
bool dwdstHMM::compile(const dwdstLexfreqs &lexfreqs,
		       const dwdstNgrams &ngrams,
		       const dwdstTagString &start_tag_str)
{
  //-- sanity check
  if (ngrams.ugtotal == 0) {
    carp("dwdstHMM::compile(): Error: bad unigram total in 'ngrams'!\n");
    return false;
  }

  //-- assign IDs
  assign_ids_lf(lexfreqs);
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
  TokenType                   toktyp;         //-- current token-type
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
  bool                     is_valid_token;    //-- watch out for special tokens (@CARD & friends)
  int                      i;
 
  //-- compile lexical probabilities : for all lexical keys (lfti)
  for (dwdstLexfreqs::LexfreqStringTable::const_iterator lfti = lexfreqs.lftable.begin();
       lfti != lexfreqs.lftable.end();
       lfti++)
    {
      const dwdstTokString &tokstr = lfti->first.first;

      //-- check for special tokens
      is_valid_token = true;
      for (i = 0; i < NTokTypes; i++) {
	if (tokstr == typnames[i]) {
	  is_valid_token = false;
	  break;
	}
      }
      if (!is_valid_token) continue;

      const dwdstTagString &tagstr = lfti->first.second;
      const dwdstLexfreqs::LexfreqCount toktotal = lexfreqs.lookup(tokstr);
      const dwdstLexfreqs::LexfreqCount tagtotal = lexfreqs.taglookup(tagstr);

      //-- sanity check
      if (toktotal == 0 || tagtotal == 0) continue;

      //-- continue here?!

      //-- get IDs
      tagid  = tagids.name2id(tagstr);
      toktyp = token2type(tokstr);
      tokid  = toktyp == TokTypeAlpha ? tokids.name2id(tokstr) : typids[toktyp];

      //-- "unknown" token check
      if (toktyp == TokTypeAlpha && toktotal <= unknown_lex_threshhold) {
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

  //-- Normalize "unknown" lexical probabilities
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
void dwdstHMM::assign_ids_lf(const dwdstLexfreqs &lexfreqs)
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

      TokenType typ = token2type(tokstr);
      if (typ != TokTypeAlpha) continue; //-- ignore special token-types

      if (
      #ifndef DWDST_LEX_UNKNOWN_TOKENS
	  //-- unknown threshhold check
          toktotal > unknown_lex_threshhold &&
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


bool dwdstHMM::estimate_wlambdas(const dwdstLexfreqs &lf)
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

/*--------------------------------------------------------------------------
 * Top-level: stream
 *--------------------------------------------------------------------------*/
void dwdstHMM::tag_stream(FILE *in, FILE *out, char *srcname)
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
	  viterbi_step(token2id(tokens.back()), curtags);
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
	     (srcname ? srcname : "(unknown)"),
	     lexer.yytext,
	     lexer.theLine,
	     lexer.theColumn);
	break;
      }
  }
}


/*--------------------------------------------------------------------------
 * Top-level: strings
 *--------------------------------------------------------------------------*/
void dwdstHMM::tag_strings(int argc, char **argv, FILE *out, char *infilename)
{
  //-- prepare variables
  tokens.clear();
  curtags.clear();
  viterbi_clear();

  for ( ; --argc >= 0; argv++) {
    tokens.push_back((const char *)*argv);
    viterbi_step(tokens.back());
    ntokens++;
  }
  viterbi_finish();
  tag_print_best_path(out);
  nsents++;

  //-- cleanup
  tokens.clear();
  curtags.clear();
  viterbi_clear();
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
  fprintf(file, "%%start_tag\tID=%u(\"%s\")\n", start_tagid, tagids.id2name(start_tagid).c_str());
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
 * Binary I/O: save
 *--------------------------------------------------------------------------*/

const HeaderInfo::VersionT BINCOMPAT_MIN_VER = 1;
const HeaderInfo::VersionT BINCOMPAT_MIN_REV = 3;

const HeaderInfo::VersionT BINCOMPAT_VER = 1;
const HeaderInfo::VersionT BINCOMPAT_REV = 3;


bool dwdstHMM::save(const char *filename, int compression_level)
{
  //-- sanity checks
  if (compression_level != Z_DEFAULT_COMPRESSION
      && (compression_level > Z_BEST_COMPRESSION || compression_level < Z_NO_COMPRESSION))
    {
      carp("dwdstHMM::save(): bad compression level %d defaults to %d\n",
	   compression_level, Z_DEFAULT_COMPRESSION);
      compression_level = Z_DEFAULT_COMPRESSION;
    }

  //-- open file
  gzFile gzf = gzopen(filename, "wb");
  if (!gzf) {
    carp("dwdstHMM::save(): open failed for file '%s': %s\n",
	 filename, gzerror(gzf, &errno));
    return false;
  }
  gzsetparams(gzf, compression_level, Z_DEFAULT_STRATEGY);


  //-- setup stream-hack
  dwdstBinStream::ozBinStream ozs(gzf);

  //-- and save
  bool rc = save(ozs, filename);
  ozs.close();
  return rc;
}

bool dwdstHMM::save(dwdstBinStream::oBinStream &obs, const char *filename)
{
  HeaderInfo hi(string("dwdstHMM"),
		BINCOMPAT_VER,     BINCOMPAT_REV,
		BINCOMPAT_MIN_VER, BINCOMPAT_MIN_REV,
		0);
  Item<HeaderInfo> hi_item;
  Item<size_t>     size_item;
  Item<char *>     cmt_item;
  char comment[512];
  sprintf(comment, "\ndwdstHMM Version %u.%u\n", BINCOMPAT_VER, BINCOMPAT_REV);

  //-- get checksum
  size_t crc = start_tagid + n_tags + n_toks;
  if (! (hi_item.save(obs, hi)
	 && size_item.save(obs, crc)
	 && cmt_item.save(obs, comment)
	 ))
    {
      carp("dwdstHMM::save(): could not save header%s%s\n",
	   (filename ? " to file " : ""), (filename ? filename : ""));
      return false;
    }

  return _bindump(obs, filename);
}

bool dwdstHMM::_bindump(dwdstBinStream::oBinStream &obs, const char *filename)
{
  //-- variables
  Item<size_t> size_item;
  Item<TagID> tagid_item;
  Item<TokID> tokid_item;
  Item<ProbT> probt_item;
  Item<TokIDTable> tokids_item;
  Item<TagIDTable> tagids_item;
  Item<LexProbTable> lexprobs_item;
  Item<dwdstTokString> tokstr_item;

  if (! (tagid_item.save(obs, start_tagid)
	 && probt_item.save(obs, unknown_lex_threshhold)
	 && probt_item.save(obs, nglambda1)
	 && probt_item.save(obs, nglambda2)
#ifdef DWDST_USE_TRIGRAMS
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
	 && lexprobs_item.save(obs, lexprobs)
	 && probt_item.save_n(obs, ngprobs1, n_tags)
	 && probt_item.save_n(obs, ngprobs2, n_tags*n_tags)
	 ))
    {
      carp("dwdstHMM::save(): could not save data%s%s\n",
	   (filename ? " to file " : ""), (filename ? filename : ""));
      return false;
    }

  int i;
  for (i = 0; i < NTokTypes; i++) {
    if (!tokid_item.save(obs, typids[i]) && tokstr_item.save(obs, typnames[i])) {
      carp("dwdstHMM::save(): could not save type data%s%s\n",
	   (filename ? " to file " : ""), (filename ? filename : ""));
      return false;
    }
  }

  return true;
}

/*--------------------------------------------------------------------------
 * Binary I/O: load
 *--------------------------------------------------------------------------*/

bool dwdstHMM::load(const char *filename)
{
  //-- setup gzFile
  gzFile gzs = gzopen(filename, "rb");
  if (!gzs) {
    carp("dwdstHMM::load(): could not open file '%s' for read: %s\n",
	 filename, strerror(errno));
    return false;
  }

  //-- setup stream-hack
  dwdstBinStream::izBinStream izs(gzs);

  bool rc = load(izs, filename);
  izs.close();
  return rc;
}

bool dwdstHMM::load(dwdstBinStream::iBinStream &ibs, const char *filename)
{
  clear(true); //-- make sure the object is totally empty

  HeaderInfo hi, hi_magic(string("dwdstHMM"));
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
      carp("dwdstHMM::load(): could not load header%s%s\n",
	   (filename ? " from file " : ""), (filename ? filename : ""));
      return false;
    }
  else if (hi.magic != hi_magic.magic)
    {
      carp("dwdstHMM::load(): bad magic 0x%x%s%s\n",
	   hi.magic,
	   (filename ? " in file " : ""), (filename ? filename : ""));
      return false;
    }
  else if (hi.version < BINCOMPAT_MIN_VER
	   || (hi.version == BINCOMPAT_MIN_VER && hi.revision < BINCOMPAT_MIN_REV)
	   || BINCOMPAT_VER < hi.minver
	   || (BINCOMPAT_VER == hi.minver && BINCOMPAT_MIN_REV < hi.minrev))
    {
      carp("dwdstHMM::load(): incompatible file version %u.%u%s%s\n",
	   hi.version, hi.revision,
	   (filename ? " in file " : ""), (filename ? filename : ""));
    }

  if(!_binload(ibs, filename))
    return false;

  if (crc != (start_tagid + n_tags + n_toks)) {
    carp("dwdstHMM::load(): checksum failed%s%s\n",
	 (filename ? " for file " : ""), (filename ? filename : ""));
  }

  viterbi_clear(); //-- (re-)initialize Viterbi table
  return true;
}


bool dwdstHMM::_binload(dwdstBinStream::iBinStream &ibs, const char *filename)
{
  //-- variables
  Item<size_t> size_item;
  Item<TagID> tagid_item;
  Item<TokID> tokid_item;
  Item<ProbT> probt_item;
  Item<TokIDTable> tokids_item;
  Item<TagIDTable> tagids_item;
  Item<LexProbTable> lexprobs_item;
  Item<dwdstTokString>  tokstr_item;
#ifndef DWDST_USE_TRIGRAMS
  ProbT dummy_nglambda3;
#endif

  size_t ngprobs1_size  = 0;
  size_t ngprobs2_size  = 0;

  if (! (tagid_item.load(ibs, start_tagid)
	 && probt_item.load(ibs, unknown_lex_threshhold)
	 && probt_item.load(ibs, nglambda1)
	 && probt_item.load(ibs, nglambda2)
#ifdef DWDST_USE_TRIGRAMS
	 && probt_item.load(ibs, nglambda3)
#else
	 && probt_item.load(ibs, dummy_nglambda3)
#endif
	 && probt_item.load(ibs, wlambda1)
	 && probt_item.load(ibs, wlambda2)
	 ))
    {
      carp("dwdstHMM::load(): could not load smoothing constants%s%s\n",
	   (filename ? " from file " : ""), (filename ? filename : ""));
      return false;
    }

  if (!(tokids_item.load(ibs, tokids)
	&& tagids_item.load(ibs, tagids)))
    {
      carp("dwdstHMM::load(): could not load ID data%s%s\n",
	   (filename ? " from file " : ""), (filename ? filename : ""));
      return false;
    }

  if (! (size_item.load(ibs, n_tags) && size_item.load(ibs, n_toks)) )
    {
      carp("dwdstHMM::load(): could not load base data%s%s\n",
	   (filename ? " from file " : ""), (filename ? filename : ""));
      return false;
    }

  if (! (lexprobs_item.load(ibs, lexprobs)
	 && probt_item.load_n(ibs, ngprobs1, ngprobs1_size)
	 && probt_item.load_n(ibs, ngprobs2, ngprobs2_size)
	 ))
    {
      carp("dwdstHMM::load(): could not load table data%s%s\n",
	   (filename ? " from file " : ""), (filename ? filename : ""));
      return false;
    }

  int i;
  for (i = 0; i < NTokTypes; i++) {
    if (!tokid_item.load(ibs, typids[i]) && tokstr_item.load(ibs, typnames[i])) {
      carp("dwdstHMM::save(): could not load type data%s%s\n",
	   (filename ? " to file " : ""), (filename ? filename : ""));
      return false;
    }
  }
  
  return true;
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
