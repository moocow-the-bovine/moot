#include <stdio.h>

#include "mootConfig.h"
#include "mootLexfreqsParser.h"
#include "mootLexfreqsCompiler.h"

using namespace moot;

void churntest(int argc, char **argv) {
  FILE *infile = stdin;
    //-- init
  mootLexfreqsLexer lexer;

  //-- guts
  lexer.select_streams(infile,stdout);
  int lxtyp;
  YY_mootLexfreqsParser_STYPE yylval;
  YY_mootLexfreqsParser_LTYPE yylloc;
  for (lxtyp = lexer.yylex(&yylval,&yylloc); lxtyp != 0; lxtyp = lexer.yylex(&yylval,&yylloc)) {
      const char *typestr = "OTHER";
      if      (lxtyp==mootLexfreqsParser::COUNT) { typestr="COUNT"; }
      else if (lxtyp==mootLexfreqsParser::TOKEN) { typestr="TOKEN"; }
      fprintf(stderr, "yylex() -> %d:%s ~ \"%s\"\n", lxtyp, typestr, lexer.yytext);
  }
}

int main (int argc, char **argv)
{
  char *argv_null[1] = {NULL};
  fprintf(stderr, "%s (%s v%s)\n", *argv, PACKAGE_NAME, PACKAGE_VERSION);

  churntest((argc > 1 ? argc-1 : 1),
	    (argc > 1 ? argv+1 : argv_null));

  return 0;
}
