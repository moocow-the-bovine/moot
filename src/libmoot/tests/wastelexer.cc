#include <iostream>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <wasteScanner.h>
#include <wasteLexer.h>
#include <wasteTypes.h>
#include <mootCxxIO.h>

using namespace moot;

//======================================================================
const char *token_type_name(int tok)
{
  return wasteLexerTypeNames[tok];
}

//======================================================================
int main ( int argc, char* argv[] )
{
  const char *prog = argv[0];
  std::ios_base::sync_with_stdio ( false );
  std::istream *is = &std::cin;
  std::ostream *os = &std::cout;
  TokenWriterNative writer(tiofMediumRare|tiofLocation);
  moot::wasteTokenScanner scanner;
  moot::wasteLexer lexer;
  lexer.from_reader(&scanner);

  //-- command-line: help
  if (argc > 1 && (strcmp(argv[1],"-h")==0 || strcmp(argv[1],"--help")==0)) {
    std::cerr << "Usage: " << prog << " [INFILE [OUTFILE]]\n";
    exit(0);
  }

  //-- command-line: input file
  if (argc > 1 && strcmp(argv[1],"-")!=0) {
    is = new std::ifstream(argv[1]);
    if (!is || !is->good()) {
      std::cerr << prog << ": open failed for input file `" << argv[1] << "': " << strerror(errno) << "\n";
      exit(1);
    }
  }

  //-- command-line: output file
  if (argc > 2 && strcmp(argv[2],"-")!=0) {
    os = new std::ofstream(argv[2], std::ofstream::out|std::ofstream::trunc);
    if (!os || !os->good()) {
      std::cerr << prog << ": open failed for output file `" << argv[2] << "': " << strerror(errno) << "\n";
      exit(1);
    }
  }

  //-- ye olde guttes
  mootio::micxxstream mis(is);
  mootio::mocxxstream mos(os);
  writer.to_mstream(&mos);
  scanner.from_mstream(&mis);
  int toktyp;
  while ( (toktyp=lexer.get_token()) != TokTypeEOF ) {
    writer.put_token( *(lexer.token()) );
  }
  writer.printf_comment("$EOF\t%lu 0\tEOF\n", lexer.byte_number() );

  //-- cleanup
  writer.close();
  mos.close();
  if (is != &std::cin)  { delete is; }
  if (os != &std::cout) { delete os; }

  return 0;
}
