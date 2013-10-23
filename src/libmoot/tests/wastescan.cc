#include <iostream>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <wasteScanner.h>
#include <wasteTypes.h>
#include <mootCxxIO.h>

using namespace moot;

//======================================================================
const char *token_type_name(int tok)
{
  return wasteScannerTypeNames[tok];
}

const char *token_type_name_OLD(int tok)
{
  switch (tok) {
  case wst_TOKEN_UNKNOWN: return "TOKEN_UNKNOWN";
  case wst_TOKEN_WORD: return "TOKEN_WORD";
  case wst_TOKEN_HYPH: return "TOKEN_HYPH";
  case wst_TOKEN_SPACE: return "TOKEN_SPACE";
  case wst_TOKEN_NL: return "TOKEN_NL";
  case wst_TOKEN_GREEK: return "TOKEN_GREEK";
  case wst_TOKEN_NUM: return "TOKEN_NUM";
  case wst_TOKEN_DOT: return "TOKEN_DOT";
  case wst_TOKEN_EOS: return "TOKEN_EOS";
  case wst_TOKEN_COMMA: return "TOKEN_COMMA";
  case wst_TOKEN_QUOTE: return "TOKEN_QUOTE";
  case wst_TOKEN_SC: return "TOKEN_SC";
  case wst_TOKEN_ROMAN_NUM: return "TOKEN_ROMAN_NUM";
  case wst_STOP_LOWER: return "STOP_LOWER";
  case wst_STOP_UPPER: return "STOP_UPPER";
  case wst_STOP_CAPS: return "STOP_CAPS";
  case wst_LATIN_LOWER: return "LATIN_LOWER";
  case wst_LATIN_UPPER: return "LATIN_UPPER";
  case wst_LATIN_CAPS: return "LATIN_CAPS";
  case wst_TOKEN_MONEY: return "TOKEN_MONEY";
  case wst_TOKEN_SB: return "TOKEN_SB";
  case wst_TOKEN_PERCENT: return "TOKEN_PERCENT";
  case wst_TOKEN_XML: return "TOKEN_XML";
  case wst_ABBREV: return "ABBREV";
  case wst_TOKEN_APOS: return "TOKEN_APOS";
  case wst_TOKEN_PLUS: return "TOKEN_PLUS";
  case wst_TOKEN_LBR: return "TOKEN_LBR";
  case wst_TOKEN_RBR: return "TOKEN_RBR";
  case wst_TOKEN_SLASH: return "TOKEN_SLASH";
  case wst_TOKEN_COLON: return "TOKEN_COLON";
  case wst_TOKEN_SEMICOLON: return "TOKEN_SEMICOLON";
  case wst_TOKEN_WB: return "TOKEN_WB";
  case wst_TOKEN_REST: return "TOKEN_REST";
  default: break;
  }
  return "";
}

//======================================================================
int main ( int argc, char* argv[] )
{
  const char *prog = argv[0];
  std::ios_base::sync_with_stdio ( false );
  std::istream *is = &std::cin;
  std::ostream *os = &std::cout;
  moot::wasteScanner scanner;

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
  scanner.from_mstream(&mis);
  int rtok;
  std::string s_text;  
  while ( (rtok=scanner.yylex()) != 0 ) {
    const char *s_typ = token_type_name(rtok);

    switch ( rtok )
    {
      case wst_TOKEN_NL:
	s_text = "\\n";
	break;
      default:
	s_text = scanner.yytext();
	break;
    }

    (*os) << s_text << '\t' << scanner.theByte - scanner.yyleng () << ' ' << scanner.yyleng () << '\t' << s_typ << '\n';
  }
  (*os) << "%%$EOF\t" << scanner.theByte << " 0\tEOF\n\n";

  //-- cleanup
  os->flush();
  if (is != &std::cin)  { delete is; }
  if (os != &std::cout) { delete os; }

  return 0;
}
