#include <stdio.h>

#include "mootConfig.h"
#include "mootToken.h"
#include "mootTokenLexer.h"
#include "mootTokenIO.h"

#include "dotest.h"
const bool checkMemory = false;

using namespace moot;

void churntest(int argc, char **argv) {
  for ( ; argc > 0 ; argc--, argv++) {
    const char *infilename = argv && *argv ? *argv : "-";

    //-- init
    int tr_format = (tiofNative
		     | tiofWellDone
		     );

    int tw_format = tr_format;

    TokenReaderNative tr(tr_format);
    TokenWriterNative tw(tw_format);

    DOTEST_START ;

    //-- guts
    //tr.from_file(infile);
    //tw.to_file(stdout);

    tr.from_filename(infilename);
    tw.to_filename("-");

    size_t ntokens = 0;
    int lxtyp;
    for (lxtyp = tr.get_sentence(); lxtyp != TokTypeEOF; lxtyp = tr.get_sentence())
      {
	mootSentence *sent = tr.sentence();
	tw.put_sentence(*sent);

	//-- count tokens
	for (mootSentence::const_iterator si = sent->begin(); si != sent->end(); si++)
	  if (si->toktype() == TokTypeVanilla) ntokens++;
      }
    printf("%%%%--EOF--\n");

    tr.close();
    tw.close();

    DOTEST_STOP ;
    dotest_report((*argv ? *argv : "-"), ntokens);


    //-- check memory
    if (checkMemory) {
      fprintf(stderr, "Check memory usage ? ");
      fgetc(stdin);
    }
  }
}

int main (int argc, char **argv)
{
  char *argv_default[1] = { NULL };
  fprintf(stderr, "%s (%s v%s)\n", *argv, PACKAGE_NAME, PACKAGE_VERSION);

  if (checkMemory) {
    fprintf(stderr, "Check memory usage ? "); fgetc(stdin);
  }

  churntest((argc > 1 ? argc-1 : 1),
	    (argc > 1 ? argv+1 : argv_default));

  return 0;
}
