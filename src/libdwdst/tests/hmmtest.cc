#include <stdio.h>
#include <string.h>

#include <string>

#include "dwdstLexfreqs.h"
#include "dwdstNgrams.h"
#include "dwdstHMM.h"

dwdstLexfreqs lf;
dwdstNgrams   ng;
dwdstHMM hmm;

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
  if (!hmm.compile(lf,ng,"",1)) {
    fprintf(stderr, "FAILED.\n");
    exit(3);
  }
  fprintf(stderr, "compiled.\n");

  fprintf(stderr, "Done. (Press return to dump) ");
  fgetc(stdin);

  hmm.txtdump(stdout);
  return 0;
}


