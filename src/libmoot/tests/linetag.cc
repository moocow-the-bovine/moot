#include "../libmoot/mootTypes.h"
#include "../libmoot/mootTokenIO.h"

using namespace moot;

int main(int argc, char **argv) {
  TokenReaderNative tr;
  TokenWriterNative tw;
  FILE *infile = stdin;

  tr.lexer.first_analysis_is_best = true;
  tr.lexer.ignore_first_analysis  = true;

  if (argc > 1) {
    infile = fopen(argv[1], "r");
  }
  if (!infile) {
    fprintf(stderr, "open failed for '%s'\n", argv[1]);
    exit(1);
  }

  tr.from_file(infile);
  tw.to_file(stdout);

  mootTokenType typ;
  long prevbyte = tr.byte_number();
  for (typ=tr.get_token(); typ != TokTypeEOF; typ=tr.get_token()) {
    mootToken *tok = tr.token();
    long byte = tr.byte_number();
    long len  = byte-prevbyte;
    printf("[line=%u, col=%u, byte=%ld, len=%ld] ",
	   tr.line_number(), tr.column_number(), byte-len, len);
    tw.put_token(*tok);
    prevbyte = byte;
  }

  tr.close();
  tw.close();

  return 0;
}
