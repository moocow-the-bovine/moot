#include <stdio.h>
#include <string.h>
#include <iostream.h>

#include <string>

#include "mootLexfreqs.h"
#include "mootNgrams.h"
#include "mootMorph.h"
#include "mootCHMM.h"

#define ESTIMATE_LAMBDAS 1

//#define UNKNOWN_LEX_THRESH 0
#define UNKNOWN_LEX_THRESH 1

#define START_TAG_STR "__$"

mootLexfreqs lf;
mootNgrams   ng;
mootCHMM    hmm;


int main (int argc, char **argv) {
  char *progname = *argv;
  int compression_level = -1;

  if (argc < 6) {
    fprintf(stderr, "Usage: %s LEXFREQS NGRAMS SYMBOLS MORPH OUTFILE [COMPRESS_LEVEL]\n", progname);
    exit(1);
  }
  char *lexfile = *(++argv);
  char *ngfile  = *(++argv);
  char *symfile = *(++argv);
  char *fstfile = *(++argv);
  char *hmmfile = *(++argv);
  if (argc > 6) compression_level = atoi(*(++argv));


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
  fprintf(stderr, "%s: loading symbols from '%s'... ", progname, symfile);
  if (!hmm.morph.load_morph_symbols(symfile)) {
    fprintf(stderr, "FAILED.\n");
    exit(4);
  }
  fprintf(stderr, "loaded.\n");

  //-- load FST
  fprintf(stderr, "%s: loading FST from '%s'... ", progname, fstfile);
  if (!hmm.morph.load_morph_fst(fstfile)) {
    fprintf(stderr, "FAILED.\n");
    exit(5);
  }
  fprintf(stderr, "loaded.\n");

  //-- compile HMM
  fprintf(stderr, "%s: compiling HMM... ", progname);
  if (!hmm.compile(lf, ng, START_TAG_STR, UNKNOWN_LEX_THRESH)) {
    fprintf(stderr, "FAILED.\n");
    exit(6);
  }
  fprintf(stderr, "compiled.\n");

#ifdef ESTIMATE_LAMBDAS
  fprintf(stderr, "%s: estimating lambdas... ", progname);
  if (!hmm.estimate_lambdas(ng) || !hmm.estimate_wlambdas(lf)) {
    fprintf(stderr, "FAILED.\n");
    exit(7);
  }
  fprintf(stderr, "done.\n");
#else
  //-- dont't estimate lambdas: use constants
  fprintf(stderr, "%s: NOT estimating lambdas.\n", progname);
  hmm.nglambda1 = 0.0;
  hmm.nglambda2 = 1.0;
#endif /* ESTIMATE_LAMBDAS */


  fprintf(stderr, "%s: dumping binary data to '%s' , compress=%d...",
	  progname, hmmfile, compression_level);
  if (!hmm.save(hmmfile,compression_level)) {
    fprintf(stderr, " FAILED.\n");
  } else {
    fprintf(stderr, " done.\n");
  }
  return 0;
}
