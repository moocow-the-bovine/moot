#include <stdio.h>

#include "mootMorph.h"

int main (int argc, char **argv) {
  char *progname = *(argv++);
  argc--;

  mootMorph morph;
  fprintf(stderr, "%s: intializing... ", progname);
  if (!(morph.load_morph_symbols("moot.sym")
	&& morph.load_morph_fst("moot.fst")
	&& morph.load_tagx_fst("tagx.fst")))
    {
      fprintf(stderr, "\nInitialization FAILED.\n");
      exit(1);
    }
  fprintf(stderr, "done.\n");

  morph.tag_strings(argc, argv, stdout, "(stdin)");
  return 0;
}
