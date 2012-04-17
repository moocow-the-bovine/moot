#include <assert.h>
#include <stdio.h>
#include "mootToken.h"
#include "mootTokenIO.h"

using namespace moot;

//#define TRACE

void churntest(int argc, char **argv) {
  const char *ifilename = "-";
  const char *ofilename = "-";
  if (argc > 1) ifilename = argv[1];
  if (argc > 2) ofilename = argv[2];

  TokenReader *tr = TokenIO::file_reader(ifilename,NULL,tiofWellDone);
  TokenWriter *tw = TokenIO::file_writer(ofilename,NULL,tiofWellDone);
  assert(tr!=NULL);
  assert(tw!=NULL);

#ifdef TRACE
  //-- report
  fprintf(stderr, "%s: reader format = %s\n", *argv, TokenIO::format_canonical_string(tr->tr_format).c_str());
  fprintf(stderr, "%s: writer format = %s\n", *argv, TokenIO::format_canonical_string(tw->tw_format).c_str());
#endif

  int rtok = TokTypeUnknown;
  while (rtok != TokTypeEOF) {
    rtok=tr->get_token();
    mootToken& tok = *(tr->tr_token);
    tok.tok_besttag = "STREAMED";
#ifdef TRACE
    fprintf(stderr,
	    "TOKEN: type=`%s' ; text=`%s' ; besttag=`%s'\n",
	    mootTokenTypeNames[rtok],
	    (rtok==TokTypeEOF ? "" : tok.tok_text.c_str()),
	    (rtok==TokTypeEOF ? "" : tok.tok_besttag.c_str())
	    );
#endif
    tw->put_token(tok);
  }

  delete tr;
  delete tw;
}

int main (int argc, char **argv) {

  //for (int i = 0; i < 10; i++)
  churntest(argc, argv);

  return 0;
}
