#include <stdio.h>
#include "mootToken.h"
#include "mootTokenXmlDoc.h"

using namespace moot;

void churntest(int argc, char **argv) {
  TokenXmlDoc doc;

  if (argc > 1)
    doc.loadFilename(argv[1]);
  else
    doc.loadFilename("-");

  int typ;
  mootSentence sent;
  while ((typ = doc.get_sentence()) != TokTypeEOF) {
    mootSentence &sent = doc.sentence();

    printf("SENTENCE:\n");
    for (mootSentence::const_iterator si = sent.begin();
	 si != sent.end();
	 si++)
      {
	printf("\t+ TOKEN: type=`%s' ; toktext=`%s'\t ; besttag=`%s'\n",
	       mootTokenTypeNames[si->toktype()],
	       si->text().c_str(),
	       si->besttag().c_str());
	for (mootToken::AnalysisSet::const_iterator ai = si->analyses().begin();
	     ai != si->analyses().end();
	     ai++)
	  {
	    printf("\t  - ANALYSIS: cost=%g\t ; tag=`%s'\t ; details=`%s'\t\n",
		   ai->cost,
		   ai->tag.c_str(),
		   ai->details.c_str());
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
