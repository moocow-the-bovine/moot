#include <stdio.h>
#include <string.h>

#include <string>

#include "mootLexfreqs.h"
#include "mootNgrams.h"
#include "mootHMM.h"

mootHMM     hmm;

int main (int argc, char **argv) {
  char *progname = *argv;
  if (argc < 2) {
    fprintf(stderr, "Usage: %s HMM_BINFILE\n", progname);
    exit(1);
  }
  char *hmmfile = *(++argv);

  //-- load hmm
  fprintf(stderr, "%s: loading binary HMM from '%s'... ", progname, hmmfile);
  if (!hmm.load(hmmfile)) {
    fprintf(stderr, "FAILED.\n");
    exit(2);
  }
  fprintf(stderr, "loaded.\n");

  fprintf(stderr, "Done. (Press return to dump) ");
  fgetc(stdin);

  hmm.txtdump(stdout);
  return 0;
}
