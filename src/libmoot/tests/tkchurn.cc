#include <stdio.h>
#include "mootToken.h"
#include "mootTokenLexer.h"

using namespace moot;

char typnames[mootTokenLexer::TLNTYPES][64] =
    {
      "TLUNKNOWN",
      "TLEOF",
      "TLEOS",
      "TLTOKEN",
      "TLTEXT",
      "TLTAB",
      "TLTAG",
      "TLDETAILS",
      "TLCOST",
      "TLNEWLINE",
      "TLIGNORE"
    };


void churntest(int argc, char **argv) {
  mootTokenLexer lex;
  FILE *infile = stdin;
  int
    typ,
    eoftyp = mootTokenLexer::TLEOF;

  if (argc > 1) {
    infile = fopen(argv[1], "r");
  }
  if (!infile) {
    fprintf(stderr, "open failed for '%s'\n", argv[1]);
    exit(1);
  }

  lex.select_streams(infile,stdout);

  while ((typ = lex.yylex()) != eoftyp) {
    printf("TOKEN (%d=%s): `%s'\n", typ, typnames[typ], lex.yytext);
    if (typ == mootTokenLexer::TLTOKEN) {
      printf("   Text=`%s'\n", lex.mtoken.toktext.c_str());
      for (mootToken::AnalysisSet::const_iterator ai = lex.mtoken.analyses.begin();
	   ai != lex.mtoken.analyses.end();
	   ai++)
	{
	  printf("  Analysis: Tag=`%s' ; Details=`%s' ; Cost=%g\n",
		 ai->tag.c_str(), ai->details.c_str(), ai->cost);
	}
    }
  }
  printf("EOF\n");
}

int main (int argc, char **argv) {

  //for (int i = 0; i < 10; i++)
  churntest(argc, argv);

  return 0;
}
