#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dwdstHMMTrainer.h"

char *progname = NULL;
int main (int argc, char **argv)
{
  progname = *argv;
  if (argc < 3) {
    fprintf(stderr, "Usage: %s MODEL SRC\n", progname);
    exit(1);
  }
  char *model = *(++argv);
  char *src = *(++argv);

  dwdstHMMTrainer hmmt;
  char *lexfile = (char *)malloc(strlen(model)+4);
  char *ngrfile = (char *)malloc(strlen(model)+4);
  strcpy(lexfile, model);
  strcpy(ngrfile, model);
  strcat(lexfile, ".lex");
  strcat(ngrfile, ".123");

  //-- train
  fprintf(stderr, "%s: training from '%s'...", progname, src);
  if (!hmmt.train_from_file(src)) {
    fprintf(stderr, " FAILED.\n");
    exit(1);
  }
  fprintf(stderr, " done.\n");

  //-- save
  fprintf(stderr, "%s: saving data to model files '%s' and '%s'...", progname, lexfile, ngrfile);
  if (! (hmmt.lexfreqs.save(lexfile) && hmmt.ngrams.save(ngrfile, false)) ) {
    fprintf(stderr, " FAILED.\n");
  }
  fprintf(stderr, " saved.\n");
  
  return 0;
}
