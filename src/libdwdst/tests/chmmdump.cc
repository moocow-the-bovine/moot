#include <stdio.h>
#include <string.h>

#include <string>

#include "dwdstLexfreqs.h"
#include "dwdstNgrams.h"
#include "dwdstMorph.h"
#include "dwdstHMM.h"
#include "dwdstCHMM.h"

dwdstCHMM    hmm;


int main (int argc, char **argv) {
  char *progname = *argv;
  if (argc < 2) {
    fprintf(stderr, "Usage: %s CHMMFILE\n", progname);
    exit(1);
  }
  char *chmmfile = *(++argv);

  //-- load chmm
  fprintf(stderr, "%s: loading CHMM from '%s'... ", progname, chmmfile);
  if (!hmm.load(chmmfile)) {
    fprintf(stderr, "FAILED.\n");
    exit(2);
  }
  fprintf(stderr, "loaded.\n");



  fprintf(stderr, "Done. (Press return to dump) ");
  fgetc(stdin);

  hmm.txtdump(stdout);
  return 0;
}
