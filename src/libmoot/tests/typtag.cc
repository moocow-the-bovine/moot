#include "../libmoot/mootTypes.h"
#include "../libmoot/mootTokenIO.h"

using namespace moot;

int main(int argc, char **argv) {
  TokenReader tr;
  TokenWriter tw;
  FILE *infile = stdin;

  tr.lexer.first_analysis_is_best = true;
  tr.lexer.ignore_first_analysis = false;

  if (argc > 1) {
    infile = fopen(argv[1], "r");
  }
  if (!infile) {
    fprintf(stderr, "open failed for '%s'\n", argv[1]);
    exit(1);
  }

  tr.select_stream(infile);

  mootSentence sent;
  do {
    sent = tr.get_sentence();
    for (mootSentence::iterator si = sent.begin(); si != sent.end(); si++) {
      //si->tok_analyses.clear();
      si->besttag(TokenTypeNames[token2type(si->text())]);
    }
    for (mootSentence::const_iterator si2 = sent.begin(); si2 != sent.end(); si2++) {
      fprintf(stdout, "%s\n", tw.token_string(*si2).c_str());
    }
  } while (!sent.empty() || !feof(infile));

  return 0;
}
