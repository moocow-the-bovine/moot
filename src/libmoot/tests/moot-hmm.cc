#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

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

timeval t1, t2, t3;

vector<string> tokens;


#define ESTIMATE_LAMBDAS 1
//#define USE_TNT_LAMBDAS  1
//#define USE_FLAT_WORDPROBS 1

#define UNKNOWN_LEX_THRESH 1
//#define UNKNOWN_LEX_THRESH 0

#define USE_TRIGRAMS


#ifndef ESTIMATE_LAMBDAS

# ifdef USE_FLAT_WORDPROBS
   const double WLAMBDA1 = 1.0;
   const double WLAMBDA2 = 0.0;
# else // USE_FLAT_WORDPROBS
# include <float.h>
   const double WLAMBDA1 = 1 - DBL_EPSILON;
   const double WLAMBDA2 = DBL_EPSILON;
# endif // USE_FLAT_WORDPROBS

# ifdef USE_TNT_LAMBDAS
const double NGLAMBDA1 = 0.1684092;
const double NGLAMBDA2 = 0.8315908;
# else
const double NGLAMBDA1 = 0.0;
const double NGLAMBDA2 = 1.0;
# endif // USE_TNT_LAMBDAS

#endif // ESTIMATE_LAMBDAS


void dump_best(void) {
  //-- print intermediate best path

  const mootHMM::ViterbiPath &path = hmm.viterbi_best_path();
  unsigned i;

  for (i = 0; i < tokens.size(); i++) {
    if (i >= path.size()) {
      fprintf(stderr, "--------Error: no tag for %u-th token '%s'!\n", i, tokens[i].c_str());
      printf("%s\t%s\n", tokens[i].c_str(), "<UNKNOWN>");
      continue;
    }
    printf("%s\t%s\n",
	   tokens[i].c_str(),
	   hmm.tagids.id2name(path[i+1]->tagid).c_str());
  }
  printf("\n");
}


int main (int argc, char **argv) {
  char *progname = *argv;
  if (argc < 3) {
    fprintf(stderr, "Usage: %s LEXFREQS NGRAMS [TNTFILE]\n", progname);
    exit(1);
  }

  gettimeofday(&t1, NULL);

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
  fprintf(stderr, "%s: moot_ADD_ONE_HACK %s\n",
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
  if (!hmm.compile(lf, ng, "__$", UNKNOWN_LEX_THRESH)) {
    fprintf(stderr, "FAILED.\n");
    exit(3);
  }
  fprintf(stderr, "compiled.\n");

  //-- set lambdas
#ifdef ESTIMATE_LAMBDAS
  fprintf(stderr, "%s: using estimated lambdas.\n", progname);
#else
  fprintf(stderr, "%s: NOT using estimated lambdas.\n", progname);
  hmm.nglambda1 = NGLAMBDA1;
  hmm.nglambda2 = NGLAMBDA2;
# ifdef USE_TRIGRAMS
   hmm.nglambda3 = NGLAMBDA3;
# endif /* USE_TRIGRAMS */
  hmm.wlambda1  = WLAMBDA1;
  hmm.wlambda2  = WLAMBDA2;
#endif /* ESTIMATE_LAMBDAS */

  //--report
  fprintf(stderr, "%s: start_tag=%u(\"%s\")\n",
	  progname, hmm.start_tagid, hmm.tagids.id2name(hmm.start_tagid).c_str());
  fprintf(stderr, "%s: unknownLexThreshhold=%u\n", progname, UNKNOWN_LEX_THRESH);
  fprintf(stderr, "%s: wlambda1=%g\n", progname, hmm.wlambda1);
  fprintf(stderr, "%s: wlambda2=%g\n", progname, hmm.wlambda2);
  fprintf(stderr, "%s: nglambda1=%g\n", progname, hmm.nglambda1);
  fprintf(stderr, "%s: nglambda2=%g\n", progname, hmm.nglambda2);
#ifdef USE_TRIGRAMS
  fprintf(stderr, "%s: nglambda3=%g\n", progname, hmm.nglambda3);
#endif
  fprintf(stderr, "%s: reading input... \n", progname);

  //--prepare lexer
  lexer.step_streams(infile,stdout);
  lexer.theLine = 1;
  lexer.theColumn = 0;

  //--prepare vars
  int tok;
  size_t ntoks = 0;

  gettimeofday(&t2, NULL);

  while ((tok = lexer.yylex()) != mootTaggerLexer::DTEOF) {
      switch (tok) {
      case mootTaggerLexer::EOS:
	hmm.viterbi_finish();
	dump_best();
	tokens.clear();
	hmm.viterbi_clear();
	break;

      case mootTaggerLexer::TOKEN:
	tokens.push_back((const char *)lexer.yytext);
	hmm.viterbi_step((const char *)lexer.yytext);
	ntoks++;
	break;

      default:
	fprintf(stderr,
		"%s: Error: unknown token '%s' at line %d, column %d\n",
		progname, lexer.yytext, lexer.theLine, lexer.theColumn);
	break;
      }
  }

  //-- Summarize
  gettimeofday(&t3, NULL);

  double ielapsed = (t2.tv_sec-t1.tv_sec + (double)(t2.tv_usec-t1.tv_usec)/1000000.0);
  double relapsed = (t3.tv_sec-t2.tv_sec + (double)(t3.tv_usec-t2.tv_usec)/1000000.0);

  fprintf(stderr, "\n%s Summary:\n", progname);
  fprintf(stderr, "   Tokens Processed : %u tok\n", ntoks);
  fprintf(stderr, "    Initialize time : %.2f sec\n", ielapsed);
  fprintf(stderr, "    Processing time : %.2f sec\n", relapsed);
  fprintf(stderr, "         Throughput : %.2f tok/sec\n", (double)ntoks/relapsed);

  return 0;
}


