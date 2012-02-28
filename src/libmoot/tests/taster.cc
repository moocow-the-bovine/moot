#include "../mootTypes.h"
#include "../mootTokenIO.h"
#include "mootFlavor.h"

using namespace moot;

int main(int argc, char **argv) {
  TokenReaderNative tr;
  TokenWriterNative tw;
  mootTaster tas("@ALPHA");
  FILE *infile = stdin;

  tr.lexer.first_analysis_is_best = true;
  tr.lexer.ignore_first_analysis  = false;

  if (argc <= 1) {
    fprintf(stderr, "Usage: %s FLAVORS [INFILE]\n", *argv);
    exit(1);
  }
  tas.load(argv[1]);
  fprintf(stderr, "%s: loaded %u rules from file '%s'\n", *argv, tas.size(), argv[1]);

  if (argc > 2) {
    infile = fopen(argv[2], "r");
  }
  if (!infile) {
    fprintf(stderr, "open failed for input file '%s'\n", argv[2]);
    exit(1);
  }
  tr.from_file(infile);
  tw.to_file(stdout);

  while (tr.get_sentence() != TokTypeEOF) {
    mootSentence *sent = tr.tr_sentence;
    for (mootSentence::iterator si = sent->begin(); si != sent->end(); si++) {
      //si->tok_analyses.clear();
      const std::string &f = tas.match(si->text());
      si->besttag(f);
    }
    tw.put_sentence(*sent);
  }

  return 0;
}
