#include <stdio.h>
#include <string.h>

#include <string>

#include "mootLexfreqs.h"
#include "mootNgrams.h"
#include "mootHMM.h"
#include "mootTaggerLexer.h"
#include "mootEnum.h"
#include "mootTypes.h"

using namespace std;

mootLexfreqs lf;
mootNgrams   ng;
mootHMM     hmm;
mootTaggerLexer lexer;

#define ESTIMATE_LAMBDAS 1

void hmm_vdump(const char *token, bool dostep=true) {
  mootTokString tokstr = token;
  mootEnumID    tokid = hmm.tokids.name2id(tokstr);
  printf("----------------Token: '%s'  ; id=%u ; name='%s'\n",
	 tokstr.c_str(), tokid, hmm.tokids.id2name(tokid).c_str());
  if (dostep) {
    hmm.viterbi_step(tokstr);
  }
  hmm.viterbi_txtdump(stdout);
  printf("\n");
}


vector<string> tokens;
void dump_best(bool force_eos=false) {
  //-- print intermediate best path

  mootHMM::ViterbiPath path;
  if (force_eos) {
    tokens.push_back("(EOS)");
    path = hmm.viterbi_best_path(hmm.start_tagid);
  } else {
    path = hmm.viterbi_best_path();
  }

  printf("\n------------------------------------------------\n");
  printf("Best Path (p=%g)", path.size() == 0 ? 0 : path.back()->prob);
  printf("\nvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n");

  unsigned i;
  for (i = 0; i < tokens.size(); i++) {
    if (i >= path.size()) {
      printf("--------Error: no tag for %u-th token '%s'!\n", i, tokens[i].c_str());
      continue;
    }
    printf("%s\t%s\n",
	   tokens[i].c_str(),
	   hmm.tagids.id2name(path[i]->tagid).c_str());
  }
  printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n\n");
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

  //-- report add_one_hack status
  printf("%s: moot_ADD_ONE_HACK %s\n",
	 progname,
#ifdef moot_ADD_ONE_HACK
	 "enabled"
#else
	 "NOT enabled"
#endif
	 );

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
  if (!hmm.compile(lf,ng,"__$",1)) {
    fprintf(stderr, "FAILED.\n");
    exit(3);
  }
  fprintf(stderr, "compiled.\n");

  //-- set lambdas
#ifndef ESTIMATE_LAMBDAS
  fprintf(stderr, "%s: NOT estimating lambdas.\n", progname);
  hmm.nglambda1 = 0.0;
  hmm.nglambda2 = 1.0;
  hmm.wlambda1  = 1.0;
  hmm.wlambda2  = 0.0;
#endif /* ESTIMATE_LAMBDAS */

  //--report
  fprintf(stderr, "%s: start_tag=%u(\"%s\")\n",
	  progname, hmm.start_tagid, hmm.tagids.id2name(hmm.start_tagid).c_str());
  fprintf(stderr, "%s: wlambda1=%g\n", progname, hmm.wlambda1);
  fprintf(stderr, "%s: wlambda2=%g\n", progname, hmm.wlambda2);
  fprintf(stderr, "%s: nglambda1=%g\n", progname, hmm.nglambda1);
  fprintf(stderr, "%s: nglambda2=%g\n", progname, hmm.nglambda2);


  fprintf(stderr, "%s: reading input:\n", progname);

  //--prepare lexer
  lexer.step_streams(infile,stdout);
  lexer.theLine = 1;
  lexer.theColumn = 0;

  //--prepare tokens
  tokens.push_back("(BOS)");
  hmm.viterbi_clear();

  //--preliminary dump
  hmm_vdump("--BOF--", false);

  //--prepare vars
  int tok;
  while ((tok = lexer.yylex()) != mootTaggerLexer::DTEOF) {
      switch (tok) {
      case mootTaggerLexer::EOS:
	//hmm.viterbi_step(0,hmm.start_tagid);
	hmm.viterbi_finish();
	hmm_vdump("(EOS)", false);
	
	dump_best(true);
	tokens.clear();
	tokens.push_back("(BOS)");

	hmm.viterbi_clear();
	hmm_vdump("(BOS)", false);
	break;

      case mootTaggerLexer::TOKEN:
	hmm_vdump((const char *)lexer.yytext, true);
	tokens.push_back((const char *)lexer.yytext);

	dump_best();
	break;

      default:
	fprintf(stderr, "%s: Error: unknown token '%s' at line %d, column %d\n",
		progname, lexer.yytext, lexer.theLine, lexer.theColumn);
	break;
      }
  }

  return 0;
}


