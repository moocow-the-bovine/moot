#include <stdio.h>
#include "mootToken.h"
#include "mootTokenLexer.h"
#include "mootTokenIO.h"

using namespace moot;

void churntest(int argc, char **argv) {
  TokenReader tr;
  FILE *infile = stdin;

  tr.lexer.first_analysis_is_best = true;
  tr.lexer.ignore_first_analysis = true;

#ifdef MOOT_TOKEN_VERSION
  printf("MOOT_TOKEN_VERSION=%d\n", MOOT_TOKEN_VERSION);
  printf("MOOT_TOKEN_REVISION=%d\n", MOOT_TOKEN_REVISION);
#else
  printf("MOOT_TOKEN_VERSION undefined!\n");
#endif

  if (argc > 1) {
    infile = fopen(argv[1], "r");
  }
  if (!infile) {
    fprintf(stderr, "open failed for '%s'\n", argv[1]);
    exit(1);
  }

  tr.select_stream(infile);

  mootToken tok;
  mootTokenLexer::TokenType typ;
  while ((typ = tr.get_token()) != mootTokenLexer::TLEOF) {
    if (typ == mootTokenLexer::TLEOS) continue;
    tok = tr.token();

    printf("+ TOKEN: toktext=`%s'\t ; besttag=`%s'\n",
	   tok.text().c_str(), tok.besttag().c_str());
    for (mootToken::AnalysisSet::const_iterator ai = tok.analyses().begin();
	 ai != tok.analyses().end();
	 ai++)
      {
	printf("\t- ANALYSIS: cost=%g\t ; tag=`%s'\t ; details=`%s'\n",
	       ai->cost, ai->tag.c_str(), ai->details.c_str());
      }

    //------- bounds
    mootToken::AnalysisSet::const_iterator
      bound = tok.lower_bound(tok.besttag());
    printf("  : LB(besttag())=");
    if (bound == tok.analyses().end()) { printf("end()\n"); }
    else {
      printf("{cost=%g\t ; tag=%s\t ; details=`%s'}\n",
	     bound->cost, bound->tag.c_str(), bound->details.c_str());
    }
    //--
    printf("  : UB(besttag())=");
    bound = tok.upper_bound(tok.besttag());
    if (bound == tok.analyses().end()) { printf("end()\n"); }
    else {
      printf("{cost=%g\t ; tag=%s\t ; details=`%s'}\n",
	     bound->cost, bound->tag.c_str(), bound->details.c_str());

    }

    //------- prune
    tok.prune();
    printf("  : PRUNED:\n");
    for (mootToken::AnalysisSet::const_iterator ai = tok.analyses().begin();
	 ai != tok.analyses().end();
	 ai++)
      {
	printf("\t- ANALYSIS: cost=%g\t ; tag=`%s'\t ; details=`%s'\n",
	       ai->cost, ai->tag.c_str(), ai->details.c_str());
      }
  }
  printf("EOF\n");
}

int main (int argc, char **argv) {

  //for (int i = 0; i < 10; i++)
  churntest(argc, argv);

  return 0;
}
