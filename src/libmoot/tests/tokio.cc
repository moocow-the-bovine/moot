#include <stdio.h>
#include "mootToken.h"
#include "mootTokenLexer.h"
#include "mootTokenIO.h"

using namespace moot;

void churntest(int argc, char **argv) {
  char *filename = "-";
  if (argc > 1) { filename = argv[1]; }
  mifstream mif(filename);
  if (!mif.valid()) {
    fprintf(stderr, "open failed for '%s'\n", filename);
    exit(1);
  }

  int fmt = (tiofNative
	     | tiofWellDone
	     );
  TokenReaderNative tr(fmt);
  tr.from_mstream(&mif);

  mootSentence *sent;
  int lxtyp;
  while ((lxtyp = tr.get_sentence()) != TokTypeEOF) {
    sent = tr.sentence();

    printf("SENTENCE:\n");
    for (mootSentence::const_iterator si = sent->begin();
	 si != sent->end();
	 si++)
      {
	printf("\t+ TOKEN: type=`%s' ; toktext=`%s'\t ; besttag=`%s'\n",
	       mootTokenTypeNames[si->toktype()],
	       si->text().c_str(), si->besttag().c_str());
	for (mootToken::Analyses::const_iterator ai = si->analyses().begin();
	     ai != si->analyses().end();
	     ai++)
	  {
	    printf("\t  - ANALYSIS: tag=`%s'\t ; details=`%s'\t\n",
		   ai->tag.c_str(), ai->details.c_str());
	  }
      }
  }
  printf("EOF\n");
}

int main (int argc, char **argv) {

  //for (int i = 0; i < 10; i++)
  churntest(argc, argv);

  return 0;
}
