#include <stdio.h>

#include "mootConfig.h"
#include "mootToken.h"
#include "mootTokenLexer.h"

#include "dotest.h"

const bool checkMemory = false;
const bool do_bench = false;

using namespace moot;

void churntest(int argc, char **argv) {
  FILE *infile = stdin;
    //-- init
    mootTokenLexer lexer;
    lexer.first_analysis_is_best = true;
    lexer.ignore_first_analysis = true;
  for ( ; argc > 0 ; argc--, argv++) {
    //-- open
    if (*argv) {
      infile = fopen(*argv, "r");
      if (!infile) {
	fprintf(stderr, "open failed for '%s'\n", *argv);
	exit(1);
      }
    } else {
      infile = stdin;
    }


    if (do_bench) { DOTEST_START ; }

    //-- guts
    lexer.select_streams(infile,stderr);
    size_t ntokens = 0;
    int lxtyp;
    for (lxtyp = lexer.yylex(); lxtyp != TokTypeEOF; lxtyp = lexer.yylex())
      {
	if (lxtyp == TokTypeVanilla) ntokens++;
	if (!do_bench) {
	  printf("Token typ=%s , text=`%s'\n",
		 mootTokenTypeNames[lxtyp], lexer.mtoken->text().c_str());
	}
      }
    //printf("%%%%--EOF--\n");

    if (do_bench) {
      DOTEST_STOP ;
      dotest_report((*argv ? *argv : "-"), ntokens);
    }

    if (infile != stdin) fclose(infile);

    //-- check memory
    if (checkMemory) {
      fprintf(stderr, "Check memory usage ? ");
      fgetc(stdin);
    }
  }
}

int main (int argc, char **argv)
{
  fprintf(stderr, "%s (%s v%s)\n", *argv, PACKAGE_NAME, PACKAGE_VERSION);

  if (checkMemory) {
    fprintf(stderr, "Check memory usage ? ");
    fgetc(stdin);
  }

  churntest((argc > 1 ? argc-1 : 1),
	    (argc > 1 ? argv+1 : (char**){ NULL }));

  return 0;
}