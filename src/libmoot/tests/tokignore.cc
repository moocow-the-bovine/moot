#include <stdio.h>
#include "mootToken.h"
#include "mootTokenLexer.h"
#include "mootTokenIO.h"

using namespace moot;

void churntest(int argc, char **argv) {
  TokenReader tr;
  FILE *infile = stdin;

  if (argc > 1) {
    infile = fopen(argv[1], "r");
  }
  if (!infile) {
    fprintf(stderr, "open failed for '%s'\n", argv[1]);
    exit(1);
  }

  tr.select_stream(infile);

  mootSentence sent;
  do {
    sent = tr.get_sentence();
  } while (!sent.empty() || !feof(infile));
  printf("EOF\n");
}

int main (int argc, char **argv) {

  //for (int i = 0; i < 10; i++)
  churntest(argc, argv);

  return 0;
}
