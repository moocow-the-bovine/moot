#include <stdio.h>

#include <deque>
#include "dwdstTypes.h"
#include "dwdstLexfreqs.h"

dwdstLexfreqs lfreqs;

typedef deque<dwdstTokString> TokDeque;
TokDeque toks;


void lexLoad(int argc, char **argv) {
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
}

void lexBenchPrep(void) {
  toks.clear();
  for (dwdstLexfreqs::LexfreqTotalTable::const_iterator lftoti = lfreqs.lftotals.begin();
       lftoti != lfreqs.lftotals.end();
       lftoti++)
    {
      toks.push_back(lftoti->first);
    }
}

void lexBench(unsigned iters) {
  unsigned i;
  dwdstLexfreqs::LexfreqCount lfc;
  for (i = 0; i < iters; i++) {
    for (TokDeque::const_iterator tdi = toks.begin(); tdi != toks.end(); tdi++) {
      lfc = lfreqs.lookup(*tdi);
    }
  }
}

#define BENCH_ITERS 128

int main (int argc, char **argv) {
  fprintf(stderr, "Load...\n");
  lexLoad(argc, argv);

  fprintf(stderr, "Prepare...\n");
  lexBenchPrep();

  fprintf(stderr, "Bench...\n");
  lexBench(BENCH_ITERS);

  fprintf(stderr, "Cleanup...\n");
  lfreqs.clear();

  fprintf(stderr, "Exit...\n");
  return 0;
}
