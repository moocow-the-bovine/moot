#include <stdio.h>
#include "mootLexfreqs.h"

void churntest(int argc, char **argv) {
  mootLexfreqs lfreqs;
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

  //-- check buckets used
  fprintf(stderr, "-- using %u buckets\n", lfreqs.lftable.bucket_count());

  //-- test lookup
  fprintf(stderr, "lookup(foo,NN)=%g\n", lfreqs.lookup("foo","NN"));
  fprintf(stderr, "lookup(bar,VVFIN)=%g\n", lfreqs.lookup("bar","VVFIN"));
  fprintf(stderr, "lookup(---,NN)=%g\n", lfreqs.lookup("---","NN"));
  fprintf(stderr, "lookup(foo,---)=%g\n", lfreqs.lookup("foo","---"));

  if (!lfreqs.save(stdout)) {
    fprintf(stderr, "save failed!\n");
    exit(2);
  }

  /*
  fprintf(stderr, "clearing... (press return to continue) ");
  char c;
  c = fgetc(stdin);

  lfreqs.clear();

  fprintf(stderr, "cleared. (press return to continue)");
  c = fgetc(stdin);
  */
}

int main (int argc, char **argv) {

  //for (int i = 0; i < 10; i++)
  churntest(argc, argv);

  return 0;
}
