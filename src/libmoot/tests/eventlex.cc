#include <stdio.h>
#include "mootEvent.h"
#include "mootEventLexer.h"

using namespace std;
using namespace moot;

void churntest(int argc, char **argv) {
  FILE *infile = stdin;
  FILE *outfile = stdout;
  if (argc > 1) {
    infile = fopen(argv[1], "r");
    if (!infile) {
      fprintf(stderr, "open failed for '%s'\n", argv[1]);
      exit(1);
    }
  }

  //-- init
  mootEventLexer lexer;
  size_t         initial_bufsize = lexer.ebuffer.alloc;

  lexer.first_analysis_is_best = true;
  lexer.select_streams(infile, stderr);

  int etyp;
  while ((etyp = lexer.yylex()) != etEOF) {
    mootEvent &e = lexer.event;
#if 0
    fprintf(outfile, "[EVENT(%s)-->", mootEventTypeNames[etyp]);
    e.dump(outfile);
    fprintf(outfile, "<--/EVENT]\n");
    lexer.ebuffer.clear();
#elif 1
    switch (e.etype) {
    case etComment:
      e.dump(outfile);
      fputc('\n', outfile);
      break;
    case etTokAnlBest:
    case etEOA:
      fputc('\t', outfile);
      break;
    case etEOT:
      fputc('\n', outfile);
      break;
    case etEOS:
      fputc('\n', outfile);
      lexer.ebuffer.clear();
      break;
    default:
      e.dump(outfile);
      break;
    }
#endif
  }
  fprintf(outfile, "%%%%--EOF--\n");

  fprintf(stderr, "%%%%-- Buffer size (initial/final) = %u / %u\n",
	  initial_bufsize, lexer.ebuffer.alloc);
}

int main (int argc, char **argv) {

  //for (int i = 0; i < 10; i++)
  churntest(argc, argv);

  return 0;
}
