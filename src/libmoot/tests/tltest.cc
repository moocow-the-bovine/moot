#include <stdio.h>
#include "mootTaggerLexer.h"

mootTaggerLexer lexer;

int main (int argc, char **argv) {
  lexer.step_streams(stdin,stdout);
  int tok;
  while ( (tok = lexer.yylex()) ) {
    switch (tok) {
    case mootTaggerLexer::TOKEN:
      printf("TOKEN: '%s'\n", lexer.yytext);
      break;
    case mootTaggerLexer::TAG:
      printf("TAG: '%s'\n", lexer.yytext);
      break;
    case mootTaggerLexer::EOT:
      printf("EOT: '%s'\n", lexer.yytext);
      break;
    case mootTaggerLexer::EOS:
      printf("EOS: '%s'\n", lexer.yytext);
      break;
    case mootTaggerLexer::DTEOF:
      printf("DTEOF: '%s'\n", lexer.yytext);
      break;
    default:
      printf("??? tok=%d ???: '%s'\n", tok, lexer.yytext);
      break;
    }
  }
  return 0;
}
