#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include <string>

#include "dwdstHMM.h"
#include "dwdstTaggerLexer.h"
#include "dwdstEnum.h"
#include "dwdstTypes.h"

using namespace std;

dwdstHMM     hmm;

timeval t1, t2, t3;

int main (int argc, char **argv) {
  char *progname = *argv;
  if (argc < 2) {
    fprintf(stderr, "Usage: %s HMMFILE [TNTFILE]\n", progname);
    exit(1);
  }

  gettimeofday(&t1, NULL);

  char *hmmfile = *(++argv);
  char *infilename  = "-";
  FILE *infile = stdin;
  if (argc > 2) {
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

  //-- load HMM
  fprintf(stderr, "%s: loading HMM from '%s'... ", progname, hmmfile);
  if (!hmm.load(hmmfile)) {
    fprintf(stderr, "FAILED.\n");
    exit(2);
  }
  fprintf(stderr, "loaded.\n");

  //--report
  fprintf(stderr, "%s: start_tag=%u(\"%s\")\n",
	  progname, hmm.start_tagid, hmm.tagids.id2name(hmm.start_tagid).c_str());
  fprintf(stderr, "%s: wlambda1=%g\n", progname, hmm.wlambda1);
  fprintf(stderr, "%s: wlambda2=%g\n", progname, hmm.wlambda2);
  fprintf(stderr, "%s: nglambda1=%g\n", progname, hmm.nglambda1);
  fprintf(stderr, "%s: nglambda2=%g\n", progname, hmm.nglambda2);
  fprintf(stderr, "%s: reading input... \n", progname);

  //-- tag
  gettimeofday(&t2, NULL);
  hmm.tag_stream(infile, stdout, infilename);

  //-- Summarize
  gettimeofday(&t3, NULL);

  double ielapsed = (t2.tv_sec-t1.tv_sec + (double)(t2.tv_usec-t1.tv_usec)/1000000.0);
  double relapsed = (t3.tv_sec-t2.tv_sec + (double)(t3.tv_usec-t2.tv_usec)/1000000.0);

  fprintf(stderr, "\n%s Summary:\n", progname);
  fprintf(stderr, "   Tokens Processed : %u tok\n", hmm.ntokens);
  fprintf(stderr, "    Initialize time : %.2f sec\n", ielapsed);
  fprintf(stderr, "    Processing time : %.2f sec\n", relapsed);
  fprintf(stderr, "         Throughput : %.2f tok/sec\n", (double)hmm.ntokens/relapsed);

  return 0;
}


