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
    size_t bufsize =
      TokenReader::TR_DEFAULT_BUFSIZE //-- default
      //1
      //4096
      ;
    int tr_format = (tiofNative
		     | tiofWellDone
		     );
    int tw_format = tr_format;
    TokenReaderNative tr(tr_format,bufsize);
    TokenWriterNative tw(tw_format,bufsize);


    DOTEST_START ;
    size_t bufsize_initial = tr.tr_buffer.capacity();

    //-- guts
    size_t nfills = 0;
    size_t ntokens = 0;
    tr.fromFile(infile);
    tw.toFile(stdout);
    for (tr.tr_buffer_fill(); !tr.tr_buffer.empty(); tr.tr_buffer_fill()) {
      nfills++;

      //-- count tokens
      for (TokenIOBuffer::const_iterator bufi = tr.tr_buffer.begin();
	   bufi != tr.tr_buffer.end();
	   bufi++)
	{
	  if (bufi->toktype() == TokTypeVanilla) ntokens++;
	}

      //-- flush buffer
      tw.put_buffer(tr.tr_buffer);
      tr.tr_buffer_clear();
    }
    printf("%%%%--EOF--\n");
    fprintf(stderr,
	    "%%%% Read-buffer: initial_size / final_size / nfills: %u / %u / %u\n",
	    bufsize_initial, tr.tr_buffer.capacity(), nfills+1);

    //-- cleanup
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
