#include <stdio.h>
#include "dwdstTaggerLexer.h"

dwdstTaggerLexer lexer;

int main (int argc, char **argv) {
  lexer.step_streams(stdin,stdout);
  int tok;
  while ( (tok = lexer.yylex()) ) {
    switch (tok) {
    case dwdstTaggerLexer::TOKEN:
      printf("TOKEN: '%s'\n", lexer.yytext);
      break;
    case dwdstTaggerLexer::TAG:
      printf("TAG: '%s'\n", lexer.yytext);
      break;
    case dwdstTaggerLexer::EOT:
      printf("EOT: '%s'\n", lexer.yytext);
      break;
    case dwdstTaggerLexer::EOS:
      printf("EOS: '%s'\n", lexer.yytext);
      break;
    case dwdstTaggerLexer::DTEOF:
      printf("DTEOF: '%s'\n", lexer.yytext);
      break;
    default:
      printf("??? tok=%d ???: '%s'\n", tok, lexer.yytext);
      break;
    }
  }
  return 0;
}
