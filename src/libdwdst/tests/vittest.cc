#include <stdio.h>
#include <string.h>

#include <string>

#include "dwdstLexfreqs.h"
#include "dwdstNgrams.h"
#include "dwdstHMM.h"
#include "dwdstTaggerLexer.h"
#include "dwdstEnum.h"
#include "dwdstTypes.h"

using namespace std;

dwdstLexfreqs lf;
dwdstNgrams   ng;
dwdstHMM     hmm;
dwdstTaggerLexer lexer;

void hmm_vdump(const char *token, bool dostep=true) {
  dwdstTokString tokstr = token;
  dwdstEnumID    tokid = hmm.tokids.name2id(tokstr);
  printf("--Token: '%s'  ; id=%u ; name='%s'\n",
	 tokstr.c_str(), tokid, hmm.tokids.id2name(tokid).c_str());
  if (dostep) {
    hmm.viterbi_step(tokstr);
  }
  hmm.viterbi_txtdump(stdout);
  printf("\n");
}

int main (int argc, char **argv) {
  char *progname = *argv;
  if (argc < 3) {
    fprintf(stderr, "Usage: %s LEXFREQS NGRAMS [TNTFILE]\n", progname);
    exit(1);
  }
  char *lexfile = *(++argv);
  char *ngfile  = *(++argv);
  char *infilename  = "-";
  FILE *infile = stdin;
  if (argc > 3) {
    infilename = *(++argv);
    infile = fopen(infilename, "r");
    if (!infile) {
      fprintf(stderr, "%s: Error: open failed for input-file '%s'\n", progname, infilename);
      exit(1);
    }
  }

  //-- load lexfreqs
  fprintf(stderr, "%s: loading lexical frequencies from '%s'... ",
	  progname, lexfile);
  if (!lf.load(lexfile)) {
    fprintf(stderr, "FAILED.\n");
    exit(2);
  }
  fprintf(stderr, "loaded.\n");

  //-- load ngrams
  fprintf(stderr, "%s: loading ngram frequencies from '%s'... ",
	  progname, ngfile);
  if (!ng.load(ngfile)) {
    fprintf(stderr, "FAILED.\n");
    exit(3);
  }
  fprintf(stderr, "loaded.\n");

  //-- compile HMM
  fprintf(stderr, "%s: compiling HMM... ", progname);
  if (!hmm.compile(lf,ng,"EOS",1)) {
    fprintf(stderr, "FAILED.\n");
    exit(3);
  }
  fprintf(stderr, "compiled.\n");

  //-- set lambdas
  hmm.lambda1 = 0.5;
  hmm.lambda2 = 0.5;

  //--report
  fprintf(stderr, "%s: start_tag=%u(\"%s\")\n",
	  progname, hmm.start_tagid, hmm.tagids.id2name(hmm.start_tagid).c_str());

  fprintf(stderr, "%s: reading input:\n", progname);
  //--prepare lexer
  lexer.step_streams(infile,stdout);
  lexer.theLine = 1;
  lexer.theColumn = 0;

  //--preliminary dump
  hmm_vdump("--BOF--", false);

  //--prepare vars
  int tok;
  while ((tok = lexer.yylex()) != dwdstTaggerLexer::DTEOF) {
      switch (tok) {
      case dwdstTaggerLexer::EOS:
	hmm.viterbi_step(0,hmm.start_tagid);
	hmm_vdump("(EOS)", false);
	printf("\n\n--EOS--\n\n");
	hmm.viterbi_clear();
	hmm_vdump("(BOS)", false);
	break;
      case dwdstTaggerLexer::TOKEN:
	hmm_vdump((const char *)lexer.yytext, true);
	break;
      default:
	fprintf(stderr, "%s: Error: unknown token '%s' at line %d, column %d\n",
		progname, lexer.yytext, lexer.theLine, lexer.theColumn);
	break;
      }
  }


  return 0;
}


