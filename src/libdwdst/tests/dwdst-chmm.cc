#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include <string>

#include "dwdstLexfreqs.h"
#include "dwdstNgrams.h"
#include "dwdstMorph.h"
#include "dwdstCHMM.h"
#include "dwdstTaggerLexer.h"
#include "dwdstEnum.h"
#include "dwdstTypes.h"

using namespace std;

dwdstLexfreqs lf;
dwdstNgrams   ng;
dwdstCHMM     chmm;
dwdstTaggerLexer lexer;

timeval t1, t2, t3;


#define ESTIMATE_NGLAMBDAS 1

//#define USE_TNT_LAMBDAS  1
//#define USE_FLAT_WORDPROBS 1

#define UNKNOWN_LEX_THRESH 1
//#define UNKNOWN_LEX_THRESH 0

#define MORPHCACHE_THRESH 1

//#define USE_TRIGRAMS

#define ESTIMATE_WLAMBDAS 1
//#undef ESTIMATE_WLAMBDAS


#ifndef ESTIMATE_WLAMBDAS

# ifdef USE_FLAT_WORDPROBS
   const double WLAMBDA1 = 0.5;
   const double WLAMBDA2 = 0.0;
   const double WLAMBDAC = 0.5;
# else // USE_FLAT_WORDPROBS
# include <float.h>
/*
//-- 93.18%
const double WLAMBDA1 = 0.5 - 2.0*DBL_EPSILON;
const double WLAMBDA2 = DBL_EPSILON;
const double WLAMBDAC = 0.5 - 2.0*DBL_EPSILON;
*/
/*
//-- 93.34%
const double WLAMBDA1 = 0.9-DBL_EPSILON;
const double WLAMBDA2 = DBL_EPSILON;
const double WLAMBDAC = 0.1;
*/
/*
//-- 94.48%
const double WLAMBDA1 = 1.0-DBL_EPSILON;
const double WLAMBDA2 = DBL_EPSILON;
const double WLAMBDAC = 0.0;
*/

# endif // USE_FLAT_WORDPROBS

#endif // ESTIMATE_WLAMBDAS


#ifdef ESTIMATE_NGLAMBDAS

# ifdef USE_TNT_LAMBDAS
const double NGLAMBDA1 = 0.1684092;
const double NGLAMBDA2 = 0.8315908;
# else
const double NGLAMBDA1 = 0.0;
const double NGLAMBDA2 = 1.0;
# endif // USE_TNT_LAMBDAS

#endif // ESTIMATE_NGLAMBDAS


