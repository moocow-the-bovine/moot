#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include <string>

#include "dwdstLexfreqs.h"
#include "dwdstNgrams.h"
#include "dwdstMorph.h"
#include "dwdstCHMM.h"
#include "dwdstTaggerLexer.h"
#include "dwdstEnum.h"
#include "dwdstTypes.h"

using namespace std;

dwdstCHMM        chmm;
timeval t1, t2, t3;


int main (int argc, char **argv) {
  char *progname = *argv;
  if (argc < 4) {
    fprintf(stderr, "Usage: %s CHMMFILE SYMBOLS FST [TNTFILE]\n", progname);
    exit(1);
  }

  gettimeofday(&t1, NULL);

  char *chmmfile = *(++argv);
  char *symfile = *(++argv);
  char *fstfile = *(++argv);
  char *infilename  = "-";
  FILE *infile = stdin;
  if (argc > 4) {
    infilename = *(++argv);
    infile = fopen(infilename, "r");
    if (!infile) {
      fprintf(stderr, "%s: Error: open failed for input-file '%s'\n", progname, infilename);
      exit(1);
    }
  }

  //-- report add_one_hack status
  fprintf(stderr, "%s: DWDST_ADD_ONE_HACK %s\n",
	 progname,
#ifdef DWDST_ADD_ONE_HACK
	 "enabled"
#else
	 "NOT enabled"
#endif
	 );

  //-- load CHMM
  fprintf(stderr, "%s: loading CHMM file '%s'... ", progname, chmmfile);
  if (!chmm.load(chmmfile)) {
    fprintf(stderr, "FAILED.\n");
    exit(2);
  }
  fprintf(stderr, "loaded.\n");

  //-- load symbols
  fprintf(stderr, "%s: loading morphological symbols from '%s'... ",
	  progname, symfile);
  if (!chmm.morph.load_morph_symbols(symfile)) {
    fprintf(stderr, "FAILED.\n");
    exit(4);
  }
  fprintf(stderr, "loaded.\n");

  //-- load morphology
  fprintf(stderr, "%s: loading morphological FST from '%s'... ",
	  progname, fstfile);
  if (!chmm.morph.load_morph_fst(fstfile)) {
    fprintf(stderr, "FAILED.\n");
    exit(5);
  }
  fprintf(stderr, "loaded.\n");

  //-- report
  fprintf(stderr, "%s: reading input... \n", progname);

  //-- report to stream
  fprintf(stdout, "%%%% %s: wlambda1=%g\n", progname, chmm.wlambda1);
  fprintf(stdout, "%%%% %s: wlambda2=%g\n", progname, chmm.wlambda2);
  fprintf(stdout, "%%%% %s: nglambda1=%g\n", progname, chmm.nglambda1);
  fprintf(stdout, "%%%% %s: nglambda2=%g\n", progname, chmm.nglambda2);
#ifdef USE_TRIGRAMS
  fprintf(stdout, "%%%% %s: nglambda3=%g\n", progname, chmm.nglambda3);
#endif // USE_TRIGRAMS

  //-- tag
  gettimeofday(&t2, NULL);
  chmm.tag_stream(infile, stdout, infilename);

  //-- Summarize
  gettimeofday(&t3, NULL);

  double ielapsed = (t2.tv_sec-t1.tv_sec + (double)(t2.tv_usec-t1.tv_usec)/1000000.0);
  double relapsed = (t3.tv_sec-t2.tv_sec + (double)(t3.tv_usec-t2.tv_usec)/1000000.0);

  fprintf(stderr, "\n%s Summary:\n", progname);
  fprintf(stderr, "   Tokens Processed : %u tok\n", chmm.ntokens);
  fprintf(stderr, "    Tokens Analyzed : %u tok\n", chmm.morph.ntokens);
  fprintf(stderr, "     Morph-Unknowns : %u tok\n", chmm.morph.nunknown);
  fprintf(stderr, "   Recognition Rate : %.2f %%\n",
	  100.0*((double)chmm.morph.nunknown/(double)chmm.ntokens));
  fprintf(stderr, " MorphCache Lookups : %u tok\n", chmm.ntokens-chmm.morph.ntokens);
  fprintf(stderr, "    MorphCache Rate : %.2f %%\n",
	  100.0*(chmm.ntokens-chmm.morph.ntokens)/(float)chmm.ntokens);
  fprintf(stderr, "    Initialize time : %.2f sec\n", ielapsed);
  fprintf(stderr, "    Processing time : %.2f sec\n", relapsed);
  fprintf(stderr, "         Throughput : %.2f tok/sec\n", (double)chmm.ntokens/relapsed);

  return 0;
}
