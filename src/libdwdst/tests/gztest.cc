#include <stdio.h>
#include <zlib.h>

char *progname = NULL;

int main (int argc, char **argv) {
  progname = *argv;
  if (--argc == 0) {
    fprintf(stderr, "Usage: %s FILENAME\n", progname);
    exit(1);
  }
  char *infilename = *(++argv);
  FILE *infile = fopen(infilename, "r");

  //-- try gzopen
  gzFile gzf = gzdopen(fileno(infile), "rb");
  if (!gzf) {
    fprintf(stderr, "%s: gzopen() failed for %s\n", progname, infilename);
    exit(1);
  }

  //-- write data
  

  return 0;
}
