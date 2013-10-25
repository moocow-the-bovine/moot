#include <mootCIO.h>
#include <wasteLexer.h>

//======================================================================
int main ( int argc, char* argv[] )
{
  const char *prog = argv[0], *word=NULL, *lexfile="-";
  moot::wasteLexicon lex;

  //-- command-line: help
  if (argc <= 1
      || (argc >= 2 && (strcmp(argv[1],"-h")==0 || strcmp(argv[1],"--help")==0)) )
    {
      fprintf(stderr, "Usage: %s WORD [LEXFILE=-]\n", prog);
      exit(0);
    }

  //-- command-line: test word
  word = argv[1];

  //-- command-line: lex file
  if (argc > 2)
    lexfile = argv[2];
  if (!lex.load(lexfile)) {
    fprintf(stderr, "%s: failed to load lexicon from `%s': %s\n", prog, lexfile, strerror(errno));
    exit(1);
  }

  //-- test
  printf("lookup(\"%s\", lexfile(%s)) = %d\n", word, lexfile, (lex.lookup(word) ? 1 : 0));

  return 0;
}
