#include <stdio.h>
//#include "mootToken.h"
#include "mootTokenLexer.h"

using namespace moot;

#define typnames mootTokFlavorNames

void churntest(int argc, char **argv) {
  mootTokenLexer lex;
  FILE *infile = stdin;
  int
    typ,
    eoftyp = TF_EOF;

  if (argc > 1) {
    infile = fopen(argv[1], "r");
  }
  if (!infile) {
    fprintf(stderr, "open failed for '%s'\n", argv[1]);
    exit(1);
  }

  lex.select_streams(infile,stdout);

  while ((typ = lex.yylex()) != eoftyp) {
    printf("TOKEN (%d=%s): `%s' (tok.txt=`%s')\n",
	   typ, typnames[typ], lex.yytext, lex.mtoken.text().c_str());
  }
  printf("EOF\n");
}

int main (int argc, char **argv) {

  //for (int i = 0; i < 10; i++)
  churntest(argc, argv);

  return 0;
}
