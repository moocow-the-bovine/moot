#include <stdio.h>
#include "dwdstNgrams.h"

int main (int argc, char **argv) {
  dwdstNgrams ngrams;
  FILE *infile = stdin;
  bool compact = false;

  if (argc > 1) {
    infile = fopen(argv[1], "r");
  }
  if (!infile) {
    fprintf(stderr, "open failed for '%s'\n", argv[1]);
    exit(1);
  }

  if (!ngrams.load(infile)) {
    fprintf(stderr, "load failed!\n");
    exit(1);
  }

  //-- test lookup
  dwdstNgrams::NgramString ng;
  ng.push_back("a");
  fprintf(stderr, "lookup(a) = %g\n", ngrams.lookup(ng));

  ng.push_back("b");
  fprintf(stderr, "lookup(a,b) = %g\n", ngrams.lookup(ng));

  ng.push_back("X");
  fprintf(stderr, "lookup(a,b,X) = %g\n", ngrams.lookup(ng));

  ng.clear();
  ng.push_back("__$");
  ng.push_back("VMFIN");
  ng.push_back("NN");
  fprintf(stderr, "lookup(__$,VMFIN,NN) = %g\n", ngrams.lookup(ng));


  if (!ngrams.save(stdout, "STDOUT", compact)) {
    fprintf(stderr, "save failed!\n");
    exit(2);
  }

  return 0;
}