int main (int argc, char **argv) {
  char *progname = *argv;
  if (argc < 5) {
    fprintf(stderr, "Usage: %s LEXFREQS NGRAMS SYMBOLS FST [TNTFILE]\n", progname);
    exit(1);
  }

  gettimeofday(&t1, NULL);

  char *lexfile = *(++argv);
  char *ngfile  = *(++argv);
  char *symfile = *(++argv);
  char *fstfile = *(++argv);
  char *infilename  = "-";
  FILE *infile = stdin;
  if (argc > 5) {
    infilename = *(++argv);
    infile = fopen(infilename, "r");
    if (!infile) {
      fprintf(stderr, "%s: Error: open failed for input-file '%s'\n", progname, infilename);
      exit(1);
    }
  }

  //-- report add_one_hack status
  fprintf(stderr, "%s: DWDST_ADD_ONE_HACK %s\n",
	 progname,
#ifdef DWDST_ADD_ONE_HACK
	 "enabled"
#else
	 "NOT enabled"
#endif
	 );

  //-- load lexfreqs
  fprintf(stderr, "%s: loading lexical frequencies from '%s'... ",
	  progname, lexfile);
  if (!lf.load(lexfile)) {
    fprintf(stderr, "FAILED.\n");
    exit(2);
  }
  fprintf(stderr, "loaded.\n");

  //-- load ngrams
  fprintf(stderr, "%s: loading ngram frequencies from '%s'... ",
	  progname, ngfile);
  if (!ng.load(ngfile)) {
    fprintf(stderr, "FAILED.\n");
    exit(3);
  }
  fprintf(stderr, "loaded.\n");

  //-- load symbols
  fprintf(stderr, "%s: loading morphological symbols from '%s'... ",
	  progname, symfile);
  if (!chmm.morph.load_morph_symbols(symfile)) {
    fprintf(stderr, "FAILED.\n");
    exit(4);
  }
  fprintf(stderr, "loaded.\n");

  //-- load morphology
  fprintf(stderr, "%s: loading morphological FST from '%s'... ",
	  progname, fstfile);
  if (!chmm.morph.load_morph_fst(fstfile)) {
    fprintf(stderr, "FAILED.\n");
    exit(5);
  }
  fprintf(stderr, "loaded.\n");

  //-- compile CHMM
  fprintf(stderr, "%s: compiling CHMM... ", progname);
  if (!chmm.compile(lf, ng, "__$", UNKNOWN_LEX_THRESH, MORPHCACHE_THRESH)) {
    fprintf(stderr, "FAILED.\n");
    exit(6);
  }
  fprintf(stderr, "compiled.\n");

  //-- set lambdas
#ifdef ESTIMATE_NGLAMBDAS
  fprintf(stderr, "%s: estimating n-gram lambdas... ", progname);
  if (!chmm.estimate_lambdas(ng)) {
    fprintf(stderr, "FAILED.\n");
    exit(7);
  }
  fprintf(stderr, "done.\n");
#else
  fprintf(stderr, "%s: NOT using estimated lambdas.\n", progname);
  chmm.nglambda1 = NGLAMBDA1;
  chmm.nglambda2 = NGLAMBDA2;
# ifdef USE_TRIGRAMS
   chmm.nglambda3 = NGLAMBDA3;
# endif /* USE_TRIGRAMS */
#endif /* ESTIMATE_NGLAMBDAS */

#ifdef ESTIMATE_WLAMBDAS
  fprintf(stderr, "%s: estimating lexical lambdas... ", progname);
  if (!chmm.estimate_wlambdas(lf)) {
    fprintf(stderr, "FAILED.\n");
    exit(7);
  }
  fprintf(stderr, "done.\n");
#else
  fprintf(stderr, "%s: NOT estimating lexical lambdas.\n", progname);
  chmm.wlambda1  = WLAMBDA1;
  chmm.wlambda2  = WLAMBDA2;
  chmm.wlambdaC  = WLAMBDAC;
#endif // ESTIMATE_WLAMBDAS

  //--report
  fprintf(stderr, "%s: start_tag=%u(\"%s\")\n",
	  progname, chmm.start_tagid, chmm.tagids.id2name(chmm.start_tagid).c_str());
  fprintf(stderr, "%s: unknownLexThreshhold=%u\n", progname, UNKNOWN_LEX_THRESH);
  fprintf(stderr, "%s: morphCacheThreshhold=%u\n", progname, MORPHCACHE_THRESH);
  fprintf(stderr, "%s: wlambda1=%g\n", progname, chmm.wlambda1);
  //fprintf(stderr, "%s: wlambdaC=%g\n", progname, chmm.wlambdaC);
  fprintf(stderr, "%s: wlambda2=%g\n", progname, chmm.wlambda2);
  fprintf(stderr, "%s: nglambda1=%g\n", progname, chmm.nglambda1);
  fprintf(stderr, "%s: nglambda2=%g\n", progname, chmm.nglambda2);
#ifdef USE_TRIGRAMS
  fprintf(stderr, "%s: nglambda3=%g\n", progname, chmm.nglambda3);
#endif
  fprintf(stderr, "%s: reading input... \n", progname);

  //-- report to stream
  fprintf(stdout, "%%%% %s: unknownLexThreshhold=%u\n", progname, UNKNOWN_LEX_THRESH);
  fprintf(stdout, "%%%% %s: morphCacheThreshhold=%u\n", progname, MORPHCACHE_THRESH);
  fprintf(stdout, "%%%% %s: wlambda1=%g\n", progname, chmm.wlambda1);
  //fprintf(stdout, "%%%% %s: wlambdaC=%g\n", progname, chmm.wlambdaC);
  fprintf(stdout, "%%%% %s: wlambda2=%g\n", progname, chmm.wlambda2);
  fprintf(stdout, "%%%% %s: nglambda1=%g\n", progname, chmm.nglambda1);
  fprintf(stdout, "%%%% %s: nglambda2=%g\n", progname, chmm.nglambda2);
#ifdef USE_TRIGRAMS
  fprintf(stdout, "%%%% %s: nglambda3=%g\n", progname, chmm.nglambda3);
#endif // USE_TRIGRAMS

  //-- tag
  gettimeofday(&t2, NULL);
  chmm.tag_stream(infile, stdout, infilename);

  //-- Summarize
  gettimeofday(&t3, NULL);

  double ielapsed = (t2.tv_sec-t1.tv_sec + (double)(t2.tv_usec-t1.tv_usec)/1000000.0);
  double relapsed = (t3.tv_sec-t2.tv_sec + (double)(t3.tv_usec-t2.tv_usec)/1000000.0);

  fprintf(stderr, "\n%s Summary:\n", progname);
  fprintf(stderr, "   Tokens Processed : %u tok\n", chmm.ntokens);
  fprintf(stderr, "    Tokens Analyzed : %u tok\n", chmm.morph.ntokens);
  fprintf(stderr, "     Morph-Unknowns : %u tok\n", chmm.morph.nunknown);
  fprintf(stderr, "   Recognition Rate : %.2f %%\n",
	  100.0*((double)chmm.morph.nunknown/(double)chmm.ntokens));
  fprintf(stderr, " MorphCache Lookups : %u tok\n", chmm.ntokens-chmm.morph.ntokens);
  fprintf(stderr, "    MorphCache Rate : %.2f %%\n",
	  100.0*(chmm.ntokens-chmm.morph.ntokens)/(float)chmm.ntokens);
  fprintf(stderr, "    Initialize time : %.2f sec\n", ielapsed);
  fprintf(stderr, "    Processing time : %.2f sec\n", relapsed);
  fprintf(stderr, "         Throughput : %.2f tok/sec\n", (double)chmm.ntokens/relapsed);

  return 0;
}


