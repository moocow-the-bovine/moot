#include <stdio.h>

#include "mootConfig.h"
#include "mootToken.h"
#include "mootTokenLexer.h"
#include "mootTokenIO.h"

#include "dotest.h"

using namespace moot;

void churntest(int argc, char **argv) {
  FILE *infile = stdin;
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

    //-- init
    int tr_format = (tiofNative
		     | tiofWellDone
		     );
    int tw_format = tr_format;

    TokenReaderNative tr(tr_format);
    TokenWriterNative tw(tw_format);

    DOTEST_START ;

    //-- guts
    tr.fromFile(infile);
    tw.toFile(stdout);
    int lxtyp;
    for (lxtyp = tr.get_sentence(); lxtyp != TokTypeEOF; lxtyp = tr.get_sentence())
      {
	mootSentence &sent = tr.sentence();
	tw.put_sentence(sent);
      }

    tw.tw_buffer_flush();
    printf("%%%%--EOF--\n");

    DOTEST_STOP ;
    dotest_report((*argv ? *argv : "-"), ntokens);

    if (infile != stdin) fclose(infile);
  }
}

int main (int argc, char **argv)
{
  fprintf(stderr, "%s (%s v%s)\n", *argv, PACKAGE_NAME, PACKAGE_VERSION);
  churntest((argc > 1 ? argc-1 : 1),
	    (argc > 1 ? argv+1 : (char**){ NULL }));

  return 0;
}
