#include <stdio.h>
#include "dwdstLexfreqs.h"

int main (int argc, char **argv) {
  dwdstLexfreqs lfreqs;
  FILE *infile = stdin;

  if (argc > 1) {
    infile = fopen(argv[1], "r");
  }
  if (!infile) {
    fprintf(stderr, "open failed for '%s'\n", argv[1]);
    exit(1);
  }

  if (!lfreqs.load(infile)) {
    fprintf(stderr, "load failed!\n");
    exit(1);
  }

  //-- test lookup
  fprintf(stderr, "lookup(foo,NN)=%g\n", lfreqs.lookup("foo","NN"));
  fprintf(stderr, "lookup(bar,VVFIN)=%g\n", lfreqs.lookup("bar","VVFIN"));
  fprintf(stderr, "lookup(---,NN)=%g\n", lfreqs.lookup("---","NN"));
  fprintf(stderr, "lookup(foo,---)=%g\n", lfreqs.lookup("foo","---"));

  if (!lfreqs.save(stdout)) {
    fprintf(stderr, "save failed!\n");
    exit(2);
  }

  return 0;
}
