#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mootToken.h>

using namespace std;
using namespace moot;

const size_t ntoks = 35311;

mootToken token;

void dotok(const char *filename)
{
  FILE *in = stdin;
  if (filename) {
    in = fopen(filename,"r");
    if (!in) {
      fprintf(stderr,"Error: could not open file!\n");
      exit(1);
    }
  }

  //-- guts
  char *line=NULL;
  size_t line_alloc = 0;
  for (getline(&line,&line_alloc,in); !feof(in); getline(&line,&line_alloc,in)) {
    mootToken &t = token;
    t.textAppend((const char *)line);
    t.insert(mootToken::Analysis((const char *)line));
    t.clear();
  }
  if (in != stdin) fclose(in);
}

int main (int argc, char **argv)
{
  fprintf(stderr, "(PRE): Check memory usage ? "); fgetc(stdin);
  dotok(argc>1 ? argv[1] : NULL);

  fprintf(stderr, "(POST-1): Check memory usage ? "); fgetc(stdin);
  dotok(argc>1 ? argv[1] : NULL);

  fprintf(stderr, "(POST-2): Check memory usage ? "); fgetc(stdin);
  dotok(argc>1 ? argv[1] : NULL);

  //fprintf(stderr, "(POST-3): Check memory usage ? "); fgetc(stdin);
  //dotok(argc>1 ? argv[1] : NULL);

  return 0;
}
