#include <stdio.h>
#include "mootToken.h"
#include "mootTokenIO.h"

using namespace moot;

void churntest(int argc, char **argv) {
  const char *filename = "-";
  if (argc > 1) filename = argv[1];

  TokenReader *tr = TokenIO::file_reader(filename);

  int rtok = TokTypeUnknown;
  while (rtok != TokTypeEOF) {
    rtok=tr->get_token();
    mootToken& tok = *(tr->tr_token);
    printf("TOKEN: type=`%s' ; text=`%s' ; besttag=`%s'\n",
	   mootTokenTypeNames[rtok],
	   (rtok==TokTypeEOF ? "" : tok.tok_text.c_str()),
	   (rtok==TokTypeEOF ? "" : tok.tok_besttag.c_str())
	   );
  }

  delete tr;
}

int main (int argc, char **argv) {

  //for (int i = 0; i < 10; i++)
  churntest(argc, argv);

  return 0;
}
