#include <stdio.h>
#include "mootToken.h"
#include "mootTokenLexer.h"
#include "mootTokenIO.h"

using namespace moot;

void churntest(int argc, char **argv) {
  TokenReader tr;
  TokenWriter tw;
  FILE *infile = stdin;

  //-- flags
  tr.lexer.first_analysis_is_best = true;
  tr.lexer.ignore_first_analysis = true;
  tr.lexer.ignore_comments =
    //true
    false
    ;

  if (argc > 1) {
    infile = fopen(argv[1], "r");
  }
  if (!infile) {
    fprintf(stderr, "open failed for '%s'\n", argv[1]);
    exit(1);
  }

  tr.select_stream(infile);

  mootSentence sent;
  for (;;) {
    sent = tr.get_sentence();
    if (sent.empty() && tr.lexer.lasttyp == TF_EOF) break;

    tw.sentence_put(stdout, sent);
  }
    
  printf("--EOF--\n");
}

int main (int argc, char **argv) {

  //for (int i = 0; i < 10; i++)
  churntest(argc, argv);

  return 0;
}
