/*--------------------------------------------------------------------------
 * File: dwdspp.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + Tokenizer for KDWDS project : the guts
 *--------------------------------------------------------------------------*/

#include "dwdspp.h"
#include "dwdspp_lexer.h"

using namespace std;

bool dwdspp_tokenize_stream(FILE *in, FILE *out)
{
  class dwdspp_lexer lexer;
  int tok;
  lexer.yyin = in;
  lexer.yyout = out;
  while ((tok = lexer.yylex()) != PPEOF) {
      switch (tok) {
      case EOS:
	  if (strcmp((char *)lexer.yytext,".") == 0) {
	      fprintf(out, "%s\n", lexer.yytext);
	  } else {
	      fprintf(out, "\n");
	  }
	  break;

      case  START_XML_TAG:
      case END_XML_TAG:
	  /* ignore XML tags */
	  break;

      default:
	  /* write it as its own token */
	  fprintf(out, "%s ", lexer.yytext);
	  //fwrite((char *)lexer.yytext, lexer.yyleng, 1, out);
      }
  }
  fprintf(out, "\n");
  return true;
}
