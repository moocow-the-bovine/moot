#include <stdio.h>
#include "mootPPLexer.h"
#include "mootIO.h"

using namespace mootio;

int main (void) {
  mootPPLexer lexer;
  mcstream min(stdin);
  mcstream mout(stdout);

  lexer.select_input(&min);
  lexer.select_output(&mout);

  int tok;
  while ((tok = lexer.yylex()) != mootPPLexer::PPEOF) {
    switch (tok) {
    case mootPPLexer::EOS:
      mout.printf("%s\n\n", lexer.yytext);
      break;
    default:
      mout.printf("%s\n", (const char *)lexer.yytext);
      break;
    }
  }

  return 0;
}
