#include <stdio.h>
#include <string.h>

#include <string>

#include "mootLexfreqs.h"
#include "mootNgrams.h"
#include "mootHMM.h"

#define ESTIMATE_LAMBDAS 1

#define START_TAG_STR "__$"

//#define UNKNOWN_LEX_THRESH 0
#define UNKNOWN_LEX_THRESH 1


mootLexfreqs lf;
mootNgrams   ng;
mootHMM     hmm;


int main (int argc, char **argv) {
  char *progname = *argv;
  if (argc < 3) {
    fprintf(stderr, "Usage: %s LEXFREQS NGRAMS\n", progname);
    exit(1);
  }
  char *lexfile = *(++argv);
  char *ngfile  = *(++argv);

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

  //-- compile HMM
  fprintf(stderr, "%s: compiling HMM... ", progname);
  if (!hmm.compile(lf, ng, START_TAG_STR, UNKNOWN_LEX_THRESH)) {
    fprintf(stderr, "FAILED.\n");
    exit(3);
  }
  fprintf(stderr, "compiled.\n");

#ifdef ESTIMATE_LAMBDAS
  fprintf(stderr, "%s: using estimated lambdas.\n", progname);
#else
  //-- dont't estimate lambdas: use constants
  fprintf(stderr, "%s: NOT estimating lambdas.\n", progname);
  hmm.nglambda1 = 0.0;
  hmm.nglambda2 = 1.0;
#endif /* ESTIMATE_LAMBDAS */

  fprintf(stderr, "Done. (Press return to dump) ");
  fgetc(stdin);

  hmm.txtdump(stdout);
  return 0;
}
