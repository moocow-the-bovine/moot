#include <stdio.h>
#include "mootToken.h"
#include "mootTokenLexer.h"
#include "mootTokenIO.h"

using namespace moot;

void churntest(int argc, char **argv) {
  TokenReader tr;
  FILE *infile = stdin;

  tr.lexer.first_analysis_is_best = true;
  tr.lexer.ignore_first_analysis = true;

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
    printf("SENTENCE:\n");
    for (mootSentence::const_iterator si = sent.begin();
	 si != sent.end();
	 si++)
      {
	printf("\t+ TOKEN: toktext=`%s'\t ; besttag=`%s'\n",
	       si->text().c_str(), si->besttag().c_str());
	for (mootToken::AnalysisSet::const_iterator ai = si->analyses().begin();
	     ai != si->analyses().end();
	     ai++)
	  {
	    printf("\t  - ANALYSIS: cost=%g\t ; tag=`%s'\t ; details=`%s'\t\n",
		   ai->cost, ai->tag.c_str(), ai->details.c_str());
	  }
      }
  } while (!sent.empty() || !feof(infile));
  printf("EOF\n");
}

int main (int argc, char **argv) {

  //for (int i = 0; i < 10; i++)
  churntest(argc, argv);

  return 0;
}
